/* POSTER FAST-SCROLL HARNESS (Samsung .tpk policy, Mac host).
 *
 * Drives the REAL tex_cache.c (network + decode threads, disk cache, GL
 * uploads on the main context) with a simulated slow network and a home of
 * ROWS x COLS posters that is scrolled the way the owner does on the TV:
 * cold start, fast key-repeat down, back up, and a second pass.
 *
 * Only the network is fake: rede_baixar_bin sleeps a per-URL latency
 * (300..1500 ms, median ~650 ms, the C9/S90C trace numbers) and returns a real
 * 500x750 JPEG. Everything else is the production path.
 *
 * Per frame it replicates what home.c does for posters: tex_bombear(3),
 * tex_novo_quadro(), homeResidencia (focus row +-N), homePrecarregarVertical
 * (one lookahead request on the entering side), then the draw requests for
 * every card inside the cull band with the same tex_pedido_fora levels.
 *
 * Output: hit rate (visible card-frames that had a texture), load latency
 * (first visible -> first texture), "reloads" (a poster that had a texture,
 * lost it, and was visible without one), downloads/decodes per URL and
 * main-thread cache time per frame. Mac timings are a proxy, not TV numbers. */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "../src/perfiltv.h"
static PtvPlataforma testPlatform = PTV_TPK;
#define ptv_plataforma() testPlatform
#define rede_baixar_bin redeTeste
/* tex_cache.c declares the hook as `extern void NV_TEX_TEST_AFTER_POP(void);`
 * and calls `NV_TEX_TEST_AFTER_POP();` in threadDecode. Pasting the argument
 * turns the first into a plain declaration and the second into a call that
 * passes the URL just popped (urlOrig is in scope there). */
void psDecodeAtraso(const char *url);
void psDecodeDecl(void);
#define PS_POP_void psDecodeDecl(void)
#define PS_POP_ psDecodeAtraso(urlOrig)
#define NV_TEX_TEST_AFTER_POP(...) PS_POP_ ## __VA_ARGS__
#include "../src/tex_cache.c"
#undef rede_baixar_bin
#undef ptv_plataforma

#define ROWS 20
#define COLS 20
#define TELA_W 1920.0f
#define TELA_H 1080.0f
#define CARD_W 212.0f
#define CARD_H 318.0f
#define CARD_PASSO 232.0f
#define ROW_PASSO 410.0f
#define FOCO_Y 560.0f

static unsigned char *jpg; static long njpg;
static volatile int downloads;
static int dlPorUrl[ROWS][COLS];
static pthread_mutex_t dlMtx = PTHREAD_MUTEX_INITIALIZER;

static int urlRC(const char *url, int *r, int *c) {
  const char *p = strstr(url, "/r");
  return p && sscanf(p, "/r%d_c%d", r, c) == 2;
}
char *redeTeste(const char *url, int timeout, long *n) {
  int r = 0, c = 0;
  unsigned h = 2166136261u;
  (void)timeout;
  for (const char *p = url; *p; p++) h = (h ^ (unsigned char)*p) * 16777619u;
  /* 300..1500 ms, skewed low: median ~650 ms. */
  if (!getenv("PS_PREWARM")) {
    double u = (h % 1000) / 1000.0; int ms = 300 + (int)(1200.0 * u * u);
    usleep((useconds_t)ms * 1000); }
  char *b = malloc((size_t)njpg);
  memcpy(b, jpg, (size_t)njpg); *n = njpg;
  __sync_fetch_and_add(&downloads, 1);
  if (urlRC(url, &r, &c) && r >= 0 && r < ROWS && c >= 0 && c < COLS) {
    pthread_mutex_lock(&dlMtx); dlPorUrl[r][c]++; pthread_mutex_unlock(&dlMtx);
  }
  return b;
}

/* TV-like decode cost (Mac decodes a poster ~5x faster than the S90C). */
static volatile int decodes;
/* PS_HERO_MS: decode cost of a full-screen hero backdrop (the S90C log shows
 * ~1000 ms for a 3840x2160 source reduced to 1280x720). */
void psDecodeAtraso(const char *u) {
  static int ms = -1, heroMs = -1;
  if (ms < 0) ms = getenv("PS_PREWARM") ? 0 : getenv("PS_DECODE_MS") ? atoi(getenv("PS_DECODE_MS")) : 35;
  if (heroMs < 0) heroMs = getenv("PS_PREWARM") ? 0 : getenv("PS_HERO_MS") ? atoi(getenv("PS_HERO_MS")) : 0;
  int hero = strstr(u, "/hero/") != NULL;
  if (!hero) __sync_fetch_and_add(&decodes, 1);
  int d = hero ? heroMs : ms;
  if (d > 0) usleep((useconds_t)d * 1000);
}
static double agoraMs(void) {
  struct timeval tv; gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}
static void heroUrl(char *dst, int r) {
  snprintf(dst, 160, "https://img.example/hero/h%d.jpg", r);
}
static void url(char *dst, int r, int c) {
  snprintf(dst, 160, "https://image.tmdb.org/t/p/w342/r%d_c%d.jpg", r, c);
}

/* per poster bookkeeping */
static double primeiroVisto[ROWS][COLS], primeiraTex[ROWS][COLS];
static int teveTex[ROWS][COLS], perdeu[ROWS][COLS], faltandoVisivel[ROWS][COLS];
static double perdaEm[ROWS][COLS];
static double esperaRecarga[4096]; static int nRecarga;

typedef struct { long visiveis, comTex; } Fase;
static Fase fases[8];
static const char *nomeFase[8] = {"cold-dwell", "down-1", "bottom", "up-1", "top",
                                  "down-2", "up-2", "top-2"};
static double frameMs[20000]; static int nFrame;

static float scrollY = 0.0f, velY = 0.0f;
static int foco = 0;
static TexLookahead adiante;

static float rowY(int r) { return FOCO_Y + r * ROW_PASSO - scrollY; }

static void homeResidencia(void) {
  tex_residencia_inicio(foco);
  for (int r = 0; r < ROWS; r++)
    for (int c = 0; c < COLS; c++) {
      float x = 80.0f + c * CARD_PASSO;
      if (x < -CARD_W * 1.5f || x > TELA_W + CARD_W) continue;
      char u[160]; url(u, r, c); tex_residencia_marcar(u, r);
    }
}

static void homePrecarregarVertical(void) {
  int parado = tex_lookahead_parado(&adiante, scrollY);
  int moving = fabsf(velY) > 120.0f;
  if (!parado && !moving) return;
  tex_grade_movendo(moving); tex_pedido_fora(0);
  int primeira = -1, ultima = -1;
  for (int r = 0; r < ROWS; r++) {
    float fy = rowY(r);
    if (fy < TELA_H + 200 && fy + 50 + CARD_H > -200) { if (primeira < 0) primeira = r; ultima = r; }
  }
  int dir = adiante.direcao < 0 ? -1 : 1, pediu = 0;
  for (int passo = 1; primeira >= 0 && passo <= tex_residencia_fileiras() && !pediu; passo++) {
    int r = (dir > 0 ? ultima : primeira) + passo * dir;
    if (r < 0 || r >= ROWS) break;
    for (int c = 0; c < COLS && !pediu; c++) {
      float x = 80.0f + c * CARD_PASSO;
      if (x < -CARD_W * 1.5f || x > TELA_W + CARD_W) continue;
      char u[160]; url(u, r, c);
      pediu = tex_lookahead_pedir(u, CARD_W);
    }
  }
  tex_grade_movendo(0); tex_grade_velocidade(0); tex_pedido_fora(0);
}

static void desenhar(int fase, double t) {
  for (int r = 0; r < ROWS; r++) {
    float y = rowY(r);
    if (!(y < TELA_H + 200 && y + 50 + CARD_H > -200)) continue;
    int naTela = y + 50 + CARD_H > 0 && y < TELA_H;
    tex_grade_movendo(0); tex_grade_velocidade(0);
    for (int c = 0; c < COLS; c++) {
      float cx = 80.0f + c * CARD_PASSO + CARD_W * 0.5f;
      if (cx < -CARD_W * 1.5f || cx > TELA_W + CARD_W) continue;
      int fora = (cx + CARD_W * 0.5f < 0.0f || cx - CARD_W * 0.5f > TELA_W) &&
                 !(r == foco && c == 0);
      tex_pedido_fora(fora ? 2 : 0);
      char u[160]; url(u, r, c);
      GLuint tx = tex_obter_larg(u, CARD_W);
      if (fora || !naTela) continue;
      fases[fase].visiveis++;
      if (tx) fases[fase].comTex++;
      if (!primeiroVisto[r][c]) primeiroVisto[r][c] = t;
      if (tx) {
        if (!teveTex[r][c]) { teveTex[r][c] = 1; primeiraTex[r][c] = t; }
        if (faltandoVisivel[r][c]) {
          if (nRecarga < 4096) esperaRecarga[nRecarga++] = t - perdaEm[r][c];
          faltandoVisivel[r][c] = 0;
        }
      } else if (teveTex[r][c] && !faltandoVisivel[r][c]) {
        faltandoVisivel[r][c] = 1; perdeu[r][c]++; perdaEm[r][c] = t;
      }
    }
  }
  tex_pedido_fora(0);
}

static int cmpd(const void *a, const void *b) {
  double x = *(const double *)a, y = *(const double *)b; return x < y ? -1 : x > y;
}
static double pct(double *v, int n, double p) {
  if (!n) return 0; qsort(v, (size_t)n, sizeof *v, cmpd);
  int i = (int)(p * (n - 1) + 0.5); return v[i];
}

/* one simulated frame at 60 fps */
static double t0;
static int heroLigado;
static void quadro(int fase) {
  double ini = agoraMs();
  tex_bombear(3);
  tex_novo_quadro();
  { float alvo = foco * ROW_PASSO, antes = scrollY;
    scrollY += (alvo - scrollY) * 0.22f;
    if (fabsf(alvo - scrollY) < 0.3f) scrollY = alvo;
    velY = (scrollY - antes) * 60.0f; }
  homeResidencia();
  homePrecarregarVertical();
  if (heroLigado) {   /* home.c: focused hero every frame + heroAquecerVizinhos once per focus */
    static int vizDe = -1; char u[160];
    heroUrl(u, foco); (void)tex_obter_hero(u);
    if (vizDe != foco) {
      vizDe = foco;
      if (foco > 0) { heroUrl(u, foco - 1); (void)tex_obter_hero_quente(u); }
      if (foco + 1 < ROWS) { heroUrl(u, foco + 1); (void)tex_obter_hero_quente(u); }
    }
  }
  desenhar(fase, ini - t0);
  double fim = agoraMs();
  if (nFrame < 20000) frameMs[nFrame++] = fim - ini;
  double resto = 16.6 - (fim - ini);
  if (resto > 0) usleep((useconds_t)(resto * 1000));
}
static void dwell(int fase, int ms) {
  double fim = agoraMs() + ms; while (agoraMs() < fim) quadro(fase);
}
static void rolar(int fase, int de, int ate, int passoMs) {
  int d = ate > de ? 1 : -1;
  for (foco = de; foco != ate; ) {
    foco += d;
    double fim = agoraMs() + passoMs; while (agoraMs() < fim) quadro(fase);
  }
}

int main(int argc, char **argv) {
  const char *poster = argc > 1 ? argv[1] : "tests/amostra.jpg";
  const char *dir = argc > 2 ? argv[2] : "/tmp/nuvio-poster-scroll";
  long ram = getenv("PS_RAM") ? atol(getenv("PS_RAM")) : 2400;
  int passo = getenv("PS_PASSO") ? atoi(getenv("PS_PASSO")) : 120;
  FILE *f = fopen(poster, "rb");
  if (!f) { perror(poster); return 2; }
  fseek(f, 0, SEEK_END); njpg = ftell(f); fseek(f, 0, SEEK_SET);
  jpg = malloc((size_t)njpg); fread(jpg, 1, (size_t)njpg, f); fclose(f);
  if (!getenv("PS_WARM")) { char cmd[700]; snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir); system(cmd); }
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) return 2;
  SDL_Window *w = SDL_CreateWindow("ps", 0, 0, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  SDL_GLContext gl = SDL_GL_CreateContext(w);
  if (!gl) return 2;
  tex_cache_dir(dir);
  tex_iniciar(192);
  orcMemTotal = ram;
  { PtvPerfil pf; ptv_padrao(PTV_TPK, ram, &pf); tex_definir_fios_rede(getenv("PS_FIOS") ? atoi(getenv("PS_FIOS")) : pf.fiosRede); }
  tex_definir_orcamento_auto_mb(0);
  if (getenv("PS_MB")) tex_definir_orcamento_mb(atoi(getenv("PS_MB")));
  heroLigado = getenv("PS_HERO_MS") && !getenv("PS_PREWARM");
  /* PS_LIVRE: one MemAvailable sample (MB), like main.c's 3 s report. */
  if (getenv("PS_LIVRE")) tex_memoria_livre(atol(getenv("PS_LIVRE")));
  { int mb, slots; tex_orcamento_info(&mb, NULL, NULL, &slots);
    printf("[ps] RAM=%ld budget=%d MB cap=%.0f MB slots=%d net-threads=%d step=%d ms hero-decode=%s ms\n",
           ram, mb, limiteBytes() / 1048576.0, slots, tex_fios_rede(), passo,
           getenv("PS_HERO_MS") ? getenv("PS_HERO_MS") : "off"); }
  t0 = agoraMs();
  if (getenv("PS_PREWARM")) {   /* fill the disk cache only, like days of use */
    for (foco = 0; foco < ROWS; foco++) dwell(0, 700);
    tex_cache_esperar_gravacoes();
    printf("[ps] prewarm downloads=%d\n", downloads);
    return 0;
  }
  dwell(0, 5000);
  rolar(1, 0, ROWS - 1, passo);
  dwell(2, 1500);
  rolar(3, ROWS - 1, 0, passo);
  dwell(4, 2500);
  rolar(5, 0, ROWS - 1, passo);
  rolar(6, ROWS - 1, 0, passo);
  dwell(7, 2500);

  /* cold load latency of the first screen */
  double lat[ROWS * COLS]; int nl = 0, nunca = 0;
  for (int r = 0; r < ROWS; r++) for (int c = 0; c < COLS; c++)
    if (primeiroVisto[r][c] && primeiroVisto[r][c] < 100.0) {
      if (teveTex[r][c]) lat[nl++] = primeiraTex[r][c] - primeiroVisto[r][c]; else nunca++;
    }
  printf("[ps] first-screen posters=%d load p50=%.0f ms p90=%.0f ms max=%.0f ms never=%d\n",
         nl + nunca, pct(lat, nl, .5), pct(lat, nl, .9), pct(lat, nl, 1.0), nunca);
  nl = 0; nunca = 0;
  for (int r = 0; r < ROWS; r++) for (int c = 0; c < COLS; c++)
    if (primeiroVisto[r][c]) {
      if (teveTex[r][c]) lat[nl++] = primeiraTex[r][c] - primeiroVisto[r][c]; else nunca++;
    }
  printf("[ps] all-seen posters=%d load p50=%.0f ms p90=%.0f ms never-loaded=%d\n",
         nl + nunca, pct(lat, nl, .5), pct(lat, nl, .9), nunca);
  for (int k = 0; k < 8; k++)
    printf("[ps] phase %-10s visible card-frames=%6ld with texture=%5.1f%%\n", nomeFase[k],
           fases[k].visiveis, fases[k].visiveis ? 100.0 * fases[k].comTex / fases[k].visiveis : 0);
  { int lost = 0, refetch = 0, distinct = 0;
    for (int r = 0; r < ROWS; r++) for (int c = 0; c < COLS; c++) {
      lost += perdeu[r][c]; if (dlPorUrl[r][c]) distinct++;
      if (dlPorUrl[r][c] > 1) refetch += dlPorUrl[r][c] - 1;
    }
    printf("[ps] reloads (had texture, later visible without)=%d wait p50=%.0f ms p90=%.0f ms\n",
           lost, pct(esperaRecarga, nRecarga, .5), pct(esperaRecarga, nRecarga, .9));
    printf("[ps] downloads=%d distinct=%d re-downloads=%d decodes=%d disk-direct=%d evictions=%ld hot-evictions=%ld uploads=%d\n",
           downloads, distinct, refetch, decodes, tex_disco_direto, tex_despejos_total,
           tex_despejos_quentes_total, tex_upl_n);
  }
  printf("[ps] main-thread cache ms/frame p50=%.2f p95=%.2f max=%.2f frames=%d\n",
         pct(frameMs, nFrame, .5), pct(frameMs, nFrame, .95), pct(frameMs, nFrame, 1.0), nFrame);
  { int it, pe, q; long by, bq; tex_estatisticas(&it, &pe, &by, &q, &bq);
    int w = 0, h = 0;
    for (int i = 0; i < nMax; i++) if (itens[i].estado == PRONTO) { w = itens[i].w; h = itens[i].h; break; }
    printf("[ps] end: textures=%d pending=%d bytes=%.1f MB (budget %.0f MB) poster texture %dx%d\n",
           it, pe, by / 1048576.0, orcamento / 1048576.0, w, h); }
  tex_cache_esperar_gravacoes();
  return 0;
}

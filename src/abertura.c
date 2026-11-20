// Abertura do app (#213). Ver abertura.h.
#include "abertura.h"
#include "gfx.h"
#include "anim.h"
#include "layout.h"
#include "ajustes.h"
#include "logoapp.h"
#include "tex_cache.h"
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// O fundo do splash.png (#0E0F12, lido do arquivo) e o lugar da marca nele:
// 780x247 a partir de (570,416), o centro em (960,540).
#define AB_FUNDO_R (14.0f / 255.0f)
#define AB_FUNDO_G (15.0f / 255.0f)
#define AB_FUNDO_B (18.0f / 255.0f)
#define AB_MARCA_W 780.0f
// A marca do logo Novo (play + nome empilhados, 1200x895) em 1920x1080.
#define AB_NOVO_W 720.0f
#define AB_CENTRO_Y 540.0f
// Nunca segura a home alem disto, com arte chegando ou nao.
#define ABERTURA_TETO_MS 1500u

// OS TRES ESTILOS (Ajustes > Aparencia > "Abertura do app", 2.0 N1). Cada um e so
// um conjunto de numeros para a MESMA conta de saida, usada pela abertura de
// verdade e pela previa dos Ajustes (abertura_previa):
//   Padrao       parada minima de 350 ms ("le como abriu, nao como piscada"),
//                saida de 560 ms, a marca cresce 1,018 -> 1,09 e esmaece;
//   So esmaece   a mesma parada e a mesma saida, sem crescer (e o caminho das
//                animacoes reduzidas);
//   Direto       sem parada minima e saida curta de 220 ms, sem crescer: assim
//                que a home tem as artes do primeiro quadro, ela aparece.
// CINEMATIC (owner, 18-19 Nov 2026: "some sort of animation like Disney Plus
// when the app is opened" ... "it just doesnt feel premium" ... chosen from
// twelve renders: "4 Crossing lights"). Nuvio+'s own reveal: on near-black the
// logo is a pure black shape; a cyan light from the left and a violet one from
// the right pass behind it in opposite directions, so only its outline glows;
// where they cross there is a brief bright bloom and the logo's colour floods
// in. About 2.3 s, any key skips it, reduced animations get the plain one. Only
// the new logo; with the Classic art it stays the old one. It is what
// "Padrão" (0) now means; the old still logo with a slow zoom became
// "Clássica" (3).
#define AB_CINEMA 0
#define AB_CLASSICA 3
#define AB_CINEMA_MS 2300u
typedef struct { Uint32 minMs; float saidaMs; int zoom; } AbEstilo;
static AbEstilo abEstilo(int e, int reduzida) {
  AbEstilo r = { 350u, 560.0f, 1 };
  if (e == AB_CINEMA && !reduzida) r.minMs = AB_CINEMA_MS;   /* AB_CLASSICA: the 350 ms default above */
  if (e == 2) { r.minMs = 0u; r.saidaMs = 220.0f; r.zoom = 0; }
  else if (e == 1 || reduzida) r.zoom = 0;
  return r;
}
// A saida em `p` (0..1): o veu sai com cubica de saida; a marca, mais depressa.
static void abSaida(float p, float *veu, float *alfaMarca) {
  float q = 1.0f - p, pm = p * 1.6f;
  if (pm > 1.0f) pm = 1.0f;
  *veu = q * q * q;
  *alfaMarca = (1.0f - pm) * (1.0f - pm);
}

static GLuint marca;
static float marcaAsp;
// ARTE CHEIA (splash 1.7.2): a mesma imagem do splash.png da LG e do fundo da
// janela no Android, em 1280x720 (decodifica em ~1/2 do tempo da 1920 e some em
// menos de 1,5 s, entao a ampliacao nao chega a ser vista). Com ela, a marca e
// o fundo liso ficam de fora: a arte ja tem os dois.
static int arteCheia;
static int fundoFica;

void abertura_fundo_fica(int sim) { fundoFica = sim; }
static int estado;          // 0 nao iniciou, 1 parada, 2 saindo, 3 acabou
static Uint32 inicio, saidaEm;
static float escala = 1.0f, escalaVel;
static int estiloAb;        // ajustes_abertura() lido no arranque
static float marcaW = AB_MARCA_W;   // largura da marca no 1080p

// --- the cinematic reveal ---------------------------------------------------------
#define AB_SIMBOLO_FIM 592.0f   // logo-novo-marca.png (1000 x 746): last row of the symbol; the text starts at 655
#define AB_FUNDO_CINEMA 0.010f, 0.010f, 0.016f
static float abSuave(float x) { x = x < 0 ? 0 : x > 1 ? 1 : x; return x * x * (3 - 2 * x); }
static void abBrilho(float cx, float cy, float d, float r, float g, float b, float a) {
  if (a <= 0.004f) return;
  gfx_rect((GfxRect){ cx - d * 0.5f, cy - d * 0.5f, d, d }, 0, GFX_SOMBRA, 1.0f, 0, 0, 0.5f, r, g, b, a);
}
// The lockup in its two parts, symbol and the "Nuvio+" line: in colour, or
// (`sombra`) as a flat black shape (GFX_MARCA: the image's alpha, tinted).
static void abLogo(float aSimb, float aTexto, int sombra, float veu) {
  const float k = AB_NOVO_W / 1000.0f, w = AB_NOVO_W, h = w / (marcaAsp > 0 ? marcaAsp : 1.34f);
  const float ox = NV_TELA_W * 0.5f - w * 0.5f, oy = AB_CENTRO_Y - h * 0.5f;
  const GfxModo modo = sombra ? GFX_MARCA : GFX_TEXTO;
  const float c = sombra ? 0.0f : 1.0f;
  if (!marca) return;
  gfx_tex_aspect_atual = 0.0f;
  if (aSimb > 0.004f) {
    gfx_recorte(0, 0, NV_TELA_W, oy + AB_SIMBOLO_FIM * k);
    gfx_rect((GfxRect){ ox, oy, w, h }, marca, modo, 0, 0, 0, 0.0f, c, c, c, aSimb * veu);
    gfx_sem_recorte();
  }
  if (aTexto > 0.004f) {
    gfx_recorte(0, oy + 640.0f * k, NV_TELA_W, NV_TELA_H);
    gfx_rect((GfxRect){ ox, oy, w, h }, marca, modo, 0, 0, 0, 0.0f, c, c, c, aTexto * veu);
    gfx_sem_recorte();
  }
}
static void abCinema(float t, float veu) {
  const float h = AB_NOVO_W / (marcaAsp > 0 ? marcaAsp : 1.34f), cx = NV_TELA_W * 0.5f;
  const float cy = AB_CENTRO_Y - h * 0.5f + 330 * (AB_NOVO_W / 1000.0f);
  float luz = abSuave((t - 0.05f) / 0.6f), p = abSuave((t - 0.1f) / 2.0f), cor = abSuave((t - 1.05f) / 0.8f);
  float l = luz * (1.0f - 0.55f * cor), xa = cx - 520 + 1040 * p, xb = cx + 520 - 1040 * p;
  float cruza = t > 0.95f ? expf(-(t - 1.05f) * (t - 1.05f) * 18.0f) : 0.0f;
  gfx_cor((GfxRect){ 0, 0, NV_TELA_W, NV_TELA_H }, 0.0f, AB_FUNDO_CINEMA, veu);
  abBrilho(xa, cy - 30, 1000, 0.14f, 0.80f, 1.0f, 0.42f * l * veu);   // cyan, from the left
  abBrilho(xb, cy + 30, 1000, 0.62f, 0.30f, 1.0f, 0.42f * l * veu);   // violet, from the right
  abBrilho(cx, cy, 700, 0.80f, 0.85f, 1.0f, 0.40f * cruza * veu);     // where they cross
  abLogo(luz, luz, 1, veu);    // the black shape, rim-lit by the lights behind it
  abLogo(cor, cor, 0, veu);    // then its colour floods in
}
static int abCinemaAtivo(void) { return estiloAb == AB_CINEMA && !anim_politica_reduzida && marca && !arteCheia; }
void abertura_cinema_quadro(float t) { abCinema(t, 1.0f); }
// The same reveal, scaled into `r` (the What's new card): the screen becomes
// a band as wide as `r`, centred in it. The opening's own texture is gone once
// it ends, so this borrows the cached lockup and gives `marca` back after.
void abertura_cinema_previa(GfxRect r, float t, float a) {
  GLuint antes = marca;
  float aspAntes = marcaAsp, s = r.w / NV_TELA_W;
  if (a <= 0.004f || r.w < 8.0f) return;
  if (!marca) {
    const char *c = logoapp_caminho(LOGO_NOVO, LOGO_F_MARCA);
    marca = tex_obter(c);
    if (marca) marcaAsp = tex_aspecto(c);
  }
  if (marca) {
    gfx_transformar(0.0f, 0.0f, s, r.x, r.y + (r.h - NV_TELA_H * s) * 0.5f);
    abCinema(t, a);
    gfx_sem_transformar();
  }
  marca = antes; marcaAsp = aspAntes;
}

void abertura_iniciar(const char *dirArte) {
  char cam[600];
  SDL_Surface *s, *c;
  estado = 0;
  arteCheia = 0;
  estiloAb = ajustes_abertura();
  marcaW = AB_MARCA_W;
  s = NULL;
  logoapp_iniciar(dirArte);
  if (logoapp_atual() == LOGO_NOVO) {
    // O logo Novo: sem arte cheia, o fundo liso e a marca (PNG com alfa) nitidos
    // em qualquer resolucao. Sem o arquivo, cai no Classico.
    s = IMG_Load(logoapp_caminho(LOGO_NOVO, LOGO_F_MARCA));
    if (s) marcaW = AB_NOVO_W;
  }
  if (!s) {
  snprintf(cam, sizeof cam, "%s/marcas/abertura.jpg", dirArte ? dirArte : ".");
  s = IMG_Load(cam);
  if (s) arteCheia = 1;
  else {
    snprintf(cam, sizeof cam, "%s/marcas/nuvio_wordmark.png", dirArte ? dirArte : ".");
    s = IMG_Load(cam);
  }
  }
  if (!s) { printf("[abertura] sem a marca (%s)\n", cam); return; }
  c = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_ABGR8888, 0);   // RGBA em bytes
  SDL_FreeSurface(s);
  if (!c) return;
  glGenTextures(1, &marca);
  glBindTexture(GL_TEXTURE_2D, marca);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c->w, c->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, c->pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  gfx_tex_esquecer(0);   // o gfx guarda a ultima textura ligada
  marcaAsp = c->h > 0 ? (float)c->w / (float)c->h : 3.15f;
  SDL_FreeSurface(c);
}

// A key: the exit starts on the next frame, on that frame's clock (the one the
// exit is measured in), not SDL_GetTicks.
static int pediuSair;
void abertura_tecla(void) {
  if (estado == 1) pediuSair = 1;
}

int abertura_ativa(void) { return estado < 3; }

static void acabar(void) {
  estado = 3;
  if (marca) { gfx_tex_esquecer(marca); glDeleteTextures(1, &marca); marca = 0; }
  printf("[abertura] fim em %u ms\n", (unsigned)(SDL_GetTicks() - inicio));
  fflush(stdout);
}

int abertura_desenhar(Uint32 agora, float dt, int pendentes) {
  float veu = 1.0f, alfaMarca = 1.0f;
  const AbEstilo es = abEstilo(estiloAb, anim_politica_reduzida);
  float alvo = es.zoom ? 1.018f : 1.0f;
  if (estado == 3) return 0;
  if (estado == 0) { estado = 1; inicio = agora; escala = 1.0f; escalaVel = 0.0f; pediuSair = 0; }
  if (estado == 1 && pediuSair) { pediuSair = 0; estado = 2; saidaEm = agora; }
  if (estado == 1) {
    Uint32 passou = agora - inicio;
    // NUVIO_ABERTURA_MS segura a parada (captura de tela no Mac/TV).
    static long segura = -1;
    if (segura < 0) { const char *v = getenv("NUVIO_ABERTURA_MS"); segura = v ? atol(v) : 0; }
    if (segura > 0 && passou < (Uint32)segura) pendentes = 1, passou = 0;
    // Sai quando as artes do primeiro quadro chegaram (nada em voo) ou no teto.
    if ((passou >= es.minMs && pendentes <= 0) || passou >= (abCinemaAtivo() ? AB_CINEMA_MS + 1200u : ABERTURA_TETO_MS)) {
      estado = 2; saidaEm = agora;
    }
  }
  if (estado == 2) {
    float p = (float)(agora - saidaEm) / es.saidaMs;
    if (p >= 1.0f) { acabar(); return 0; }
    // A marca cresce um pouco por mola (a mesma de segunda ordem do resto do
    // app) — abre para a home.
    abSaida(p, &veu, &alfaMarca);
    if (es.zoom) alvo = 1.09f;
  }
  if (estado == 1 && abCinemaAtivo()) { abCinema((float)(agora - inicio) / 1000.0f, 1.0f); return 1; }
  // Sem zoom (So esmaece, Direto, animacoes reduzidas): so o esvanecimento.
  escala = !es.zoom ? 1.0f : anim_mola2(&escalaVel, escala, alvo, dt, 9.0f);
  if (marca && arteCheia) {
    // A arte inteira cresce a partir do centro e sai junto com o veu. Sobre o
    // login ela fica parada: o fundo de baixo continua as listras, e quem
    // some e o logo.
    float k = fundoFica ? 1.0f : escala;
    float w = NV_TELA_W * k, h = NV_TELA_H * k;
    gfx_tex_aspect_atual = 0.0f;
    gfx_rect((GfxRect){ (NV_TELA_W - w) * 0.5f, (NV_TELA_H - h) * 0.5f, w, h },
             marca, GFX_TEXTO, 0, 0, 0, 0.0f, 1, 1, 1, veu);
    return 1;
  }
  if (abCinemaAtivo())   // the reveal's last frame opens into the app like the others
    gfx_cor((GfxRect){ 0, 0, NV_TELA_W, NV_TELA_H }, 0.0f, AB_FUNDO_CINEMA, veu);
  else
  gfx_cor((GfxRect){ 0, 0, NV_TELA_W, NV_TELA_H }, 0.0f,
          AB_FUNDO_R, AB_FUNDO_G, AB_FUNDO_B, veu);
  if (marca && alfaMarca > 0.004f) {
    float w = marcaW * escala, h = w / marcaAsp;
    gfx_tex_aspect_atual = 0.0f;
    gfx_rect((GfxRect){ NV_TELA_W * 0.5f - w * 0.5f, AB_CENTRO_Y - h * 0.5f, w, h },
             marca, GFX_TEXTO, 0, 0, 0, 0.0f, 1, 1, 1, alfaMarca);
  }
  return 1;
}

// PREVIA DOS AJUSTES (Aparencia > "Abertura do app"): uma volta da abertura em
// laco dentro de `r`, com a MESMA conta de saida (abEstilo/abSaida) e o logo
// pedido. Quem chama desenha antes o que fica por baixo (a "home") e recorta;
// aqui so entram o veu e a marca. Linha do tempo: entra (200 ms), para (a do
// estilo, esticada a 900 ms para dar para ver), sai (a do estilo) e descansa
// 900 ms com a home a mostra.
void abertura_previa(GfxRect r, float raio, int estilo, int logo, Uint32 agora, float alfa) {
  const AbEstilo es = abEstilo(estilo, 0);
  const float entra = 200.0f, para = es.minMs ? 900.0f : 120.0f, descansa = 900.0f;
  const float ciclo = entra + para + es.saidaMs + descansa;
  float t = (float)(agora % (Uint32)ciclo), veu = 1.0f, am = 1.0f, k = 1.0f, a;
  GLuint tx;
  if (alfa <= 0.004f || r.w < 8.0f) return;
  if (t < entra) { am = veu = t / entra; }
  else if (t < entra + para) {
    float u = (t - entra) / (es.minMs ? es.minMs : 1.0f);
    if (u > 1.0f) u = 1.0f;
    u = 1.0f - (1.0f - u) * (1.0f - u);
    k = es.zoom ? 1.0f + 0.018f * u : 1.0f;
  } else if (t < entra + para + es.saidaMs) {
    float p = (t - entra - para) / es.saidaMs, e = 1.0f - (1.0f - p) * (1.0f - p) * (1.0f - p);
    abSaida(p, &veu, &am);
    k = es.zoom ? 1.018f + (1.09f - 1.018f) * e : 1.0f;
  } else { veu = 0.0f; am = 0.0f; }
  a = veu * alfa;
  if (logo == LOGO_CLASSICO) {
    // Arte cheia: o fundo e a marca ja estao na imagem.
    const char *c = logoapp_caminho(LOGO_CLASSICO, LOGO_F_MARCA);
    tx = a > 0.004f ? tex_obter_larg(c, r.w) : 0;
    if (tx) {
      float w = r.w * k, h = r.h * k;
      gfx_tex_aspect_atual = 0.0f;
      gfx_rect((GfxRect){ r.x + (r.w - w) * 0.5f, r.y + (r.h - h) * 0.5f, w, h },
               tx, GFX_TEXTO, 0, 0, 0, raio, 1, 1, 1, a);
    } else if (a > 0.004f) gfx_cor(r, raio, AB_FUNDO_R, AB_FUNDO_G, AB_FUNDO_B, a);
    return;
  }
  gfx_cor(r, raio, AB_FUNDO_R, AB_FUNDO_G, AB_FUNDO_B, a);
  if (am * alfa > 0.004f) {
    const char *c = logoapp_caminho(LOGO_NOVO, LOGO_F_MARCA);
    float asp = tex_aspecto(c), w, h;
    tx = tex_obter_larg(c, r.w * (AB_NOVO_W / NV_TELA_W));
    if (!tx) return;
    if (asp <= 0.0f) asp = 1.34f;
    w = r.w * (AB_NOVO_W / NV_TELA_W) * k; h = w / asp;
    gfx_tex_aspect_atual = 0.0f;
    gfx_rect((GfxRect){ r.x + (r.w - w) * 0.5f, r.y + r.h * 0.5f - h * 0.5f, w, h },
             tx, GFX_TEXTO, 0, 0, 0, 0.0f, 1, 1, 1, am * alfa);
  }
}

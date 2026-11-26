// .tpk: Apple trailers WITH sound (owner, 26 Nov 2026: "get apple trailers to
// work with audio on samsung tv"). No network, no TV. The master is a real
// Apple trailer master (tests/fixtures/trailer/apple-master.m3u8, fetched
// 26/11/2026: three CDN pathways, content steering, audio as separate AAC
// renditions). Proves AVPlay gets, from the local server, ONE avc1 variant
// within the quality ceiling plus the ONE audio rendition of its group, with no
// steering or pathway left, and that the video-only playlist stays the fallback.
#define NV_TPK 1
#define static
#include "../src/trailerapple.c"
#undef static
#include <assert.h>
#include <unistd.h>

static char pasta[512];
char *dados_caminho(char *dst, unsigned tam, const char *nome) {
  snprintf(dst, tam, "%s/%s", pasta, nome);
  return dst;
}
void dados_marcar_sujo(int leve) { (void)leve; }
static int teto;
int ajustes_trailer_qualidade(void) { return teto; }
char *rede_baixar_com(const char *url, int segundos, const char *const *cab) { (void)url; (void)segundos; (void)cab; return NULL; }

static int servidor = 1;
static char pedido[64];
static int urlFalsa(const char *id, char *dst, size_t n) {
  if (!servidor) return 0;
  snprintf(pedido, sizeof pedido, "%s", id);
  snprintf(dst, n, "http://127.0.0.1:8123/apl/%s.m3u8", id);
  return 1;
}

static int falhas;
static void confere(const char *nome, int ok) { printf("%s %s\n", ok ? "ok " : "FALHOU", nome); if (!ok) falhas++; }

static char *lerArquivo(const char *c) {
  FILE *f = fopen(c, "r");
  long n;
  char *b;
  if (!f) return NULL;
  fseek(f, 0, SEEK_END); n = ftell(f); rewind(f);
  b = malloc((size_t)n + 1);
  n = (long)fread(b, 1, (size_t)n, f); b[n] = 0;
  fclose(f);
  return b;
}
static void gravarMaster(const char *imdb, const char *m) {
  char cm[600];
  char d[600];
  snprintf(d, sizeof d, "%s/trailer", pasta); mkdir(d, 0755);
  caminhoMaster(imdb, cm, sizeof cm, ".m3u8");
  FILE *g = fopen(cm, "w"); assert(g); fputs(m, g); fclose(g);
}
static int conta(const char *s, const char *agulha) {
  int k = 0;
  for (const char *p = s; (p = strstr(p, agulha)) != NULL; p++) k++;
  return k;
}
// The served playlist: one variant, its audio, nothing else to choose.
static int largura(const char *pl) {
  char res[32];
  const char *p = strstr(pl, "#EXT-X-STREAM-INF:");
  if (!p) return 0;
  atributo(p, "RESOLUTION", res, sizeof res);
  return atoi(res);
}
static void confereServida(const char *rotulo, const char *id, int larguraMax) {
  char pl[32768], grupoV[160], grupoA[160], cod[128], nome[200];
  const char *v, *a;
  int ok = trailerapple_manifesto(id, pl, sizeof pl);
  snprintf(nome, sizeof nome, "%s: o servidor tem a playlist", rotulo); confere(nome, ok);
  if (!ok) return;
  v = strstr(pl, "#EXT-X-STREAM-INF:"); a = strstr(pl, "#EXT-X-MEDIA:");
  snprintf(nome, sizeof nome, "%s: UMA variante e UM audio", rotulo);
  confere(nome, conta(pl, "#EXT-X-STREAM-INF:") == 1 && conta(pl, "#EXT-X-MEDIA:") == 1 && a && strstr(a, "TYPE=AUDIO"));
  atributo(v, "AUDIO", grupoV, sizeof grupoV);
  atributo(a, "GROUP-ID", grupoA, sizeof grupoA);
  atributo(v, "CODECS", cod, sizeof cod);
  snprintf(nome, sizeof nome, "%s: o audio e o grupo da variante (AAC)", rotulo);
  confere(nome, grupoV[0] && !strcmp(grupoV, grupoA) && strstr(cod, "mp4a.40.2"));
  snprintf(nome, sizeof nome, "%s: avc1 dentro do teto (%dpx)", rotulo, largura(pl));
  confere(nome, strstr(cod, "avc1") && largura(pl) > 0 && largura(pl) <= larguraMax);
  snprintf(nome, sizeof nome, "%s: sem steering, pathway nem I-frames", rotulo);
  confere(nome, !strstr(pl, "STEERING") && !strstr(pl, "PATHWAY-ID") && !strstr(pl, "I-FRAME"));
  snprintf(nome, sizeof nome, "%s: URIs absolutas", rotulo);
  confere(nome, strstr(a, "URI=\"https://") && strstr(v, "\nhttps://"));
}

int main(void) {
  char u[160], *master;
  snprintf(pasta, sizeof pasta, "%s/nuvio-apple-som-XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  assert(mkdtemp(pasta));
  master = lerArquivo("tests/fixtures/trailer/apple-master.m3u8");
  assert(master && strstr(master, "#EXT-X-CONTENT-STEERING"));
  trailerapple_definir_local(urlFalsa);

  gravarMaster("tt0000001", master);
  teto = 0;
  confere("Maxima: monta e entrega a url local", montarReduzido("tt0000001", 0, u, sizeof u) &&
          !strcmp(u, "http://127.0.0.1:8123/apl/tt0000001.m3u8"));
  confereServida("Maxima", "tt0000001", 1920);
  confere("teto 720", montarReduzido("tt0000001", 720, u, sizeof u));
  confereServida("720p", "tt0000001", 1280);

  // Series: "tt...:3:2" travels as "tt...-3-2" and comes back.
  gravarMaster("tt0000002:3:2", master);
  confere("serie: id da url sem ':'", montarReduzido("tt0000002:3:2", 1080, u, sizeof u) && !strcmp(pedido, "tt0000002-3-2"));
  confereServida("serie", "tt0000002-3-2", 1920);

  // trailerapple_url: the one with sound first, the video-only one without server.
  { Entrada *e = reservar("tt0000001");
    e->respondeu = 1; e->expira = (long)time(NULL) + 3600;
    snprintf(e->url, sizeof e->url, "https://play-edge.itunes.apple.com/master.m3u8");
    snprintf(e->toca, sizeof e->toca, "https://vod-ap-amt.tv.apple.com/video-only.m3u8");
    teto = 1080;
    const char *r = trailerapple_url("tt0000001");
    confere("trailerapple_url: a playlist com som", r && strstr(r, "/apl/tt0000001.m3u8"));
    servidor = 0; e->localQual = 0;
    r = trailerapple_url("tt0000001");
    confere("sem servidor: a variante so de video, como antes", r && !strcmp(r, "https://vod-ap-amt.tv.apple.com/video-only.m3u8"));
    servidor = 1; }

  // A variant whose audio group is missing would play mute: not served.
  { char *semAudio = strdup(master), *p;
    while ((p = strstr(semAudio, "TYPE=AUDIO")) != NULL) memcpy(p, "TYPE=XXXXX", 10);
    gravarMaster("tt0000003", semAudio);
    confere("grupo de audio ausente: nao monta", !montarReduzido("tt0000003", 0, u, sizeof u));
    free(semAudio); }
  confere("id que nao e do IMDb: nada", !trailerapple_manifesto("../etc", u, sizeof u));
  confere("sem master no disco: nada", !montarReduzido("tt0000009", 0, u, sizeof u));

  free(master);
  puts(falhas ? "trailerapple-som: FALHOU" : "trailerapple-som: tudo ok");
  return falhas ? 1 : 0;
}

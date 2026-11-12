// Source autoplay with AIOStreams-style TorBox links ("Mani | TB Instant").
//
// Each stream carries the addon's own playback URL AND a torrent hash. The
// parser keeps that hash in hashTorrent (only for cache evidence). Stage D
// sent every such stream to the native "existing account item" resolver, so a
// cached torrent that was not already in the TorBox account never produced a
// URL and autoplay ended in "nenhuma fonte serve" (BlueStacks log, 12/11:
// "verificacao/debrid em 29316 ms (8 candidata(s))", none served). The
// addon's link must be probed like any direct link again, while:
//   - provider evidence of Uncached still keeps the link out of autoplay;
//   - links are probed one at a time (#130), never in the parallel race;
//   - no automatic TorBox account write happens from Native.
// Covers the source screen autoplay (stream_primeira_boa_sessao), the
// next-episode preparation (stream_preparo_resolver, proximopreparo.c) and the
// playback-failure fallback (stream_falha_verificar).
#include "../src/streams.c"
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>

static const char *H1 = "1111111111111111111111111111111111111111";
static const char *H2 = "2222222222222222222222222222222222222222";
static const char *dir;
static _Atomic int cacheCalls, listCalls, probes, mutations;
static int uncached, probeFails;
static char lastProbe[512];

static void configure(int resolve, int optIn) {
  char path[1024]; snprintf(path, sizeof path, "%s/ajustes.txt", dir);
  FILE *f = fopen(path, "w"); assert(f);
  // Native 0=On / 1=Off convention. Autoplay on (escolherFonteManual Off).
  fprintf(f, "resolvePlayableLinksLocal %d\npreferredResolverProviderLocal 1\nescolherFonteManual 1\n"
             "torboxCachedAddLocal %d\n", resolve ? 0 : 1, optIn ? 0 : 1);
  fclose(f); ajustes_dir(dir);
  debrid_definir_chave_local("torbox", "synthetic-tb");
  debrid_definir_chave_local("premiumize", ""); debrid_definir_chave_local("realdebrid", "");
  debrid_definir_chave_local("alldebrid", "");
}

char *rede_postar_st(const char *u, int t, const char *const *h, const char *body, int *status) {
  (void)t; (void)h; *status = 200;
  if (strstr(u, "torbox.app") && strstr(u, "checkcached")) {
    cacheCalls++; assert(strstr(body, "hashes"));
    if (uncached) return strdup("{\"success\":true,\"data\":{}}");
    char r[400]; snprintf(r, sizeof r, "{\"success\":true,\"data\":{\"%s\":{},\"%s\":{}}}", H1, H2);
    return strdup(r);
  }
  mutations++; fprintf(stderr, "FAIL unexpected automatic provider POST %s\n", u); assert(0); return NULL;
}
char *rede_baixar_st(const char *u, int t, const char *const *h, int *status) {
  (void)t; (void)h; *status = 200;
  // Cached on TorBox but NOT in this account: the native read-only resolver
  // cannot produce a URL without creating an item.
  if (strstr(u, "torbox.app") && strstr(u, "torrents/mylist")) { listCalls++; return strdup("{\"success\":true,\"data\":[]}"); }
  fprintf(stderr, "FAIL unexpected GET %s\n", u); assert(0); return NULL;
}
char *rede_colocar_st(const char *u, int t, const char *const *h, const char *b, int *s) {
  (void)u; (void)t; (void)h; (void)b; (void)s; mutations++; assert(0); return NULL;
}
int rede_url_final_tipo(const char *url, int segundos, const char *const *cab, char *dst, unsigned tam,
                        int *status, char *mime, unsigned mimeTam, long *corpo) {
  (void)segundos; (void)cab;
  probes++; snprintf(lastProbe, sizeof lastProbe, "%s", url);
  assert(!strncmp(url, "https://aio.invalid/playback/", 29));
  if (status) *status = probeFails ? 502 : 200;
  if (corpo) *corpo = -1;
  if (probeFails) { probeFails--; return 0; }
  if (mime && mimeTam) snprintf(mime, mimeTam, "video/x-matroska");
  snprintf(dst, tam, "https://cdn.aio.invalid/final/%s.mkv", url + 29);
  return 1;
}

static int load(Stream **out) {
  char json[2000];
  snprintf(json, sizeof json, "{\"streams\":["
    "{\"name\":\"[TB\xe2\x9a\xa1] Mani 1080p\",\"url\":\"https://aio.invalid/playback/one\","
    "\"description\":\"Show.S06E05.1080p.WEB-DL.mkv\",\"infoHash\":\"%s\",\"behaviorHints\":{\"filename\":\"Show.S06E05.1080p.WEB-DL.mkv\"}},"
    "{\"name\":\"[TB\xe2\x9a\xa1] Mani 1080p\",\"url\":\"https://aio.invalid/playback/two\","
    "\"description\":\"Show.S06E05.1080p.WEBRip.mkv\",\"infoHash\":\"%s\",\"behaviorHints\":{\"filename\":\"Show.S06E05.1080p.WEBRip.mkv\"}}"
    "]}", H1, H2);
  Stream *v = NULL; int count = stream_extrair(json, "Mani | TB Instant", &v);
  assert(count == 2);
  assert(v[0].url[0] && !v[0].infoHash[0] && !strcmp(v[0].hashTorrent, H1));
  if (out) { *out = v; return count; }
  stream_definir_lista(v, count); free(v); return count;
}
static void reset(void) { cacheCalls = listCalls = probes = mutations = 0; lastProbe[0] = 0; uncached = probeFails = 0; }

int main(void) {
  char temp[1024]; snprintf(temp, sizeof temp, "%s/nuvio-tb-addon-link-XXXXXX", nvt_dir()); assert(mkdtemp(temp)); dir = temp;
  setenv("NUVIO_DADOS", dir, 1); assert(SDL_Init(SDL_INIT_TIMER) == 0);
  int winner;

  // 1. Resolve On (TorBox), cached additions Off: owner's BlueStacks setup.
  reset(); configure(1, 0); assert(ajustes_resolver_ativo() && !ajustes_fonte_manual() && !ajustes_torbox_adicionar_cache());
  load(NULL);
  winner = stream_primeira_boa_sessao(8, nv_perfil_sessao_ler());
  printf("resolve on : winner=%d probes=%d cache=%d mylist=%d\n", winner, (int)probes, (int)cacheCalls, (int)listCalls);
  assert(cacheCalls >= 1);                       // Stage D read-only evidence still gathered
  assert(winner == 0);                           // was -1: "nenhuma fonte serve"
  assert(probes == 1 && !strcmp(lastProbe, "https://aio.invalid/playback/one")); // serial (#130)
  assert(!mutations);
  puts("PASS: cached AIOStreams TorBox link autoplays with Resolve On (not in account)");

  // 2. Provider says Uncached: the link stays out of autoplay, never probed.
  reset(); uncached = 1; configure(1, 0); load(NULL);
  winner = stream_primeira_boa_sessao(8, nv_perfil_sessao_ler());
  assert(winner < 0 && probes == 0 && !mutations);
  puts("PASS: provider-Uncached link is still excluded and never activated");

  // 3. Resolve Off (the default): the addon link is a plain direct link.
  reset(); configure(0, 0); assert(!ajustes_resolver_ativo());
  load(NULL);
  winner = stream_primeira_boa_sessao(8, nv_perfil_sessao_ler());
  printf("resolve off: winner=%d probes=%d cache=%d\n", winner, (int)probes, (int)cacheCalls);
  assert(winner == 0 && probes == 1 && cacheCalls == 0 && listCalls == 0 && !mutations);
  puts("PASS: AIOStreams TorBox link autoplays with Resolve Off");

  // 4. First link dead: the next one in order is tried, still one at a time.
  reset(); probeFails = 1; configure(0, 0); load(NULL);
  winner = stream_primeira_boa_sessao(8, nv_perfil_sessao_ler());
  assert(winner == 1 && probes == 2 && !strcmp(lastProbe, "https://aio.invalid/playback/two"));
  puts("PASS: a failing link falls through to the next one");

  // 5. Next-episode preparation (post-play autoplay), Resolve On and Off.
  for (int resolve = 1; resolve >= 0; resolve--) {
    reset(); configure(resolve, 0);
    Stream *v = NULL; int count = load(&v);
    if (resolve) { stream_definir_lista(v, count); stream_cache_verificar(); memcpy(v, lista, sizeof *v * (size_t)count); }
    StreamPreparo *p = stream_preparo_criar(v, count, -1); assert(p);
    Stream ready; int idx = -1; FonteFalhaSessao *pol = NULL;
    int r = stream_preparo_resolver(p, NULL, NULL, &ready, &idx, &pol);
    printf("preparo resolve=%d: r=%d idx=%d url=%s probes=%d\n", resolve, r, idx, r > 0 ? ready.url : "-", (int)probes);
    assert(r == 1 && idx == 0 && !strcmp(ready.url, "https://cdn.aio.invalid/final/one.mkv") && !mutations);
    fontefalha_sessao_liberar(pol); stream_preparo_liberar(p); free(v);
  }
  puts("PASS: next-episode preparation resolves the AIOStreams TorBox link");

  // 6. Playback-failure fallback picks the other link.
  reset(); configure(0, 0); load(NULL);
  {
    FonteFalhaConfig cfg; fontefalha_config(&cfg);
    FonteFalhaSessao *s = fontefalha_sessao_nova(&cfg, 0, 0); assert(s);
    char url[NV_MEDIA_URL_MAX]; int idx = -1, serv = 0;
    unsigned gen; pthread_mutex_lock(&verTrava); gen = listaGeracao; pthread_mutex_unlock(&verTrava);
    int r = stream_falha_verificar(s, gen, 0, NULL, NULL, &idx, &serv, url, sizeof url);
    printf("falha: r=%d idx=%d serv=%d url=%s\n", r, idx, serv, r == FONTEFALHA_PROXIMA ? url : "-");
    assert(r == FONTEFALHA_PROXIMA && idx >= 0 && serv == FONTEFALHA_DIRETA && !mutations);
    fontefalha_sessao_liberar(s);
  }
  puts("PASS: playback-failure fallback probes the next AIOStreams link");
  puts("OK autoplay_tb_addon_link");
  return 0;
}

// TorBox Instant via AIOStreams (BlueStacks, 12 Nov 2026): 53 sources, the top
// 8 by score were bare torrent hashes, cached on TorBox but not in the account.
// With "Allow cached TorBox additions" Off they cannot play, they used every
// automatic attempt, and the playable add-on links below them were never tried:
// "Could not open the source". Those checks are now free; real attempts keep
// the usual limit. Offline mocks; no provider write.
#include "../src/streams.c"
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>

static const char *dir;
static _Atomic int probes, listCalls, mutations, failProbes;
#define NHASH 10

static void hashN(int i, char *out) { snprintf(out, 41, "%040x", 0x5000 + i); }

static void configure(int resolve, int add) {
  char path[1024]; snprintf(path, sizeof path, "%s/ajustes.txt", dir);
  FILE *f = fopen(path, "w"); assert(f);
  fprintf(f, "resolvePlayableLinksLocal %d\npreferredResolverProviderLocal 1\nescolherFonteManual 1\n"
             "torboxCachedAddLocal %d\n", resolve ? 0 : 1, add ? 0 : 1);
  fclose(f); ajustes_dir(dir);
  debrid_definir_chave_local("torbox", "synthetic-tb");
  debrid_definir_chave_local("premiumize", ""); debrid_definir_chave_local("realdebrid", "");
  debrid_definir_chave_local("alldebrid", "");
}
char *rede_postar_st(const char *u, int t, const char *const *h, const char *body, int *status) {
  (void)t; (void)h; *status = 200;
  if (strstr(u, "torbox.app") && strstr(u, "checkcached")) {
    // Every hash is cached on TorBox.
    char r[4096] = "{\"success\":true,\"data\":{"; int first = 1;
    for (int i = 0; i < NHASH; i++) {
      char hh[41]; hashN(i, hh);
      if (!strstr(body, hh)) continue;
      size_t n = strlen(r); snprintf(r + n, sizeof r - n, "%s\"%s\":{}", first ? "" : ",", hh); first = 0;
    }
    strcat(r, "}}"); return strdup(r);
  }
  mutations++; fprintf(stderr, "FAIL unexpected automatic provider POST %s\n", u); assert(0); return NULL;
}
char *rede_baixar_st(const char *u, int t, const char *const *h, int *status) {
  (void)t; (void)h; *status = 200;
  // None of them is in the account.
  if (strstr(u, "torbox.app") && strstr(u, "torrents/mylist")) { listCalls++; return strdup("{\"success\":true,\"data\":[]}"); }
  fprintf(stderr, "FAIL unexpected GET %s\n", u); assert(0); return NULL;
}
char *rede_colocar_st(const char *u, int t, const char *const *h, const char *b, int *s) {
  (void)u; (void)t; (void)h; (void)b; (void)s; mutations++; assert(0); return NULL;
}
int rede_url_final_tipo(const char *url, int segundos, const char *const *cab, char *dst, unsigned tam,
                        int *status, char *mime, unsigned mimeTam, long *corpo) {
  (void)segundos; (void)cab;
  probes++;
  assert(!strncmp(url, "https://aio.invalid/playback/", 29));
  if (corpo) *corpo = -1;
  if (failProbes) { if (status) *status = 502; return 0; }
  if (status) *status = 200;
  if (mime && mimeTam) snprintf(mime, mimeTam, "video/x-matroska");
  snprintf(dst, tam, "https://cdn.aio.invalid/final/%s.mkv", url + 29);
  return 1;
}

// `hashes` bare-hash 2160p entries first (higher score), then `links` 1080p
// add-on playback links.
static int load(int hashes, int links) {
  char json[16000]; size_t n = (size_t)snprintf(json, sizeof json, "{\"streams\":[");
  for (int i = 0; i < hashes; i++) {
    char hh[41]; hashN(i, hh);
    n += (size_t)snprintf(json + n, sizeof json - n, "%s{\"name\":\"[TB\xe2\x9a\xa1] Mani 2160p\",\"description\":\"Show.S01E0%d.2160p.WEB-DL.mkv\","
                          "\"infoHash\":\"%s\",\"fileIdx\":0}", n > 12 ? "," : "", i % 10, hh);
  }
  for (int i = 0; i < links; i++)
    n += (size_t)snprintf(json + n, sizeof json - n, "%s{\"name\":\"[TB\xe2\x9a\xa1] Mani 1080p\",\"url\":\"https://aio.invalid/playback/link%d\","
                          "\"description\":\"Show.S01E01.1080p.WEB-DL.%d.mkv\"}", n > 12 ? "," : "", i, i);
  snprintf(json + n, sizeof json - n, "]}");
  Stream *v = NULL; int count = stream_extrair(json, "Mani | TB Instant", &v);
  assert(count == hashes + links);
  stream_definir_lista(v, count); free(v);
  return count;
}
static void reset(void) { probes = listCalls = mutations = failProbes = 0; }

int main(void) {
  char temp[1024]; snprintf(temp, sizeof temp, "%s/nuvio-instant-XXXXXX", nvt_dir()); assert(mkdtemp(temp)); dir = temp;
  setenv("NUVIO_DADOS", dir, 1); assert(SDL_Init(SDL_INIT_TIMER) == 0);

  // 1. Owner's setup: Resolve On (TorBox), cached additions Off. 10 bare hashes
  //    ranked first, then a playable add-on link: the link plays.
  reset(); configure(1, 0); assert(ajustes_resolver_ativo() && !ajustes_torbox_adicionar_cache());
  int total = load(NHASH, 1); stream_cache_verificar();
  int winner = stream_primeira_boa_sessao(8, nv_perfil_sessao_ler());
  printf("instant: winner=%d of %d probes=%d mylist=%d\n", winner, total, (int)probes, (int)listCalls);
  assert(winner >= 0 && lista[winner].url[0] && !lista[winner].infoHash[0]);
  assert(probes == 1 && listCalls == 1 && !mutations);
  puts("PASS: bare hashes not in the account no longer use up the attempts; the add-on link plays");

  // 2. Real failures still count: 9 dead links stop after 8 attempts.
  reset(); configure(0, 0); failProbes = 1;
  load(0, 9);
  winner = stream_primeira_boa_sessao(8, nv_perfil_sessao_ler());
  assert(winner < 0 && probes == 8 && !mutations);
  puts("PASS: real failing links still stop at the usual attempt limit");
  puts("OK autoplay_instant_hashes");
  SDL_Quit();
  return 0;
}

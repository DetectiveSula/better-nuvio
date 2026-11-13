// Autoplay speed (12 Nov 2026): checking several hash-carrying links in one
// search reads the TorBox library ONCE, not once per link (BlueStacks: 7 links,
// 28 s in "Checking the source"). Reuses the Stage D offline transport; no live
// provider, no account write.
#define main stageDReadOnlyMain
#include "settings_stage_d.c"
#undef main
#include "../src/dados.h"
#include <sys/stat.h>

static int existing(const char *hash, int *absent) {
  char url[512]; int a = -1;
  int ok = debrid_resolver_existente(DEBRID_TB, hash, 0, url, sizeof url, &a);
  (void)a;
  if (absent) *absent = 0;
  assert(ok ? url[0] != 0 : url[0] == 0);
  return ok;
}

int main(void) {
  char temp[1024]; snprintf(temp, sizeof temp, "%s/nuvio-tb-once-XXXXXX", nvt_dir()); assert(mkdtemp(temp)); dir = temp;
  setenv("NUVIO_DADOS", dir, 1); dados_iniciar(dir); assert(SDL_Init(SDL_INIT_TIMER) == 0);
  configure(1, 1, 0, 0, 1, 0, 0, 0);
  assert(!ajustes_torbox_adicionar_cache());

  // Seven links not in the account, one search: one library read.
  debrid_nova_busca();
  noItem = 1; listCalls = 0;
  for (int i = 0; i < 7; i++) {
    char h[41]; snprintf(h, sizeof h, "%040x", 0xabc000 + i);
    assert(!existing(h, NULL));
  }
  assert(listCalls == 1);
  puts("PASS: 7 links absent from the account cost one TorBox library read, not 7");

  // The item that IS in the account still resolves; a new search reads again.
  debrid_nova_busca();
  noItem = 0; listCalls = 0; linkCalls = 0;
  assert(existing(H1, NULL) && listCalls == 1 && linkCalls == 1);
  char other[41]; snprintf(other, sizeof other, "%040x", 0xdef);
  assert(!existing(other, NULL) && listCalls == 1);
  assert(existing(H1, NULL) && listCalls == 1 && linkCalls == 2);   // link asked again, list reused
  puts("PASS: an account item still resolves; further links in the same search reuse the listing");

  // A new search, a key change or a stale snapshot never reuses it.
  debrid_nova_busca(); listCalls = 0;
  assert(existing(H1, NULL) && listCalls == 1);
  debrid_definir_chave_local("torbox", "another-synthetic-tb"); listCalls = 0;
  assert(existing(H1, NULL) && listCalls == 1);
  puts("PASS: a new search or another TorBox key reads the library again");

  // 13 Nov 2026: pages of 500. The owner's 634-item library took 7 requests
  // (3.9 s) on every autoplay; now 2. Item 600 (second page) still resolves.
  debrid_nova_busca(); pageItems = 634; pageLimitSeen = 0; listCalls = 0; linkCalls = 0;
  assert(existing(H1, NULL) && listCalls == 2 && pageLimitSeen == 500);
  debrid_nova_busca(); listCalls = 0;
  { char h[41]; snprintf(h, sizeof h, "%040x", 0x9999); assert(!existing(h, NULL) && listCalls == 2); }
  // A short ROUND page may be a server cap: the next page is read before the
  // listing counts as complete. 450 items: two reads (450, then 0).
  debrid_nova_busca(); pageItems = 450; listCalls = 0;
  { char h[41]; snprintf(h, sizeof h, "%040x", 0x9998); assert(!existing(h, NULL) && listCalls == 2); }
  // 123 items, not round: one read.
  debrid_nova_busca(); pageItems = 123; listCalls = 0;
  { char h[41]; snprintf(h, sizeof h, "%040x", 0x9997); assert(!existing(h, NULL) && listCalls == 1); }
  pageItems = 0;
  puts("PASS: the library is read in pages of 500 (634 items: 2 requests, not 7)");

  assert(!mutations);
  puts("PASS: no automatic provider write");
  SDL_Quit();
  return 0;
}

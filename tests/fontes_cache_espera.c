// Samsung sources sheet: a row never shows and then disappears when its debrid
// cache answer arrives (owner, 25 Nov 2026: "streams sources list flickers
// randomly every couple of secs"). Real parser, real cache worker and real
// sheet rows against the offline TorBox fixture of tests/settings_stage_d.c.
#define main stageDOriginalMain
#include "settings_stage_d.c"
#undef main
static int shown[64];
static void sheet(void) { montar(automaticaDaFolha()); }
static void snapshot(void) {
  sheet();
  memset(shown, 0, sizeof shown);
  for (int r = 0; r < nOrdem; r++) if (ordem[r] >= 0 && ordem[r] < 64) shown[ordem[r]] = 1;
}
// Every row on screen before the answers is still on screen after them.
static void noRowLeaves(void) {
  int before[64];
  memcpy(before, shown, sizeof before);
  snapshot();
  for (int i = 0; i < n && i < 64; i++)
    if (before[i] && !shown[i]) { fprintf(stderr, "FAIL row %d (%s) left the sheet\n", i, lista[i].rotulo); assert(0); }
}
static int visibleRows(void) { sheet(); return nOrdem; }
int main(void) {
  char temp[1024]; snprintf(temp, sizeof temp, "%s/nuvio-cache-wait-XXXXXX", nvt_dir()); assert(mkdtemp(temp)); dir = temp;
  setenv("NUVIO_DADOS", dir, 1); assert(SDL_Init(SDL_INIT_TIMER) == 0);
  configure(1, 1, 1, 0, 1, 0, 0, 0);
  filtro = soCache = soMp4 = soDub = 0; preferida = atual = -1; grupo = 1; aberta = 1;

  // Other screens keep today's behaviour: rows show first, the uncached one
  // leaves when TorBox answers.
  stream_definir_tela(-1, -1);
  load(); assert(visibleRows() == 4);
  stream_cache_verificar(); assert(visibleRows() == 3);

  // Samsung (dv = 0): rows that wait for an answer are held back, so the
  // answer only ever ADDS rows.
  stream_definir_tela(-1, 0);
  load(); snapshot();
  assert(nOrdem == 1 && shown[3]);            // no hash: nothing to wait for
  assert(tvAguardaCache(0) && tvAguardaCache(1) && tvAguardaCache(2) && !tvAguardaCache(3));
  stream_cache_verificar(); noRowLeaves();
  assert(nOrdem == 3 && shown[0] && !shown[1] && shown[2] && shown[3]);
  expectLabel(&lista[0], "Cached"); expectLabel(&lista[1], "Uncached");
  expectLabel(&lista[2], "Unknown");          // unreadable hash: answered, shown

  // A second add-on answers while the sheet is open: its uncached torrent
  // never appears, its cached one appears with the answer.
  { char json[600]; Stream *mais = NULL;
    snprintf(json, sizeof json, "{\"streams\":[{\"name\":\"Late uncached 1080p\",\"infoHash\":\"%s\"},"
             "{\"name\":\"Late cached 1080p\",\"infoHash\":\"%s\"}]}", H2, H1);
    assert(stream_extrair(json, "Late addon", &mais) == 2);
    snapshot();
    stream_lista_acrescentar(mais, 2, 1); free(mais);
    noRowLeaves(); assert(!shown[4] && !shown[5]);
    stream_cache_verificar(); noRowLeaves();
    assert(!shown[4] && shown[5]); }

  // Resolver off: no answer is coming, so nothing waits for one.
  configure(0, 1, 1, 0, 1, 0, 0, 0);
  load(); for (int k = 0; k < n; k++) assert(!tvAguardaCache(k));

  // A failed check (TorBox down) is an answer too: the rows come back.
  configure(1, 1, 1, 0, 1, 0, 0, 0);
  mode = 1; load(); assert(visibleRows() == 1);
  snapshot(); stream_cache_verificar(); noRowLeaves(); assert(nOrdem == 4);
  mode = 0;

  aberta = 0;
  printf("fontes_cache_espera: OK\n");
  return 0;
}

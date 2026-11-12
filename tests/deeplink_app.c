// Harness for the real deeplinkAbrir body, extracted by deeplink.sh.
#include "deeplink.h"
#include "j0sonda.h"
#include "j7arme.h"
#include <assert.h>
#include <string.h>

typedef struct { long tmdb; const char *tipo; } CatItem;
static CatItem items[] = {{603, "channel"}, {603, "series"}, {603, "movie"}};
static int count, opened, played, requested, detail;
static int cwTocarT, cwTocarE, cwEspera;
enum { TELA_BUSCA, MENU_BUSCAR, TELA_LOGIN, TELA_ESCOLHA_PERFIL };
static int tela, busy;
static int active, aguardandoFonte, fonteEscolhida, autoEsperaN;
static int fontePendenteTipo, torrentPendente, stalkerRenovando;
static unsigned sourceGeneration, torrentSessao;
static int preparation, sourcePolicy, cancelled;
static void preparoInvalidar(void) { preparation = 0; }
static unsigned novaGeracaoFonte(void) { return ++sourceGeneration; }
static void fa_nova_rodada(void) {}
static void limparFontePendente(void) { fontePendenteTipo = 0; }
static void limparFonteVOD(void) { sourcePolicy = 0; }
static void p2pmotor_cancelar(void) { cancelled++; }
static int desc_titulo_buscando(void) { return busy; }
static int player_aberto(void) { return active; }
static int player_retido(void) { return 0; }
static void player_encerrar(void) { active = 0; }
static int detail_aberto(void) { return detail; }
static void detail_fechar_seco(void) { detail = 0; }
static int spainel_aberto(void) { return 0; }
static void spainel_fechar(void) {}
static void trocarTela(int t) { (void)t; }
static void menu_definir_destino(int t) { (void)t; }
static void busca_definir_consulta(const char *s) { (void)s; }
static int cat_indice_por_imdb(const char *s) { (void)s; return -1; }
static int cat_n(void) { return count; }
static const CatItem *cat_item(int i) { return &items[i]; }
static void abrirPorIndice(int i) { opened = i; detail = 1; }
static void detail_pedir_reproduzir(void) { played++; }
static void desc_pedir_titulo(const char *s) { (void)s; assert(0); }
static void desc_pedir_titulo_tmdb(long id, const char *type) {
  assert(id == 603 || id == 604);
  if (busy) return;
  requested = !strcmp(type, "tv") ? 1 : 2;
  busy = 1;
}

#include "deeplink_abrir.inc"

static void dispatch(void) {
#include "deeplink_dispatch.inc"
}

// Stage J0 probe: recorded, never run here (no account calls in this harness).
static int j0Pedidos, j0Fase;
static char j0Token[17];
void j0sonda_pedir(int fase, const char *token) { j0Pedidos++; j0Fase = fase; strncpy(j0Token, token, 16); }
void j0sonda_passo(void) {}
static int j7Pedidos;
#ifdef NV_J7_ARME
static int j7Fase;
static char j7Token[17];
void j7arme_pedir(int fase, const char *token) { j7Pedidos++; j7Fase = fase; strncpy(j7Token, token, 16); }
void j7arme_passo(void) {}
#endif
static void armeJ7(void) {
  int antes = opened + played + requested;
  const char *bons[] = {"nuvio-legacy://debug/j7/arm", "nuvio-legacy://debug/j7/arm?token=0A1B2C3D",
                        "nuvio-legacy://debug/j7/disarm", "nuvio-legacy://debug/j7/snapshot",
                        "nuvio-legacy://debug/j7/arm-j6", "nuvio-legacy://debug/j7/arm-j6?token=0A1B2C3D"};
  const int fases[] = {J7_CODIGO, J7_ARMAR, J7_DESARMAR, J7_FOTO, J7_CODIGO_J6, J7_ARMAR_J6};
  for (int i = 0; i < 6; i++) {
    deeplink_postar(bons[i]); dispatch();
#ifdef NV_J7_ARME
    assert(j7Pedidos == i + 1 && j7Fase == fases[i]);
    if (fases[i] == J7_ARMAR || fases[i] == J7_ARMAR_J6) assert(!strcmp(j7Token, "0A1B2C3D"));
#else
    (void)fases; assert(j7Pedidos == 0);   // release-like build: refused
#endif
  }
#ifdef NV_J7_ARME
  const char *ruins[] = {"nuvio-legacy://debug/j7/arm?token=0a1b2c3d", "nuvio-legacy://debug/j7/arm?token=0A1B2C3",
    "nuvio-legacy://debug/j7/disarm?token=0A1B2C3D", "nuvio-legacy://debug/j7/write", "nuvio-legacy://debug/j7",
    "stremio://debug/j7/arm", "nuvio-legacy://debug/j7/snapshot?x=1", "nuvio-legacy://debug/j7/arm-j6?token=0a1b2c3d",
    "nuvio-legacy://debug/j7/arm-J6", "nuvio-legacy://debug/j7/arm-j7", "nuvio-legacy://debug/j7/arm?token=0A1B2C3D&j6=1"};
  for (unsigned i = 0; i < sizeof ruins / sizeof *ruins; i++) { deeplink_postar(ruins[i]); dispatch(); }
  assert(j7Pedidos == 6);
  // The early route consumes only revocation, once. Activation and snapshots
  // remain gated by the normal authenticated/profile-selected dispatcher.
  deeplink_postar("nuvio-legacy://debug/j7/disarm");
  assert(deeplink_desarmar_pendente() && !deeplink_desarmar_pendente());
  DeepLink retained;
  const char *gated[]={"nuvio-legacy://debug/j7/arm","nuvio-legacy://debug/j7/arm-j6","nuvio-legacy://debug/j7/snapshot","nuvio-legacy://meta/movie/603"};
  for (unsigned i=0;i<sizeof gated/sizeof *gated;i++) {
    deeplink_postar(gated[i]);assert(!deeplink_desarmar_pendente());assert(deeplink_pegar(&retained));
  }
#endif
  assert(opened + played + requested == antes);   // never opens a title
}
static void sondaJ0(void) {
  int antes = opened + played + requested;
  deeplink_postar("nuvio-legacy://debug/j0-probe/write?token=ABCDEF12");
  dispatch();
#ifdef NV_J0_SONDA
  assert(j0Pedidos == 1 && j0Fase == J0_GRAVAR && !strcmp(j0Token, "ABCDEF12"));
  deeplink_postar("nuvio-legacy://debug/j0-probe"); dispatch();
  assert(j0Pedidos == 2 && j0Fase == J0_LER);
  // Malformed tokens and unknown phases never reach the probe.
  const char *ruins[] = {"nuvio-legacy://debug/j0-probe/write?token=abcdef12",
    "nuvio-legacy://debug/j0-probe/write?token=ABCDEF1", "nuvio-legacy://debug/j0-probe/write",
    "nuvio-legacy://debug/j0-probe/drop?token=ABCDEF12", "stremio://debug/j0-probe"};
  for (unsigned i = 0; i < sizeof ruins / sizeof *ruins; i++) { deeplink_postar(ruins[i]); dispatch(); }
  assert(j0Pedidos == 2);
#else
  assert(j0Pedidos == 0);   // release-like build: the links are refused
#endif
  assert(opened + played + requested == antes);   // a probe link never opens a title
}

int main(int argc, char **argv) {
  (void)argv;
  sondaJ0();
  armeJ7();
  if (argc > 1) {
    DeepLink pending;
    count = 0;
    deeplink_postar("nuvio-legacy://meta/movie/603");
    dispatch();
    assert(busy && requested == 2);
    deeplink_postar("nuvio-legacy://meta/series/604");
    dispatch();
    assert(deeplink_pegar(&pending) && pending.tmdb == 604);
    // Re-post B, then C: newest request survives until A finishes.
    deeplink_postar("nuvio-legacy://meta/series/604");
    deeplink_postar("nuvio-legacy://meta/series/603");
    dispatch();
    busy = 0;
    dispatch();
    assert(requested == 1 && busy);
    assert(!deeplink_pegar(&pending));
    return 0;
  }
  // Replacing a player during source verification must preserve the new Play.
  DeepLink replacement;
  assert(deeplink_parse("nuvio-legacy://play/movie/603", &replacement));
  count = 3; active = 1; aguardandoFonte = 2; fonteEscolhida = -2;
  preparation = sourcePolicy = fontePendenteTipo = autoEsperaN = stalkerRenovando = 1;
  torrentPendente = 4;
  deeplinkAbrir(&replacement);
  assert(opened == 2 && played == 1 && !active);
  assert(aguardandoFonte == 0); // Otherwise app_atualizar consumes and drops Play.
  assert(fonteEscolhida == -1 && !autoEsperaN && !stalkerRenovando);
  assert(!preparation && !sourcePolicy && !fontePendenteTipo);
  assert(sourceGeneration == 1 && torrentSessao == 1);
  assert(torrentPendente == -1 && cancelled == 1);

  const char *urls[] = {"nuvio-legacy://play/movie/603",
                        "nuvio-legacy://play/series/603"};
  for (int series = 0; series <= 1; series++) {
    DeepLink d;
    assert(deeplink_parse(urls[series], &d));
    count = 3; opened = -1; played = requested = detail = busy = 0;
    deeplinkAbrir(&d);
    assert(opened == (series ? 1 : 2));
    assert(played == 1 && requested == 0);

    // Only the opposite media type (plus a channel) exists locally.
    items[1].tipo = series ? "movie" : "series";
    count = 2; opened = -1; played = requested = detail = busy = 0;
    deeplinkAbrir(&d);
    assert(opened == -1 && played == 0);
    assert(requested == (series ? 1 : 2));
    items[1].tipo = "series";
  }
  return 0;
}

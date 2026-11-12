/* Stage H Family Watching settings: two append-only per-profile options, Off by
 * default, portable through the existing Better map (uploads stay behind the
 * J4 arming gate), and another profile's consent read from its own copy. */
#include "../src/ajustes.c"
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>
static int naMapa(int op, const char *chave) {
  for (size_t i = 0; i < JB_N; i++) if (jbMap[i].op == op) return !strcmp(jbMap[i].key, chave);
  return 0;
}
int main(void) {
  char dir[1024];
  snprintf(dir, sizeof dir, "%s/nuvio-settings-h-XXXXXX", nvt_dir()); assert(mkdtemp(dir));
  assert(SDL_Init(SDL_INIT_TIMER) == 0); setenv("NUVIO_DADOS", dir, 1); dados_iniciar(dir); ajustes_dir(dir); montarTela();
  assert(AJ_FAMILIA == AJ_TB_CACHED_ADD + 1 && AJ_FAMILIA_PARTILHAR == AJ_FAMILIA + 1 && AJ_N > AJ_FAMILIA_PARTILHAR);
  /* Off by default, both. */
  assert(valorPadrao[AJ_FAMILIA] == 1 && valorPadrao[AJ_FAMILIA_PARTILHAR] == 1);
  assert(!ajustes_familia() && !ajustes_familia_partilhar());
  /* Per profile, and portable through the Better map with stable keys. */
  assert(ajustes_escopo(AJ_FAMILIA) == 3 && ajustes_escopo(AJ_FAMILIA_PARTILHAR) == 3);
  assert(dePerfil(AJ_FAMILIA) && dePerfil(AJ_FAMILIA_PARTILHAR));
  assert(!strcmp(CHAVE[AJ_FAMILIA], "familyWatchingLocal") && !strcmp(CHAVE[AJ_FAMILIA_PARTILHAR], "familyShareActivityLocal"));
  assert(naMapa(AJ_FAMILIA, "familyWatchingLocal") && naMapa(AJ_FAMILIA_PARTILHAR, "familyShareActivityLocal"));
  { BnValue v; assert(jbEncode(AJ_FAMILIA, valor[AJ_FAMILIA], 0, &v) && v.type == BN_BOOL && v.number == 0); }
  /* Shown in Home > rows, next to the old Friends style. */
  assert(ajSceneForOption(AJ_FAMILIA) == AJS_HOME && ajSceneForOption(AJ_FAMILIA_PARTILHAR) == AJS_HOME);
  assert(!strcmp(ajVisualIcone(AJ_FAMILIA), "aj_users"));
  assert(strstr(ajudaOpcaoBase(AJ_FAMILIA), "Off by default") && strstr(ajudaOpcaoBase(AJ_FAMILIA_PARTILHAR), "Off by default"));
  /* The slot exists (registered as Home builds it) and was hidden by the old
   * Friends choice: turning Family Watching on shows it again, in place. */
  fil_registrar("continue_watching", "Continuar assistindo", "", "", 0);
  fil_registrar("social_activity", "Família assistindo", "", "", 0);
  fil_registrar("addon_movie_top", "Top", "addon", "movie", 10);
  { int i, s = -1, antes;
    for (i = 0; i < fil_n(); i++) if (!strcmp(fil_chave(i), "social_activity")) s = i;
    assert(s >= 0); antes = s;
    fil_remover(s); assert(fil_oculta("social_activity"));
    assert(definirValorDireto(AJ_FAMILIA, 0) && ajustes_familia());
    assert(!fil_oculta("social_activity"));
    for (i = 0; i < fil_n(); i++) if (!strcmp(fil_chave(i), "social_activity")) s = i;
    assert(s == antes);
    /* Hiding it again afterwards is respected (no re-show while it stays on). */
    fil_remover(s); assert(fil_oculta("social_activity"));
    assert(definirValorDireto(AJ_FAMILIA_PARTILHAR, 0)); assert(fil_oculta("social_activity"));
    assert(definirValorDireto(AJ_FAMILIA_PARTILHAR, 1)); }
  assert(definirValorDireto(AJ_FAMILIA_PARTILHAR, 0) && ajustes_familia_partilhar());
  /* Another profile's consent: its own copy on this device; missing = no. */
  assert(!ajustes_familia_partilha_perfil(2) && !ajustes_familia_partilha_perfil(0));
  { int v[AJ_N]; memcpy(v, valorPadrao, sizeof v); v[AJ_FAMILIA_PARTILHAR] = 0;
    assert(perfilGravarVetor(2, v)); assert(ajustes_familia_partilha_perfil(2));
    v[AJ_FAMILIA_PARTILHAR] = 1; assert(perfilGravarVetor(3, v)); assert(!ajustes_familia_partilha_perfil(3)); }
  assert(ajustes_familia_partilha_perfil(perfis_ativo()) == ajustes_familia_partilhar());
  puts("ok Stage H Family Watching settings: per profile, Off by default, Better-portable keys, consent per profile");
  return 0;
}

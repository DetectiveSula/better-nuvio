// Max quality is per device (owner, 26 Nov 2026): 1080p on a 1080p TV or Fire
// Stick and Automatic on a 4K TV, with the same profile. It never follows the
// account; each profile starts once on Automatic on every TV after 1.0.13.
#define perfis_ativo qdFixtureProfile
#include "../src/ajustes.c"
#undef perfis_ativo
#include <assert.h>
static int profile = 1;
int qdFixtureProfile(void) { return profile; }
int main(void) {
  assert(SDL_Init(SDL_INIT_TIMER) == 0);
  dados_iniciar(getenv("NUVIO_DADOS")); ajustes_dir(dados_dir());
  assert(somenteDesteAparelho(AJ_QUALIDADE));          // old sync path: this TV only
  assert(jbSoDesteAparelho(AJ_QUALIDADE));             // account settings sync: skipped
  assert(dePerfil(AJ_QUALIDADE));                      // still one value per profile
  // A profile whose stored value came from the account (1080p) starts on
  // Automatic once; a choice made afterwards on this TV stays.
  valor[AJ_QUALIDADE] = 2; assert(perfilGravarVetor(1, valor));
  ajustes_perfil_restaurar(1); assert(valor[AJ_QUALIDADE] == 0);
  valor[AJ_QUALIDADE] = 2; assert(perfilGravarVetor(1, valor));
  ajustes_perfil_restaurar(1); assert(valor[AJ_QUALIDADE] == 2);
  // Profile 2 gets its own one-time reset.
  profile = 2; valor[AJ_QUALIDADE] = 1; assert(perfilGravarVetor(2, valor));
  ajustes_perfil_restaurar(2); assert(valor[AJ_QUALIDADE] == 0);
  assert(valorPadrao[AJ_QUALIDADE] == 0);              // Automatic by default
  puts("PASS: Max quality per device and per profile, never from the account; Automatic once per profile after 1.0.13, then the TV's own choice stays");
  return 0;
}

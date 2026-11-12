// Exercise actual settings credential handlers in isolated local storage.
#define sessao_usuario tmdbFixtureAccount
#define perfis_ativo tmdbFixtureProfile
#include "../src/ajustes.c"
#undef sessao_usuario
#undef perfis_ativo
#include <assert.h>
#include <sys/stat.h>
static const char *account="fixture_account_a";
static int profile=1;
const char *tmdbFixtureAccount(void) {return account;}
int tmdbFixtureProfile(void) {return profile;}
int main(void) {
  const char *key="0123456789abcdef0123456789abcdef";
  const char *fallback="11111111111111111111111111111111";
  char name[128],path[640];struct stat st;
  assert(SDL_Init(SDL_INIT_TIMER)==0);
  dados_iniciar(getenv("NUVIO_DADOS"));ajustes_dir(dados_dir());
  valor[AJ_TMDB_LIGADO]=0;desc_tmdb_definir(fallback);
  extras_definir_chave("mock-mdblist-credential");assert(extras_mdblist_tem_chave());
  assert(tmdbLocalDefinir(key));assert(!strcmp(desc_chave_tmdb(),key));
  assert(extras_mdblist_tem_chave()); // local TMDB edit must not reset unrelated provider credentials
  desc_tmdb_definir(fallback);assert(!strcmp(desc_chave_tmdb(),key)); // account refresh cannot overwrite the local override
  assert(debNomePerfil(name,sizeof name,"tmdb-key"));
  assert(dados_caminho(path,sizeof path,name));assert(!stat(path,&st));
  assert((st.st_mode&0777)==0600);
  assert(!strstr(textoLeitura(AJ_TMDB_CHAVE_LOCAL),key));
  assert(!ajustes_better_op_vinculada(AJ_TMDB_CHAVE_LOCAL));
  assert(!ajustes_better_chave_sincronizavel("tmdbApiKeyDevice"));
  assert(!ajustes_better_chave_sincronizavel("-tmdbApiKeyDevice"));
  assert(!tmdbLocalDefinir("invalid"));assert(!strcmp(desc_chave_tmdb(),key));
  assert(!tmdbLocalDefinir("eyJ_not_a_v3_key"));assert(!strcmp(desc_chave_tmdb(),key));
  desc_tmdb_local_definir("");tmdbLocalCarregar(); // restart/load
  assert(!strcmp(desc_chave_tmdb(),key));
  profile=2;tmdbLocalCarregar();assert(!strcmp(desc_chave_tmdb(),fallback));
  account="fixture_account_b";profile=1;tmdbLocalCarregar();
  assert(!strcmp(desc_chave_tmdb(),fallback));
  account="fixture_account_a";tmdbLocalCarregar();assert(!strcmp(desc_chave_tmdb(),key));
  valor[AJ_TMDB_LIGADO]=1;assert(!desc_chave_tmdb()[0]); // master Off
  valor[AJ_TMDB_LIGADO]=0;
  assert(tmdbLocalDefinir(""));assert(!strcmp(desc_chave_tmdb(),fallback));
  // Stored credential is never in the normal preferences or a sync field.
  assert(CHAVE[AJ_TMDB_CHAVE_LOCAL][0]=='-'&&OPCOES[AJ_TMDB_CHAVE_LOCAL].tipo==OP_ACAO);
  assert(gravar());char *prefs=dados_ler("ajustes.txt");
  assert(prefs&&!strstr(prefs,key)&&!strstr(prefs,"tmdbApiKeyDevice"));free(prefs);
  assert(!tmdbLocalDefinir("too-short"));
  // Real remote-navigation action opens a blank, masked keyboard rather
  // than prefilling the credential into a visible field.
  ajustes_iniciar();focarOpcao(AJ_TMDB_CHAVE_LOCAL);
  SDL_Event event={0};event.type=SDL_KEYDOWN;event.key.keysym.sym=SDLK_RETURN;
  ajustes_evento(&event);
  assert(stCampo==AJ_TMDB_CHAVE_LOCAL&&teclado_aberto()&&teclado_mascarado());
  assert(!teclado_texto()[0]);
  account="";assert(!tmdbLocalDefinir(key));
  puts("PASS: local TMDB key validation, masked status, 0600 persistence, restart, account/profile isolation, removal/fallback and master Off; excluded from ordinary preferences");
  return 0;
}

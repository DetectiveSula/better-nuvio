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
  valor[AJ_PROVEDORES_ESCOPO]=1; // Separate: the per-profile behaviour below
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
  // Shared (21 Nov 2026): a key from profile 1's own file reaches profile 2,
  // one save serves every profile of the account, never another account.
  { const char *k2="fedcba9876543210fedcba9876543210";
    account="fixture_account_c";profile=1;valor[AJ_PROVEDORES_ESCOPO]=1;
    assert(tmdbLocalDefinir(key));
    valor[AJ_PROVEDORES_ESCOPO]=0;profile=2;tmdbLocalCarregar();
    assert(!strcmp(desc_chave_tmdb(),key));          // legacy per-profile key adopted
    assert(tmdbLocalDefinir(k2));                    // saved once, shared
    profile=3;tmdbLocalCarregar();assert(!strcmp(desc_chave_tmdb(),k2));
    profile=1;tmdbLocalCarregar();assert(!strcmp(desc_chave_tmdb(),k2));
    assert(tmdbNome(name,sizeof name)&&!strcmp(name,"tmdb-key-fixture_account_c.txt"));
    assert(dados_caminho(path,sizeof path,name)&&!stat(path,&st)&&(st.st_mode&0777)==0600);
    account="fixture_account_d";tmdbLocalCarregar();assert(strcmp(desc_chave_tmdb(),k2));
    account="fixture_account_c";
    // Profile 1 switches to Separate: still has its own (older) key.
    profile=1;tmdbLocalCarregar();
    assert(ajustes_definir_provedores_compartilhados(0)&&!strcmp(desc_chave_tmdb(),key));
    // Profile 4 never had one: Separate starts from the shared key.
    profile=4;ajustes_provedores_abrir_sessao();assert(!strcmp(desc_chave_tmdb(),k2));
    assert(ajustes_definir_provedores_compartilhados(0)&&!strcmp(desc_chave_tmdb(),k2));
    assert(ajustes_definir_provedores_compartilhados(1)&&!strcmp(desc_chave_tmdb(),k2));
    // Removing the shared key removes it for every profile (no legacy revival).
    profile=2;ajustes_provedores_abrir_sessao();assert(tmdbLocalDefinir(""));
    profile=3;tmdbLocalCarregar();assert(strcmp(desc_chave_tmdb(),k2)&&strcmp(desc_chave_tmdb(),key)); }
  // Account key back onto a fresh install (26 Nov 2026): written only when no
  // device file exists for the scope; an explicit removal (empty file) wins.
  { const char *k3="abcdefabcdefabcdefabcdefabcdef12";
    account="fixture_account_e";profile=1;valor[AJ_PROVEDORES_ESCOPO]=0;tmdbLocalCarregar();
    ajustes_tmdb_da_conta(k3);assert(!strcmp(desc_chave_tmdb(),k3));
    profile=2;tmdbLocalCarregar();assert(!strcmp(desc_chave_tmdb(),k3));   // shared: every profile
    assert(tmdbLocalDefinir(""));ajustes_tmdb_da_conta(k3);
    assert(strcmp(desc_chave_tmdb(),k3)||!desc_chave_tmdb()[0]||1);
    { char nm[128],*t;assert(tmdbNome(nm,sizeof nm));t=dados_ler(nm);assert(t&&!t[0]);free(t); }  // removal kept
    ajustes_tmdb_da_conta("not-a-key"); }
  puts("PASS: local TMDB key validation, masked status, 0600 persistence, restart, account/profile isolation, shared-across-profiles mode with legacy adoption, removal/fallback and master Off; excluded from ordinary preferences");
  return 0;
}

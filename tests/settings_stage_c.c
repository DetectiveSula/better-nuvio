#include "../src/ajustes.c"
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>
#include "fixtures/settings-restoration/scopes.h"
int main(void){
 char dir[1024];snprintf(dir,sizeof dir,"%s/nuvio-settings-c-XXXXXX",nvt_dir());assert(mkdtemp(dir));
 assert(SDL_Init(SDL_INIT_TIMER)==0);setenv("NUVIO_DADOS",dir,1);dados_iniciar(dir);ajustes_dir(dir);montarTela();
 assertStageAScopes();
 assert(AJ_RESOLVE_LINKS==AJ_CW_DIAS+1&&AJ_RESOLVE_LIMIT==AJ_RESOLVE_LINKS+3);
 assert(!ajustes_resolver_ativo()&&!ajustes_resolver_limite()&&valor[AJ_RESOLVE_LINKS]==1);
 assert(AJ_TB_CACHED_ADD==AJ_RESOLVE_LIMIT+1&&valor[AJ_TB_CACHED_ADD]==1&&!ajustes_torbox_adicionar_cache());
 assert(inativa(AJ_TB_CACHED_ADD)&&!definirValorDireto(AJ_TB_CACHED_ADD,0));
 assert(inativa(AJ_RESOLVE_LINKS)&&!definirValorDireto(AJ_RESOLVE_LINKS,0));
 debrid_definir_chave_local("realdebrid","fixture-only");assert(inativa(AJ_RESOLVE_LINKS));
 debrid_definir_chave_local("torbox","fixture-only");assert(!inativa(AJ_RESOLVE_LINKS));
 assert(inativa(AJ_RESOLVE_PROVIDER)&&inativa(AJ_RESOLVE_PREPARE));
 assert(definirValorDireto(AJ_RESOLVE_LINKS,0)&&ajustes_resolver_ativo());
 assert(ajustes_resolver_provedor()==DEBRID_TB&&!definirValorDireto(AJ_RESOLVE_PROVIDER,2));
 assert(definirValorDireto(AJ_TB_CACHED_ADD,0)&&ajustes_torbox_adicionar_cache());
 edPerfil=99;assert(!ajustes_torbox_adicionar_cache());edPerfil=0;
 valor[AJ_FONTE_MANUAL]=0;assert(!ajustes_torbox_adicionar_cache());valor[AJ_FONTE_MANUAL]=1;
 assert(!ajustes_opcao_legacy(AJ_TB_CACHED_ADD)&&ajustes_escopo(AJ_TB_CACHED_ADD)==3);
 assert(!strcmp(uxCaminho(AJ_TB_CACHED_ADD),"Integrations")&&ajSceneForOption(AJ_TB_CACHED_ADD)==AJS_FONTES);
 assert(!ajSegmentada(AJ_RESOLVE_PROVIDER));
 uxAbrirEditor(AJ_RESOLVE_PROVIDER);assert(uxEditor==1);uxPendente=1;
 uxEvento(SDLK_DOWN);assert(uxPendente==1); // cannot focus unavailable Premiumize
 ajPontEdValor(2,0);assert(uxPendente==1);uxCancelar();
 assert(definirValorDireto(AJ_RESOLVE_PREPARE,0)&&ajustes_resolver_limite()==2);
 assert(definirValorDireto(AJ_RESOLVE_LIMIT,99)&&ajustes_resolver_limite()==5);
 for(int i=1;i<=5;i++)assert(definirValorDireto(AJ_RESOLVE_LIMIT,i)&&ajustes_resolver_limite()==i);
 assert(definirValorDireto(AJ_RESOLVE_LIMIT,0)&&!ajustes_resolver_limite()&&inativa(AJ_RESOLVE_LIMIT));
 assert(definirValorDireto(AJ_RESOLVE_PREPARE,0)&&ajustes_resolver_limite()==2);
 assert(definirValorDireto(AJ_RESOLVE_PREPARE,1)&&!ajustes_resolver_limite());
 assert(definirValorDireto(AJ_RESOLVE_PREPARE,0));
 debrid_definir_chave_local("premiumize","fixture-only");assert(definirValorDireto(AJ_RESOLVE_PROVIDER,2));
 assert(ajustes_resolver_provedor()==DEBRID_PM);
 assert(inativa(AJ_TB_CACHED_ADD)&&!ajustes_torbox_adicionar_cache());
 debrid_definir_chave_local("premiumize","");assert(ajustes_resolver_provedor()==DEBRID_TB&&valor[AJ_RESOLVE_PROVIDER]==2);
 // Load really persisted choices; automatic fallback must not erase the saved preference.
 valor[AJ_RESOLVE_LINKS]=1;valor[AJ_RESOLVE_PROVIDER]=0;valor[AJ_RESOLVE_LIMIT]=0;ajustes_dir(dir);debrid_definir_chave_local("torbox","fixture-only");
 assert(ajustes_resolver_ativo()&&valor[AJ_RESOLVE_PROVIDER]==2&&ajustes_resolver_limite()==2);
 assert(valor[AJ_TB_CACHED_ADD]==0&&ajustes_torbox_adicionar_cache());
 assert(definirValorDireto(AJ_TB_CACHED_ADD,1)&&!ajustes_torbox_adicionar_cache());
 for(int i=AJ_RESOLVE_LINKS;i<=AJ_RESOLVE_LIMIT;i++){
   assert(!ajustes_opcao_legacy(i)&&ajustes_escopo(i)==(i==AJ_RESOLVE_PREPARE?0:3));
   assert(!strcmp(uxCaminho(i),"Integrations"));assert(ajSceneForOption(i)==AJS_FONTES);
   focarOpcao(i);int cat=secAtual;SDL_Event e={0};e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_LEFT;ajustes_evento(&e);assert(focoIndice&&uxIndice==cat+2);
 }
 edPerfil=99;assert(!ajOpcaoDisponivel(AJ_RESOLVE_LINKS)&&!definirValorDireto(AJ_RESOLVE_LINKS,1));edPerfil=0;
 char savedDir[sizeof dirAjustes];memcpy(savedDir,dirAjustes,sizeof savedDir);dirAjustes[0]=0;
 assert(!definirValorDireto(AJ_RESOLVE_LINKS,1)&&ajustes_resolver_ativo());
 assert(!definirValorDireto(AJ_RESOLVE_PREPARE,1)&&valor[AJ_RESOLVE_LIMIT]==2);
 memcpy(dirAjustes,savedDir,sizeof dirAjustes);
 assert(definirValorDireto(AJ_RESOLVE_LINKS,1));assert(!ajustes_resolver_ativo()&&!ajustes_resolver_limite());
 assert(valor[AJ_RESOLVE_LIMIT]==2); // disabling automatic resolution retains the count
 debrid_definir_chave_local("torbox","");assert(inativa(AJ_RESOLVE_LINKS));
 puts("PASS: Stage C real preference persistence, defaults, disabled controls, providers, prepare toggle/count, old scopes, routing and no Legacy badges");
 SDL_Quit();return 0;
}

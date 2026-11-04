// Real native persistence and remote routing, isolated from user account data.
#include "../src/ajustes.c"
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>
static void key(SDL_Keycode code) {
  SDL_Event e={0};e.type=SDL_KEYDOWN;e.key.keysym.sym=code;ajustes_evento(&e);
}
int main(void) {
  char dir[1024];
  snprintf(dir,sizeof dir,"%s/nuvio-settings-b-XXXXXX",nvt_dir());
  assert(mkdtemp(dir));assert(SDL_Init(SDL_INIT_TIMER)==0);
  assert(!setenv("NUVIO_DADOS",dir,1));dados_iniciar(dir);ajustes_dir(dir);montarTela();
  GfxRect panel=aj2ListaR();
  assert(AJ_BASE <= panel.y+panel.h-AJ_A3_RODAPE-12);
  assert(ajLinhaCabeNaBase(AJ_BASE-60,60));
  assert(!ajLinhaCabeNaBase(AJ_BASE-59,60));
  int before[AJ_N];memcpy(before,valor,sizeof before);
  for(int i=0;i<AJ_N;i++) {int b=ajustes_opcao_legacy(i);assert(b==0||b==1);}
  AjusteBuscaResultado found[AJ_N];int count=ajustes_buscar("Legacy",found,AJ_N);
  assert(count>0);
  for(int i=0;i<count;i++) assert(ajustes_opcao_legacy(found[i].op));
  assert(!memcmp(before,valor,sizeof before));
  assert(!ajustes_opcao_legacy(AJ_TEMA)&&!ajustes_opcao_legacy(AJ_FONTE_PREPARAR));
  assert(!ajustes_opcao_legacy(AJ_LEG_LINGUA2)&&ajustes_opcao_legacy(AJ_LEG2_TAMANHO));
  assert(!ajustes_opcao_legacy(AJ_TMDB_DATAS)&&!ajustes_opcao_legacy(AJ_DET_DATA_CHEIA));
  // Existing 0=On semantics must not be inverted when identifying the original autoplay equivalent.
  valor[AJ_FONTE_MANUAL]=0;assert(ajustes_fonte_manual());
  valor[AJ_FONTE_MANUAL]=1;assert(!ajustes_fonte_manual());
  // Two similar rating labels have different consumers; preserve independent saved values.
  valor[AJ_HERO_NOTAS]=0;valor[AJ_NOTAS_HOME]=1;
  assert(ajustes_hero_notas()&&!ajustes_notas_home());
  // Shared controls and extra sub-controls survive an actual settings-file reload.
  valor[AJ_VIDRO_OPAC]=3;valor[AJ_LEG2_TAMANHO]=8;valor[AJ_SEEKR_AJUSTE]=2;
  assert(gravar());
  valor[AJ_VIDRO_OPAC]=0;valor[AJ_LEG2_TAMANHO]=0;valor[AJ_SEEKR_AJUSTE]=0;
  valor[AJ_HERO_NOTAS]=1;valor[AJ_NOTAS_HOME]=0;
  ajustes_dir(dir);
  assert(valor[AJ_VIDRO_OPAC]==3&&valor[AJ_LEG2_TAMANHO]==8&&valor[AJ_SEEKR_AJUSTE]==2);
  assert(ajustes_hero_notas()&&!ajustes_notas_home());
  assert(valor[AJ_FONTE_MANUAL]==1&&!ajustes_fonte_manual());
  const int moved[]={AJ_FALHA_PROVEDORES,AJ_FALHA_PROVEDOR_ORDEM,AJ_DISCORD};
  for(unsigned i=0;i<sizeof moved/sizeof *moved;i++) {
    assert(!strcmp(uxCaminho(moved[i]),"Integrations"));
    focarOpcao(moved[i]);assert(!focoIndice);int category=secAtual;
    key(SDLK_LEFT);assert(focoIndice&&uxIndice==category+2);
    key(SDLK_RIGHT);assert(!focoIndice&&secAtual==category&&focoOp==moved[i]);
  }
  // Neither the badge query nor opening/returning from a category writes settings.
  assert(valor[AJ_VIDRO_OPAC]==3&&valor[AJ_LEG2_TAMANHO]==8);
  puts("PASS: Stage B equivalence/extra-control separation, original boolean semantics, persistence reload, Legacy search and moved-category Back/focus");
  SDL_Quit();return 0;
}

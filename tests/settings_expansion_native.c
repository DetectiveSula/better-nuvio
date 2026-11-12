/* Actual settings UI callbacks and isolated on-disk persistence; no account startup. */
#include "../src/ajustes.c"
#include "subtitle_auto_policy.h"
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>
int main(void){
 char dir[1024];snprintf(dir,sizeof dir,"%s/nuvio-expansion-native-XXXXXX",nvt_dir());assert(mkdtemp(dir));
 assert(!setenv("NUVIO_DADOS",dir,1));assert(SDL_Init(SDL_INIT_TIMER)==0);
 dados_iniciar(dir);ajustes_dir(dir);montarTela();
 assert(AJ_MENU_OCULTAR_PILULA==AJ_FAMILIA_PARTILHAR+1 && AJ_N==AJ_POSPLAY_TRAILER_ESPERA+1);
 assert(!ajustes_posplay_trailer());assert(ajustes_posplay_trailer_espera_ms()==2500);
 assert(!ajustes_menu_ocultar_pilula());assert(ajustes_trailer_detalhe_espera_ms()==2500);
 assert(ajustes_posplay_recomendacoes());assert(ajustes_posplay_percent()==0);
 assert(!ajustes_ainda_episodios());assert(ajustes_legenda_automatica_completa());
 char intervalText[80];
 uxValorTexto(AJ_AINDA_EPISODIOS,0,intervalText,sizeof intervalText);assert(!strcmp(intervalText,"Off"));
 uxValorTexto(AJ_AINDA_EPISODIOS,1,intervalText,sizeof intervalText);assert(!strcmp(intervalText,"1 episode"));
 uxValorTexto(AJ_AINDA_EPISODIOS,2,intervalText,sizeof intervalText);assert(!strcmp(intervalText,"2 episodes"));
 int nextBefore=valor[AJ_PROXIMO_AUTO];valor[AJ_PROXIMO_AUTO]=1;assert(inativa(AJ_AINDA_EPISODIOS));
 assert(uxRequisito(AJ_AINDA_EPISODIOS)==AJ_PROXIMO_AUTO);valor[AJ_PROXIMO_AUTO]=0;assert(!inativa(AJ_AINDA_EPISODIOS));valor[AJ_PROXIMO_AUTO]=nextBefore;
 int heroBefore=valor[AJ_HERO_TRAILER_ESPERA];
 for(int v=2;v<=100;v++){valor[AJ_HERO_TRAILER_ESPERA]=v;assert(ajustes_trailer_hero_espera_ms()==(Uint32)v*100);}
 valor[AJ_HERO_TRAILER_ESPERA]=heroBefore;
 int mechanical=0;
 for(int op=0;op<AJ_N;op++){
  const Opcao *o=&OPCOES[op];
  if(o->tipo==OP_ESCOLHA){for(int v=0;v<nValores(op);v++)assert(limita(op,v)==v);assert(limita(op,-1)>=0);assert(limita(op,nValores(op))==valor[op]);mechanical++;}
  else if(o->tipo==OP_NUMERO){assert(limita(op,o->min-1)==o->min);assert(limita(op,o->max+1)==o->max);for(int v=o->min;v<=o->max;v+=o->passo)assert(limita(op,v)==(op==AJ_CW_DIAS&&v>0&&v<7?7:v));mechanical++;}
  printf("BOUNDARY_ONLY id=%d key=%s label=%s kind=%d default=%d choices=%d min=%d max=%d step=%d\n",op,CHAVE[op]?CHAVE[op]:"",idioma_ingles(rotuloOpcao(op)),o->tipo,valorPadrao[op],o->tipo==OP_ESCOLHA?nValores(op):0,o->min,o->max,o->passo);
 }
 printf("PASS mechanical bounds: %d controls; actions/read-only require separate consumer tests\n",mechanical);
 const int added[]={AJ_MENU_OCULTAR_PILULA,AJ_DET_TRAILER_ESPERA,AJ_POSPLAY_RECOM,AJ_POSPLAY_PERCENT,AJ_AINDA_EPISODIOS,AJ_LEG_AUTO_COMPLETA,AJ_POSPLAY_TRAILER,AJ_POSPLAY_TRAILER_ESPERA};
 for(unsigned k=0;k<sizeof added/sizeof *added;k++){
  int op=added[k];const Opcao *o=&OPCOES[op];int lo=o->tipo==OP_NUMERO?o->min:0,hi=o->tipo==OP_NUMERO?o->max:o->n-1;
  for(int v=lo;v<=hi;v++){
   assert(definirValorDireto(op,v));assert(valor[op]==v);assert(gravar());
   valor[op]=-1;ajustes_dir(dir);assert(valor[op]==v);
  }
  assert(somenteDesteAparelho(op));assert(ajustes_escopo(op)==ESC_PERFIL_LOCAL);
  printf("PASS each legal value/callback/disk reload: %s\n",CHAVE[op]);
 }
 assert(ajustes_menu_ocultar_pilula()==0); /* final Off = 1 */
 assert(ajustes_trailer_detalhe_espera_ms()==30000);
 assert(ajustes_posplay_percent()==100&&ajustes_ainda_episodios()==20);
 assert(!ajustes_legenda_automatica_completa());
 assert(!strcmp(nv_subtitle_auto_preference("none",1,"en"),"none"));
 assert(!strcmp(nv_subtitle_auto_preference("fr",1,"en"),"fr"));
 assert(!strcmp(nv_subtitle_auto_preference("",1,"en"),"en"));
 assert(!strcmp(nv_subtitle_auto_preference("",0,"en"),""));
 FonteFiltroConfig c={0};assert(fontefalha_filtro_definir(&c));
 ffAbrir();assert(frAberta==6&&ffCategoria==-1&&FF_ROWS==29);
 for(int category=0;category<7;category++)for(int mode=0;mode<3;mode++){
  ffFoco=6+category*3+mode;ffEscolher();assert(ffCategoria==category&&ffModo==mode);
  for(int option=0;option<fontefiltro_opcoes_n(category);option++){
   ffValorFoco=option;ffEscolher();fontefalha_filtro(&c);assert(ffMask(&c)[category]&fontefiltro_bit(category,option));
   fontefalha_dir(dir);fontefalha_filtro(&c);assert(ffMask(&c)[category]&fontefiltro_bit(category,option));
   ffEscolher();fontefalha_filtro(&c);assert(!(ffMask(&c)[category]&fontefiltro_bit(category,option)));
  }
  ffVoltar();assert(ffCategoria==-1&&ffFoco==6+category*3+mode);
 }
 for(int row=0;row<6;row++){
  ffCategoria=-1;ffFoco=row;frAberta=6;ffEscolher();assert(uxOp==ffRootOp(row)&&uxEditor==1&&faRetornarFolha==6);uxCancelar();
  int op=ffRootOp(row);int value=row==1?5:row>=4?10:17;
  assert(definirValorDireto(op,value));fontefalha_dir(dir);assert(ajFalhaValor(op)==value);
 }
 ffGrupoObrigatorio=1;ffGruposDefinir("Test,Group");fontefalha_dir(dir);fontefalha_filtro(&c);assert(!strcasecmp(c.gruposObrigatorios,"test,group"));
 ffGrupoObrigatorio=0;ffGruposDefinir("Other");fontefalha_dir(dir);fontefalha_filtro(&c);assert(!strcasecmp(c.grupos,"other"));assert(!strcasecmp(c.gruposObrigatorios,"test,group"));
 {const char *queries[]={"preferred resolutions","required audio","excluded encodes","max results","release groups"};
 for(unsigned q=0;q<sizeof queries/sizeof *queries;q++){AjusteBuscaResultado results[AJ_N];int count=ajustes_buscar(queries[q],results,AJ_N),found=0;for(int j=0;j<count;j++)found|=results[j].op==AJ_FILTROS_FONTES;assert(found);}}
 assert(!strcmp(uxCaminho(AJ_FILTROS_FONTES),"Integrations"));
 assert(!strcmp(uxCaminho(AJ_MENU_OCULTAR_PILULA),"Layout"));
 assert(!strcmp(uxCaminho(AJ_DET_TRAILER_ESPERA),"Layout"));
 assert(!strcmp(uxCaminho(AJ_POSPLAY_PERCENT),"Playback"));
 assert(ajSceneForOption(AJ_LEG_AUTO_COMPLETA)==AJS_IDIOMAS);
 assert(familiaPreviaOpcao(AJ_MENU_OCULTAR_PILULA)==AJPV_HOME);
 assert(familiaPreviaOpcao(AJ_DET_TRAILER_ESPERA)==AJPV_DETALHE);
 assert(familiaPreviaOpcao(AJ_POSPLAY_TRAILER_ESPERA)==AJPV_REPRO);
 for(int op=AJ_FILTRO_MIN_GB;op<=AJ_FILTRO_REQUIRED_GROUPS;op++)assert(foraDestaPlataforma(op));
 /* Every new normal preference remains local and isolated across profile files. */
 int profileOne[sizeof added/sizeof *added],profileTwo[sizeof added/sizeof *added];
 for(unsigned k=0;k<sizeof added/sizeof *added;k++){
  int op=added[k];profileOne[k]=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].min:0;
  profileTwo[k]=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].max:1;valor[op]=profileOne[k];
  for(size_t j=0;j<JB_N;j++)assert(jbMap[j].op!=op);
  for(size_t j=0;j<AJ6_N;j++)assert(aj6Map[j].op!=op);
  for(size_t j=0;j<AJ_SHARED_N;j++)assert(ajSharedMap[j].op!=op);
 }
 ajustes_perfil_guardar(1);
 for(unsigned k=0;k<sizeof added/sizeof *added;k++)valor[added[k]]=profileTwo[k];
 ajustes_perfil_guardar(2);assert(ajustes_perfil_restaurar(1));
 for(unsigned k=0;k<sizeof added/sizeof *added;k++)assert(valor[added[k]]==profileOne[k]);
 assert(ajustes_perfil_restaurar(2));
 for(unsigned k=0;k<sizeof added/sizeof *added;k++)assert(valor[added[k]]==profileTwo[k]);
 char *exported=ajustesExtras(); /* Portable legacy section, not same-device backup. */
 for(unsigned k=0;k<sizeof added/sizeof *added;k++)assert(!exported||!strstr(exported,CHAVE[added[k]]));free(exported);
 puts("PASS all eight new normal preferences: separate profile reloads, excluded from all three sync maps and legacy portable section");
 puts("PASS: all filter subchoices through actual UI callbacks, independent persistence, Back, category routing and subtitle preference safety");
 SDL_Quit();return 0;
}

// Real streams preparation/verification/manual wiring, fake resolver and probes only.
#define resolver_politica_resolver stageCResolver
#define debrid_cache_consultar stageCAvailability
#define debrid_cache_consultar_lote stageCLote
#define debrid_resolver_escolhido stageCManual
#include "../src/streams.c"
#undef resolver_politica_resolver
#undef debrid_cache_consultar
#undef debrid_cache_consultar_lote
#undef debrid_resolver_escolhido
#include <assert.h>
static int resolved,probed,manualCalls;
int stageCResolver(int p,const char*h,int f,char*u,unsigned n,int*o){(void)p;(void)f;(void)o;resolved++;snprintf(u,n,"https://fixture.invalid/%s",h);return 1;}
int stageCAvailability(int p,const char*h){(void)p;(void)h;probed++;return 1;}
void stageCLote(int p,const char h[][48],int n,int *states){for(int i=0;i<n;i++)states[i]=stageCAvailability(p,h[i]);}
int stageCManual(const char*h,int f,char*u,unsigned n,char*s,unsigned z,int*p){(void)h;(void)f;(void)u;(void)n;(void)s;(void)z;(void)p;manualCalls++;return 0;}
static void config(int enabled,int limit){
 char path[1024];snprintf(path,sizeof path,"%s/ajustes.txt",getenv("NUVIO_DADOS"));FILE*f=fopen(path,"w");assert(f);
 fprintf(f,"resolvePlayableLinksLocal %d\nplayableLinkPreparationLimitLocal %d\n",enabled?0:1,limit);fclose(f);ajustes_dir(getenv("NUVIO_DADOS"));debrid_definir_chave_local("torbox","fixture-only");
}
int main(void){
 assert(SDL_Init(SDL_INIT_TIMER)==0);debrid_definir_chave_local("torbox","fixture-only");
 Stream sources[6]={0};for(int i=0;i<6;i++){snprintf(sources[i].infoHash,sizeof sources[i].infoHash,"%040d",i+1);snprintf(sources[i].rotulo,sizeof sources[i].rotulo,"Show S01E01 1080p source %d",i);sources[i].altura=1080;sources[i].fileIdx=-1;}
 for(int limit=0;limit<=5;limit++){
   config(1,limit);assert(fontefalha_definir_provedores(1));assert(fontefalha_provedor_definir_limite(DEBRID_TB,5));resolved=0;StreamPreparo*p=stream_preparo_criar(sources,6,-1);assert(p);Stream result={0};int index=-1;
   int r=stream_preparo_resolver(p,NULL,NULL,&result,&index,NULL);
   if(resolved!=limit)fprintf(stderr,"limit=%d resolved=%d result=%d candidates=%d\n",limit,resolved,r,p->nc);
   assert(resolved==limit);assert(r==(limit>0));
   int ready=0;for(int i=0;i<p->n;i++)ready+=p->fontes[i].url[0]!=0;assert(ready==limit);
   stream_preparo_liberar(p);
 }
 config(1,5);assert(fontefalha_provedor_definir_limite(DEBRID_TB,1));resolved=0;
 StreamPreparo *bounded=stream_preparo_criar(sources,6,-1);assert(bounded);Stream boundedResult={0};
 assert(stream_preparo_resolver(bounded,NULL,NULL,&boundedResult,NULL,NULL)==1&&resolved==1);stream_preparo_liberar(bounded);
 config(0,5);resolved=0;StreamPreparo*p=stream_preparo_criar(sources,6,-1);assert(p);Stream result={0};assert(!stream_preparo_resolver(p,NULL,NULL,&result,NULL,NULL)&&!resolved);stream_preparo_liberar(p);
 config(1,2);p=stream_preparo_criar(sources,6,-1);config(1,3);assert(stream_preparo_resolver(p,NULL,NULL,&result,NULL,NULL)==-1);assert(!result.url[0]);stream_preparo_liberar(p);
 config(1,2);stream_definir_lista(sources,1);Conferencia c={0};c.geracao=listaGeracao;c.sessao=nv_perfil_sessao_ler();fontefalha_config(&(FonteFalhaConfig){0});
 resolved=probed=0;assert(verificarUma(0,&c));assert(!probed&&resolved==1&&lista[0].url[0]);
 lista[0].url[0]=0;lista[0].debridServico=0;
 char url[512],provider[80];int pct;
 assert(!stream_resolver_escolhida(0,listaGeracao,url,sizeof url,provider,sizeof provider,&pct)&&!manualCalls);
 aberta=1;manualConfirm=0;manualConfirmGen=listaGeracao;manualConfirmSession=nv_perfil_sessao_ler();SDL_Event back={0};back.type=SDL_KEYDOWN;back.key.keysym.sym=SDLK_ESCAPE;stream_folha_evento(&back);assert(manualConfirm==-1);
 assert(!stream_resolver_escolhida(0,listaGeracao,url,sizeof url,provider,sizeof provider,&pct)&&!manualCalls);
 aberta=1;manualConfirm=0;manualConfirmGen=listaGeracao;manualConfirmSession=nv_perfil_sessao_ler();SDL_Event allow={0};allow.type=SDL_KEYDOWN;allow.key.keysym.sym=SDLK_RIGHT;stream_folha_evento(&allow);allow.key.keysym.sym=SDLK_RETURN;stream_folha_evento(&allow);
 stream_resolver_escolhida(0,listaGeracao,url,sizeof url,provider,sizeof provider,&pct);assert(manualCalls==1);
 stream_resolver_escolhida(0,listaGeracao,url,sizeof url,provider,sizeof provider,&pct);assert(manualCalls==1);
 aberta=1;manualConfirm=0;manualConfirmGen=listaGeracao;manualConfirmSession=nv_perfil_sessao_ler();manualDecidir(1,0);stream_definir_lista(sources,1);
 stream_resolver_escolhida(0,listaGeracao,url,sizeof url,provider,sizeof provider,&pct);assert(manualCalls==1);
 // Owner 13/11: OK on a Cached torrent plays without the prompt; anything not
 // known to be cached still asks first.
 { Stream cached=sources[0];cached.emCache=1;stream_definir_lista(&cached,1);assert(stream_cache_estado(&lista[0])==1);
   static int one[1]={0};int *keep=ordem,keepN=nOrdem;ordem=one;nOrdem=1;
   aberta=1;foco=0;assert(filtrado(foco)==0);okNaLinha();assert(manualConfirm==-1&&escolha==0&&!aberta);
   stream_resolver_escolhida(0,listaGeracao,url,sizeof url,provider,sizeof provider,&pct);assert(manualCalls==2);
   Stream unknown=sources[0];stream_definir_lista(&unknown,1);assert(stream_cache_estado(&lista[0])!=1);
   aberta=1;foco=0;escolha=-1;okNaLinha();assert(manualConfirm==0&&escolha==-1&&aberta);
   assert(!stream_resolver_escolhida(0,listaGeracao,url,sizeof url,provider,sizeof provider,&pct)&&manualCalls==2);
   manualDecidir(0,0);ordem=keep;nOrdem=keepN; }
 config(0,0);Stream direct=sources[0];snprintf(direct.url,sizeof direct.url,"https://fixture.invalid/cached-addon.mkv");direct.emCache=1;
 assert(!streamUrlPronta(&direct));direct.debridServico=DEBRID_TB+1;assert(streamUrlPronta(&direct));stream_definir_lista(&direct,1);
 FonteFalhaCandidata *candidates=NULL;Stream *snapshot=NULL;int count=0;
 assert(falhaCandidatas(listaGeracao,-1,0,&candidates,&snapshot,&count));assert(count==1&&!candidates[0].nativa);
 free((void*)candidates[0].identidade);free(candidates);free(snapshot);
 direct.foraCache=1;assert(!streamUrlPronta(&direct));direct.foraCache=0;direct.emCache=0;direct.debridServico=0;assert(!streamUrlPronta(&direct));
 puts("PASS: real preparation 0–5 limits, disabled gate, stale cancellation, read-only availability and explicit one-shot manual consent");SDL_Quit();return 0;
}

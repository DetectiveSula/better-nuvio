// Real addon parser -> cache API -> policy -> selection -> playable URL and UI caption.
// All provider operations are mocked, other HTTP is disabled at transport load.
#include "text.h"
static TxtLinha stageDText(TxtEstilo,const char*,int,int,int,int,float);
#define txt_linha_corta stageDText
#include "../src/streams.c"
#undef txt_linha_corta
#include <SDL2/SDL_image.h>
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>
static int captureLabels,seenLabels[4];
static TxtLinha stageDText(TxtEstilo style,const char *text,int r,int g,int b,int a,float width) {
  const char *names[]={"Unknown","Cached","Uncached","Error"};
  if(captureLabels)for(int i=0;i<4;i++)if(!strncmp(text,names[i],strlen(names[i]))&&strstr(text," · "))seenLabels[i]++;
  return txt_linha_corta(style,text,r,g,b,a,width);
}
static const char *H1="1111111111111111111111111111111111111111";
static const char *H2="2222222222222222222222222222222222222222";
static _Atomic int cacheCalls,listCalls,linkCalls;
static int mode,mutations,noItem,linkMode,replaceList,disableDuring,rotateKey;
// Used only by the cached-addition extension fixture. Ordinary Stage D stays read-only.
static int compatFixture,compatOptIn,compatCreateMode,compatReadyMode,compatListMode,compatVisible,compatSecondCached;
static int compatFreshResult,compatDisableFresh,compatDisablePost;
static _Atomic int compatCreateCalls,compatCacheCalls,compatLastApi;
static const char *compatHash;
static const char *dir;
static void configure(int enabled,int pref,int manual,int quality,int tb,int pm,int rd,int ad) {
  char path[1024];snprintf(path,sizeof path,"%s/ajustes.txt",dir);FILE*f=fopen(path,"w");assert(f);
  fprintf(f,"resolvePlayableLinksLocal %d\npreferredResolverProviderLocal %d\nescolherFonteManual %d\nqualidade %d\nfonteOrdemUsoLocal 2\ntorboxCachedAddLocal %d\n",enabled?0:1,pref,manual?0:1,quality,compatOptIn?0:1);
  fclose(f);ajustes_dir(dir);
  debrid_definir_chave_local("torbox",tb?"synthetic-tb":"");debrid_definir_chave_local("premiumize",pm?"synthetic-pm":"");
  debrid_definir_chave_local("realdebrid",rd?"synthetic-rd":"");debrid_definir_chave_local("alldebrid",ad?"synthetic-ad":"");
}
char *rede_postar_st(const char*u,int timeout,const char *const*h,const char*body,int*status) {
  (void)timeout;(void)h;*status=200;
  if(strstr(u,"torbox.app")&&strstr(u,"checkcached")) {
    if(compatFixture) {
      int check=atomic_fetch_add(&compatCacheCalls,1)+1;compatLastApi=1;
      if(check>=2 && compatDisableFresh){compatDisableFresh=0;compatOptIn=0;configure(1,1,0,0,1,0,0,0);}
      if(check>=2 && compatFreshResult==1)return strdup("{\"success\":true,\"data\":{}}");
      if(check>=2 && compatFreshResult==2){*status=503;return NULL;}
    }
    cacheCalls++;assert(strstr(body,"hashes")&&strstr(u,"format=object"));
    if(replaceList){replaceList=0;Stream replacement={0};snprintf(replacement.url,sizeof replacement.url,"https://fixture.invalid/new");stream_definir_lista(&replacement,1);}
    if(rotateKey){rotateKey=0;debrid_definir_chave_local("torbox","rotated-synthetic-key");}
    if(disableDuring){disableDuring=0;configure(0,1,0,0,1,0,0,0);}
    if(mode==1){*status=503;return strdup("{}");}if(mode==2){*status=0;return NULL;}
    if(mode==3)return strdup("{\"success\":true,\"data\":");
    if(mode==4)return strdup("{\"success\":true}");
    if(mode==5)return strdup("{\"wrapper\":{\"success\":true,\"data\":[]}}");
    if(mode==6)return strdup("{\"success\":true,\"data\":[]}");
    if(mode==7)return strdup(" \n{\"success\" : true , \"data\": {}} \n");
    if(compatFixture && compatSecondCached){char pair[300];snprintf(pair,sizeof pair,"{\"success\":true,\"data\":{\"%s\":{},\"%s\":{}}}",H1,H2);return strdup(pair);}
    char result[1024];snprintf(result,sizeof result,"{\"success\":true,\"data\":{\"%s\":{\"name\":\"provider name must not replace original\",\"size\":999}}}",compatFixture?compatHash:H1);return strdup(result);
  }
  if(compatFixture && strstr(u,"torbox.app") && strstr(u,"createtorrent")) {
    assert(ajustes_torbox_adicionar_cache()&&compatLastApi==1);
    assert(strstr(body,"name=\"add_only_if_cached\"\r\n\r\ntrue\r\n"));
    assert(strstr(body,"name=\"allow_zip\"\r\n\r\nfalse\r\n"));assert(strstr(body,compatHash));
    compatCreateCalls++;compatLastApi=3;
    if(compatDisablePost){compatDisablePost=0;compatOptIn=0;configure(1,1,0,0,1,0,0,0);}
    if(compatCreateMode==1){*status=0;return NULL;}
    if(compatCreateMode==2){*status=409;return strdup("{\"success\":false,\"error\":\"NOT_CACHED\"}");}
    if(compatCreateMode==3)return strdup("{\"success\":false,\"data\":{\"torrent_id\":99}}");
    if(compatCreateMode==4)return strdup("{\"success\":true");
    if(compatCreateMode==5)return strdup("{\"success\":true,\"data\":{\"id\":99}}");
    if(compatCreateMode==6)return strdup("{\"success\":true,\"data\":{\"queued_id\":99}}");
    return strdup("{\"success\":true,\"data\":{\"torrent_id\":99}}");
  }
  if(strstr(u,"premiumize.me")&&strstr(u,"cache/check")){cacheCalls++;return strdup("{\"status\":\"success\",\"response\":[false]}");}
  // Uncached/mutating actions must NEVER be reached automatically.
  mutations++;fprintf(stderr,"FAIL unexpected automatic provider POST\n");assert(0);return NULL;
}
char *rede_baixar_st(const char*u,int timeout,const char *const*h,int*status) {
  (void)timeout;(void)h;*status=200;
  if(strstr(u,"torbox.app")&&strstr(u,"torrents/mylist")) {
    listCalls++;
    if(compatFixture) {
      compatLastApi=2;
      if(strstr(u,"?id=99") || compatVisible) {
        char item[1800],result[1900],hashPart[100];
        snprintf(hashPart,sizeof hashPart,"\"hash\":\"%s\",",compatReadyMode==3?H2:compatHash);
        if(compatReadyMode==4)hashPart[0]=0;
        snprintf(item,sizeof item,"{\"id\":99,%s\"download_finished\":%s,\"download_state\":\"%s\",\"files\":[{\"id\":3,\"name\":\"Show.S01E01.1080p.mkv\",\"size\":2000000000}]}",hashPart,compatReadyMode==1?"false":"true",compatReadyMode==1||compatReadyMode==2?"downloading":"cached");
        snprintf(result,sizeof result,strstr(u,"?id=99")?"{\"success\":true,\"data\":%s}":"{\"success\":true,\"data\":[%s]}",item);return strdup(result);
      }
      if(compatListMode==1){*status=503;return NULL;}
      if(compatListMode==2)return strdup("{\"success\":true,\"data\":[{\"hash\":\"invalid\"}]}");
      if(compatListMode==3){char result[10000]="{\"success\":true,\"data\":[";for(int i=0;i<100;i++){size_t len=strlen(result);snprintf(result+len,sizeof result-len,"%s{\"hash\":\"%s\"}",i?",":"",H2);}strcat(result,"]}");return strdup(result);}
      return strdup("{\"success\":true,\"data\":[]}");
    }
    if(noItem)return strdup("{\"success\":true,\"data\":[]}");
    char result[1500];snprintf(result,sizeof result,"{\"success\":true,\"data\":[{\"hash\":\"%s\",\"files\":[{\"id\":3,\"name\":\"Show.S01E01.1080p.mkv\",\"size\":2000000000}],\"id\":77,\"download_finished\":true}]}",H1);return strdup(result);
  }
  if(strstr(u,"torbox.app")&&strstr(u,"requestdl")){linkCalls++;if(linkMode==1)return strdup("{\"success\":false,\"data\":\"https://fixture.invalid/must-not-play\"}");if(linkMode==2){*status=503;return NULL;}if(linkMode==3)return strdup("{\"success\":true");assert(strstr(u,compatFixture?"torrent_id=99":"torrent_id=77")&&strstr(u,"file_id=3"));return strdup("{\"success\":true,\"data\":\"https://cdn.fixture.invalid/ready.mkv\"}");}
  fprintf(stderr,"FAIL unexpected automatic GET\n");assert(0);return NULL;
}
char *rede_colocar_st(const char*u,int t,const char *const*h,const char*b,int*s){(void)u;(void)t;(void)h;(void)b;(void)s;mutations++;assert(0);return NULL;}
static int sources(Stream **out) {
  char json[3000];snprintf(json,sizeof json,"{\"streams\":["
    "{\"name\":\"[TB download] Native test 1080p\",\"description\":\"Original filename Show.S01E01.1080p.mkv | WEB-DL\",\"title\":\"Original title\",\"clientResolve\":{\"infoHash\":\"%s\",\"isCached\":false},\"fileIdx\":0},"
    "{\"name\":\"Uncached 2160p\",\"infoHash\":\"%s\"},"
    "{\"name\":\"Unknown hash 720p\",\"url\":\"https://fixture.invalid/media\",\"infoHash\":\"invalid\"},"
    "{\"name\":\"Missing hash 720p\",\"url\":\"https://fixture.invalid/missing\",\"clientResolve\":{\"type\":\"debrid\",\"service\":\"torbox\"}}]}",H1,H2);
  return stream_extrair(json,"Fixture addon",out);
}
static void expectLabel(const Stream*s,const char*expected) {
  char label[300];legendaProvedor(s,label,sizeof label);
  assert(!strncmp(label,expected,strlen(expected))&&strstr(label,"Fixture addon"));
  assert(!strcmp(stream_cache_rotulo(s),expected));
}
static void load(void){Stream*v=NULL;int count=sources(&v);assert(count==4);stream_definir_lista(v,count);free(v);}
static void drawLabels(const char *output) {
  assert(SDL_InitSubSystem(SDL_INIT_VIDEO)==0);IMG_Init(IMG_INIT_PNG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
  SDL_Window *window=SDL_CreateWindow("Better Nuvio offline Stage D",0,0,64,64,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);assert(window);
  SDL_GLContext gl=SDL_GL_CreateContext(window);assert(gl);
  GLuint fbo,texture;glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1920,1080,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
  glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
  glViewport(0,0,1920,1080);gfx_tamanho_alvo(1920,1080);assert(gfx_iniciar());
  assert(txt_iniciar("deploy/app",1));tex_iniciar(64);gfx_icones_dir("deploy/app/art");badges_carregar("deploy/app/art");
  for(int addon=0;addon<=1;addon++) {
    configure(1,1,1,0,1,0,0,0);
    char path[1024];snprintf(path,sizeof path,"%s/ajustes.txt",dir);FILE *settings=fopen(path,"a");assert(settings);fprintf(settings,"fonteTextoLocal %d\n",addon);fclose(settings);ajustes_dir(dir);debrid_definir_chave_local("torbox","synthetic-tb");
    Stream rows[4]={0};
    for(int i=0;i<4;i++) {
      snprintf(rows[i].rotulo,sizeof rows[i].rotulo,"Fixture source 1080p");snprintf(rows[i].provedor,sizeof rows[i].provedor,"Fixture addon");
      snprintf(rows[i].descricao,sizeof rows[i].descricao,"Show.S01E01.1080p.WEB-DL.mkv | Original addon information");
      snprintf(rows[i].infoHash,sizeof rows[i].infoHash,"%040d",i+1);rows[i].altura=1080;rows[i].fileIdx=0;
      rows[i].cacheConsulta=i;rows[i].cacheConsultaServico=DEBRID_TB+1;rows[i].cacheConsultaGeracao=debrid_cache_geracao();rows[i].cacheConsultaSessao=nv_perfil_sessao_ler();rows[i].cacheConsultaQuando=SDL_GetTicks();
    }
    stream_definir_lista(rows,4);stream_folha_nome("Stage D cache status");stream_folha_contexto("S01:E01 · Offline fixture");stream_folha_abrir();
    memset(seenLabels,0,sizeof seenLabels);captureLabels=1;
    for(int frame=0;frame<100;frame++) {
      stream_folha_atualizar(.016f,SDL_GetTicks());gfx_novo_quadro();gfx_cor((GfxRect){0,0,1920,1080},0,.025f,.035f,.055f,1);stream_folha_desenhar(SDL_GetTicks());
    }
    captureLabels=0;for(int i=0;i<4;i++)assert(seenLabels[i]>0);
    SDL_Surface *shot=SDL_CreateRGBSurface(0,1920,1080,24,0xff,0xff00,0xff0000,0);assert(shot);
    unsigned char *pixels=malloc(1920*1080*3);assert(pixels);glFinish();glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,1920,1080,GL_RGB,GL_UNSIGNED_BYTE,pixels);
    for(int y=0;y<1080;y++)memcpy((char*)shot->pixels+y*shot->pitch,pixels+(1080-y-1)*1920*3,1920*3);
    char filename[1400];snprintf(filename,sizeof filename,"%s-%s.png",output,addon?"addon":"native");assert(!IMG_SavePNG(shot,filename));SDL_FreeSurface(shot);free(pixels);
  }
  SDL_GL_DeleteContext(gl);SDL_DestroyWindow(window);
  puts("PASS: real 1920×1080 stream sheet renders Cached/Uncached/Unknown/Error in both text modes");
}
int main(int argc,char **argv) {
  (void)argc;(void)argv;char temp[1024];snprintf(temp,sizeof temp,"%s/nuvio-stage-d-XXXXXX",nvt_dir());assert(mkdtemp(temp));dir=temp;
  setenv("NUVIO_DADOS",dir,1);assert(SDL_Init(SDL_INIT_TIMER)==0);
  configure(0,1,0,0,1,0,0,0);load();cacheCalls=0;stream_cache_verificar();assert(!cacheCalls);expectLabel(&lista[0],"Uncached");expectLabel(&lista[3],"Unknown");
  configure(1,1,0,0,1,1,1,1);load();Stream original=lista[0];unsigned revision=stream_lista_revisao();stream_cache_verificar();assert(stream_lista_revisao()!=revision);
  Stream after=lista[0];after.cacheConsulta=0;after.cacheConsultaServico=0;after.cacheConsultaGeracao=0;after.cacheConsultaSessao=0;after.cacheConsultaQuando=0;
  assert(!memcmp(&original,&after,sizeof original));
  assert(cacheCalls==1);expectLabel(&lista[0],"Cached");expectLabel(&lista[1],"Uncached");expectLabel(&lista[2],"Unknown");expectLabel(&lista[3],"Unknown");assert(foraDoAuto(3));
  assert(!strcmp(original.rotulo,lista[0].rotulo)&&!strcmp(original.descricao,lista[0].descricao)&&!strcmp(original.arquivo,lista[0].arquivo)&&original.altura==lista[0].altura);
  assert(original.foraCache&&lista[0].foraCache);assert(!streamForaCache(&lista[0]));assert(!foraDoAuto(0)&&foraDoAuto(1));
  // Before Stage D this candidate was rejected as uncached, or selected with an empty URL.
  int winner=stream_primeira_boa_sessao(8,nv_perfil_sessao_ler());assert(winner==0);assert(!strcmp(lista[0].url,"https://cdn.fixture.invalid/ready.mkv"));assert(listCalls&&linkCalls&&!mutations);
  assert(!ajustes_fonte_manual()); // original autoplay preference remains unchanged
  configure(1,1,1,0,1,0,0,0);assert(ajustes_fonte_manual());
  for(mode=1;mode<=5;mode++){
    load();stream_cache_verificar();expectLabel(&lista[0],"Error");expectLabel(&lista[1],"Error");assert(!cacheConfirmado(&lista[0]));
  }
  for(mode=6;mode<=7;mode++){load();stream_cache_verificar();expectLabel(&lista[0],"Uncached");}mode=0;
  configure(1,2,0,0,1,1,1,1);load();stream_cache_verificar();expectLabel(&lista[0],"Uncached");assert(lista[0].cacheConsultaServico==DEBRID_PM+1);
  configure(1,1,0,0,1,1,1,1);load();stream_cache_verificar();expectLabel(&lista[0],"Cached");
  configure(1,2,0,0,1,0,1,1);load();stream_cache_verificar();expectLabel(&lista[0],"Cached");assert(ajustes_resolver_provedor()==DEBRID_TB);
  configure(1,1,0,2,1,0,0,0);Stream options[2]={original,original};
  options[0].altura=2160;options[1].altura=1080;snprintf(options[0].rotulo,sizeof options[0].rotulo,"Original 2160p");snprintf(options[1].rotulo,sizeof options[1].rotulo,"Original 1080p");
  stream_definir_lista(options,2);stream_cache_verificar();assert(stream_automatico()==1);
  snprintf(options[0].provedor,sizeof options[0].provedor,"Other addon");snprintf(options[1].provedor,sizeof options[1].provedor,"Preferred addon");
  configure(1,1,0,0,1,0,0,0);const char *order[]={"Preferred addon","Other addon"};fonteregra_ordem_definir(order,2);assert(ajustes_fonte_ordem_uso()==FR_ORDEM_ESTRITA);
  stream_definir_lista(options,2);stream_cache_verificar();assert(stream_primeira_boa_sessao(8,nv_perfil_sessao_ler())==1);fonteregra_ordem_definir(NULL,0);
  load();stream_cache_verificar();
  noItem=1;char url[512];int outside=0;assert(!resolver_politica_resolver(-1,H1,0,url,sizeof url,&outside)&&!outside&&!url[0]&&!mutations);noItem=0;expectLabel(&lista[0],"Cached");
  for(linkMode=1;linkMode<=3;linkMode++){assert(!resolver_politica_resolver(-1,H1,0,url,sizeof url,&outside)&&!outside&&!url[0]&&!mutations);expectLabel(&lista[0],"Cached");}linkMode=0;
  rotateKey=1;load();stream_cache_verificar();assert(!lista[0].cacheConsultaServico);
  replaceList=1;load();stream_cache_verificar();assert(n==1&&!lista[0].cacheConsultaServico);
  disableDuring=1;load();stream_cache_verificar();assert(!lista[0].cacheConsultaServico);
  configure(1,0,0,0,0,0,1,1);assert(!ajustes_resolver_ativo());int before=cacheCalls;load();stream_cache_verificar();assert(before==cacheCalls&&!mutations);
  puts("PASS: Stage D addon/API/actual UI captions/autoplay URL, mixed states, failures, provider preference, Off, stale list/settings and zero account mutations");if(argc>1)drawLabels(argv[1]);SDL_Quit();return 0;
}

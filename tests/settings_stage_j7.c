/* Stage J7a: two independent Better Nuvio installations (separate data dirs,
 * separate processes) sharing one file-backed mock account server keyed by
 * (owner, profile, platform). All tiers active: Stage I (32), J6 (40), J4 (93),
 * J5 Home and the J7 debug arming gate. No network; the live backend is never used. */
#include "bncodec.h"
#include "rede.h"
#include "validation_readonly.h"
#define nuvem_rpc_com fixture_regular_rpc
#define nuvem_post fixture_regular_post
#include "j4-transport.inc"
#undef nuvem_rpc_com
#undef nuvem_post

static const char *TEST_TOKEN="eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJzZXR0aW5ncy10ZXN0IiwiZXhwIjo0MTAyNDQ0ODAwfQ.test";
static const char *OTHER_TOKEN="eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJzZXR0aW5ncy1vdGhlciIsImV4cCI6NDEwMjQ0NDgwMH0.test";
static const char *server;
static int offline,pulls,betterPushes,tvPushes;

static void path(char *out,size_t cap,const char *platform,const char *owner,int profile){snprintf(out,cap,"%s/%s-%s-p%d.json",server,platform,owner,profile);}
static char *slurp(const char *p){FILE *f=fopen(p,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);char *s=calloc((size_t)n+1,1);assert(s&&fread(s,1,(size_t)n,f)==(size_t)n);fclose(f);return s;}
static void spit(const char *p,const char *s){FILE *f=fopen(p,"wb");assert(f&&fputs(s,f)>=0);fclose(f);}
static char *serverRpc(const char *fn,const char *body,const char *token,int *st){
 if(strcmp(fn,"sync_pull_profile_settings_blob")&&strcmp(fn,"sync_push_profile_settings_blob"))return fixture_regular_rpc(fn,body,token,st);
 if(offline){*st=503;return strdup("offline");}
 char platform[40]="",file[600];const char *pp=strstr(body,"\"p_platform\":\"");assert(pp);pp+=14;size_t n=strcspn(pp,"\"");assert(n<sizeof platform);memcpy(platform,pp,n);
 int profile=(int)js_num(body,NULL,"p_profile_id",-1);assert(profile>0);
 const char *owner=token&&!strcmp(token,OTHER_TOKEN)?"settings-other":"settings-test";
 path(file,sizeof file,platform,owner,profile);*st=200;
 if(!strcmp(fn,"sync_pull_profile_settings_blob")){pulls++;char *s=slurp(file);if(!s)return strdup("[]");char *r=malloc(strlen(s)+40);assert(r);sprintf(r,"[{\"settings_json\":%s}]",s);free(s);return r;}
 const char *p=strstr(body,"\"p_settings_json\":");assert(p);p=strchr(p,':')+1;const char *e=js_fim(p);assert(e);char *doc=strndup(p,(size_t)(e-p));spit(file,doc);free(doc);
 if(!strcmp(platform,"better_nuvio"))betterPushes++;else if(!strcmp(platform,"tv"))tvPushes++;else abort(); /* never mobile/home */
 *st=204;return strdup("");
}
char *nuvem_rpc_com(const char *f,const char *c,const char *token,int *st){return serverRpc(f,c,token,st);}
char *nuvem_post(const char *p,const char *c,const char *token,int *st){
 if(NV_VALIDATION_READONLY_ENABLED && !strncmp(p,"/auth/",6)) { *st=NV_VALIDATION_DENIED_STATUS;return NULL; }
 if(!strncmp(p,"/rest/v1/rpc/",13))return serverRpc(p+13,c,token,st);
 return fixture_regular_post(p,c,token,st);
}

/* App plumbing: one real frame and one real sync cycle. */
static void frame(void){j7arme_passo();jbNextApply=0;ajustes_better_passo();}
static void sync1(void){frame();cycle();frame();}
static void signIn(const char *token){
 char s[400];snprintf(s,sizeof s,"%s\nrefresh\n0\n",token);assert(dados_gravar("sessao.txt",s));
 perfis_invalidar_sessao();sessao_iniciar();assert(sessao_logada());sync_reaplicar_ajustes();frame();
}
static void boot(const char *dir){
 setenv("NUVIO_DADOS",dir,1);dados_iniciar(NULL);player_dir(dir);legfontes_dir(dir);ajustes_dir(dir);fil_definir_perfil(1);
 ajustes_better_loading();signIn(TEST_TOKEN);ajustes_better_boot();sync_reaplicar_ajustes();frame();
 const char *ids[]={"j7.fixture"},*bases[]={"https://fixture.invalid/j7-config"};fil_podar_catalogos(ids,bases,1,1);
 fil_registrar("j7.fixture_movie_top","Fixture","Fixture","movie",1);frame();
}
static void selectProfile(int profile){int old=perfis_ativo();perfis_definir_ativo(profile);sync_trocar_perfil(old);frame();}
/* The code must sit inside what the one-line island shows before cutting
 * (about 68 characters on a 1080p/4K TV; measured on BlueStacks): within the first 30. */
static char *token7(void){const char *r=j7arme_resultado(),*t=strstr(r,"codigo ");assert(t&&t-r+7+8<=30);static char tok[9];memcpy(tok,t+7,8);tok[8]=0;
 for(int i=0;i<8;i++)assert(strchr("0123456789ABCDEF",tok[i]));return tok;}
static void armJ4(void){j7arme_pedir(J7_CODIGO,NULL);char *t=token7();j7arme_pedir(J7_ARMAR,t);assert(strstr(j7arme_resultado(),"LIGADOS"));frame();assert(j7arme_permite_j4(sessao_usuario(),perfis_ativo()));}
static void armJ6(void){j7arme_pedir(J7_CODIGO_J6,NULL);char *t=token7();j7arme_pedir(J7_ARMAR_J6,t);assert(strstr(j7arme_resultado(),"LIGADOS"));frame();assert(j7arme_permite_j6(sessao_usuario(),perfis_ativo()));}
/* Both scopes, each with its own code: the earlier J7a scenarios exercise J4/J5 and J6 together. */
static void arm(void){armJ4();armJ6();}
static int serverHas(const char *platform,int profile,const char *needle){char file[600];path(file,sizeof file,platform,"settings-test",profile);char *s=slurp(file);int r=s&&strstr(s,needle);free(s);return r;}
static unsigned long long serverPrint(const char *platform,const char *owner,int profile){char file[600];path(file,sizeof file,platform,owner,profile);char *s=slurp(file);unsigned long long h=1469598103934665603ULL;for(const char *p=s;p&&*p;p++)h=(h^(unsigned char)*p)*1099511628211ULL;free(s);return s?h:0;}

/* Values used by each device step; written for the peer to compare. */
static int nextJ4(int op){int v=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].min:0;if(v==valor[op])v+=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].passo:1;return v;}
static int nextJ6(size_t i){int op=aj6Map[i].op;const Opcao *o=&OPCOES[op];int lo=o->tipo==OP_NUMERO?o->min:0,hi=o->tipo==OP_NUMERO?o->max:nValores(op)-1;
 for(int n=hi;n>=lo;n--)if(n!=valor[op]&&aj6Supported(i,n))return n;abort();}
static void writeExpect(const char *name){
 char file[600],buf[8192];size_t n=0;snprintf(file,sizeof file,"%s/%s",server,name);
 for(size_t i=0;i<JB_N;i++)n+=(size_t)snprintf(buf+n,sizeof buf-n,"%d %d\n",jbMap[i].op,valor[jbMap[i].op]);
 for(size_t i=0;i<AJ6_N;i++)n+=(size_t)snprintf(buf+n,sizeof buf-n,"%d %d\n",aj6Map[i].op,valor[aj6Map[i].op]);
 n+=(size_t)snprintf(buf+n,sizeof buf-n,"%d %d\n%d %d\n",AJ_HERO,valor[AJ_HERO],-1,fil_tipo("j7.fixture_movie_top"));spit(file,buf);
}
static int matchExpect(const char *name){
 char file[600];snprintf(file,sizeof file,"%s/%s",server,name);char *s=slurp(file);assert(s);int bad=0,count=0;
 for(char *l=s;l&&*l;){int op,v;if(sscanf(l,"%d %d",&op,&v)==2){count++;int have=op<0?fil_tipo("j7.fixture_movie_top"):valor[op];if(have!=v){bad++;fprintf(stderr,"MISMATCH %s want %d have %d\n",op<0?"home-shape":CHAVE[op],v,have);}}l=strchr(l,'\n');if(l)l++;}
 free(s);assert(count==(int)(JB_N+AJ6_N+2));return bad;
}
static void pass(const char *m){printf("PASS: %s\n",m);fflush(stdout);}

int main(int argc,char **argv){
 setvbuf(stdout,NULL,_IOLBF,0);assert(argc==3);server=getenv("J7_SERVER");assert(server);offline=getenv("J7_OFFLINE")!=NULL;
 const char *step=argv[2];
 if(!strcmp(step,"readonly-arming")) {
  char file[1000],armed[300];unsigned long long owner=1469598103934665603ULL;
  for(const char *p="settings-test";*p;p++) owner=(owner^(unsigned char)*p)*1099511628211ULL;
  snprintf(file,sizeof file,"%s/j7-arme.txt",argv[1]);
  snprintf(armed,sizeof armed,"v1 %llx - 0 0 0 1\narmed 1 %lld 1\narmed 1 %lld 2\n",owner,(long long)time(NULL)+600,(long long)time(NULL)+600);
  spit(file,armed);
 }
 boot(argv[1]);
 int ops4[]={AJ_HERO_GENERO,AJ_HERO_ANO,AJ_HERO_SINOPSE,AJ_HERO_DURACAO};
 for(size_t k=0;k<4;k++){int found=0;for(size_t i=0;i<JB_N;i++)found|=jbMap[i].op==ops4[k];assert(found);}
 if(!strcmp(step,"readonly-arming")) {
  for(int p=1;p<=2;p++) {
   assert(!j7arme_permite_j4(sessao_usuario(),p));
   assert(!j7arme_permite_j6(sessao_usuario(),p));
  }
  int actions[]={J7_CODIGO,J7_ARMAR,J7_CODIGO_J6,J7_ARMAR_J6};
  for(unsigned i=0;i<sizeof actions/sizeof actions[0];i++) {
   j7arme_pedir(actions[i],"00000000");
   assert(strstr(j7arme_resultado(),"read-only validation"));
   assert(!j7arme_permite_j4(sessao_usuario(),1));
   assert(!j7arme_permite_j6(sessao_usuario(),1));
  }
  j7arme_pedir(J7_DESARMAR,NULL);frame();
  assert(!j7arme_permite_j4(sessao_usuario(),1));
  assert(!j7arme_permite_j6(sessao_usuario(),1));
  int denied=0;
  char *blocked=rede_postar_st("https://provider.invalid/create",1,NULL,"{}",&denied);
  assert(!blocked && denied==NV_VALIDATION_DENIED_STATUS);
  blocked=rede_colocar_st("https://provider.invalid/item",1,NULL,"{}",&denied);
  assert(!blocked && denied==NV_VALIDATION_DENIED_STATUS);
  blocked=rede_apagar("https://provider.invalid/item",1,NULL,&denied);
  assert(!blocked && denied==NV_VALIDATION_DENIED_STATUS);
  blocked=rede_postar_seguro_st("https://provider.invalid/oauth",1,NULL,"{}",&denied);
  assert(!blocked && denied==NV_VALIDATION_DENIED_STATUS);
  RedePedido request={0};RedeResposta response={0};
  request.metodo="PATCH";request.url="https://provider.invalid/item";
  assert(!rede_pedir(&request,&response) && response.status==NV_VALIDATION_DENIED_STATUS);
  rede_resposta_limpar(&response);
  char *loginBefore=dados_ler("sessao.txt");assert(loginBefore);
  sessao_token_recusado(TEST_TOKEN);
  char *loginAfter=dados_ler("sessao.txt");
  assert(sessao_logada() && loginAfter && !strcmp(loginBefore,loginAfter));
  assert(!strcmp(sessao_usuario(),"settings-test"));
  free(loginBefore);free(loginAfter);
  pass(NV_VALIDATION_LOGIN_ENABLED ? "login validation refuses both sync scopes and native provider mutations; mocked failed refresh preserves stored session" : "read-only build refuses both scopes and native HTTP mutations; blocked refresh preserves stored session");
  return 0;
 }
 if(!strcmp(step,"a-default-off")){
  assert(!j7arme_permite_j4(sessao_usuario(),1));
  for(size_t i=0;i<JB_N;i++)valor[jbMap[i].op]=nextJ4(jbMap[i].op);
  for(size_t i=0;i<AJ6_N;i++)valor[aj6Map[i].op]=nextJ6(i);
  valor[AJ_HERO]=!valor[AJ_HERO];assert(gravar());assert(fil_definir_tipo("j7.fixture_movie_top",FIL_TIPO_FICHA));sync1();
  assert(betterPushes==0&&!serverHas("better_nuvio",1,"heroGeneroLocal"));
  assert(tvPushes==1&&serverHas("tv",1,"hero_section_enabled")&&!serverHas("tv",1,"mdblist_show_imdb"));
  /* Stored permission flags alone cannot upload: the J7 gate must agree. */
  assert(ajustes_better_writes(1)&&ajustes_oficiais_permitir_escritas(1));cycle();
  assert(betterPushes==0&&tvPushes==1&&!serverHas("tv",1,"mdblist_show_imdb"));frame();
  pass("default Off: 93 J4 + 40 J6 + J5 edits stay local; stored flags cannot bypass the gate; only Stage I's 32 (explicit exception) uploaded");
 }else if(!strcmp(step,"a-arm")){
  j7arme_pedir(J7_ARMAR,"00000000");assert(strstr(j7arme_resultado(),"invalido")&&!j7arme_permite_j4(sessao_usuario(),1));
  arm();sync1();assert(betterPushes==1&&tvPushes==1);
  assert(serverHas("better_nuvio",1,"heroGeneroLocal")&&serverHas("better_nuvio",1,"\"home_")&&serverHas("tv",1,"mdblist_show_imdb"));
  writeExpect("expect-a.txt");int before=betterPushes+tvPushes;sync1();sync1();assert(betterPushes+tvPushes==before);
  pass("A armed with a code for the test account: pending J4/J5/J6 uploaded once each, no loop");
 }else if(!strcmp(step,"b-restore")){
  assert(!j7arme_permite_j4(sessao_usuario(),1));sync1();sync1();
  assert(matchExpect("expect-a.txt")==0);assert(betterPushes==0&&tvPushes==0);
  pass("B fresh install restores all 93 J4, 40 J6, Stage I and J5 Home values from A; uploads nothing (no defaults)");
 }else if(!strcmp(step,"b-edit")){
  arm();sync1();
  for(size_t i=0;i<JB_N;i+=3)valor[jbMap[i].op]=nextJ4(jbMap[i].op);
  for(size_t i=0;i<AJ6_N;i+=3)valor[aj6Map[i].op]=nextJ6(i);
  assert(gravar());assert(fil_definir_tipo("j7.fixture_movie_top",FIL_TIPO_PALCO));sync1();
  assert(betterPushes==1&&tvPushes==1);writeExpect("expect-b.txt");
  pass("B armed and edited 30 J4, 14 J6 and a Home shape: uploaded");
 }else if(!strcmp(step,"a-receive")){
  assert(j7arme_permite_j4(sessao_usuario(),1));sync1();sync1();
  assert(matchExpect("expect-b.txt")==0&&betterPushes==0&&tvPushes==0);
  pass("A after restart: arming remembered, B's changes applied, no echo upload");
 }else if(!strcmp(step,"a-offline-edit")){
  assert(offline);valor[AJ_HERO_SINOPSE]=!valor[AJ_HERO_SINOPSE];int mdb=-1;for(size_t i=0;i<AJ6_N;i++)if(aj6Map[i].op==AJ_MDB_IMDB)mdb=(int)i;assert(mdb>=0);
  valor[AJ_MDB_IMDB]=!valor[AJ_MDB_IMDB];assert(gravar());sync1();assert(betterPushes==0&&tvPushes==0);
  pass("A offline: edits kept locally, nothing sent");
 }else if(!strcmp(step,"a-reconnect")){
  sync1();assert(betterPushes==1&&tvPushes==1);writeExpect("expect-a2.txt");sync1();assert(betterPushes==1&&tvPushes==1);
  pass("A after restart and reconnect: offline edits uploaded once");
 }else if(!strcmp(step,"b-follow")){
  sync1();assert(matchExpect("expect-a2.txt")==0&&betterPushes==0);
  pass("B receives A's offline edits");
 }else if(!strcmp(step,"conflict-a")||!strcmp(step,"conflict-b")){
  int a=!strcmp(step,"conflict-a");assert(offline);
  valor[AJ_HERO_GENERO]=a?0:1;valor[a?AJ_HERO_ANO:AJ_HERO_DURACAO]=!valor[a?AJ_HERO_ANO:AJ_HERO_DURACAO];
  int cw=-1;for(size_t i=0;i<AJ6_N;i++)if(aj6Map[i].op==AJ_CW_ESTILO)cw=(int)i;assert(cw>=0);valor[AJ_CW_ESTILO]=a?1:2;
  assert(gravar());sync1();assert(!betterPushes&&!tvPushes);
  char file[600],buf[64];snprintf(file,sizeof file,"%s/conflict-%s.txt",server,a?"a":"b");snprintf(buf,sizeof buf,"%d %d\n",a?AJ_HERO_ANO:AJ_HERO_DURACAO,valor[a?AJ_HERO_ANO:AJ_HERO_DURACAO]);spit(file,buf);
  pass(a?"A offline: same field (genre, CW style) set to X plus an A-only field":"B offline: same fields set to Y plus a B-only field");
 }else if(!strcmp(step,"sync")){
  sync1();sync1();writeExpect(!strcmp(argv[1]+strlen(argv[1])-1,"a")?"final-a.txt":"final-b.txt");
  printf("STATE genre=%d year=%d runtime=%d cwstyle=%d pushes=%d/%d\n",valor[AJ_HERO_GENERO],valor[AJ_HERO_ANO],valor[AJ_HERO_DURACAO],valor[AJ_CW_ESTILO],betterPushes,tvPushes);
 }else if(!strcmp(step,"settle")){
  for(int k=0;k<3;k++)sync1();assert(betterPushes==0&&tvPushes==0);writeExpect(!strcmp(argv[1]+strlen(argv[1])-1,"a")?"settled-a.txt":"settled-b.txt");
  char file[600];snprintf(file,sizeof file,"%s/conflict-a.txt",server);char *s=slurp(file);int op,v;assert(s&&sscanf(s,"%d %d",&op,&v)==2&&valor[op]==v);free(s);
  snprintf(file,sizeof file,"%s/conflict-b.txt",server);s=slurp(file);assert(s&&sscanf(s,"%d %d",&op,&v)==2&&valor[op]==v);free(s);
  pass("settled: different-field edits from both devices survive; no further uploads (no ping-pong)");
 }else if(!strcmp(step,"a-profiles")){
  unsigned long long p1=serverPrint("better_nuvio","settings-test",1);selectProfile(2);
  assert(!j7arme_permite_j4(sessao_usuario(),2));valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];assert(gravar());sync1();
  assert(betterPushes==0&&!serverPrint("better_nuvio","settings-test",2));
  arm();sync1();assert(betterPushes==1&&serverPrint("better_nuvio","settings-test",2)&&serverPrint("better_nuvio","settings-test",1)==p1);
  selectProfile(1);assert(j7arme_permite_j4(sessao_usuario(),1));
  pass("profile 2 not armed by profile 1's arming; after arming only profile 2's document changes");
 }else if(!strcmp(step,"a-account")){
  assert(j7arme_permite_j4(sessao_usuario(),1));signIn(OTHER_TOKEN);
  assert(!j7arme_permite_j4(sessao_usuario(),1));valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];assert(gravar());sync1();
  assert(betterPushes==0&&!serverPrint("better_nuvio","settings-other",1));
  j7arme_pedir(J7_CODIGO,NULL);char *t=token7();
  signIn(TEST_TOKEN);j7arme_pedir(J7_ARMAR,t);assert(strstr(j7arme_resultado(),"invalido"));
  assert(!j7arme_permite_j4(sessao_usuario(),1));
  pass("other account: uploads off at once, enrolment cleared; code from one account cannot arm another; returning needs a new code");
 }else if(!strcmp(step,"a-expiry")){
  arm();j7arme_teste_avancar(J7_JANELA_S+1);frame();assert(!j7arme_permite_j4(sessao_usuario(),1));
  valor[AJ_HERO_ANO]=!valor[AJ_HERO_ANO];assert(gravar());sync1();assert(betterPushes==0);j7arme_teste_avancar(-(J7_JANELA_S+1));
  /* Restart with a stored expiry in the past and a tampered one beyond 4 h. */
  arm();char *s=dados_ler("j7-arme.txt");assert(s);char *a=strstr(s,"armed 1 ");assert(a);long long now=(long long)time(NULL);
  char fixed[600];snprintf(fixed,sizeof fixed,"%.*sarmed 1 %lld\n",(int)(a-s),s,now-5);assert(dados_gravar("j7-arme.txt",fixed));free(s);
  pass("expiry enforced in a running app");
 }else if(!strcmp(step,"a-expired-restart")){
  assert(!j7arme_permite_j4(sessao_usuario(),1));sync1();assert(betterPushes==0);
  char *s=dados_ler("j7-arme.txt");assert(s);char *a=strstr(s,"v1 ");assert(a);free(s);
  arm();s=dados_ler("j7-arme.txt");char *b=strstr(s,"armed 1 ");assert(b);char fixed[600];snprintf(fixed,sizeof fixed,"%.*sarmed 1 %lld\n",(int)(b-s),s,(long long)time(NULL)+5*3600);assert(dados_gravar("j7-arme.txt",fixed));free(s);
  pass("expiry enforced after restart");
 }else if(!strcmp(step,"a-tampered-restart")){
  assert(!j7arme_permite_j4(sessao_usuario(),1));sync1();assert(betterPushes==0);
  pass("an expiry beyond the 4-hour window is refused after restart");
 }else if(!strcmp(step,"a-local-only")){
  arm();int dev=valor[AJ_RESOLUCAO],soc=valor[AJ_SOCIAL];valor[AJ_RESOLUCAO]=!dev;valor[AJ_SOCIAL]=!soc;assert(gravar());
  debrid_definir_chave_local("torbox","j7-secret-credential-value");sync1();
  for(int i=0;i<3;i++){const char *ns[]={"better_nuvio","tv","home_catalog_shared"};char file[600];path(file,sizeof file,ns[i],"settings-test",1);char *s=slurp(file);
   assert(!s||(!strstr(s,"j7-secret")&&!strstr(s,CHAVE[AJ_RESOLUCAO])&&!strstr(s,CHAVE[AJ_SOCIAL])));free(s);}
  j7arme_teste_foto();assert(strstr(j7arme_resultado(),"- limpo"));
  /* Inject a foreign field and a credential-like string: the snapshot must flag both. */
  char file[600];path(file,sizeof file,"better_nuvio","settings-test",1);char *s=slurp(file);char *f=strstr(s,"\"fields\":{");assert(f);
  size_t at=(size_t)(f-s)+10;char *bad=malloc(strlen(s)+200);assert(bad);
  sprintf(bad,"%.*s\"providerApiToken\":{\"type\":\"string\",\"value\":\"https://x.invalid\",\"rev\":1,\"device\":\"x\"},%s",(int)at,s,s+at);spit(file,bad);free(bad);free(s);
  j7arme_teste_foto();assert(strstr(j7arme_resultado(),"VAZAMENTO")&&!strstr(j7arme_resultado(),"fora da lista 0"));
  pass("device-local, consent and credential values never leave the device; snapshot reports clean, then flags an injected foreign field");
 }else if(!strcmp(step,"a-disarm")){
  char file[600];path(file,sizeof file,"better_nuvio","settings-test",1);char *s=slurp(file);char *f=strstr(s,"\"providerApiToken\"");if(f){char *e=strstr(f,"},");memmove(f,e+2,strlen(e+2)+1);spit(file,s);}free(s);
  arm();j7arme_pedir(J7_DESARMAR,NULL);frame();assert(!j7arme_permite_j4(sessao_usuario(),1));
  valor[AJ_HERO_DURACAO]=!valor[AJ_HERO_DURACAO];assert(gravar());sync1();assert(betterPushes==0);
  pass("disarm: further edits stay local");
 }else if(!strcmp(step,"p-first")){
  unsigned long long p1=serverPrint("better_nuvio","settings-test",1),p2=serverPrint("better_nuvio","settings-test",2);assert(p1&&p2);
  int genre=valor[AJ_HERO_GENERO];sync1();sync1();
  assert(betterPushes==0&&valor[AJ_HERO_GENERO]==genre&&serverPrint("better_nuvio","settings-test",1)==p1&&serverPrint("better_nuvio","settings-test",2)==p2);
  arm();sync1();sync1();assert(betterPushes==0&&serverPrint("better_nuvio","settings-test",1)==p1);
  pass("J0 probe on the account: first sync reads it as uninitialised, applies nothing, uploads nothing (unarmed or armed without edits); probe kept byte-identical");
 }else if(!strcmp(step,"p-restore")){
  unsigned long long p1=serverPrint("better_nuvio","settings-test",1);sync1();sync1();assert(betterPushes==0&&serverPrint("better_nuvio","settings-test",1)==p1);
  pass("second device over the probe: no upload, probe kept");
 }else if(!strcmp(step,"p-offline-edit")){
  assert(offline&&j7arme_permite_j4(sessao_usuario(),1));valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];assert(gravar());sync1();assert(betterPushes==0);
  char file[600],buf[32];snprintf(file,sizeof file,"%s/probe-genre.txt",server);snprintf(buf,sizeof buf,"%d\n",valor[AJ_HERO_GENERO]);spit(file,buf);
  pass("armed device offline over the probe: edit kept locally");
 }else if(!strcmp(step,"p-reconnect")){
  unsigned long long p2=serverPrint("better_nuvio","settings-test",2);sync1();assert(betterPushes==1);
  assert(serverHas("better_nuvio",1,"better-nuvio-profile")&&!serverHas("better_nuvio",1,"better-nuvio-probe")&&serverHas("better_nuvio",1,"heroGeneroLocal"));
  assert(serverPrint("better_nuvio","settings-test",2)==p2);sync1();assert(betterPushes==1);
  pass("after restart and reconnect the armed edit replaces profile 1's probe once; profile 2's probe untouched; no loop");
 }else if(!strcmp(step,"p-follow")){
  char file[600];snprintf(file,sizeof file,"%s/probe-genre.txt",server);char *s=slurp(file);assert(s);int want=atoi(s);free(s);
  sync1();assert(valor[AJ_HERO_GENERO]==want&&betterPushes==0);
  pass("other device receives the first real document; no echo");
 }else if(!strcmp(step,"p-profile2")){
  unsigned long long p1=serverPrint("better_nuvio","settings-test",1),p2=serverPrint("better_nuvio","settings-test",2);
  selectProfile(2);sync1();assert(betterPushes==0&&serverPrint("better_nuvio","settings-test",2)==p2);
  arm();sync1();assert(betterPushes==0&&serverPrint("better_nuvio","settings-test",2)==p2);
  valor[AJ_HERO_ANO]=!valor[AJ_HERO_ANO];assert(gravar());sync1();assert(betterPushes==1&&serverHas("better_nuvio",2,"better-nuvio-profile"));
  assert(serverPrint("better_nuvio","settings-test",1)==p1);
  pass("profile 2: probe kept until its own armed edit; replacing it leaves profile 1 unchanged");
 }else if(!strcmp(step,"u-unknown")){
  unsigned long long p1=serverPrint("better_nuvio","settings-test",1);assert(p1);arm();
  valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];assert(gravar());sync1();sync1();
  assert(betterPushes==0&&serverPrint("better_nuvio","settings-test",1)==p1);
  pass("unknown-schema document: refused even when armed with real edits; never overwritten");
 }else if(!strcmp(step,"s-j4only")){
  /* Default activation arms J4/J5 only; J6 stays off and its pending names are logged. */
  int j6op=AJ_MDB_IMDB,before=valor[j6op];armJ4();assert(!j7arme_permite_j6(sessao_usuario(),1));
  valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];valor[j6op]=!before;assert(gravar());sync1();sync1();
  assert(betterPushes==1&&tvPushes==0&&serverHas("better_nuvio",1,"heroGeneroLocal")&&!serverHas("tv",1,"mdblist_show_imdb"));
  pass("J4/J5 armed alone: the J4 edit uploads to better_nuvio; the pending J6 edit is not sent and tv is not written");
 }else if(!strcmp(step,"s-restart")){
  /* After a restart: J4/J5 still armed, J6 still off; a Stage I edit (D1) writes tv with Stage I only. */
  assert(j7arme_permite_j4(sessao_usuario(),1)&&!j7arme_permite_j6(sessao_usuario(),1));
  sync1();assert(betterPushes==0&&tvPushes==0);
  int stageI=ajSharedMap[0].op;valor[stageI]=nextJ4(stageI);assert(gravar());sync1();
  assert(tvPushes==1&&betterPushes==0&&serverHas("tv",1,ajSharedMap[0].key)&&!serverHas("tv",1,"mdblist_show_imdb")&&!serverHas("tv",1,"heroGeneroLocal"));
  sync1();assert(tvPushes==1);
  pass("restart: scopes kept independently; a Stage I edit writes only Stage I's field to tv (J6 pending and J4 keys excluded)");
 }else if(!strcmp(step,"s-codes")){
  /* A J4/J5 code cannot arm J6 and a J6 code cannot arm J4/J5. */
  j7arme_pedir(J7_CODIGO,NULL);char *t=token7();j7arme_pedir(J7_ARMAR_J6,t);assert(strstr(j7arme_resultado(),"invalido")&&!j7arme_permite_j6(sessao_usuario(),1));
  j7arme_pedir(J7_DESARMAR,NULL);frame();assert(!j7arme_permite_j4(sessao_usuario(),1));
  j7arme_pedir(J7_CODIGO_J6,NULL);t=token7();j7arme_pedir(J7_ARMAR,t);assert(strstr(j7arme_resultado(),"invalido")&&!j7arme_permite_j4(sessao_usuario(),1));
  sync1();assert(betterPushes==0&&tvPushes==0);
  armJ6();assert(!j7arme_permite_j4(sessao_usuario(),1));sync1();
  assert(tvPushes==1&&betterPushes==0&&serverHas("tv",1,"mdblist_show_imdb"));
  valor[AJ_HERO_ANO]=!valor[AJ_HERO_ANO];assert(gravar());sync1();assert(betterPushes==0);
  pass("codes are scope-bound; J6 armed alone sends the waiting J6 field to tv and never better_nuvio");
 }else if(!strcmp(step,"s-profile")){
  /* Profile 2 inherits neither scope; arming J4/J5 there leaves profile 1's J6 arming and tv alone. */
  assert(j7arme_permite_j6(sessao_usuario(),1)&&!j7arme_permite_j4(sessao_usuario(),1));
  unsigned long long tv1=serverPrint("tv","settings-test",1);selectProfile(2);
  assert(!j7arme_permite_j4(sessao_usuario(),2)&&!j7arme_permite_j6(sessao_usuario(),2));
  armJ4();valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];valor[AJ_MDB_IMDB]=!valor[AJ_MDB_IMDB];assert(gravar());sync1();
  assert(betterPushes==1&&tvPushes==0&&serverHas("better_nuvio",2,"heroGeneroLocal")&&!serverPrint("tv","settings-test",2)&&serverPrint("tv","settings-test",1)==tv1);
  selectProfile(1);assert(j7arme_permite_j6(sessao_usuario(),1)&&!j7arme_permite_j4(sessao_usuario(),1));
  j7arme_pedir(J7_DESARMAR,NULL);frame();assert(!j7arme_permite_j4(sessao_usuario(),2)&&!j7arme_permite_j6(sessao_usuario(),1));
  pass("profile switching: scopes per profile; J4/J5 on profile 2 writes only its better_nuvio; disarm clears both scopes everywhere");
 }else if(!strcmp(step,"s-legacy")){
  /* A J7a-format state file (no scope) arms J4/J5 only, never J6. */
  char line[300];snprintf(line,sizeof line,"v1 %llx - 0 0 0\narmed 1 %lld\n",(unsigned long long)0,(long long)time(NULL)+3600);
  armJ4();char *st=dados_ler("j7-arme.txt");assert(st);unsigned long long d=0;assert(sscanf(st,"v1 %llx",&d)==1);free(st);
  snprintf(line,sizeof line,"v1 %llx - 0 0 0\narmed 1 %lld\n",d,(long long)time(NULL)+3600);assert(dados_gravar("j7-arme.txt",line));
  pass("J7a-format state written");
 }else if(!strcmp(step,"s-legacy-check")){
  assert(j7arme_permite_j4(sessao_usuario(),1)&&!j7arme_permite_j6(sessao_usuario(),1));
  pass("restart with a J7a-format state file (no scope): J4/J5 only, J6 off");
 }else if(!strcmp(step,"layout-offline")){
  assert(offline);valor[AJ_HOME_LAYOUT]=HOME_LAYOUT_DINAMICA;assert(gravar());sync1();
  assert(!betterPushes&&!tvPushes);
  pass("Home layout: offline edit journaled without a backend write");
 }else if(!strncmp(step,"layout-send-",12)){
  int wanted=atoi(step+12);assert(wanted>=0&&wanted<HOME_LAYOUT_N);
  unsigned long long tv=serverPrint("tv","settings-test",1);
  assert(!j7arme_permite_j6(sessao_usuario(),1));armJ4();
  valor[AJ_HOME_LAYOUT]=wanted;assert(gravar());sync1();
  assert(ajustes_home_layout()==wanted&&betterPushes==1&&tvPushes==0);
  assert(serverHas("better_nuvio",1,"homeLayoutLocal")&&serverPrint("tv","settings-test",1)==tv);
  sync1();assert(betterPushes==1&&tvPushes==0);
  j7arme_pedir(J7_DESARMAR,NULL);frame();
  pass("Home layout: one Better-only upload, official tv unchanged, no echo");
 }else if(!strncmp(step,"layout-receive-",15)){
  int wanted=atoi(step+15);unsigned long long tv=serverPrint("tv","settings-test",1);
  assert(!j7arme_permite_j4(sessao_usuario(),1));sync1();
  assert(ajustes_home_layout()==wanted&&betterPushes==0&&tvPushes==0);
  selectProfile(2);sync1();assert(!betterPushes&&!tvPushes&&!serverPrint("better_nuvio","settings-test",2));
  selectProfile(1);sync1();assert(ajustes_home_layout()==wanted&&!betterPushes&&!tvPushes);
  assert(serverPrint("tv","settings-test",1)==tv);
  pass("Home layout: remote apply/restart/profile return, no defaults, no echo, official tv unchanged");
 }else if(!strcmp(step,"privacy")){
  /* J6 journal: hashed owner only; a legacy v1 journal (raw id) still loads,
   * keeps its pending edit and is rewritten as v2 at once. */
  char file[128];aj6Name(sessao_usuario(),1,file,sizeof file);char *j=dados_ler(file);assert(j);
  assert(!strncmp(j,"v2 ",3)&&!strstr(j,"settings-test")&&!strstr(j,"settings-other"));
  char *nl=strchr(j,'\n');assert(nl);int mdb=-1;for(size_t i=0;i<AJ6_N;i++)if(aj6Map[i].op==AJ_MDB_IMDB)mdb=(int)i;assert(mdb>=0);
  char *legacy=malloc(strlen(j)+64);assert(legacy);sprintf(legacy,"v1 %s 1%s",sessao_usuario(),nl);
  /* mark one field pending in the legacy copy */
  char key[96];snprintf(key,sizeof key,"\n%s ",CHAVE[AJ_MDB_IMDB]);char *k=strstr(legacy,key);assert(k);
  int o,b,kn,pe,de,ho;assert(sscanf(k+strlen(key),"%d %d %d %d %d %d",&o,&b,&kn,&pe,&de,&ho)==6);
  char *eol=strchr(k+1,'\n');char line[160];snprintf(line,sizeof line,"\n%s %d %d %d %d %d %d",CHAVE[AJ_MDB_IMDB],!o,b,kn,1,!o,0);
  char *fixed=malloc(strlen(legacy)+200);assert(fixed);sprintf(fixed,"%.*s%s%s",(int)(k-legacy),legacy,line,eol);
  assert(dados_gravar(file,fixed));free(fixed);free(legacy);free(j);
  aj6Forget();aj6Context();assert(aj6Ready&&aj6State[mdb].pending&&valor[AJ_MDB_IMDB]==!o);
  j=dados_ler(file);assert(j&&!strncmp(j,"v2 ",3)&&!strstr(j,"settings-test"));
  /* Never migrate a journal belonging to another owner/profile, or accept a
   * malformed header. Rejection must not alter the in-memory pending edit. */
  Aj6Field retained[AJ6_N],check[AJ6_N];memcpy(retained,aj6State,sizeof retained);
  const char *badHeaders[]={"v1 settings-other 1\n","v2 0000000000000000 1\n","v2 xyz 1\n","v2 00000000000000000 1\n","v3 0000000000000000 1\n"};
  for(size_t badi=0;badi<sizeof badHeaders/sizeof *badHeaders;badi++){
   assert(dados_gravar(file,badHeaders[badi]));memcpy(check,retained,sizeof check);
   assert(aj6Read(sessao_usuario(),1,check)==-1&&!memcmp(check,retained,sizeof check));
  }
  char badHeader[128];snprintf(badHeader,sizeof badHeader,"v2 %016llx 2\n",aj6OwnerHash(sessao_usuario()));
  assert(dados_gravar(file,badHeader)&&aj6Read(sessao_usuario(),1,check)==-1);
  snprintf(badHeader,sizeof badHeader,"v2 %016llx 1 trailing\n",aj6OwnerHash(sessao_usuario()));
  assert(dados_gravar(file,badHeader)&&aj6Read(sessao_usuario(),1,check)==-1);
  assert(dados_gravar(file,j));free(j);
  /* Home-rows editor message follows the real upload state. */
  assert(!ajustes_better_envia());armJ6();assert(!ajustes_better_envia());
  assert(ajustes_better_op_vinculada(AJ_HOME_LAYOUT));
  armJ4();assert(ajustes_better_envia());j7arme_pedir(J7_DESARMAR,NULL);frame();assert(!ajustes_better_envia());
  pass("privacy: J6 journal holds only a hashed owner; a legacy v1 journal keeps its pending edit and is rewritten as v2; editor message tracks J4/J5 uploads");
 }else abort();
 return 0;
}

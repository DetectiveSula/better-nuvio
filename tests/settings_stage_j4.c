#include "bncodec.h"
#include <sys/stat.h>
static int j4Cw;
#define desc_refazer_continuar j4_cw_refresh
#define nuvem_rpc_com fixture_regular_rpc
#include "j4-transport.inc"
#undef nuvem_rpc_com
#undef desc_refazer_continuar
void j4_cw_refresh(void){j4Cw++;}
static char *betterCloud;
static int betterPushes,betterPulls,j4Offline,j4Pause,j4Paused,j4Release;
static pthread_mutex_t j4Mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t j4Cond=PTHREAD_COND_INITIALIZER;
static char otherToken[3000];
static void serverName(char *name,size_t cap,const char *owner,int profile){snprintf(name,cap,"mock-j4-%s-p%d.json",owner,profile);}
static void serverSave(void){char name[160];serverName(name,sizeof name,sessao_usuario(),perfis_ativo());assert(dados_gravar(name,betterCloud));}
static void selectProfile(int profile){int old=perfis_ativo();perfis_definir_ativo(profile);sync_trocar_perfil(old);jbNextApply=0;ajustes_better_passo();}
static void *worker(void *unused){(void)unused;ajustes_better_ciclo(nv_perfil_sessao_ler());return NULL;}
static void blockCheckpoint(const char *dir,const char *name,int block){char path[900],backup[920];snprintf(path,sizeof path,"%s/%s",dir,name);snprintf(backup,sizeof backup,"%s.saved",path);if(block){assert(!rename(path,backup));assert(!mkdir(path,0700));}else{assert(!rmdir(path));assert(!rename(backup,path));}}
char *nuvem_rpc_com(const char *f,const char *c,const char *token,int *st){
 if(!strstr(c,"\"p_platform\":\"better_nuvio\""))return fixture_regular_rpc(f,c,token,st);
 *st=200;
 int profile=(int)js_num(c,NULL,"p_profile_id",-1);assert(profile>0);
 const char *owner=otherToken[0]&&!strcmp(token,otherToken)?"settings-other":"settings-test";
 char name[160];serverName(name,sizeof name,owner,profile);
 if(!strcmp(f,"sync_pull_profile_settings_blob")){
  betterPulls++;if(j4Offline){*st=503;return strdup("offline");}
  char *stored=dados_ler(name);char *r=stored?malloc(strlen(stored)+40):strdup("[]");assert(r);
  if(stored){sprintf(r,"[{\"settings_json\":%s}]",stored);free(stored);}
  pthread_mutex_lock(&j4Mutex);if(j4Pause){j4Paused=1;pthread_cond_signal(&j4Cond);while(!j4Release)pthread_cond_wait(&j4Cond,&j4Mutex);j4Pause=0;}pthread_mutex_unlock(&j4Mutex);
  return r;
 }
 assert(!strcmp(f,"sync_push_profile_settings_blob"));betterPushes++;const char *p=strstr(c,"\"p_settings_json\":");assert(p);p=strchr(p,':')+1;const char *end=js_fim(p);free(betterCloud);betterCloud=strndup(p,(size_t)(end-p));assert(dados_gravar(name,betterCloud));return strdup("null");
}
int main(int argc,char **argv){
 setvbuf(stdout,NULL,_IOLBF,0);
 assert(argc==2||argc==3);setenv("NUVIO_DADOS",argv[1],1);dados_iniciar(NULL);player_dir(argv[1]);legfontes_dir(argv[1]);ajustes_dir(argv[1]);fil_definir_perfil(1);
 if(argc==3){
  ajustes_perfil_padrao();ajustes_perfil_restaurar(1);login();jbNextApply=0;ajustes_better_passo();
  assert(valor[AJ_HERO_GENERO]==atoi(argv[2])&&!bna_writes_enabled(jbCurrent->account));j4Offline=1;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(!betterPushes);ajustes_better_esquecer();puts("PASS: fresh process recovers offline Native edits even after both checkpoint/profile fallback writes failed");return 0;
 }
 ajustes_better_loading();login();assert(!jbCurrent);ajustes_better_boot();sync_reaplicar_ajustes();
 BnDef defs[JB_N];if(!jbDefs(valor,1,defs)){
  for(size_t i=0;i<JB_N;i++){BnValue v;if(!jbEncode(jbMap[i].op,valor[jbMap[i].op],1,&v))fprintf(stderr,"BAD BINDING %s value %d kind %d n %d\n",jbMap[i].key,valor[jbMap[i].op],OPCOES[jbMap[i].op].tipo,OPCOES[jbMap[i].op].n);}
  abort();
 }
 assert(jbCurrent&&JB_N==214);assert(!bna_writes_enabled(jbCurrent->account));
 /* Existing 90-binding checkpoints grow without resetting old edits or
  * turning the new Home layout baseline into an upload. */
 BnEngine *legacy=calloc(1,sizeof *legacy),*expanded=calloc(1,sizeof *expanded);
 BnCheckpoint *prior=calloc(1,sizeof *prior);assert(legacy&&expanded&&prior);
 assert(bn_init(legacy,sessao_usuario(),perfis_ativo(),dados_cliente_id(),defs,90));
 BnValue edit=defs[0].initial;edit.number=edit.number==defs[0].max?defs[0].min:edit.number+1;
 assert(bn_edit(legacy,defs[0].key,edit)&&bn_checkpoint(legacy,prior));
 assert(bn_init(expanded,sessao_usuario(),perfis_ativo(),dados_cliente_id(),defs,JB_N)&&bn_restore(expanded,prior));
 assert(expanded->state.count==JB_N&&!memcmp(expanded->state.slots,prior->slots,prior->count*sizeof *prior->slots));
 assert(!strcmp(expanded->state.slots[90].current.key,"homeLayoutLocal")&&!expanded->state.slots[90].known&&!expanded->state.slots[90].dirty);
 for(size_t k=90;k<JB_N;k++)assert(!expanded->state.slots[k].known&&!expanded->state.slots[k].dirty);
 free(legacy);free(expanded);free(prior);
 puts("PASS: 90-binding checkpoint migration retains existing pending edits; Home layout starts unobserved without a default upload");
 for(size_t i=0;i<JB_N;i++){
  /* Full sync (12 Nov 2026): device-wide bindings sit from JB_PERFIL_FIM on, and
   * store-backed ones use the stored key without its "-". */
  int op=jbMap[i].op;assert(!strcmp(CHAVE[op][0]=='-'?CHAVE[op]+1:CHAVE[op],jbMap[i].key)&&(dePerfil(op)||i>=JB_PERFIL_FIM));
  printf("J4_BINDING %s %d %lld %lld %d\n",jbMap[i].key,(int)defs[i].type,(long long)defs[i].min,(long long)defs[i].max,OPCOES[op].tipo==OP_NUMERO?OPCOES[op].passo:0);assert(OPCOES[op].tipo==OP_NUMERO||OPCOES[op].tipo==OP_ESCOLHA);
  int n=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].min:0;
  int max=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].max:nValores(op)-1;
  if(op==AJ_LEG2_FONTE)max=ajFonteBase(op)-1;
  if(op==AJ_LEG_LINGUA2)max=ling_opcao_n()-1;
  if(op==AJ_TRAILER_AUDIO)max=nTrAud-1;
  printf("J4_BINDING %s %d %lld %lld %d %d %d\n",jbMap[i].key,(int)defs[i].type,(long long)defs[i].min,(long long)defs[i].max,OPCOES[op].tipo==OP_NUMERO?OPCOES[op].passo:0,n,max);
  int step=1;
  for(;n<=max;n+=step){BnValue v;int decoded=-999;assert(jbEncode(op,n,0,&v));assert(jbDecode(op,v,&decoded)&&decoded==n);}
 }
 puts("PASS: all 214 IDs/keys, profile scopes, integer ranges (including values between UI steps) and every local dropdown value round trip");
 int original[AJ_N];memcpy(original,valor,sizeof original);
 assert(ajustes_better_writes(1));ajustes_better_ciclo(nv_perfil_sessao_ler());assert(!betterPushes); // no defaults
 for(size_t i=0;i<JB_N;i++){
  int op=jbMap[i].op;int changed=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].min:0;
  if(changed==valor[op])changed+=OPCOES[op].tipo==OP_NUMERO?OPCOES[op].passo:1;
  assert(changed!=valor[op]);valor[op]=changed;
 }
 assert(gravar());ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==1&&betterCloud);
 BnDoc sent;char *envelope=malloc(strlen(betterCloud)+40);assert(envelope);sprintf(envelope,"[{\"settings_json\":%s}]",betterCloud);
 BnEngine *e=calloc(1,sizeof *e);assert(e&&bn_init(e,sessao_usuario(),perfis_ativo(),dados_cliente_id(),defs,JB_N));assert(bnc_response(e,envelope,&sent));free(envelope);
 /* Store-backed bindings (stage 2a) read their store, not a direct value[] write:
  * settings_sync_auto covers them through their real setters. */
 {size_t esperados=0;for(size_t i=0;i<JB_N;i++){if(jbLoja(jbMap[i].op))continue;esperados++;int found=0;for(size_t j=0;j<sent.count;j++)found|=!strcmp(sent.fields[j].key,jbMap[i].key);assert(found);}
  assert(sent.count>=esperados);}
 for(int k=0;k<3;k++)ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==1&&!pushes);
 puts("PASS: every binding captured through real Native save, original-format Better payload, no defaults or loops and no Stage I writes");
 for(size_t i=0;i<sent.count;i++){
  int op=-1;for(size_t j=0;j<JB_N;j++)if(!strcmp(jbMap[j].key,sent.fields[i].key))op=jbMap[j].op;assert(op>=0);
  assert(jbEncode(op,op==AJ_LEG2_ATRASO?-321:original[op],0,&sent.fields[i].value));sent.fields[i].rev+=100;strcpy(sent.fields[i].device,"peer");
 }
 free(betterCloud);betterCloud=NULL;assert(bn_json(e,&sent,&betterCloud));serverSave();ajustes_better_ciclo(nv_perfil_sessao_ler());assert(ajustes_better_passo());
 for(size_t i=0;i<JB_N;i++)assert(valor[jbMap[i].op]==(jbMap[i].op==AJ_LEG2_ATRASO?-321:original[jbMap[i].op]));
 int before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==before&&!pushes&&j4Cw>0);
 for(int op=0;op<AJ_N;op++){int mapped=0;for(size_t i=0;i<JB_N;i++)mapped|=jbMap[i].op==op;if(!mapped)assert(valor[op]==original[op]);}
 puts("PASS: every remote binding restores Native values and effects, with no echo and no unlisted control changes");
 assert(ajustes_leg2_definir_atraso(137));assert(ajustes_leg2_atraso()==137);ajustes_better_ciclo(nv_perfil_sessao_ler());
 BnValue invalid;for(size_t i=0;i<JB_N;i++){
  int op=jbMap[i].op,n;invalid=defs[i].initial;
  if(invalid.type==BN_STRING)snprintf(invalid.text,sizeof invalid.text,"invalid://not-a-setting");else invalid.number=defs[i].max+1;
  assert(!jbDecode(op,invalid,&n));assert(!bn_edit(e,jbMap[i].key,invalid));
 }
 BnValue unavailable={.type=BN_STRING};snprintf(unavailable.text,sizeof unavailable.text,"font:0123456789abcdef.ttf");
 for(size_t i=0;i<sent.count;i++)if(!strcmp(sent.fields[i].key,CHAVE[AJ_LEG2_FONTE])){sent.fields[i].value=unavailable;sent.fields[i].rev+=10;}
 free(betterCloud);betterCloud=NULL;assert(bn_json(e,&sent,&betterCloud));serverSave();ajustes_better_ciclo(nv_perfil_sessao_ler());jbNextApply=0;ajustes_better_passo();
 assert(!strcmp(legfontes_escolha_id(LEGFONTE_SECUNDARIA),"0123456789abcdef.ttf")&&legfontes_selecionada(LEGFONTE_SECUNDARIA)==-1);
 before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(before==betterPushes);
 puts("PASS: all 214 reject invalid conversions; missing custom font IDs remain preferences with bundled fallback, no import or echo");
 assert(ajustes_better_writes(0));int wanted=!valor[AJ_HERO_GENERO];
 blockCheckpoint(argv[1],"bn-checkpoint-v1-settings-test-p1.json",1);blockCheckpoint(argv[1],"ajustes-p1.txt",1);
 valor[AJ_HERO_GENERO]=wanted;assert(gravar());jbNextApply=0;ajustes_better_passo();assert(valor[AJ_HERO_GENERO]==wanted);
 blockCheckpoint(argv[1],"bn-checkpoint-v1-settings-test-p1.json",0);blockCheckpoint(argv[1],"ajustes-p1.txt",0);
 ajustes_better_esquecer();pid_t child=fork();assert(child>=0);if(!child){char value[16];snprintf(value,sizeof value,"%d",wanted);execl(argv[0],argv[0],argv[1],value,(char *)NULL);_exit(127);}int status;assert(waitpid(child,&status,0)==child&&WIFEXITED(status)&&!WEXITSTATUS(status));
 ajustes_dir(argv[1]);sync_reaplicar_ajustes();jbNextApply=0;ajustes_better_passo();assert(valor[AJ_HERO_GENERO]==wanted);
 before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==before);assert(ajustes_better_writes(1));ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==before+1);
 // An inactive editor persists only its own changes, including font selection.
 assert(ajustes_edicao_iniciar(2));valor[AJ_HERO_ANO]=!valor[AJ_HERO_ANO];int targetYear=valor[AJ_HERO_ANO];assert(gravar());ajustes_better_editor_observar();ajustes_edicao_encerrar();assert(valor[AJ_HERO_ANO]==original[AJ_HERO_ANO]);
 // Retire a context while its network response is still pending.
 j4Pause=1;pthread_t thread;assert(!pthread_create(&thread,NULL,worker,NULL));pthread_mutex_lock(&j4Mutex);while(!j4Paused)pthread_cond_wait(&j4Cond,&j4Mutex);pthread_mutex_unlock(&j4Mutex);
 selectProfile(2);assert(valor[AJ_HERO_ANO]==targetYear&&!bna_writes_enabled(jbCurrent->account));
 pthread_mutex_lock(&j4Mutex);j4Release=1;pthread_cond_signal(&j4Cond);pthread_mutex_unlock(&j4Mutex);assert(!pthread_join(thread,NULL));
 assert(valor[AJ_HERO_ANO]==targetYear);before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==before);
 selectProfile(1);assert(valor[AJ_HERO_GENERO]==wanted&&valor[AJ_HERO_ANO]==original[AJ_HERO_ANO]);
 puts("PASS: inactive-profile journal, profile switching, per-profile permission and retirement of an in-flight context");
 // Switching accounts with the same numeric profile must not interpret the
 // previous account's global Native file as a new edit in this account.
 const char *otherSession="eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJzZXR0aW5ncy1vdGhlciIsImV4cCI6NDEwMjQ0NDgwMH0.test\nrefresh\n0\n";
 assert(dados_gravar("sessao.txt",otherSession));perfis_invalidar_sessao();sessao_iniciar();assert(!strcmp(sessao_usuario(),"settings-other"));assert(sessao_token_copiar(otherToken,sizeof otherToken));
 sync_reaplicar_ajustes();jbNextApply=0;ajustes_better_passo();assert(!bna_writes_enabled(jbCurrent->account));
 before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(before==betterPushes);
 BnDef *otherDefs=calloc(JB_N,sizeof *otherDefs);BnEngine *other=calloc(1,sizeof *other);assert(otherDefs&&other&&jbDefs(valor,1,otherDefs)&&bn_init(other,sessao_usuario(),1,dados_cliente_id(),otherDefs,JB_N));
 BnDoc otherDoc={0};strcpy(otherDoc.schema,BN_SCHEMA);otherDoc.version=1;strcpy(otherDoc.owner,sessao_usuario());otherDoc.profile=1;otherDoc.count=1;
 strcpy(otherDoc.fields[0].key,CHAVE[AJ_HERO_GENERO]);otherDoc.fields[0].value=(BnValue){.type=BN_BOOL,.number=wanted};otherDoc.fields[0].rev=1;strcpy(otherDoc.fields[0].device,"other-peer");
 free(betterCloud);betterCloud=NULL;assert(bn_json(other,&otherDoc,&betterCloud));serverSave();ajustes_better_ciclo(nv_perfil_sessao_ler());jbNextApply=0;ajustes_better_passo();assert(valor[AJ_HERO_GENERO]==!wanted);
 free(otherDefs);free(other);perfis_invalidar_sessao();login();jbNextApply=0;ajustes_better_passo();assert(!strcmp(sessao_usuario(),"settings-test")&&valor[AJ_HERO_GENERO]==wanted);
 before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(betterPushes==before);
 puts("PASS: separate accounts sharing profile index 1 restore only their own state, permissions and pending edits");
 selectProfile(3);ajustes_better_esquecer();
 dados_apagar("bn-checkpoint-v1-settings-test-p3.json");dados_apagar("bn-native-v1-settings-test-p3.json");
 valor[AJ_HERO_DURACAO]=1;assert(gravar());assert(perfilGravarVetor(3,valor));sync_reaplicar_ajustes();jbNextApply=0;ajustes_better_passo();
 assert(jbCurrent&&valor[AJ_HERO_DURACAO]==1);assert(ajustes_better_writes(1));before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(before==betterPushes);
 assert(dados_gravar("sessao.txt",otherSession));perfis_invalidar_sessao();sessao_iniciar();sync_reaplicar_ajustes();
 otherDefs=calloc(JB_N,sizeof *otherDefs);other=calloc(1,sizeof *other);assert(otherDefs&&other&&jbDefs(valor,1,otherDefs)&&bn_init(other,sessao_usuario(),3,dados_cliente_id(),otherDefs,JB_N));
 memset(&otherDoc,0,sizeof otherDoc);strcpy(otherDoc.schema,BN_SCHEMA);otherDoc.version=1;strcpy(otherDoc.owner,sessao_usuario());otherDoc.profile=3;otherDoc.count=1;
 strcpy(otherDoc.fields[0].key,CHAVE[AJ_HERO_DURACAO]);otherDoc.fields[0].value=(BnValue){.type=BN_BOOL,.number=1};otherDoc.fields[0].rev=1;strcpy(otherDoc.fields[0].device,"other-peer");
 free(betterCloud);betterCloud=NULL;assert(bn_json(other,&otherDoc,&betterCloud));serverSave();ajustes_better_ciclo(nv_perfil_sessao_ler());jbNextApply=0;ajustes_better_passo();assert(valor[AJ_HERO_DURACAO]==0);free(otherDefs);free(other);
 perfis_invalidar_sessao();login();jbNextApply=0;ajustes_better_passo();assert(valor[AJ_HERO_DURACAO]==1);before=betterPushes;ajustes_better_ciclo(nv_perfil_sessao_ler());assert(before==betterPushes);
 puts("PASS: unobserved owner-local preferences restore across account switches without becoming default uploads");


 free(e);free(betterCloud);free(cloud);free(lastWrite);ajustes_better_esquecer();return 0;
}

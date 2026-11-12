#include "bnaccount.h"
#include "bncodec.h"
#include "js.h"
#include "sessao.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
static char root[512],owner[80]="owner1",device[65]="deviceA";
static int profile=1,side,logged=1,posts,pulls,refreshes,deny401,failPull,timeoutPush,diskFail,backoff,hook;
static uint64_t generation=1;static pthread_mutex_t scopeLock=PTHREAD_MUTEX_INITIALIZER;
static BnAccount *active;static char *lastWrite;
static BnDef defs[3];
int sessao_logada(void){return logged;}
const char *sessao_usuario(void){return owner;}
int perfis_ativo(void){return profile;}
uint64_t perfis_sessao_geracao(void){return generation;}
int perfis_sessao_atual(uint64_t g){return generation==g;}
int perfis_sessao_travar(uint64_t g){pthread_mutex_lock(&scopeLock);if(g!=generation){pthread_mutex_unlock(&scopeLock);return 0;}return 1;}
void perfis_sessao_destravar(void){pthread_mutex_unlock(&scopeLock);}
const char *dados_cliente_id(void){return device;}
int nuvem_freio_ativo(void){return backoff;}
void nuvem_falhou(void){backoff=1;}
void nuvem_ok(void){backoff=0;}
int sessao_token_copiar(char *out,size_t cap){snprintf(out,cap,"mock-bearer-%s-%d",owner,refreshes);return logged;}
int sessao_identidade_local_copiar(char *out,size_t cap,char *token,size_t tc){snprintf(out,cap,"%s",owner);return sessao_token_copiar(token,tc);}
void sessao_token_recusado(const char *token){assert(strstr(token,"mock-bearer"));refreshes++;}
static char *readfile(const char *path){FILE *f=fopen(path,"rb");if(!f)return NULL;assert(!fseek(f,0,SEEK_END));long n=ftell(f);rewind(f);char *s=calloc((size_t)n+1,1);assert(s);assert(fread(s,1,(size_t)n,f)==(size_t)n);fclose(f);return s;}
static int writefile(const char *path,const char *s){char tmp[900];snprintf(tmp,sizeof tmp,"%s.tmp",path);FILE *f=fopen(tmp,"wb");assert(f);fputs(s,f);assert(!fclose(f));assert(!rename(tmp,path));return 1;}
static void localpath(char *path,size_t cap,const char *name){snprintf(path,cap,"%s/%c/%s",root,side?'B':'A',name);}
char *dados_ler(const char *name){char path[800];assert(!strchr(name,'/'));localpath(path,sizeof path,name);return readfile(path);}
int dados_gravar(const char *name,const char *s){if(diskFail)return 0;char path[800];assert(!strchr(name,'/'));localpath(path,sizeof path,name);return writefile(path,s);}
static void serverpath(char *path,size_t cap){snprintf(path,cap,"%s/server-%s-%d.json",root,owner,profile);}
static BnValue val(int n){BnValue v={0};v.type=BN_INT;v.number=n;return v;}
char *nuvem_rpc_com(const char *rpc,const char *body,const char *bearer,int *st){
 char expected[200];snprintf(expected,sizeof expected,"mock-bearer-%s-%d",owner,refreshes);assert(!strcmp(bearer,expected));
 assert(strstr(body,"\"p_platform\":\"better_nuvio\""));assert(!strstr(body,"home_catalog_shared")&&!strstr(body,"\"tv\"")&&!strstr(body,"\"mobile\""));
 assert((int)js_num(body,NULL,"p_profile_id",-1)==profile);*st=200;
 if(deny401){deny401=0;*st=401;return strdup("{}");}
 char path[800];serverpath(path,sizeof path);
 if(!strcmp(rpc,"sync_pull_profile_settings_blob")){
  pulls++;if(failPull){*st=503;return strdup("offline");}
  char *raw=readfile(path),*response;
  if(raw){response=malloc(strlen(raw)+80);assert(response);sprintf(response,"[{\"profile_id\":%d,\"settings_json\":%s}]",profile,raw);free(raw);}else response=strdup("[]");
  if(hook==1){hook=0;strcpy(owner,"owner2");generation++;}
  else if(hook==2){hook=0;profile=2;generation++;}
  else if(hook==3){hook=0;assert(bna_writes(active,0));}
  return response;
 }
 assert(!strcmp(rpc,"sync_push_profile_settings_blob"));posts++;free(lastWrite);lastWrite=strdup(body);
 const char *p=strstr(body,"\"p_settings_json\":");assert(p);p=strchr(p,':')+1;const char *end=js_fim(p);assert(end);char *raw=strndup(p,(size_t)(end-p));
 writefile(path,raw);free(raw);
 if(hook==4){hook=0;assert(bna_edit(active,"testCount",val(10)));assert(!bna_close(active));}
 if(hook==6){hook=0;diskFail=1;}
 if(timeoutPush){timeoutPush=0;*st=503;return strdup("timeout after commit");}
 *st=204;return strdup("");
}
static int number(BnAccount *a,const char *key){BnValue v;assert(bna_get(a,key,&v));return (int)v.number;}
static void schema(void){
 const char *keys[]={"testCount","testMode","testText"};for(int i=0;i<3;i++){strcpy(defs[i].key,keys[i]);defs[i].type=i==2?BN_STRING:i==1?BN_BOOL:BN_INT;defs[i].portable=1;defs[i].max=i==1?1:99;defs[i].initial.type=defs[i].type;}
}
static void child(const char *program,const char *mode){pid_t p=fork();assert(p>=0);if(!p){execl(program,program,root,mode,(char *)NULL);_exit(127);}int st;assert(waitpid(p,&st,0)==p&&WIFEXITED(st)&&!WEXITSTATUS(st));}
static void codec_tests(void){
 BnEngine *e=malloc(sizeof *e);assert(e&&bn_init(e,"owner1",1,"deviceA",defs,3));BnDoc d;int w=0;
 const char *bad[]={"[", "[]junk", "[{\"settings_json\":{},\"settings_json\":null}]", "[{\"settings_json\":{},\"nested\":{\"a\":1,\"a\":2}}]", "[{\"settings_json\":{\"schema\":\"better-nuvio-profile\",\"version\":2,\"fields\":{}}}]", "[{\"profile_id\":2,\"settings_json\":null}]", "[{\"settings_json\":null},{}]", "[{\"settings_json\":{\"schema\":\"better-nuvio-profile\",\"version\":1,\"fields\":{},\"home\":{}}}]"};
 for(size_t i=0;i<sizeof bad/sizeof *bad;i++)assert(!bnc_response(e,bad[i],&d));
 const char *unicode="[{\"settings_json\":{\"schema\":\"better-nuvio-profile\",\"version\":1,\"fields\":{\"testText\":{\"type\":\"string\",\"value\":\"\\u00e9\\ud83d\\ude03\",\"rev\":1,\"device\":\"peer\"}}}}]";
 assert(bnc_response(e,unicode,&d)&&bn_pull(e,&d));assert(!strcmp(bn_get(e,"testText")->text,"é😃"));
 const char *alias="[{\"settings_json\":{\"schema\":\"better-nuvio-profile\",\"version\":1,\"fields\":{},\"f\\u0069elds\":{}}}]";
 assert(!bnc_response(e,alias,&d));
 char *huge=malloc(BN_LIMIT+4098);assert(huge);memset(huge,' ',BN_LIMIT+4097);huge[BN_LIMIT+4097]=0;assert(!bnc_response(e,huge,&d));free(huge);
 char *text=NULL;assert(bnc_save(e,0,&text));assert(bnc_load(e,text,&w)&&!w);
 BnDef expanded[4];memcpy(expanded,defs,sizeof defs);expanded[3]=defs[0];strcpy(expanded[3].key,"newPortable");expanded[3].initial.number=33;
 BnEngine *grown=calloc(1,sizeof *grown);assert(grown&&bn_init(grown,"owner1",1,"deviceA",expanded,4));
 assert(bnc_load(grown,text,&w)&&bn_get(grown,"newPortable")->number==33);
 for(size_t i=0;i<grown->state.count;i++)if(!strcmp(grown->state.slots[i].current.key,"newPortable"))assert(!grown->state.slots[i].known&&!grown->state.slots[i].dirty);
 free(grown);free(text);
 assert(!bnc_load(e,"{}",&w));free(e);puts("PASS: strict RPC/checkpoint codec, duplicate/unknown/future/truncated/mismatched envelopes rejected");
}
int main(int argc,char **argv){
 assert(argc==2||argc==3);snprintf(root,sizeof root,"%s",argv[1]);schema();
 if(argc==3){
  if(!strncmp(argv[2],"recover",7)){int expected=atoi(argv[2]+7);active=bna_open(defs,3);assert(active&&number(active,"testCount")==expected);assert(bna_cycle(active)&&posts==0);}
  else if(!strcmp(argv[2],"peer")){side=1;strcpy(device,"deviceB");active=bna_open(defs,3);assert(active&&!bna_writes_enabled(active));assert(bna_cycle(active)&&number(active,"testCount")==8);assert(bna_writes(active,1));BnValue v={.type=BN_BOOL,.number=1};assert(bna_edit(active,"testMode",v)&&bna_cycle(active));}
  else {active=bna_open(defs,3);assert(active&&bna_writes_enabled(active)&&number(active,"testCount")==8);assert(bna_cycle(active)&&posts==1);assert(bna_cycle(active)&&posts==1);}
  assert(bna_close(active));free(lastWrite);return 0;
 }
 char path[800];snprintf(path,sizeof path,"%s/A",root);assert(!mkdir(path,0700));snprintf(path,sizeof path,"%s/B",root);assert(!mkdir(path,0700));
 for(int i=0;i<3;i++){snprintf(path,sizeof path,"%s/regular-%d",root,i);writefile(path,"regular-document-sentinel");}
 codec_tests();serverpath(path,sizeof path);
 writefile(path,"{\"schema\":\"better-nuvio-profile\",\"version\":1,\"fields\":{\"testCount\":{\"type\":\"int\",\"value\":7,\"rev\":5,\"device\":\"peer\"}}}");
 active=bna_open(defs,3);assert(active&&!bna_writes_enabled(active));assert(bna_cycle(active)&&number(active,"testCount")==7&&!posts);
 assert(bna_writes(active,1)&&bna_cycle(active)&&!posts);assert(bna_edit(active,"testCount",val(8)));
 failPull=1;assert(!bna_cycle(active)&&!posts&&number(active,"testCount")==8);int n=pulls;assert(!bna_cycle(active)&&pulls==n);failPull=backoff=0;
 assert(bna_close(active));active=NULL;child(argv[0],"restart");
 active=bna_open(defs,3);assert(active&&number(active,"testCount")==8&&bna_writes_enabled(active));child(argv[0],"peer");assert(bna_cycle(active)&&number(active,"testMode")==1&&!posts);
 puts("PASS: authenticated namespace-only calls, default-Off, first-login restoration, offline disk checkpoint, fresh-process restart and two-device backend");
 deny401=1;assert(bna_cycle(active)&&refreshes==1);
 assert(bna_edit(active,"testCount",val(9)));timeoutPush=1;assert(!bna_cycle(active)&&posts==1);backoff=0;assert(bna_cycle(active)&&posts==1&&number(active,"testCount")==9);
 assert(bna_edit(active,"testCount",val(11)));hook=4;assert(bna_cycle(active)&&number(active,"testCount")==10);assert(bna_cycle(active)&&number(active,"testCount")==10);
 hook=3;assert(bna_edit(active,"testCount",val(12)));int before=posts;assert(bna_cycle(active)&&posts==before&&!bna_writes_enabled(active));
 assert(bna_writes(active,1)&&bna_cycle(active)&&posts==before+1);
 assert(lastWrite&&!strstr(lastWrite,"mock-bearer")&&!strstr(lastWrite,"owner1")&&!strstr(lastWrite,"checkpoint"));
 diskFail=1;assert(!bna_edit(active,"testCount",val(13))&&number(active,"testCount")==12);assert(!bna_writes(active,0)&&bna_writes_enabled(active));diskFail=0;
 BnValue tv={.type=BN_STRING};snprintf(tv.text,sizeof tv.text,"Café \"quoted\" \\ 😃");assert(bna_edit(active,"testText",tv)&&bna_cycle(active));
 assert(bna_close(active));active=bna_open(defs,3);BnValue roundtrip;assert(active&&bna_get(active,"testText",&roundtrip)&&!strcmp(roundtrip.text,tv.text));
 BnEngine *mock=calloc(1,sizeof *mock);BnDoc *remote=calloc(1,sizeof *remote);assert(mock&&remote&&bn_init(mock,"owner1",1,"deviceA",defs,3));
 char *raw=readfile(path);assert(raw);char *envelope=malloc(strlen(raw)+40);assert(envelope);sprintf(envelope,"[{\"settings_json\":%s}]",raw);free(raw);
 assert(bnc_response(mock,envelope,remote));free(envelope);
 for(size_t i=0;i<remote->count;i++)if(!strcmp(remote->fields[i].key,"testCount")){remote->fields[i].value.number=14;remote->fields[i].rev=100;strcpy(remote->fields[i].device,"peerWinner");}
 assert(bn_json(mock,remote,&raw));writefile(path,raw);free(raw);free(mock);free(remote);
 assert(bna_edit(active,"testCount",val(13)));before=posts;assert(bna_cycle(active)&&number(active,"testCount")==14&&posts==before);
 assert(bna_edit(active,"testCount",val(15)));hook=6;assert(!bna_cycle(active));diskFail=0;assert(bna_close(active));child(argv[0],"recover15");active=bna_open(defs,3);assert(active&&number(active,"testCount")==15);
 puts("PASS: Unicode/escaped-string disk round trip, backend conflict winner and restart after failed durable acknowledgement");
 puts("PASS: 401 refresh/retry, transient backoff, ambiguous commit without duplicate write, edit during push, disable gate and failed-persistence rollback");
 hook=1;assert(!bna_cycle(active));BnValue v;assert(!bna_get(active,"testCount",&v));assert(bna_close(active));active=bna_open(defs,3);assert(active&&!bna_writes_enabled(active)&&number(active,"testCount")==0);assert(bna_cycle(active));assert(bna_close(active));
 strcpy(owner,"owner1");generation++;active=bna_open(defs,3);assert(active&&number(active,"testCount")==15);hook=2;assert(!bna_cycle(active));assert(bna_close(active));active=bna_open(defs,3);assert(active&&!bna_writes_enabled(active)&&number(active,"testCount")==0);assert(bna_cycle(active));assert(bna_close(active));
 profile=1;generation++;active=bna_open(defs,3);assert(active&&number(active,"testCount")==15);char *saved=readfile(path);assert(saved);writefile(path,"{\"schema\":\"future-schema\"}");before=posts;assert(!bna_cycle(active)&&posts==before&&number(active,"testCount")==15);writefile(path,saved);free(saved);assert(bna_close(active));
 char *check=dados_ler("bn-checkpoint-v1-owner1-p1.json");assert(check&&!strstr(check,"mock-bearer"));dados_gravar("bn-checkpoint-v1-owner1-p1.json","corrupted");assert(!bna_open(defs,3));dados_gravar("bn-checkpoint-v1-owner1-p1.json",check);free(check);
 profile=3;generation++;serverpath(path,sizeof path);writefile(path,"{\"schema\":\"better-nuvio-profile\",\"version\":1,\"fields\":{\"testCount\":{\"type\":\"int\",\"value\":77,\"rev\":4,\"device\":\"peer\"}}}");
 active=bna_open(defs,3);assert(active&&!bna_writes_enabled(active)&&number(active,"testCount")==0);assert(bna_writes(active,1)&&bna_close(active));
 active=bna_open(defs,3);before=posts;assert(active&&bna_writes_enabled(active)&&bna_cycle(active)&&number(active,"testCount")==77&&posts==before);assert(bna_close(active));
 logged=0;assert(!bna_open(defs,3));logged=1;
 puts("PASS: opt-in before first pull/restart still restores remote settings without uploading defaults; anonymous transport blocked");
 for(int i=0;i<3;i++){snprintf(path,sizeof path,"%s/regular-%d",root,i);char *s=readfile(path);assert(s&&!strcmp(s,"regular-document-sentinel"));free(s);}
 puts("PASS: account/profile switch in flight, corrupted/future data preserve state, no credential persistence and regular Nuvio documents untouched");
 free(lastWrite);return 0;
}

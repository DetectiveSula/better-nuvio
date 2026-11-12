#include "bnaccount.h"
#include "j7arme.h"
#include "bncodec.h"
#include "homewire.h"
#include "dados.h"
#include "sessao.h"
#include "nuvem.h"
#include "jsw.h"
#define NV_PERFIL_SESSAO_FORTE
#include "perfisessao.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
struct BnAccount {pthread_mutex_t mutex;BnEngine engine;NvPerfilSessao session;char file[160],owner[80],device[65];int profile,writes,busy,cached;};
static int current(BnAccount *a){return sessao_logada()&&nv_perfil_sessao_atual(a->session)&&perfis_ativo()==a->profile&&!strcmp(sessao_usuario(),a->owner);}
static int persist(BnAccount *a,const BnEngine *e,int writes){
 char *s=NULL;if(!current(a)||!bnc_save(e,writes,&s))return 0;
 int ok=0;if(nv_perfil_sessao_travar(a->session)){if(current(a))ok=dados_gravar(a->file,s);nv_perfil_sessao_destravar();}free(s);return ok;
}
BnAccount *bna_open(const BnDef *defs,size_t n){
 if(!sessao_logada())return NULL;BnAccount *a=calloc(1,sizeof *a);if(!a)return NULL;
 a->session=nv_perfil_sessao_ler();
 char token[3000]={0};
 if(!sessao_identidade_local_copiar(a->owner,sizeof a->owner,token,sizeof token)){free(a);return NULL;}
 memset(token,0,sizeof token);a->profile=perfis_ativo();snprintf(a->device,sizeof a->device,"%s",dados_cliente_id());
 if(!bn_init(&a->engine,a->owner,a->profile,a->device,defs,n)||!current(a)){free(a);return NULL;}
 snprintf(a->file,sizeof a->file,"bn-checkpoint-v1-%s-p%d.json",a->owner,a->profile);
 char *s=dados_ler(a->file);if(s){int ok=bnc_load(&a->engine,s,&a->writes);free(s);if(!ok){free(a);return NULL;}a->cached=1;}
 if(pthread_mutex_init(&a->mutex,NULL)){free(a);return NULL;}return a;
}
int bna_close(BnAccount *a){if(!a)return 1;pthread_mutex_lock(&a->mutex);if(a->busy){pthread_mutex_unlock(&a->mutex);return 0;}pthread_mutex_unlock(&a->mutex);pthread_mutex_destroy(&a->mutex);free(a);return 1;}
int bna_edit(BnAccount *a,const char *key,BnValue v){
 return bna_edit_many(a,&key,&v,1);
}
int bna_edit_many(BnAccount *a,const char *const *keys,const BnValue *values,size_t count){
 if(!a)return 0;pthread_mutex_lock(&a->mutex);BnEngine *next=malloc(sizeof *next);int ok=0;
 if(next&&current(a)&&count<=BN_MAX){*next=a->engine;ok=1;
  for(size_t i=0;i<count;i++)if(!bn_edit(next,keys[i],values[i])){ok=0;break;}
  if(ok&&persist(a,next,a->writes))a->engine=*next;else ok=0;
 }
 free(next);pthread_mutex_unlock(&a->mutex);return ok;
}
int bna_get(BnAccount *a,const char *key,BnValue *v){if(!a||!v)return 0;pthread_mutex_lock(&a->mutex);const BnValue *p=current(a)?bn_get(&a->engine,key):NULL;int ok=p!=NULL;if(p)*v=*p;pthread_mutex_unlock(&a->mutex);return ok;}
int bna_writes(BnAccount *a,int enabled){if(!a||(enabled!=0&&enabled!=1))return 0;pthread_mutex_lock(&a->mutex);int ok=persist(a,&a->engine,enabled);if(ok)a->writes=enabled;pthread_mutex_unlock(&a->mutex);return ok;}
int bna_writes_enabled(BnAccount *a){if(!a)return 0;pthread_mutex_lock(&a->mutex);int enabled=current(a)&&a->writes;pthread_mutex_unlock(&a->mutex);return enabled;}
static char *rpc(BnAccount *a,const char *name,const char *body,int write,int *status){
 char token[3000]={0};char *r=NULL;*status=0;
 for(int attempt=0;attempt<2;attempt++){
  /* J7: the stored flag alone never uploads; the debug test-account gate must agree. */
  if(!current(a)||!sessao_token_copiar(token,sizeof token)||!current(a)||(write&&(!bna_writes_enabled(a)||!j7arme_permite_j4(a->owner,a->profile))))break;
  r=nuvem_rpc_com(name,body,token,status);
  if(*status>=200&&*status<300&&current(a))nuvem_ok();
  if(*status!=401||attempt||!current(a))break;
  free(r);r=NULL;sessao_token_recusado(token);
 }
 memset(token,0,sizeof token);return r;
}
static int success(int st){return st>=200&&st<300;}
/* J7 diagnostics: setting names and counts only, never values, owners or tokens. */
static int sameValue(const BnValue *x,const BnValue *y){return x->type==y->type&&x->number==y->number&&!strcmp(x->text,y->text);}
static int logNames(const char *what,int profile,const BnEngine *before,const BnEngine *after){
 char buf[800];size_t len=0;int count=0;buf[0]=0;
 for(size_t i=0;i<after->state.count;i++){
  const BnSlot *s=&after->state.slots[i];int hit;
  if(!before)hit=s->dirty;
  else{hit=1;for(size_t j=0;j<before->state.count;j++)if(!strcmp(before->state.slots[j].current.key,s->current.key)){hit=!sameValue(&before->state.slots[j].current.value,&s->current.value);break;}}
  if(!hit)continue;
  if(++count<=12&&len+sizeof s->current.key+2<sizeof buf)len+=(size_t)snprintf(buf+len,sizeof buf-len,"%s%s",len?",":"",s->current.key);
 }
 if(count>12)snprintf(buf+len,sizeof buf-len,",+%d",count-12);
 printf("[bn-sync] perfil %d %s %d [%s]\n",profile,what,count,count?buf:"-");return count;
}
int bna_cycle(BnAccount *a){
 if(!a||nuvem_freio_ativo())return 0;
 pthread_mutex_lock(&a->mutex);if(a->busy||!current(a)){pthread_mutex_unlock(&a->mutex);return 0;}a->busy=1;bn_failed(&a->engine);pthread_mutex_unlock(&a->mutex);
 int ok=0,st=0;char *response=NULL,*body=NULL;BnDoc *doc=calloc(1,sizeof *doc);BnEngine *next=malloc(sizeof *next);Jsw w;
 if(!doc||!next)goto done;
 jsw_iniciar(&w);jsw_obj_ini(&w);jsw_ci(&w,"p_profile_id",a->profile);jsw_cs(&w,"p_platform","better_nuvio");jsw_obj_fim(&w);
 const char *params=jsw_texto_final(&w);if(params)response=rpc(a,"sync_pull_profile_settings_blob",params,0,&st);jsw_livre(&w);
 pthread_mutex_lock(&a->mutex);
 if(!success(st)||!response||!current(a)){pthread_mutex_unlock(&a->mutex);printf("[bn-sync] perfil %d pull HTTP %d: nada aplicado\n",a->profile,st);goto done;}
 *next=a->engine;
 if(!bnc_response(next,response,doc)||!bn_pull(next,doc)||!persist(a,next,a->writes)){pthread_mutex_unlock(&a->mutex);printf("[bn-sync] perfil %d documento recusado ou nao gravado: nada aplicado\n",a->profile);goto done;}
 printf("[bn-sync] perfil %d pull HTTP %d: %u campo(s) recebidos\n",a->profile,st,(unsigned)doc->count);
 if(bnc_legacy_j0(response))printf("[bn-sync] perfil %d documento J0 de teste: tratado como nao inicializado (mantido no servidor)\n",a->profile);
 logNames("aplicados",a->profile,&a->engine,next);
 a->engine=*next;ok=1;
 int armed=a->writes&&j7arme_permite_j4(a->owner,a->profile);
 if(!armed){int waiting=0;for(size_t i=0;i<a->engine.state.count;i++)waiting+=a->engine.state.slots[i].dirty;
  if(waiting)printf("[bn-sync] perfil %d: %d campo(s) locais pendentes; uploads desligados\n",a->profile,waiting);}
 if(!armed||!bn_prepare(&a->engine,doc)){pthread_mutex_unlock(&a->mutex);goto done;}
 logNames("enviando",a->profile,NULL,&a->engine);
 if(!bn_json(&a->engine,doc,&body)){ok=0;pthread_mutex_unlock(&a->mutex);goto done;}
 pthread_mutex_unlock(&a->mutex);free(response);response=NULL;
 jsw_iniciar(&w);jsw_obj_ini(&w);jsw_ci(&w,"p_profile_id",doc->profile);jsw_cs(&w,"p_platform","better_nuvio");jsw_cs(&w,"p_origin_client_id",a->device);jsw_chave(&w,"p_settings_json");jsw_bruto(&w,body);jsw_obj_fim(&w);
 params=jsw_texto_final(&w);if(params)response=rpc(a,"sync_push_profile_settings_blob",params,1,&st);jsw_livre(&w);
 pthread_mutex_lock(&a->mutex);ok=0;
 if(success(st)&&current(a)){*next=a->engine;if(bn_ack(next,doc)&&persist(a,next,a->writes)){a->engine=*next;ok=1;}}
 printf("[bn-sync] perfil %d push HTTP %d%s\n",doc->profile,st,ok?": confirmado":": mantido pendente");
 pthread_mutex_unlock(&a->mutex);
 done:if((st==0||st==429||st>=500)&&current(a))nuvem_falhou();
 free(body);free(response);free(doc);free(next);
 pthread_mutex_lock(&a->mutex);if(!ok)bn_failed(&a->engine);a->busy=0;pthread_mutex_unlock(&a->mutex);return ok;
}
static int view(const BnEngine *e,const char *const *keys,size_t n,BnValue *values,unsigned char *known){
 if(n>BN_MAX)return 0;
 for(size_t i=0;i<n;i++){const BnValue *v=bn_get(e,keys[i]);if(!v)return 0;values[i]=*v;known[i]=0;
  for(size_t j=0;j<e->state.count;j++)if(!strcmp(e->state.slots[j].current.key,keys[i]))known[i]=(unsigned char)(e->state.slots[j].known||e->state.slots[j].dirty);
 }return 1;
}
int bna_view(BnAccount *a,const char *const *keys,size_t n,BnValue *values,unsigned char *known,int nonblocking){
 if(!a)return 0;if(nonblocking){if(pthread_mutex_trylock(&a->mutex))return 0;}else pthread_mutex_lock(&a->mutex);
 int ok=current(a)&&view(&a->engine,keys,n,values,known);pthread_mutex_unlock(&a->mutex);return ok;
}
int bna_cached(BnAccount *a){return a&&current(a)&&a->cached;}
/* A stack guard captures the actual session; its engine/document can target an
 * inactive profile only in these disk-only APIs. No RPC helper is called. */
static BnAccount *staged(int profile,const BnDef *defs,size_t n){
 if(profile<=0||profile==perfis_ativo()||!sessao_logada())return NULL;
 BnAccount *a=calloc(1,sizeof *a);if(!a)return NULL;a->session=nv_perfil_sessao_ler();a->profile=perfis_ativo();
 char token[3000]={0};int ok=sessao_identidade_local_copiar(a->owner,sizeof a->owner,token,sizeof token);memset(token,0,sizeof token);
 snprintf(a->device,sizeof a->device,"%s",dados_cliente_id());
 if(!ok||!bn_init(&a->engine,a->owner,profile,a->device,defs,n)||!current(a)){free(a);return NULL;}
 snprintf(a->file,sizeof a->file,"bn-checkpoint-v1-%s-p%d.json",a->owner,profile);
 char *s=dados_ler(a->file);if(s){ok=bnc_load(&a->engine,s,&a->writes);free(s);if(!ok){free(a);return NULL;}a->cached=1;}
 return a;
}
int bna_stage(int profile,const BnDef *defs,size_t n,const char *const *keys,const BnValue *values,size_t count){
 BnAccount *a=staged(profile,defs,n);if(!a)return 0;int ok=count<=BN_MAX;
 for(size_t i=0;ok&&i<count;i++)ok=bn_edit(&a->engine,keys[i],values[i]);
 if(ok)ok=persist(a,&a->engine,a->writes);free(a);return ok;
}
int bna_staged_view(int profile,const BnDef *defs,size_t n,const char *const *keys,size_t count,BnValue *values,unsigned char *known){
 BnAccount *a=staged(profile,defs,n);if(!a)return 0;int ok=a->cached&&current(a)&&view(&a->engine,keys,count,values,known);free(a);return ok;
}

int bna_home_view(BnAccount *a,BnField *out,size_t cap,size_t *n){
 if(!a||pthread_mutex_trylock(&a->mutex))return 0;*n=0;int ok=current(a);
 if(ok)for(size_t i=0;i<a->engine.state.count;i++){
  BnSlot *s=&a->engine.state.slots[i];if(!strcmp(s->current.key,HOME_HERO)||home_field(s->current.key,s->current.value)){if(*n==cap){ok=0;break;}out[(*n)++]=s->current;}
 }
 pthread_mutex_unlock(&a->mutex);return ok;
}
/* Local baseline only: no revision, known flag or upload intent. */
int bna_home_seed(BnAccount *a,const BnField *rows,size_t n){
 if(!a||pthread_mutex_trylock(&a->mutex))return 0;int ok=current(a),changed=0;BnEngine *next=NULL;
 for(size_t i=0;ok&&i<n;i++){
  const BnField *f=&rows[i];if(strcmp(f->key,HOME_HERO)?!home_field(f->key,f->value):!home_reference(&f->value)){ok=0;break;}
  if(bn_get(next?next:&a->engine,f->key))continue;
  if(!next){next=malloc(sizeof *next);if(!next){ok=0;break;}*next=a->engine;}
  if(next->state.count==BN_MAX){ok=0;break;}BnSlot *s=&next->state.slots[next->state.count++];memset(s,0,sizeof *s);strcpy(s->current.key,f->key);s->current.value=f->value;strcpy(s->current.device,next->state.device);changed=1;
 }
 if(ok&&changed){ok=persist(a,next,a->writes);if(ok)a->engine=*next;}free(next);pthread_mutex_unlock(&a->mutex);return ok;
}

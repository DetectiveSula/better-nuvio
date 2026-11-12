#include "j7arme.h"
/* J4 Native bindings. Native values/side effects stay on the SDL thread;
 * the existing account worker touches only the J3 engine. */
#include "bnaccount.h"
#include "bncodec.h"
#include "homewire.h"
#include "perfisessao.h"
#include <pthread.h>
typedef struct {int op;const char *key;} JbMap;
static const JbMap jbMap[]={
#include "ajustes_better_map.inc"
};
#define JB_N (sizeof jbMap/sizeof *jbMap)
_Static_assert(JB_N==93,"J1 portable allowlist plus reviewed Home layout and Stage H Family Watching extensions changed");
typedef struct JbContext {BnAccount *account;uint64_t session;int profile,refs,retired,cached,restoreLocal,homeSeeded;unsigned homeSeedRevision;char owner[80];BnValue observed[JB_N];struct JbContext *next;} JbContext;
static pthread_mutex_t jbLock=PTHREAD_MUTEX_INITIALIZER;
static JbContext *jbCurrent;
static int jbApplying,jbEditing,jbEditorProfile,jbEditorPreparing;
static int jbBoot[AJ_N],jbHasBoot,jbReady;
static Uint32 jbNextApply;
static BnValue jbEditorObserved[JB_N],jbEditorFont;
static char jbEditorOwner[80];static int jbEditorActive;
static int jbEqual(BnValue a,BnValue b){return a.type==b.type&&(a.type==BN_STRING?!strcmp(a.text,b.text):a.number==b.number);}
static int jbLang(const BnValue *v){for(int i=0;i<ling_opcao_n();i++)if(!strcmp(v->text,ling_opcao_codigo(i)))return 1;return 0;}
static int jbAudio(const BnValue *v){return strcmp(v->text,"*")&&jbLang(v);}
static int jbFont(const BnValue *v){
 if(!strncmp(v->text,"builtin:",8)){char *end;long n=strtol(v->text+8,&end,10);char canonical[40];snprintf(canonical,sizeof canonical,"builtin:%ld",n);return end!=v->text+8&&!*end&&n>=0&&n<TXT_FAMILIA_N+1&&!strcmp(canonical,v->text);}
 if(strncmp(v->text,"font:",5)||strlen(v->text)!=25)return 0;
 for(int i=5;i<21;i++)if(!strchr("0123456789abcdef",v->text[i]))return 0;return !strcmp(v->text+21,".ttf")||!strcmp(v->text+21,".otf");
}
static int jbEncode(int op,int native,int registry,BnValue *v){
 memset(v,0,sizeof *v);
 if(op==AJ_LEG_LINGUA2||op==AJ_TRAILER_AUDIO){
  const char *s=NULL;if(op==AJ_LEG_LINGUA2){if(native>=0&&native<ling_opcao_n())s=ling_opcao_codigo(native);}
  else if(native>=0&&native<nTrAud)s=TRAUD_COD[native];
  if(!s)return 0;v->type=BN_STRING;snprintf(v->text,sizeof v->text,"%s",s);return 1;
 }
 if(op==AJ_LEG2_FONTE){
  v->type=BN_STRING;const char *custom=registry?legfontes_escolha_id(LEGFONTE_SECUNDARIA):"";
  if(*custom)snprintf(v->text,sizeof v->text,"font:%s",custom);
  else snprintf(v->text,sizeof v->text,"builtin:%d",native);return jbFont(v);
 }
 const Opcao *o=&OPCOES[op];
 if(o->tipo==OP_NUMERO){if(native<o->min||native>o->max)return 0;v->type=BN_INT;v->number=native;return 1;}
 if(o->tipo!=OP_ESCOLHA||native<0||native>=o->n)return 0;
 v->type=o->valores==V_LIGA?BN_BOOL:BN_ENUM;v->number=v->type==BN_BOOL?!native:native;return 1;
}
static int jbDecode(int op,BnValue v,int *native){
 BnValue expected;if(!jbEncode(op,valor[op],0,&expected))return 0;if(v.type!=expected.type)return 0;
 if(op==AJ_LEG_LINGUA2){for(int i=0;i<ling_opcao_n();i++)if(!strcmp(v.text,ling_opcao_codigo(i))){*native=i;return 1;}return 0;}
 if(op==AJ_TRAILER_AUDIO){for(int i=0;i<nTrAud;i++)if(!strcmp(v.text,TRAUD_COD[i])){*native=i;return 1;}return 0;}
 if(op==AJ_LEG2_FONTE){
  if(!jbFont(&v))return 0;
  if(!strncmp(v.text,"builtin:",8)){*native=atoi(v.text+8);return 1;}
  for(int i=0;i<legfontes_n();i++)if(!strcmp(v.text+5,legfontes_id(i))){*native=ajFonteBase(op)+i;return 1;}return 0;
 }
 if(v.type==BN_BOOL){if(v.number!=0&&v.number!=1)return 0;*native=!v.number;return 1;}
 if(v.number<INT_MIN||v.number>INT_MAX)return 0;int n=(int)v.number;const Opcao *o=&OPCOES[op];
 if(o->tipo==OP_NUMERO){if(n<o->min||n>o->max)return 0;}
 else if(n<0||n>=nValores(op))return 0;*native=n;return 1;
}
static int jbDefs(const int *values,int registry,BnDef *defs){
 for(size_t i=0;i<JB_N;i++){
  int op=jbMap[i].op;const Opcao *o=&OPCOES[op];BnDef *d=&defs[i];memset(d,0,sizeof *d);
  if(strcmp(CHAVE[op],jbMap[i].key)||!dePerfil(op)||!jbEncode(op,values[op],registry,&d->initial))return 0;
  snprintf(d->key,sizeof d->key,"%s",jbMap[i].key);d->type=d->initial.type;d->portable=1;
  if(d->type==BN_STRING)d->accept=op==AJ_LEG2_FONTE?jbFont:op==AJ_TRAILER_AUDIO?jbAudio:jbLang;
  else if(d->type==BN_INT){d->min=o->min;d->max=o->max;}
  else {d->min=0;d->max=d->type==BN_BOOL?1:o->n-1;}
 }return 1;
}
static int jbRead(BnValue *v){for(size_t i=0;i<JB_N;i++)if(!jbEncode(jbMap[i].op,valor[jbMap[i].op],1,&v[i]))return 0;return 1;}
static void jbKeys(const char **keys){for(size_t i=0;i<JB_N;i++)keys[i]=jbMap[i].key;}
/* Last Native-applied values distinguish an uncaptured local save from a
 * remote checkpoint committed just before a crash. This is local-only data. */
static int jbAnchor(int profile,const int *native,BnValue *values,int write){
 if(!sessao_logada()||profile<=0)return 0;
 uint64_t session=nv_perfil_sessao_ler();char owner[80],file[160];snprintf(owner,sizeof owner,"%s",sessao_usuario());
 BnDef *defs=calloc(JB_N+1,sizeof *defs);BnEngine *e=calloc(1,sizeof *e);int ok=-1;
 if(!defs||!e||!jbDefs(native,0,defs)||!bn_init(e,owner,profile,dados_cliente_id(),defs,JB_N))goto done;
 snprintf(file,sizeof file,"bn-native-v1-%s-p%d.json",owner,profile);
 if(write){
  for(size_t i=0;i<JB_N;i++)e->state.slots[i].current.value=values[i];
  char *text=NULL;if(!bnc_save(e,0,&text))goto done;
  if(nv_perfil_sessao_travar(session)){if(!strcmp(owner,sessao_usuario()))ok=dados_gravar(file,text)?1:-1;nv_perfil_sessao_destravar();}free(text);
 }else{
  char *text=dados_ler(file);int writes=0;if(!text){ok=0;goto done;}
  int loaded=bnc_load(e,text,&writes);free(text);if(!loaded)goto done;
  for(size_t i=0;i<JB_N;i++){const BnValue *v=bn_get(e,jbMap[i].key);if(!v)goto done;values[i]=*v;}ok=1;
 }
 done:free(defs);free(e);return ok;
}
static int jbCurrentScope(JbContext *c);
static int jbHomeEdit(const char *key,BnValue v){
 if(!jbReady||jbApplying||jbEditing||edPerfil||!sessao_logada())return 0;
 ajustes_better_contexto();return jbCurrentScope(jbCurrent)&&bna_edit(jbCurrent->account,key,v);
}
void ajustes_better_loading(void){jbReady=0;aj6Loaded=0;}
void ajustes_better_boot(void){memcpy(jbBoot,valor,sizeof jbBoot);jbHasBoot=1;jbReady=1;aj6Loaded=1;fil_home_observer(jbHomeEdit);}
void ajustes_better_perfil_antes(int profile,const int *before){
 if(jbApplying||!sessao_logada()||(!edPerfil&&!ajBetterExplicitPerfil))return;
 BnValue values[JB_N];if(jbAnchor(profile,before,values,0)!=0)return;
 for(size_t i=0;i<JB_N;i++)if(!jbEncode(jbMap[i].op,before[jbMap[i].op],0,&values[i]))return;
 jbAnchor(profile,before,values,1);
}
static int jbForeign(int profile,const char *owner){
 char file[80],prior[80];snprintf(file,sizeof file,"bn-native-owner-p%d.txt",profile);char *text=dados_ler(file);
 int other=text&&sscanf(text,"%79s",prior)==1&&strcmp(prior,owner);free(text);return other;
}
static void jbMarkOwner(int profile,const char *owner){char file[80];snprintf(file,sizeof file,"bn-native-owner-p%d.txt",profile);dados_gravar(file,owner);}
static int jbCurrentScope(JbContext *c){return c&&sessao_logada()&&c->profile==perfis_ativo()&&!strcmp(c->owner,sessao_usuario())&&nv_perfil_sessao_atual(c->session);}
static int jbFontApply(BnValue desired){int n;if(!jbDecode(AJ_LEG2_FONTE,desired,&n))return jbFont(&desired)&&!strncmp(desired.text,"font:",5)&&legfontes_reservar_secundaria_id(desired.text+5);return ajFonteAplicar(AJ_LEG2_FONTE,n);}
static int jbApply(const BnValue *values,const unsigned char *known,int cachedFont){
 int old[AJ_N],changed=0,cw=0,language=0;memcpy(old,valor,sizeof old);jbApplying++;
 for(size_t i=0;i<JB_N;i++){
  int op=jbMap[i].op,n;if(op==AJ_LEG2_FONTE||!known[i]||!jbDecode(op,values[i],&n)||n==valor[op])continue;
  valor[op]=n;changed++;if(op==AJ_LEG_LINGUA2)language=1;
  if(op==AJ_CW_OK||op==AJ_CW_PROXIMO||op==AJ_CW_MINIMO||op==AJ_CW_RETIDO_TAMBEM)cw=1;
 }
 if(changed&&!gravar()){memcpy(valor,old,sizeof old);jbApplying--;return 0;}
 if(language)aplicarIdioma(AJ_LEG_LINGUA2);
 for(size_t i=0;i<JB_N;i++)if(jbMap[i].op==AJ_LEG2_FONTE&&(known[i]||cachedFont)){
  BnValue actual;if(jbEncode(AJ_LEG2_FONTE,valor[AJ_LEG2_FONTE],1,&actual)&&!jbEqual(actual,values[i])&&jbFontApply(values[i]))changed++;
 }
 jbApplying--;if(cw)desc_refazer_continuar();return changed;
}
void ajustes_better_contexto(void){
 if(!jbReady||jbApplying||edPerfil||!sessao_logada()||jbCurrentScope(jbCurrent))return;
 JbContext *c=calloc(1,sizeof *c);BnDef *defs=calloc(JB_N+1,sizeof *defs);if(!c||!defs){free(c);free(defs);return;}
 c->profile=perfis_ativo();c->session=nv_perfil_sessao_ler();snprintf(c->owner,sizeof c->owner,"%s",sessao_usuario());
 /* The custom-font registry is device-global; do not inherit another profile's
  * selection. The previous local checkpoint retains its stable font ID. */
 char *fontOwner=dados_ler("bn-font-context-v1.txt"),prior[80];int priorProfile=0;int owned=fontOwner&&sscanf(fontOwner,"%79s %d",prior,&priorProfile)==2&&!strcmp(prior,c->owner)&&priorProfile==c->profile;
 if(fontOwner&&sscanf(fontOwner,"%79s %d",prior,&priorProfile)==2&&(strcmp(prior,c->owner)||priorProfile!=c->profile)){
  jbApplying++;int ok=legfontes_escolher(LEGFONTE_SECUNDARIA,-1);jbApplying--;if(!ok){free(fontOwner);free(c);free(defs);return;}
 }
 free(fontOwner);rotulosDeIdioma();
 if(jbDefs(valor,1,defs)){home_definition(&defs[JB_N]);c->account=bna_open(defs,JB_N+1);}free(defs);
 if(!c->account){free(c);return;}c->cached=bna_cached(c->account);c->restoreLocal=c->cached&&jbForeign(c->profile,c->owner);
 /* Persist the unobserved local baseline, without marking defaults as edits. */
 if(!bna_writes(c->account,bna_writes_enabled(c->account))||!jbRead(c->observed)){bna_close(c->account);free(c);return;}
 BnValue anchor[JB_N],native[JB_N],changes[JB_N];const char *changedKeys[JB_N];size_t n=0;memcpy(native,c->observed,sizeof native);
 int anchored=jbAnchor(c->profile,valor,anchor,0);
 if(jbHasBoot&&owned)for(size_t i=0;i<JB_N;i++)if(jbMap[i].op!=AJ_LEG2_FONTE)jbEncode(jbMap[i].op,jbBoot[jbMap[i].op],0,&native[i]);
 if(anchored<0){bna_close(c->account);free(c);return;}
 BnValue cached[JB_N];unsigned char cacheKnown[JB_N];const char *allKeys[JB_N];jbKeys(allKeys);
 int cacheReady=bna_view(c->account,allKeys,JB_N,cached,cacheKnown,0);
 if(anchored&&!c->restoreLocal)for(size_t i=0;i<JB_N;i++){
  if(jbMap[i].op==AJ_LEG2_FONTE&&!owned)continue;
  int supported;if(jbMap[i].op==AJ_TRAILER_FONTE&&cacheReady&&cacheKnown[i]&&!jbDecode(AJ_TRAILER_FONTE,cached[i],&supported))continue;
  if(!jbEqual(native[i],anchor[i])){changedKeys[n]=jbMap[i].key;changes[n++]=native[i];}
 }
 if((n&&!bna_edit_many(c->account,changedKeys,changes,n))||(!anchored&&jbAnchor(c->profile,valor,c->observed,1)<0)){bna_close(c->account);free(c);return;}
 if(anchored&&!c->restoreLocal)jbAnchor(c->profile,valor,c->observed,1);
 jbHasBoot=0;jbNextApply=0;
 pthread_mutex_lock(&jbLock);JbContext *old=jbCurrent;jbCurrent=c;if(old){old->retired=1;if(!old->refs){bna_close(old->account);free(old);}}pthread_mutex_unlock(&jbLock);
 char stamp[128];snprintf(stamp,sizeof stamp,"%s %d\n",c->owner,c->profile);dados_gravar("bn-font-context-v1.txt",stamp);
}
void ajustes_better_observar(void){
 if(!jbReady||jbApplying||ajBetterFonteTransaction||jbEditing||edPerfil)return;JbContext *c=jbCurrent;if(!jbCurrentScope(c))return;
 BnValue actual[JB_N],changes[JB_N];const char *keys[JB_N];size_t n=0;
 if(!jbRead(actual))return;
 for(size_t i=0;i<JB_N;i++)if(!jbEqual(actual[i],c->observed[i])){keys[n]=jbMap[i].key;changes[n++]=actual[i];}
 if(n){
  int nativeSaved=perfilGravarVetor(c->profile,valor);
  if(bna_edit_many(c->account,keys,changes,n)){memcpy(c->observed,actual,sizeof actual);if(nativeSaved)jbAnchor(c->profile,valor,actual,1);}
 }
}
int ajustes_better_passo(void){
 if(jbApplying||ajBetterFonteTransaction)return 0;
 if(edPerfil){ajustes_better_editor_observar();return 0;}
 ajustes_better_contexto();JbContext *c=jbCurrent;if(!jbCurrentScope(c))return 0;
 fil_home_scope(c->owner,c->profile);
 ajustes_better_observar();BnValue actual[JB_N],remote[JB_N];unsigned char known[JB_N];const char *keys[JB_N];jbKeys(keys);
 if(!jbRead(actual))return 0;
 /* Failed local journalling takes priority over an older remote projection. */
 for(size_t i=0;i<JB_N;i++)if(!jbEqual(actual[i],c->observed[i]))return 0;
 if(!bna_view(c->account,keys,JB_N,remote,known,1))return 0;
 Uint32 now=SDL_GetTicks();if(jbNextApply&&(Sint32)(now-jbNextApply)<0)return 0;jbNextApply=now+250;
 if(c->restoreLocal)memset(known,1,sizeof known);
 int n=jbApply(remote,known,c->cached),homeChanged=0;jbRead(c->observed);
 BnField *home=calloc(BN_MAX,sizeof *home);size_t nh=0;
 if(home){
  unsigned revision=fil_revisao();
  if((!c->homeSeeded||revision!=c->homeSeedRevision)&&fil_home_baseline(c->owner,c->profile,home,BN_MAX,&nh)&&bna_home_seed(c->account,home,nh)){c->homeSeeded=1;c->homeSeedRevision=revision;}
  if(bna_home_view(c->account,home,BN_MAX,&nh)&&jbCurrentScope(c))homeChanged=fil_home_apply(c->owner,c->profile,home,nh);
 }
 free(home);
 if(n||c->restoreLocal){int restoring=c->restoreLocal;c->restoreLocal=0;
  if(perfilGravarVetor(c->profile,valor))jbAnchor(c->profile,valor,c->observed,1);else c->restoreLocal=restoring;
 }
 return n+homeChanged;
}
void ajustes_better_ciclo(uint64_t session){
 pthread_mutex_lock(&jbLock);JbContext *c=jbCurrent;
 if(!c||c->session!=session||!jbCurrentScope(c)){pthread_mutex_unlock(&jbLock);return;}c->refs++;pthread_mutex_unlock(&jbLock);
 bna_cycle(c->account);
 pthread_mutex_lock(&jbLock);c->refs--;if(c->retired&&!c->refs){bna_close(c->account);free(c);}pthread_mutex_unlock(&jbLock);
}
void ajustes_better_esquecer(void){fil_home_scope("",0);pthread_mutex_lock(&jbLock);JbContext *c=jbCurrent;jbCurrent=NULL;if(c){c->retired=1;if(!c->refs){bna_close(c->account);free(c);}}pthread_mutex_unlock(&jbLock);}
/* J7 leak check: may Better Nuvio write this better_nuvio field name? J4's 91
 * keys and J5's closed Home family only (homewire.h); anything else is a leak. */
int ajustes_better_chave_sincronizavel(const char *k){
 if(!k)return 0;for(size_t i=0;i<JB_N;i++)if(!strcmp(k,jbMap[i].key))return 1;
 if(!strcmp(k,HOME_HERO)||!strcmp(k,HOME_MARKER))return 1;
 if(strlen(k)!=39||strncmp(k,"home_",5)||k[37]!='_'||!strchr("szova",k[38]))return 0;
 char id[33];memcpy(id,k+5,32);id[32]=0;return home_digest(id);
}
size_t ajustes_better_n(void){return JB_N;}
const char *ajustes_better_chave(size_t i){return i<JB_N?jbMap[i].key:NULL;}
/* True only while this profile's J4/J5 (incl. Home presentation) edits are
 * actually uploaded: permission on and the J7 gate open (never in release). */
int ajustes_better_envia(void){
 ajustes_better_contexto();
 return jbCurrentScope(jbCurrent)&&bna_writes_enabled(jbCurrent->account)&&j7arme_permite_j4(jbCurrent->owner,jbCurrent->profile);
}
/* Settings panel scope label: this option is one of J4's Better Nuvio bindings. */
int ajustes_better_op_vinculada(int op){for(size_t i=0;i<JB_N;i++)if(jbMap[i].op==op)return 1;return 0;}
int ajustes_better_writes(int enabled){ajustes_better_contexto();return jbCurrentScope(jbCurrent)&&bna_writes(jbCurrent->account,enabled);}
/* Existing admin editor and profile-copy/reset paths: local journals only. */
void ajustes_better_perfil_gravado(int profile,const int *before,const int *after){
 if(profile<=0||!sessao_logada())return;
 if((jbCurrent&&!jbCurrent->restoreLocal&&profile==jbCurrent->profile&&!strcmp(jbCurrent->owner,sessao_usuario()))||
    (!jbEditorPreparing&&edPerfil==profile&&!strcmp(jbEditorOwner,sessao_usuario())))jbMarkOwner(profile,sessao_usuario());
 if(jbApplying||profile==perfis_ativo())return;
 BnDef *defs=calloc(JB_N+1,sizeof *defs);if(!defs)return;BnValue changes[JB_N],native[JB_N];const char *keys[JB_N];size_t n=0;
 int intent=edPerfil==profile||ajBetterExplicitPerfil;
 int leaving=jbCurrent&&profile==jbCurrent->profile&&!strcmp(jbCurrent->owner,sessao_usuario());
 if(jbDefs(before,0,defs)){
  for(size_t i=0;i<JB_N;i++){
   int op=jbMap[i].op;if(!jbEncode(op,after[op],(edPerfil==profile||leaving)&&!ajBetterFonteTransaction,&native[i])){free(defs);return;}
   if(leaving)defs[i].initial=jbCurrent->observed[i];
   if((leaving&&!jbEqual(native[i],jbCurrent->observed[i]))||(!leaving&&intent&&before[op]!=after[op])){keys[n]=jbMap[i].key;changes[n++]=native[i];}
  }
  home_definition(&defs[JB_N]);
  if(!n||bna_stage(profile,defs,JB_N+1,keys,changes,n))jbAnchor(profile,after,native,1);
 }free(defs);
}
void ajustes_better_editor_begin(void){
 jbEditing=1;jbEditorActive=perfis_ativo();snprintf(jbEditorOwner,sizeof jbEditorOwner,"%s",sessao_usuario());
 jbEncode(AJ_LEG2_FONTE,valor[AJ_LEG2_FONTE],1,&jbEditorFont);
}
void ajustes_better_editor_ready(void){
 jbEditorProfile=edPerfil;jbEditorPreparing=1;int foreign=jbForeign(edPerfil,sessao_usuario());BnDef *defs=calloc(JB_N+1,sizeof *defs);BnValue values[JB_N];unsigned char known[JB_N];const char *keys[JB_N];jbKeys(keys);
 jbApplying++;perfilGravarVetor(edPerfil,valor);jbApplying--;
 if(defs&&jbDefs(valor,1,defs)&&(home_definition(&defs[JB_N]),bna_staged_view(edPerfil,defs,JB_N+1,keys,JB_N,values,known))){if(foreign)memset(known,1,sizeof known);jbApply(values,known,1);}
 free(defs);jbEditorPreparing=0;jbApplying++;perfilGravarVetor(edPerfil,valor);jbApplying--;jbRead(jbEditorObserved);
 if(jbAnchor(edPerfil,valor,values,0)==0)jbAnchor(edPerfil,valor,jbEditorObserved,1);
 char stamp[128];snprintf(stamp,sizeof stamp,"%s %d\n",jbEditorOwner,edPerfil);dados_gravar("bn-font-context-v1.txt",stamp);
}
void ajustes_better_editor_observar(void){
 if(jbApplying||ajBetterFonteTransaction||!edPerfil||edPerfil!=jbEditorProfile||strcmp(jbEditorOwner,sessao_usuario()))return;
 BnDef *defs=calloc(JB_N+1,sizeof *defs);BnValue actual[JB_N],changes[JB_N];const char *keys[JB_N];size_t n=0;
 if(defs&&jbRead(actual)&&jbDefs(valor,1,defs)){
  for(size_t i=0;i<JB_N;i++){defs[i].initial=jbEditorObserved[i];if(!jbEqual(actual[i],jbEditorObserved[i])){keys[n]=jbMap[i].key;changes[n++]=actual[i];}}
  home_definition(&defs[JB_N]);
  if(!n||bna_stage(edPerfil,defs,JB_N+1,keys,changes,n)){
   memcpy(jbEditorObserved,actual,sizeof actual);
   BnValue anchor[JB_N];if(jbAnchor(edPerfil,valor,anchor,0)==1){
    for(size_t i=0;i<JB_N;i++)if(jbMap[i].op==AJ_LEG2_FONTE)anchor[i]=actual[i];
    jbAnchor(edPerfil,valor,anchor,1);
   }
  }
 }free(defs);
}
void ajustes_better_editor_end(void){
 jbApplying++;if(jbEditorActive==perfis_ativo()&&!strcmp(jbEditorOwner,sessao_usuario()))jbFontApply(jbEditorFont);
 else legfontes_escolher(LEGFONTE_SECUNDARIA,-1);
 jbApplying--;jbEditing=0;jbEditorProfile=0;
 char stamp[128];snprintf(stamp,sizeof stamp,"%s %d\n",sessao_usuario(),perfis_ativo());dados_gravar("bn-font-context-v1.txt",stamp);
}

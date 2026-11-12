/* J6: original TV document only. Included before the existing Stage I bridge.
 * Values/checkpoints are Native integers; wire conversion is explicit/lossless. */
#include <pthread.h>
#include <errno.h>
#include "j7arme.h"
#define AJ6_NATIVE_BRIDGE 1
typedef enum {AJ6_BOOL,AJ6_INT,AJ6_CHOICE,AJ6_HOURS,AJ6_FOCUS} Aj6Codec;
typedef struct {int op;const char *feature,*key;Aj6Codec codec;const char *const *choices;int n;} Aj6Map;
#include "ajustes_official_map.inc"
#define AJ6_N (sizeof aj6Map/sizeof *aj6Map)
typedef struct {int observed,base,known,pending,desired,hold;} Aj6Field;
static Aj6Field aj6State[AJ6_N];
static int aj6Sent[AJ6_N],aj6SentValue[AJ6_N];
static pthread_mutex_t aj6Lock=PTHREAD_MUTEX_INITIALIZER;
static char aj6Owner[160];static int aj6Profile,aj6Ready,aj6Permission,aj6Loaded;
static uint64_t aj6Session;
static int aj6Current(void){return aj6Loaded&&aj6Ready&&sessao_logada()&&aj6Profile==perfis_ativo()&&!strcmp(aj6Owner,sessao_usuario())&&nv_perfil_sessao_atual(aj6Session);}
static int aj6Native(size_t i,int v){int op=aj6Map[i].op;const Opcao *o=&OPCOES[op];return o->tipo==OP_NUMERO?v>=o->min&&v<=o->max:v>=0&&v<nValores(op);}
static const char *aj6Type(size_t i){Aj6Codec c=aj6Map[i].codec;return c==AJ6_BOOL||c==AJ6_FOCUS?"boolean":c==AJ6_INT||c==AJ6_HOURS?"int":"string";}
static const char *aj6String(size_t i,int n){
 const Aj6Map *m=&aj6Map[i];
 if(m->codec==AJ6_CHOICE)return n>=0&&n<m->n?m->choices[n]:NULL;
 return NULL;
}
static int aj6Supported(size_t i,int n){
 /* The map is the reviewed J1/J6 allowlist; no entry is J1 device-local. The legacy
  * #205 ajusteAparelho backup predicate is not the Stage J device rule (J4 binds
  * 81 controls it marks profile-local) and would silently disable 9 mappings. */
 if(!aj6Native(i,n))return 0;
 switch(aj6Map[i].codec){
 case AJ6_BOOL:return n==0||n==1;
 case AJ6_CHOICE:return aj6String(i,n)!=NULL;
 case AJ6_INT:return aj6Map[i].op!=AJ_CW_DIAS||n==0||n>=7;
 case AJ6_HOURS:return n>=0&&n<(int)(sizeof LINK_PRAZO_H/sizeof *LINK_PRAZO_H);
 case AJ6_FOCUS:return n>=0&&n<=2;
 }return 0;
}
static void aj6Encoded(Jsw *w,const char *key,const char *type,int number,const char *str){
 jsw_chave(w,key);jsw_obj_ini(w);jsw_cs(w,"type",type);jsw_chave(w,"value");
 if(!strcmp(type,"string"))jsw_str(w,str);else if(!strcmp(type,"boolean"))jsw_bool(w,number);else jsw_int(w,number);jsw_obj_fim(w);
}
static int aj6Encode(Jsw *w,size_t i,int n){
 if(!aj6Supported(i,n))return 0;const Aj6Map *m=&aj6Map[i];const char *type=aj6Type(i);int number=n;
 if(m->codec==AJ6_BOOL)number=!n;
 if(m->codec==AJ6_HOURS)number=LINK_PRAZO_H[n];
 if(m->codec==AJ6_FOCUS)number=n!=0;
 aj6Encoded(w,m->key,type,number,aj6String(i,n));
 if(m->codec==AJ6_FOCUS&&n)aj6Encoded(w,"focused_poster_backdrop_trailer_playback_target","string",0,n==1?"HERO_MEDIA":"EXPANDED_CARD");
 return 1;
}
static int aj6TokenString(const char *v,const char *f,const char *s){size_t n=strlen(s);return f-v==(ptrdiff_t)n+2&&*v=='"'&&v[n+1]=='"'&&!memcmp(v+1,s,n);}
static int aj6Member(const char *feature,const char *key,const char *type,const char **v,const char **f){
 const char *a,*b,*t,*te;
 if(!ajJsonGet(feature,feature+strlen(feature),key,&a,&b))return -1;
 if(*a!='{'||!ajJsonGet(a,b,"type",&t,&te)||!aj6TokenString(t,te,type)||!ajJsonGet(a,b,"value",v,f))return -2;
 return 1;
}
/* 1 valid, -1 missing, -2 malformed or unsupported: absence never resets. */
static int aj6Remote(const char *json,size_t i,int *out){
 const Aj6Map *m=&aj6Map[i];const char *features,*fe,*a,*b,*v,*f;
 if(!ajJsonGet(json,json+strlen(json),"features",&features,&fe))return -1;
 if(*features!='{')return -2;
 if(!ajJsonGet(features,fe,m->feature,&a,&b))return -1;
 if(*a!='{')return -2;char *feature=strndup(a,(size_t)(b-a));if(!feature)return -2;
 int ok=aj6Member(feature,m->key,aj6Type(i),&v,&f),n=-1;
 if(ok!=1){free(feature);return ok;}
 if(m->codec==AJ6_BOOL||m->codec==AJ6_FOCUS){
  int enabled=f-v==4&&!memcmp(v,"true",4);if(!enabled&&(f-v!=5||memcmp(v,"false",5)))goto invalid;
  n=m->codec==AJ6_BOOL?!enabled:0;
  if(m->codec==AJ6_FOCUS&&enabled){
   /* Absent target reads as HERO_MEDIA (LayoutPreferenceDataStore default). */
   int t=aj6Member(feature,"focused_poster_backdrop_trailer_playback_target","string",&v,&f);
   if(t==-1)n=1;else if(t!=1)goto invalid;
   else if(aj6TokenString(v,f,"HERO_MEDIA"))n=1;else if(aj6TokenString(v,f,"EXPANDED_CARD"))n=2;else goto invalid;
  }
 }else if(m->codec==AJ6_INT||m->codec==AJ6_HOURS){
  char *end;errno=0;long val=strtol(v,&end,10);if(errno||end!=f||end==v||val<INT_MIN||val>INT_MAX)goto invalid;
  n=(int)val;if(m->codec==AJ6_HOURS){n=-1;for(size_t j=0;j<sizeof LINK_PRAZO_H/sizeof *LINK_PRAZO_H;j++)if(LINK_PRAZO_H[j]==val)n=(int)j;}
 }else {
  int count=m->n;
  for(int j=0;j<count;j++){const char *s=aj6String(i,j);if(s&&aj6TokenString(v,f,s)){n=j;break;}}
 }
 if(!aj6Supported(i,n))goto invalid;*out=n;free(feature);return 1;
 invalid:free(feature);return -2;
}
/* The journal never stores the account id: the file name and the v2 header
 * carry only its 64-bit hash. */
static unsigned long long aj6OwnerHash(const char *owner){unsigned long long h=1469598103934665603ULL;ajustesHashBytes(&h,owner,strlen(owner));return h;}
static void aj6Name(const char *owner,int profile,char *file,size_t cap){
 snprintf(file,cap,"settings-official-j6-%016llx-p%d.txt",aj6OwnerHash(owner),profile);
}
static void aj6Initial(Aj6Field *state,const int *values){
 for(size_t i=0;i<AJ6_N;i++){int v=values[aj6Map[i].op];state[i]=(Aj6Field){.observed=v,.desired=v,.hold=!aj6Supported(i,v)&&v!=valorPadrao[aj6Map[i].op]};}
}
static int aj6Save(const char *owner,int profile,const Aj6Field *state){
 char file[128],text[16384];aj6Name(owner,profile,file,sizeof file);size_t n=(size_t)snprintf(text,sizeof text,"v2 %016llx %d\n",aj6OwnerHash(owner),profile);
 for(size_t i=0;i<AJ6_N;i++){const Aj6Field *s=&state[i];n+=(size_t)snprintf(text+n,sizeof text-n,"%s %d %d %d %d %d %d\n",CHAVE[aj6Map[i].op],s->observed,s->base,s->known,s->pending,s->desired,s->hold);if(n>=sizeof text)return 0;}
 return dados_gravar(file,text);
}
/* Partial older registries may grow; unknown or duplicate rows fail closed. */
static int aj6Read(const char *owner,int profile,Aj6Field *state){
 char file[128],who[160],version[3],header[220],tail;int p=0;aj6Name(owner,profile,file,sizeof file);char *text=dados_ler(file);if(!text)return 0;
 /* v2: hashed owner. v1 (before 17.32) held the raw account id; still accepted
  * for the same owner so pending edits survive, and the next save rewrites v2. */
 const char *firstEnd=strchr(text,'\n');
 if(strlen(text)>16383||!firstEnd||(size_t)(firstEnd-text)>=sizeof header){free(text);return -1;}
 memcpy(header,text,(size_t)(firstEnd-text));header[firstEnd-text]=0;
 if(sscanf(header,"%2s %159s %d %c",version,who,&p,&tail)!=3||p!=profile){free(text);return -1;}
 int v2=!strcmp(version,"v2");
 if(v2?(strlen(who)!=16||strspn(who,"0123456789abcdefABCDEF")!=16||strtoull(who,NULL,16)!=aj6OwnerHash(owner)):
        (strcmp(version,"v1")||strcmp(who,owner))){free(text);return -1;}
 unsigned char seen[AJ6_N]={0};const char *line=strchr(text,'\n');
 while(line&&*++line){char key[160]={0},tail;Aj6Field s={0};
  const char *end=strchr(line,'\n');if(!end){free(text);return -1;}char *copy=strndup(line,(size_t)(end-line));if(!copy){free(text);return -1;}
  int ok=sscanf(copy,"%159s %d %d %d %d %d %d %c",key,&s.observed,&s.base,&s.known,&s.pending,&s.desired,&s.hold,&tail)==7;free(copy);int at=-1;
  for(size_t i=0;i<AJ6_N;i++)if(!strcmp(key,CHAVE[aj6Map[i].op]))at=(int)i;
  if(!ok||at<0||seen[at]||!aj6Native((size_t)at,s.observed)||!aj6Native((size_t)at,s.desired)||(s.known!=0&&s.known!=1)||(s.pending!=0&&s.pending!=1)||(s.hold!=0&&s.hold!=1)||(s.hold&&s.pending)||(s.known&&!aj6Supported((size_t)at,s.base))||(s.pending&&!aj6Supported((size_t)at,s.desired))){free(text);return -1;}
  state[at]=s;seen[at]=1;line=end;
 }free(text);return 1;
}
static void aj6Effects(int op){
 if(op==AJ_CW_FONTE||op==AJ_CW_DIAS)desc_refazer_continuar();
}
static void aj6Context(void){
 if(!aj6Loaded||!sessao_logada()||edPerfil)return;pthread_mutex_lock(&aj6Lock);
 if(aj6Current()){pthread_mutex_unlock(&aj6Lock);return;}
 aj6Ready=0;aj6Permission=0;aj6Profile=perfis_ativo();snprintf(aj6Owner,sizeof aj6Owner,"%s",sessao_usuario());aj6Session=nv_perfil_sessao_ler();aj6Initial(aj6State,valor);
 int read=aj6Read(aj6Owner,aj6Profile,aj6State);
 if(read>=0){aj6Ready=1;if(read)for(size_t i=0;i<AJ6_N;i++){valor[aj6Map[i].op]=aj6State[i].observed;aj6Effects(aj6Map[i].op);}aj6Save(aj6Owner,aj6Profile,aj6State);}
 pthread_mutex_unlock(&aj6Lock);
}
static void aj6Observe(void){
 if(edPerfil)return;pthread_mutex_lock(&aj6Lock);int changed=0;
 if(aj6Current())for(size_t i=0;i<AJ6_N;i++){Aj6Field *s=&aj6State[i];int n=valor[aj6Map[i].op];if(n==s->observed)continue;
  s->observed=n;s->hold=!aj6Supported(i,n);s->desired=n;s->pending=!s->hold&&(!s->known||n!=s->base);changed=1;
 }if(changed)aj6Save(aj6Owner,aj6Profile,aj6State);pthread_mutex_unlock(&aj6Lock);
}
static void aj6Edit(int profile,const int *before,const int *after){
 if(!sessao_logada())return;pthread_mutex_lock(&aj6Lock);Aj6Field state[AJ6_N];aj6Initial(state,before);int changed=0;
 if(aj6Read(sessao_usuario(),profile,state)>=0){for(size_t i=0;i<AJ6_N;i++){int op=aj6Map[i].op;if(before[op]==after[op])continue;Aj6Field *s=&state[i];s->observed=s->desired=after[op];s->hold=!aj6Supported(i,after[op]);s->pending=!s->hold&&(!s->known||s->desired!=s->base);changed=1;}if(changed)aj6Save(sessao_usuario(),profile,state);}
 pthread_mutex_unlock(&aj6Lock);
}
/* J7 diagnostics: official key names and counts only, never values. */
static void aj6Log(const char *what,const unsigned char *hit,int profile){
 char buf[800];size_t len=0;int n=0;buf[0]=0;
 for(size_t i=0;i<AJ6_N;i++)if(hit[i]){if(++n<=12&&len+strlen(aj6Map[i].key)+2<sizeof buf)len+=(size_t)snprintf(buf+len,sizeof buf-len,"%s%s",len?",":"",aj6Map[i].key);}
 if(n>12)snprintf(buf+len,sizeof buf-len,",+%d",n-12);
 printf("[ajustes-oficial] perfil %d %s %d [%s]\n",profile,what,n,n?buf:"-");
}
/* J7: the session permission alone never uploads; the debug test-account gate must agree. */
static int aj6Allowed(void){return aj6Permission&&j7arme_permite_j6(aj6Owner,aj6Profile);}
/* J7b: J6 changes waiting while J6 uploads are off, by official name and count
 * (never values). Called with aj6Lock held after every pull, so the waiting set
 * is visible even when nothing else in the cycle would reach the merge step. */
static void aj6LogWaiting(void){
 unsigned char waiting[AJ6_N];int n=0;for(size_t i=0;i<AJ6_N;i++){waiting[i]=(unsigned char)(aj6State[i].pending!=0);n+=waiting[i];}
 if(n&&!aj6Allowed())aj6Log("pendentes com uploads J6 desligados",waiting,aj6Profile);
}
static int aj6Apply(const char *json){
 int changed=0;unsigned char applied[AJ6_N]={0},invalid[AJ6_N]={0};int received=0;pthread_mutex_lock(&aj6Lock);
 if(aj6Current())for(size_t i=0;i<AJ6_N;i++){int remote,status=aj6Remote(json,i,&remote);if(status==-2)invalid[i]=1;if(status!=1)continue;received++;Aj6Field *s=&aj6State[i];
  if(s->pending&&!s->known&&remote!=s->desired){s->base=remote;s->known=1;continue;}
  if(s->pending&&s->known&&remote==s->base&&remote!=s->desired)continue;
  s->base=remote;s->known=1;s->pending=0;
  if(s->hold)continue;s->desired=s->observed=remote;
  if(valor[aj6Map[i].op]!=remote){valor[aj6Map[i].op]=remote;aj6Effects(aj6Map[i].op);changed++;applied[i]=1;}
 }
 if(aj6Current()){aj6Save(aj6Owner,aj6Profile,aj6State);printf("[ajustes-oficial] perfil %d: %d/%d campo(s) recebidos\n",aj6Profile,received,(int)AJ6_N);aj6Log("aplicados",applied,aj6Profile);aj6Log("invalidos (mantidos)",invalid,aj6Profile);aj6LogWaiting();}
 pthread_mutex_unlock(&aj6Lock);return changed;
}
static int aj6Pending(void){int n=0;pthread_mutex_lock(&aj6Lock);if(aj6Current()&&aj6Allowed())for(size_t i=0;i<AJ6_N;i++)n+=aj6State[i].pending;pthread_mutex_unlock(&aj6Lock);return n;}
/* Local runtime opt-in only. Never imported or inherited; default Off on each
 * context/restart. This stage adds no UI that grants account-write permission. */
int ajustes_oficiais_permitir_escritas(int enabled){pthread_mutex_lock(&aj6Lock);int ok=aj6Current()&&(enabled==0||enabled==1);if(ok)aj6Permission=enabled;pthread_mutex_unlock(&aj6Lock);return ok;}
static int aj6Merge(const char *base,char **out){
 *out=NULL;int n=0;Jsw patch;jsw_iniciar(&patch);jsw_obj_ini(&patch);jsw_chave(&patch,"features");jsw_obj_ini(&patch);pthread_mutex_lock(&aj6Lock);memset(aj6Sent,0,sizeof aj6Sent);
 if(aj6Current())aj6LogWaiting();
 if(aj6Current()&&aj6Allowed())for(size_t first=0;first<AJ6_N;first++){
  int repeated=0,opened=0;for(size_t j=0;j<first;j++)if(!strcmp(aj6Map[j].feature,aj6Map[first].feature))repeated=1;if(repeated)continue;
  for(size_t i=first;i<AJ6_N;i++)if(!strcmp(aj6Map[i].feature,aj6Map[first].feature)){
   Aj6Field *s=&aj6State[i];int remote=0,status=aj6Remote(base,i,&remote);
   if(!s->pending||s->hold||status==-2||(status==1&&(remote==s->desired||!s->known||remote!=s->base)))continue;
   if(!opened){jsw_chave(&patch,aj6Map[i].feature);jsw_obj_ini(&patch);opened=1;}
   if(aj6Encode(&patch,i,s->desired)){aj6Sent[i]=1;aj6SentValue[i]=s->desired;n++;}
  }if(opened)jsw_obj_fim(&patch);
 }
 if(n){unsigned char sent[AJ6_N];for(size_t i=0;i<AJ6_N;i++)sent[i]=(unsigned char)aj6Sent[i];aj6Log("enviando",sent,aj6Profile);}
 pthread_mutex_unlock(&aj6Lock);jsw_obj_fim(&patch);jsw_obj_fim(&patch);const char *s=jsw_texto_final(&patch);if(n&&s)*out=ajJsonMerge(base,base+strlen(base),s,s+strlen(s));jsw_livre(&patch);return *out?n:0;
}
static void aj6Ack(const char *sent){
 pthread_mutex_lock(&aj6Lock);if(aj6Current())for(size_t i=0;i<AJ6_N;i++){
  int n;if(!aj6Sent[i]||aj6Remote(sent,i,&n)!=1||n!=aj6SentValue[i])continue;Aj6Field *s=&aj6State[i];s->base=n;s->known=1;s->pending=!s->hold&&s->desired!=n;
 }if(aj6Current())aj6Save(aj6Owner,aj6Profile,aj6State);pthread_mutex_unlock(&aj6Lock);
}
static void aj6Forget(void){pthread_mutex_lock(&aj6Lock);aj6Ready=aj6Permission=0;aj6Owner[0]=0;aj6Profile=0;pthread_mutex_unlock(&aj6Lock);}

/* J5: sparse per-property overlays; Native files remain the recovery baseline.
 * All helpers except public entry points require fileiras' mutex. */
#include "homewire.h"
#include <errno.h>
static struct {char key[40];BnValue value;int pending;} homeOverrides[BN_MAX];
static size_t homeCount;
static char homeOwner[80];static int homeProfile;
static FilHomeObserver homeObserver;
/* A portable digest of the official catalogue key + duplicate-instance ordinal.
 * Never hash/send an endpoint or configured URL fingerprint. */
static int homeId(const char *local,char out[33]){
 char canonical[FIL_CHAVE+40];int source=-1,instance=0,family=0,byFingerprint=0;
 if(!local||!*local)return 0;
 if(fil_origem_de(local)!=FIL_ORIGEM_CATALOGO)snprintf(canonical,sizeof canonical,"app-or-collection:%s",local);
 else {
  for(int i=0;i<nAddonsConta;i++){size_t n=strlen(addonsConta[i].fp);if(n&&strcmp(addonsConta[i].fp,addonsConta[i].id)&&!strncmp(local,addonsConta[i].fp,n)&&local[n]=='_'){source=i;byFingerprint=1;break;}}
  if(source<0)for(int i=0;i<nAddonsConta;i++){size_t n=strlen(addonsConta[i].id);if(n&&!strncmp(local,addonsConta[i].id,n)&&local[n]=='_'){source=i;break;}}
  if(source<0||!addonsContaCompleta)return 0;
  for(int i=0;i<nAddonsConta;i++)if(!strcmp(addonsConta[i].id,addonsConta[source].id)){
   family++;if(strcmp(addonsConta[i].base,addonsConta[source].base)<0)instance++;
  }
  /* Rank uses the private complete account list only. No URL/hash leaves here.
   * Family size prevents a partial duplicate family masquerading as instance 0. */
  const char *suffix=local+strlen(byFingerprint?addonsConta[source].fp:addonsConta[source].id);
  if(snprintf(canonical,sizeof canonical,"catalogue:%s%s#%d/%d",addonsConta[source].id,suffix,instance,family)>=(int)sizeof canonical)return 0;
 }
 uint64_t a=UINT64_C(14695981039346656037),b=UINT64_C(7809847782465536322);
 for(const unsigned char *p=(const unsigned char *)canonical;*p;p++){a=(a^*p)*UINT64_C(1099511628211);b=(b^*p)*UINT64_C(14029467366897019727);}
 snprintf(out,33,"%016llx%016llx",(unsigned long long)a,(unsigned long long)b);return 1;
}
int fil_home_identity(const char *local,char out[33]){pthread_mutex_lock(&trava);int ok=homeId(local,out);pthread_mutex_unlock(&trava);return ok;}
static int homeSame(BnValue a,BnValue b){return a.type==b.type&&(a.type==BN_STRING?!strcmp(a.text,b.text):a.number==b.number);}
static int homeIndex(const char *key){for(size_t i=0;i<homeCount;i++)if(!strcmp(homeOverrides[i].key,key))return (int)i;return -1;}
static int homeGet(const char *local,char prop,BnValue *out){char id[33],key[40];if(!homeId(local,id))return 0;snprintf(key,sizeof key,"home_%s_%c",id,prop);int i=homeIndex(key);if(i<0)return 0;*out=homeOverrides[i].value;return 1;}
static int homeNumber(int i,char prop,int fallback){if(i>=0&&i<nLinhas&&((prop=='s'&&formaFixa(linhas[i].chave))||(prop=='o'&&fil_origem_de(linhas[i].chave)!=FIL_ORIGEM_COLECAO)))return fallback;BnValue v;return i>=0&&i<nLinhas&&homeGet(linhas[i].chave,prop,&v)?(int)v.number:fallback;}
static int homePut(const char *key,BnValue v,int pending){int i=homeIndex(key);if(i<0){if(homeCount==BN_MAX)return 0;i=(int)homeCount++;strcpy(homeOverrides[i].key,key);}homeOverrides[i].value=v;homeOverrides[i].pending=pending;return 1;}
static void homeSend(const char *key,BnValue v){
 /* Keep failed disk commits pending; never echo a remote projection. */
 if(homePut(key,v,1)&&homeObserver&&homeObserver(key,v)){
  homeOverrides[homeIndex(key)].pending=0;
  if(homeOwner[0]){char file[80];snprintf(file,sizeof file,"bn-home-owner-p%d.txt",perfil);dados_gravar(file,homeOwner);}
 }
}
static void homeEdit(int i,char prop){
 char id[33],key[40];if(i<0||i>=nLinhas||!homeId(linhas[i].chave,id))return;
 BnValue v={0};v.type=prop=='v'?BN_BOOL:BN_ENUM;
 v.number=prop=='s'?linhas[i].tipo:prop=='z'?linhas[i].tam:prop=='o'?linhas[i].ordem:linhas[i].oculta==OC_PESSOA;
 snprintf(key,sizeof key,"home_%s_%c",id,prop);homeSend(key,v);
}
static void homeAnchors(void){
 char previous[33]="start";
 for(int i=0;i<nLinhas;i++){
  char id[33];if(!homeId(linhas[i].chave,id))continue;
  if(!strcmp(linhas[i].chave,"social_activity")||!strcmp(linhas[i].chave,"last_session")){
   char key[40];BnValue v={.type=BN_STRING};strcpy(v.text,previous);snprintf(key,sizeof key,"home_%s_a",id);homeSend(key,v);
  }
  if(!linhaOculta(i))strcpy(previous,id);
 }
}
void fil_home_observer(FilHomeObserver fn){pthread_mutex_lock(&trava);homeObserver=fn;pthread_mutex_unlock(&trava);}
static void homeClear(void){homeCount=0;homeOwner[0]=0;homeProfile=0;}
void fil_home_scope(const char *owner,int p){
 pthread_mutex_lock(&trava);if(homeProfile!=p||strcmp(homeOwner,owner?owner:"")){homeClear();snprintf(homeOwner,sizeof homeOwner,"%s",owner?owner:"");homeProfile=p;revisao++;}pthread_mutex_unlock(&trava);
}
int fil_home_apply(const char *owner,int p,const BnField *fields,size_t n){
 if(!owner||(n&&!fields)||n>BN_MAX)return 0;pthread_mutex_lock(&trava);
 if(p!=perfil||p!=homeProfile||strcmp(owner,homeOwner)){pthread_mutex_unlock(&trava);return 0;}
 for(size_t j=0;j<n;j++)if(strcmp(fields[j].key,HOME_HERO)?!home_field(fields[j].key,fields[j].value):!home_reference(&fields[j].value)){pthread_mutex_unlock(&trava);return 0;}
 for(size_t i=0;i<homeCount;i++){if(homeOverrides[i].pending==2)homeOverrides[i].pending=0;else if(homeOverrides[i].pending&&homeObserver&&homeObserver(homeOverrides[i].key,homeOverrides[i].value))homeOverrides[i].pending=2;}
 int changed=0;
 for(size_t j=0;j<n;j++){
  const BnField *f=&fields[j];int i=homeIndex(f->key);if(i>=0&&homeOverrides[i].pending)continue;
  if(i<0||!homeSame(homeOverrides[i].value,f->value)){if(homePut(f->key,f->value,0))changed=1;}
 }
 if(changed)revisao++;pthread_mutex_unlock(&trava);return changed;
}
/* Reorder only the two exclusive anchors after the Stage G shared-order merge.
 * Missing targets retain the existing relative position. Continue Watching leads. */
static void homeReorder(const char *const *keys,int *order,int n){
 const char *exclusive[]={"social_activity","last_session"};
 for(int pass=0;pass<2;pass++)for(int e=0;e<2;e++){
  BnValue v;if(!homeGet(exclusive[e],'a',&v))continue;int at=-1,target=-1;
  for(int j=0;j<n;j++){
   if(!strcmp(keys[order[j]],exclusive[e]))at=j;
   char id[33];if(homeId(keys[order[j]],id)&&!strcmp(v.text,id))target=j;
  }
  if(!strcmp(v.text,"start")){target=-1;for(int j=0;j<n;j++)if(!strcmp(keys[order[j]],"continue_watching")){target=j;break;}}
  else if(target<0)continue;
  if(at<0||at==target)continue;
  /* A two-row cycle is invalid placement, not a reason to duplicate/drop rows. */
  BnValue back;char own[33];if(target>=0&&homeId(exclusive[e],own)&&homeGet(keys[order[target]],'a',&back)&&!strcmp(back.text,own))continue;
  int item=order[at];memmove(order+at,order+at+1,sizeof(int)*(size_t)(n-at-1));if(target>at)target--;int dest=target+1;memmove(order+dest+1,order+dest,sizeof(int)*(size_t)(n-1-dest));order[dest]=item;
 }
}

/* Same bounded local read boundary as carregar(); writes retain dados_gravar's
 * atomic/IDBFS path. Keep the isolated row module's existing link dependencies. */
static char *homeReadOwner(const char *name){
 char path[600],value[81];if(!dados_caminho(path,sizeof path,name))return strdup("");
 FILE *f=fopen(path,"rb");if(!f)return errno==ENOENT?NULL:strdup("");
 size_t n=fread(value,1,80,f);int bad=ferror(f)||n>=80;fclose(f);if(bad)return strdup("");value[n]=0;if(strlen(value)!=n)return strdup("");return strdup(value);
}
int fil_home_baseline(const char *owner,int p,BnField *rows,size_t cap,size_t *n){
 if(!owner||!rows||!n)return 0;*n=0;pthread_mutex_lock(&trava);if(p!=perfil||strcmp(homeOwner,owner)){pthread_mutex_unlock(&trava);return 0;}garantir();
 char file[80];snprintf(file,sizeof file,"bn-home-owner-p%d.txt",perfil);char *prior=homeReadOwner(file);int own=!prior||!strcmp(prior,owner);free(prior);
 for(int i=0;i<nLinhas;i++){
  char id[33];if(!homeId(linhas[i].chave,id))continue;
  const char props[]="szov";
  for(int j=0;j<4;j++){
   char prop=props[j];if((prop=='s'&&formaFixa(linhas[i].chave))||(prop=='o'&&fil_origem_de(linhas[i].chave)!=FIL_ORIGEM_COLECAO))continue;
   if(*n==cap){pthread_mutex_unlock(&trava);return 0;}BnField *f=&rows[(*n)++];memset(f,0,sizeof *f);snprintf(f->key,sizeof f->key,"home_%s_%c",id,prop);f->value.type=prop=='v'?BN_BOOL:BN_ENUM;
   f->value.number=own?(prop=='s'?linhas[i].tipo:prop=='z'?linhas[i].tam:prop=='o'?linhas[i].ordem:(contaEstado(linhas[i].chave)>=0?0:linhas[i].oculta==OC_PESSOA)):(prop=='z'?FIL_TAM_PADRAO:0);
  }
 }
 if(*n==cap){pthread_mutex_unlock(&trava);return 0;}BnField *h=&rows[(*n)++];memset(h,0,sizeof *h);strcpy(h->key,HOME_HERO);h->value.type=BN_STRING;
 if(!own||!heroFonte[0])strcpy(h->value.text,"auto");else if(!strcmp(heroFonte,"*"))strcpy(h->value.text,"random");else if(!homeId(heroFonte,h->value.text))strcpy(h->value.text,"auto");
 if(own){snprintf(file,sizeof file,"bn-home-owner-p%d.txt",perfil);dados_gravar(file,owner);}
 pthread_mutex_unlock(&trava);return 1;
}

#include "bncodec.h"
#include "jsw.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
/* Bounded strict JSON tree. Tokens retain input slices; next skips a subtree. */
#define TOKENS 98304 /* bounded 4096-field checkpoints, including local flags */
typedef struct {const char *a,*b;int next,child;} Tok;
typedef struct {Tok t[TOKENS];int n;const char *p,*end;} Json;
static int hex4(const char *p,unsigned *v){*v=0;for(int i=0;i<4;i++){unsigned c=(unsigned char)p[i];int n=c>='0'&&c<='9'?(int)c-'0':c>='a'&&c<='f'?(int)c-'a'+10:c>='A'&&c<='F'?(int)c-'A'+10:-1;if(n<0)return 0;*v=(*v<<4)|(unsigned)n;}return 1;}
static int utf8(const unsigned char *p){
 while(*p){unsigned c=*p++,cp,n,min;if(c<128)continue;
  if(c>=0xc2&&c<=0xdf){n=1;cp=c&31;min=128;}else if(c>=0xe0&&c<=0xef){n=2;cp=c&15;min=2048;}else if(c>=0xf0&&c<=0xf4){n=3;cp=c&7;min=65536;}else return 0;
  for(unsigned i=0;i<n;i++){unsigned b=*p++;if(b<128||b>191)return 0;cp=(cp<<6)|(b&63);}if(cp<min||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff))return 0;
 }return 1;
}
static int unquote(const char *a,const char *b,char *out,size_t cap){
 if(b-a<2||*a!='"'||b[-1]!='"'||!cap)return 0;size_t n=0;a++;b--;
 while(a<b){unsigned cp=(unsigned char)*a++;char bytes[4];size_t count=1;bytes[0]=(char)cp;
  if(cp=='\\'){
   if(a==b)return 0;unsigned c=(unsigned char)*a++;
   if(c=='u'){
    if(b-a<4||!hex4(a,&cp))return 0;a+=4;
    if(cp>=0xd800&&cp<=0xdbff){unsigned low;if(b-a<6||a[0]!='\\'||a[1]!='u'||!hex4(a+2,&low)||low<0xdc00||low>0xdfff)return 0;a+=6;cp=0x10000+((cp-0xd800)<<10)+low-0xdc00;}
    else if(cp>=0xdc00&&cp<=0xdfff)return 0;
    if(cp<128){bytes[0]=(char)cp;count=1;}
    else if(cp<2048){bytes[0]=(char)(0xc0|(cp>>6));bytes[1]=(char)(0x80|(cp&63));count=2;}
    else if(cp<65536){bytes[0]=(char)(0xe0|(cp>>12));bytes[1]=(char)(0x80|((cp>>6)&63));bytes[2]=(char)(0x80|(cp&63));count=3;}
    else {bytes[0]=(char)(0xf0|(cp>>18));bytes[1]=(char)(0x80|((cp>>12)&63));bytes[2]=(char)(0x80|((cp>>6)&63));bytes[3]=(char)(0x80|(cp&63));count=4;}
   }else {const char *esc="\"\\/bfnrt",*at=strchr(esc,(int)c);if(!at)return 0;const char decoded[]={'"','\\','/','\b','\f','\n','\r','\t'};bytes[0]=decoded[at-esc];}
  }else if(cp=='"'||cp<32)return 0;
  // Embedded NUL cannot be represented by the engine's C strings.
  if(!bytes[0]||n+count>=cap)return 0;memcpy(out+n,bytes,count);n+=count;
 }
 out[n]=0;return utf8((const unsigned char *)out);
}
static void space(Json *j){while(j->p<j->end&&strchr(" \n\r\t",*j->p))j->p++;}
static int parse(Json *j,int depth){
 space(j);if(j->p==j->end||depth>24||j->n==TOKENS)return -1;
 int n=j->n++;Tok *t=&j->t[n];t->a=j->p;t->child=-1;
 char c=*j->p++;
 if(c=='{'||c=='['){
  space(j);if(j->p<j->end&&*j->p==(c=='{'?'}':']'))j->p++;
  else {t->child=j->n;while(1){
   if(c=='{'){
    int k=parse(j,depth+1);char key[512],prev[512];
    if(k<0||*j->t[k].a!='"'||!unquote(j->t[k].a,j->t[k].b,key,sizeof key))return -1;
    for(int old=t->child;old<k;){
     if(!unquote(j->t[old].a,j->t[old].b,prev,sizeof prev)||!strcmp(prev,key))return -1;
     int val=j->t[old].next;old=j->t[val].next;
    }
    space(j);if(j->p==j->end||*j->p++!=':')return -1;
   }
   if(parse(j,depth+1)<0)return -1;
   space(j);if(j->p==j->end)return -1;
   if(*j->p==(c=='{'?'}':']')){j->p++;break;}
   if(*j->p++!=',')return -1;
  }}
 }else if(c=='"'){
  int closed=0;while(j->p<j->end){unsigned char b=(unsigned char)*j->p++;if(b=='"'){closed=1;break;}if(b<32)return -1;
   if(b=='\\'){if(j->p==j->end)return -1;b=(unsigned char)*j->p++;if(b=='u'){for(int i=0;i<4;i++)if(j->p==j->end||!isxdigit((unsigned char)*j->p++))return -1;}else if(!strchr("\"\\/bfnrt",b))return -1;}}
  if(!closed)return -1;
  size_t cap=(size_t)(j->p-t->a)+1;char *decoded=malloc(cap);if(!decoded)return -1;
  int valid=unquote(t->a,j->p,decoded,cap);free(decoded);if(!valid)return -1;
 }else{
  const char *a=t->a;while(j->p<j->end&&!strchr(",}] \n\r\t",*j->p))j->p++;
  size_t len=(size_t)(j->p-a);
  int literal=(len==4&&(!memcmp(a,"true",4)||!memcmp(a,"null",4)))||(len==5&&!memcmp(a,"false",5));
  if(!literal){const char *p=a;if(p<j->p&&*p=='-')p++;if(p==j->p||!isdigit((unsigned char)*p))return -1;
   if(*p=='0')p++;else while(p<j->p&&isdigit((unsigned char)*p))p++;
   if(p<j->p&&*p=='.'){p++;const char *start=p;while(p<j->p&&isdigit((unsigned char)*p))p++;if(p==start)return -1;}
   if(p<j->p&&(*p=='e'||*p=='E')){p++;if(p<j->p&&(*p=='+'||*p=='-'))p++;const char *start=p;while(p<j->p&&isdigit((unsigned char)*p))p++;if(p==start)return -1;}
   if(p!=j->p)return -1;
  }
 }
 t->b=j->p;t->next=j->n;return n;
}
static Json *json(const char *p){
 if(!p||strlen(p)>BN_LIMIT+4096)return NULL;
 Json *j=calloc(1,sizeof *j);if(!j)return NULL;j->p=p;j->end=p+strlen(p);
 if(parse(j,0)!=0){free(j);return NULL;}space(j);if(j->p!=j->end){free(j);return NULL;}return j;
}
static int get(Json *j,int n,const char *key){
 if(n<0||*j->t[n].a!='{')return -1;
 for(int k=j->t[n].child;k>=0&&k<j->t[n].next;){char text[512];int v=j->t[k].next;
  if(!unquote(j->t[k].a,j->t[k].b,text,sizeof text))return -1;if(!strcmp(text,key))return v;k=j->t[v].next;}
 return -1;
}
static int only(Json *j,int n,const char *const *keys,size_t count){
 if(n<0||*j->t[n].a!='{')return 0;
 for(int k=j->t[n].child;k>=0&&k<j->t[n].next;){char text[512];int found=0;if(!unquote(j->t[k].a,j->t[k].b,text,sizeof text))return 0;
  for(size_t i=0;i<count;i++)found|=!strcmp(text,keys[i]);if(!found)return 0;k=j->t[j->t[k].next].next;}
 return 1;
}
static int string(Json *j,int n,char *out,size_t cap){return n>=0&&*j->t[n].a=='"'&&unquote(j->t[n].a,j->t[n].b,out,cap);}
static int integer(Json *j,int n,int64_t *out){
 if(n<0)return 0;const Tok *t=&j->t[n];char *end;errno=0;long long v=strtoll(t->a,&end,10);
 if(errno||end!=t->b||end==t->a)return 0;*out=v;return 1;
}
static int boolean(Json *j,int n,int *out){
 if(n<0)return 0;const Tok *t=&j->t[n];if(t->b-t->a==4&&!memcmp(t->a,"true",4)){*out=1;return 1;}
 if(t->b-t->a==5&&!memcmp(t->a,"false",5)){*out=0;return 1;}return 0;
}
static int field(Json *j,int n,BnField *f,int local){
 const char *keys[]={"type","value","rev","device","key","known","dirty"};char type[16];int64_t rev;
 if(!only(j,n,keys,local?7:4)||!string(j,get(j,n,"type"),type,sizeof type)||!integer(j,get(j,n,"rev"),&rev)||rev<0||!string(j,get(j,n,"device"),f->device,sizeof f->device))return 0;
 const char *types[]={"","bool","int","enum","string"};for(int i=1;i<=4;i++)if(!strcmp(type,types[i]))f->value.type=(BnType)i;
 int v=get(j,n,"value"),b;if(!f->value.type)return 0;f->rev=(uint64_t)rev;
 if(f->value.type==BN_STRING)return string(j,v,f->value.text,sizeof f->value.text);
 if(f->value.type==BN_BOOL){if(!boolean(j,v,&b))return 0;f->value.number=b;return 1;}
 return integer(j,v,&f->value.number);
}
/* Stage J7b: the two historical J0 probe documents (src/j0sonda.c), and only
 * those, mean "Better Nuvio sync not initialised yet": exactly
 *   {"schema":"better-nuvio-probe","version":0,"probe":"<8 uppercase hex>"}
 * (J0 write) or {} (J0 clean). Nothing is applied and nothing is written because
 * of it; the remote document stays until an armed upload with real local
 * changes replaces it. Any other unknown or malformed document is still refused. */
static int members(Json *j,int n){int c=0;for(int k=j->t[n].child;k>=0&&k<j->t[n].next;k=j->t[j->t[k].next].next)c++;return c;}
static int legacyJ0(Json *j,int doc){
 if(*j->t[doc].a!='{')return 0;
 int m=members(j,doc);if(m==0)return 1;
 const char *keys[]={"schema","version","probe"};char schema[32],probe[16];int64_t version;
 if(m!=3||!only(j,doc,keys,3)||!string(j,get(j,doc,"schema"),schema,sizeof schema)||strcmp(schema,"better-nuvio-probe")||
    !integer(j,get(j,doc,"version"),&version)||version!=0||!string(j,get(j,doc,"probe"),probe,sizeof probe)||strlen(probe)!=8)return 0;
 for(int i=0;i<8;i++)if(!strchr("0123456789ABCDEF",probe[i]))return 0;
 return 1;
}
int bnc_legacy_j0(const char *raw){
 Json *j=json(raw);if(!j)return 0;int r=0;
 if(*j->t[0].a=='['){int row=j->t[0].child;if(row>=0&&j->t[row].next==j->t[0].next&&*j->t[row].a=='{'){int doc=get(j,row,"settings_json");r=doc>=0&&legacyJ0(j,doc);}}
 free(j);return r;
}
int bnc_response(const BnEngine *e,const char *raw,BnDoc *out){
 Json *j=json(raw);if(!j)return 0;int ok=0;memset(out,0,sizeof *out);strcpy(out->schema,BN_SCHEMA);out->version=BN_VERSION;strcpy(out->owner,e->state.owner);out->profile=e->state.profile;
 if(*j->t[0].a!='[')goto done;int row=j->t[0].child;
 if(row<0){ok=1;goto done;}
 if(j->t[row].next!=j->t[0].next||*j->t[row].a!='{')goto done;
 int64_t profile;int pn=get(j,row,"profile_id");if(pn>=0&&(!integer(j,pn,&profile)||profile!=e->state.profile))goto done;
 int doc=get(j,row,"settings_json");if(doc<0)goto done;
 if(j->t[doc].b-j->t[doc].a==4&&!memcmp(j->t[doc].a,"null",4)){ok=1;goto done;}
 if(legacyJ0(j,doc)){ok=1;goto done;}
 if(j->t[doc].b-j->t[doc].a>BN_LIMIT)goto done;
 const char *keys[]={"schema","version","fields"};int64_t version;char schema[32];
 if(!only(j,doc,keys,3)||!string(j,get(j,doc,"schema"),schema,sizeof schema)||strcmp(schema,BN_SCHEMA)||!integer(j,get(j,doc,"version"),&version)||version!=BN_VERSION)goto done;
 int fields=get(j,doc,"fields");if(fields<0||*j->t[fields].a!='{')goto done;
 for(int k=j->t[fields].child;k>=0&&k<j->t[fields].next;){
  if(out->count==BN_MAX)goto done;BnField *f=&out->fields[out->count++];
  int v=j->t[k].next;if(!string(j,k,f->key,sizeof f->key)||!field(j,v,f,0))goto done;k=j->t[v].next;
 }
 ok=1;
 done:free(j);return ok;
}
static void writefield(Jsw *w,const BnField *f){
 const char *types[]={"","bool","int","enum","string"};jsw_cs(w,"type",types[f->value.type]);jsw_chave(w,"value");
 if(f->value.type==BN_STRING)jsw_str(w,f->value.text);else if(f->value.type==BN_BOOL)jsw_bool(w,(int)f->value.number);else jsw_int(w,f->value.number);
 jsw_ci(w,"rev",(long long)f->rev);jsw_cs(w,"device",f->device);
}
int bnc_save(const BnEngine *e,int writes,char **out){
 *out=NULL;if(writes!=0&&writes!=1)return 0;BnEngine *check=malloc(sizeof *check);if(!check)return 0;*check=*e;
 if(!bn_restore(check,&e->state)){free(check);return 0;}free(check);
 Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);jsw_cs(&w,"schema","better-nuvio-checkpoint");jsw_ci(&w,"version",1);jsw_cs(&w,"owner",e->state.owner);jsw_ci(&w,"profile",e->state.profile);jsw_cs(&w,"device",e->state.device);jsw_cb(&w,"writes",writes);
 jsw_chave(&w,"slots");jsw_arr_ini(&w);
 for(size_t i=0;i<e->state.count;i++){const BnSlot *s=&e->state.slots[i];jsw_obj_ini(&w);jsw_cs(&w,"key",s->current.key);writefield(&w,&s->current);jsw_cb(&w,"known",s->known);jsw_cb(&w,"dirty",s->dirty);jsw_obj_fim(&w);}
 jsw_arr_fim(&w);jsw_obj_fim(&w);const char *p=jsw_texto_final(&w);if(p&&strlen(p)<=BN_LIMIT)*out=strdup(p);jsw_livre(&w);return *out!=NULL;
}
int bnc_load(BnEngine *e,const char *raw,int *writes){
 if(!raw||strlen(raw)>BN_LIMIT)return 0;Json *j=json(raw);if(!j)return 0;BnCheckpoint *c=calloc(1,sizeof *c);if(!c){free(j);return 0;}int ok=0,enabled=0;int64_t n;char schema[40];
 const char *keys[]={"schema","version","owner","profile","device","writes","slots"};
 if(!only(j,0,keys,7)||!string(j,get(j,0,"schema"),schema,sizeof schema)||strcmp(schema,"better-nuvio-checkpoint")||!integer(j,get(j,0,"version"),&n)||n!=1)goto done;c->version=1;
 if(!string(j,get(j,0,"owner"),c->owner,sizeof c->owner)||!string(j,get(j,0,"device"),c->device,sizeof c->device)||!integer(j,get(j,0,"profile"),&n)||n<1||n>2147483647||!boolean(j,get(j,0,"writes"),&enabled))goto done;c->profile=(int)n;
 int slots=get(j,0,"slots");if(slots<0||*j->t[slots].a!='[')goto done;
 for(int k=j->t[slots].child;k>=0&&k<j->t[slots].next;k=j->t[k].next){if(c->count==BN_MAX)goto done;BnSlot *s=&c->slots[c->count++];if(!string(j,get(j,k,"key"),s->current.key,sizeof s->current.key)||!field(j,k,&s->current,1)||!boolean(j,get(j,k,"known"),&s->known)||!boolean(j,get(j,k,"dirty"),&s->dirty))goto done;}
 if(bn_restore(e,c)){*writes=enabled;ok=1;}
 done:free(c);free(j);return ok;
}

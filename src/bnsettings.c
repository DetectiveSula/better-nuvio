#include "bnsettings.h"
#include "jsw.h"
#include "homewire.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
static int text(const char *p,size_t cap) { return strnlen(p,cap)<cap; }
static int identity(const char *p,size_t cap) {
  if (!text(p,cap)||!*p) return 0;
  for (;*p;p++) if (!isalnum((unsigned char)*p)&&*p!='-'&&*p!='_') return 0;
  return 1;
}
static int safe(const char *key) {
  char lower[80]; if (!identity(key,sizeof lower)) return 0;
  size_t i; for(i=0;key[i];i++) lower[i]=(char)tolower((unsigned char)key[i]); lower[i]=0;
  const char *deny[]={"token","secret","password","credential","apikey","api_key","endpoint","url","decoder","tunneling","buffer","hardware","device","audiooffset"};
  for(i=0;i<sizeof deny/sizeof *deny;i++) if(strstr(lower,deny[i])) return 0;
  return 1;
}
static int utf8(const unsigned char *p) {
  while(*p) {
    unsigned c=*p++,cp,n,min;
    if(c<128) {if(c<32)return 0;continue;}
    if(c>=0xc2&&c<=0xdf) {n=1;cp=c&31;min=0x80;}
    else if(c>=0xe0&&c<=0xef) {n=2;cp=c&15;min=0x800;}
    else if(c>=0xf0&&c<=0xf4) {n=3;cp=c&7;min=0x10000;}
    else return 0;
    for(unsigned i=0;i<n;i++) {unsigned b=*p++;if(b<0x80||b>0xbf)return 0;cp=(cp<<6)|(b&63);}
    if(cp<min||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff))return 0;
  }
  return 1;
}
static int value(BnValue v) {
  if(v.type<BN_BOOL||v.type>BN_STRING||!text(v.text,sizeof v.text)) return 0;
  if(v.type==BN_STRING) return utf8((const unsigned char *)v.text) && !strstr(v.text,"://") && !strchr(v.text,'\n') && !strchr(v.text,'\r');
  return !*v.text && (v.type!=BN_BOOL||(v.number==0||v.number==1));
}
static int same(BnValue a,BnValue b) { return a.type==b.type && (a.type==BN_STRING?!strcmp(a.text,b.text):a.number==b.number); }
static int stamp(BnField a,BnField b) { return a.rev>b.rev?1:a.rev<b.rev?-1:strcmp(a.device,b.device); }
static int indexof(const BnEngine *e,const char *key) { for(size_t i=0;i<e->state.count;i++) if(!strcmp(e->state.slots[i].current.key,key)) return (int)i; return -1; }
static int defof(const BnEngine *e,const char *key) { for(size_t i=0;i<e->definitions;i++) if(!strcmp(e->defs[i].key,key)) return (int)i; return -1; }
static int allowed(const BnEngine *e,const char *key,BnValue v) {
  if(!safe(key)||!value(v)) return 0;
  int i=defof(e,key); if(i<0) return defof(e,HOME_MARKER)>=0 && (!strcmp(key,HOME_HERO)?home_reference(&v):home_field(key,v)); const BnDef *d=&e->defs[i];
  return d->portable && v.type==d->type && (!d->accept||d->accept(&v)) &&
    (v.type==BN_STRING || (v.number>=d->min && v.number<=d->max));
}
static int docok(const BnEngine *e,const BnDoc *d) {
  if(!text(d->schema,sizeof d->schema)||strcmp(d->schema,BN_SCHEMA)||d->version!=BN_VERSION||
     !text(d->owner,sizeof d->owner)||strcmp(d->owner,e->state.owner)||d->profile!=e->state.profile||d->count>BN_MAX) return 0;
  for(size_t i=0;i<d->count;i++) {
    const BnField *f=&d->fields[i];
    if(!allowed(e,f->key,f->value)||!f->rev||f->rev>INT64_MAX||!identity(f->device,sizeof f->device)) return 0;
    for(size_t j=0;j<i;j++) if(!strcmp(f->key,d->fields[j].key)) return 0;
  }
  return 1;
}
int bn_init(BnEngine *e,const char *owner,int profile,const char *device,const BnDef *defs,size_t n) {
  if(!e||!owner||!device||!identity(owner,80)||!identity(device,65)||profile<=0||!defs||n>BN_MAX) return 0;
  memset(e,0,sizeof *e); e->state.version=BN_VERSION; e->state.profile=profile;
  strcpy(e->state.owner,owner);strcpy(e->state.device,device);
  for(size_t i=0;i<n;i++) {
    if(!safe(defs[i].key)||defs[i].portable!=1||defs[i].type!=defs[i].initial.type||defs[i].min>defs[i].max) return 0;
    for(size_t j=0;j<i;j++) if(!strcmp(defs[i].key,defs[j].key)) return 0;
    e->defs[e->definitions++]=defs[i];
    if(!allowed(e,defs[i].key,defs[i].initial)) return 0;
    BnSlot *s=&e->state.slots[e->state.count++];strcpy(s->current.key,defs[i].key);s->current.value=defs[i].initial;
    strcpy(s->current.device,device);
  }
  return 1;
}
const BnValue *bn_get(const BnEngine *e,const char *key) { int i=indexof(e,key);return i>=0&&allowed(e,key,e->state.slots[i].current.value)?&e->state.slots[i].current.value:NULL; }
int bn_edit(BnEngine *e,const char *key,BnValue v) {
  int s=indexof(e,key); if(!allowed(e,key,v)) return 0;
  if(s<0){if(e->state.count==BN_MAX)return 0;s=(int)e->state.count++;memset(&e->state.slots[s],0,sizeof(BnSlot));strcpy(e->state.slots[s].current.key,key);strcpy(e->state.slots[s].current.device,e->state.device);}
  BnSlot *p=&e->state.slots[s]; if(same(p->current.value,v)) return 1;
  if(p->current.rev>=INT64_MAX) return 0;
  p->current.rev++;p->current.value=v;strcpy(p->current.device,e->state.device);p->dirty=1;
  e->fresh=0;return 1;
}
int bn_pull(BnEngine *e,const BnDoc *d) {
  e->fresh=0; if(!docok(e,d)) return 0;
  /* Transactional validation: equivocation/capacity must not partially apply. */
  size_t added=0;
  for(size_t i=0;i<d->count;i++) {
    int s=indexof(e,d->fields[i].key);
    if(s<0) added++;
    else if((e->state.slots[s].known||e->state.slots[s].dirty) && !stamp(d->fields[i],e->state.slots[s].current) && !same(d->fields[i].value,e->state.slots[s].current.value)) return 0;
  }
  if(e->state.count+added>BN_MAX) return 0;
  for(size_t j=0;j<e->state.count;j++) if(e->state.slots[j].known) {
    int found=0;
    for(size_t i=0;i<d->count;i++) found|=!strcmp(e->state.slots[j].current.key,d->fields[i].key);
    if(!found)e->state.slots[j].dirty=1; // A missing value is not a tombstone.
  }
  for(size_t i=0;i<d->count;i++) {
    int n=indexof(e,d->fields[i].key);
    if(n<0) { n=(int)e->state.count++;memset(&e->state.slots[n],0,sizeof(BnSlot)); }
    BnSlot *s=&e->state.slots[n];
    if((!s->known&&!s->dirty)||stamp(d->fields[i],s->current)>0) { s->current=d->fields[i];s->known=1;s->dirty=0; }
    else if(!stamp(d->fields[i],s->current)) { s->known=1;s->dirty=0; }
    else s->dirty=1; // Repair a lower-revision server snapshot from retained state.
  }
  e->fresh=1;return 1;
}
static void docinit(const BnEngine *e,BnDoc *d) { memset(d,0,sizeof *d);strcpy(d->schema,BN_SCHEMA);d->version=BN_VERSION;strcpy(d->owner,e->state.owner);d->profile=e->state.profile; }
int bn_prepare(BnEngine *e,BnDoc *d) {
  if(!e->fresh) return 0;
  docinit(e,d); int pending=0;
  for(size_t i=0;i<e->state.count;i++) {
    const BnSlot *s=&e->state.slots[i];pending|=s->dirty;
    if(s->known||s->dirty) d->fields[d->count++]=s->current;
  }
  if(!pending) return 0;
  e->prepared=*d;e->fresh=0;return 1;
}
int bn_ack(BnEngine *e,const BnDoc *d) {
  if(!docok(e,d)||!docok(e,&e->prepared)||d->count!=e->prepared.count) return 0;
  for(size_t i=0;i<d->count;i++) {
    int found=0;
    for(size_t j=0;j<e->prepared.count;j++) if(!strcmp(d->fields[i].key,e->prepared.fields[j].key) &&
      !stamp(d->fields[i],e->prepared.fields[j]) && same(d->fields[i].value,e->prepared.fields[j].value)) found=1;
    if(!found)return 0;
  }
  for(size_t i=0;i<d->count;i++) {
    int n=indexof(e,d->fields[i].key);if(n<0) continue;
    BnSlot *s=&e->state.slots[n];
    if(!stamp(s->current,d->fields[i])&&same(s->current.value,d->fields[i].value)) {s->dirty=0;s->known=1;}
  }
  memset(&e->prepared,0,sizeof e->prepared);return 1;
}
void bn_failed(BnEngine *e) {e->fresh=0;memset(&e->prepared,0,sizeof e->prepared);}
int bn_checkpoint(const BnEngine *e,BnCheckpoint *out) {*out=e->state;return 1;}
int bn_restore(BnEngine *e,const BnCheckpoint *c) {
  if(c->version!=BN_VERSION||c->profile!=e->state.profile||!text(c->owner,sizeof c->owner)||strcmp(c->owner,e->state.owner)||
     !text(c->device,sizeof c->device)||strcmp(c->device,e->state.device)||c->count>BN_MAX) return 0;
  for(size_t i=0;i<c->count;i++) {
    const BnSlot *s=&c->slots[i];
    if(!allowed(e,s->current.key,s->current.value)||!identity(s->current.device,sizeof s->current.device)||
       (s->known!=0&&s->known!=1)||(s->dirty!=0&&s->dirty!=1)||s->current.rev>INT64_MAX||((s->known||s->dirty)?!s->current.rev:s->current.rev!=0)) return 0;
    for(size_t j=0;j<i;j++) if(!strcmp(c->slots[j].current.key,s->current.key)) return 0;
  }
  /* Add newly reviewed definitions as unobserved local values, never edits.
   * Existing records were validated above: unknown/removed keys still fail. */
  BnCheckpoint *next=calloc(1,sizeof *next);if(!next)return 0;
  next->version=BN_VERSION;next->profile=c->profile;strcpy(next->owner,c->owner);strcpy(next->device,c->device);
  for(size_t i=0;i<e->definitions;i++) {
    BnSlot *s=&next->slots[next->count++];strcpy(s->current.key,e->defs[i].key);
    s->current.value=e->defs[i].initial;strcpy(s->current.device,c->device);
    for(size_t j=0;j<c->count;j++)if(!strcmp(c->slots[j].current.key,s->current.key))*s=c->slots[j];
  }
  for(size_t j=0;j<c->count;j++)if(defof(e,c->slots[j].current.key)<0){if(next->count==BN_MAX){free(next);return 0;}next->slots[next->count++]=c->slots[j];}
  e->state=*next;free(next);e->fresh=0;memset(&e->prepared,0,sizeof e->prepared);return 1;
}
int bn_json(const BnEngine *e,const BnDoc *d,char **out) {
  *out=NULL;if(!docok(e,d)) return 0;
  Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);jsw_cs(&w,"schema",BN_SCHEMA);jsw_ci(&w,"version",BN_VERSION);
  jsw_chave(&w,"fields");jsw_obj_ini(&w);
  static const char *types[]={"","bool","int","enum","string"};
  for(size_t i=0;i<d->count;i++) {
    const BnField *f=&d->fields[i]; if(f->rev>INT64_MAX){jsw_livre(&w);return 0;}
    jsw_chave(&w,f->key);jsw_obj_ini(&w);jsw_cs(&w,"type",types[f->value.type]);jsw_chave(&w,"value");
    if(f->value.type==BN_STRING) jsw_str(&w,f->value.text);else if(f->value.type==BN_BOOL)jsw_bool(&w,(int)f->value.number);else jsw_int(&w,f->value.number);
    jsw_ci(&w,"rev",(long long)f->rev);jsw_cs(&w,"device",f->device);jsw_obj_fim(&w);
  }
  jsw_obj_fim(&w);jsw_obj_fim(&w);const char *p=jsw_texto_final(&w);
  if(p&&strlen(p)<=BN_LIMIT)*out=strdup(p);jsw_livre(&w);return *out!=NULL;
}

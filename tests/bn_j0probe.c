/* Stage J7b: only the two historical J0 probe documents count as "Better Nuvio
 * sync not initialised"; every other unknown or malformed document is refused. */
#include "bncodec.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static BnDef defs[2];
static char *row(const char *doc){char *r=malloc(strlen(doc)+64);assert(r);sprintf(r,"[{\"profile_id\":1,\"settings_json\":%s}]",doc);return r;}
static int accepted(BnEngine *e,const char *doc,size_t *count){BnDoc d;char *r=row(doc);int ok=bnc_response(e,r,&d);if(ok&&count)*count=d.count;free(r);return ok;}
static int legacy(const char *doc){char *r=row(doc);int ok=bnc_legacy_j0(r);free(r);return ok;}
int main(void){
 strcpy(defs[0].key,"testCount");defs[0].type=BN_INT;defs[0].max=99;defs[0].portable=1;defs[0].initial.type=BN_INT;
 strcpy(defs[1].key,"testMode");defs[1].type=BN_BOOL;defs[1].max=1;defs[1].portable=1;defs[1].initial.type=BN_BOOL;
 BnEngine *e=calloc(1,sizeof *e);assert(e&&bn_init(e,"owner1",1,"deviceA",defs,2));
 /* Exactly what src/j0sonda.c wrote, as the server may return it (any key order/spacing). */
 const char *probes[]={"{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\"}",
  "{\"probe\": \"FFFFFFFF\", \"schema\": \"better-nuvio-probe\", \"version\": 0}","{}","{ }"};
 for(size_t i=0;i<sizeof probes/sizeof *probes;i++){size_t n=99;assert(accepted(e,probes[i],&n)&&n==0);assert(legacy(probes[i]));}
 /* Near misses and anything else stay refused. */
 const char *bad[]={
  "{\"schema\":\"better-nuvio-probe\",\"version\":0}",                                   /* missing probe */
  "{\"schema\":\"better-nuvio-probe\",\"probe\":\"0A1B2C3D\"}",                          /* missing version */
  "{\"version\":0,\"probe\":\"0A1B2C3D\"}",                                              /* missing schema */
  "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\",\"x\":1}",     /* extra member */
  "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\",\"fields\":{}}",
  "{\"schema\":\"better-nuvio-probe\",\"version\":1,\"probe\":\"0A1B2C3D\"}",            /* other version */
  "{\"schema\":\"better-nuvio-probe\",\"version\":0.0,\"probe\":\"0A1B2C3D\"}",
  "{\"schema\":\"better-nuvio-probe\",\"version\":\"0\",\"probe\":\"0A1B2C3D\"}",
  "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0a1b2c3d\"}",            /* lowercase */
  "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3\"}",             /* 7 chars */
  "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D4\"}",           /* 9 chars */
  "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3G\"}",            /* not hex */
  "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":12345678}",
  "{\"schema\":\"better-nuvio-probe \",\"version\":0,\"probe\":\"0A1B2C3D\"}",
  "{\"schema\":\"Better-Nuvio-Probe\",\"version\":0,\"probe\":\"0A1B2C3D\"}",
  "{\"schema\":\"better-nuvio-prob\\u0065\",\"version\":0,\"probe\":\"0A1B2C3D\",\"y\":0}",
  "{\"schema\":\"better-nuvio-probe\",\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\"}", /* duplicate */
  "{\"schema\":\"something-else\",\"version\":0,\"probe\":\"0A1B2C3D\"}",                /* unknown schema */
  "{\"schema\":\"better-nuvio-v2\",\"version\":1,\"fields\":{}}",
  "{\"schema\":\"better-nuvio-profile\",\"version\":2,\"fields\":{}}",
  "{\"anything\":true}","[]","[{}]","\"better-nuvio-probe\"","0","true",
 };
 for(size_t i=0;i<sizeof bad/sizeof *bad;i++){if(accepted(e,bad[i],NULL)||legacy(bad[i])){fprintf(stderr,"accepted: %s\n",bad[i]);return 1;}}
 /* Malformed JSON around a probe is refused before any recognition. */
 const char *broken[]={"[{\"settings_json\":{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\"}]",
  "[{\"settings_json\":{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\"}}]junk",
  "[{\"settings_json\":{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\",}}]",
  "[{\"profile_id\":2,\"settings_json\":{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"0A1B2C3D\"}}]", /* other profile */
  "[{\"settings_json\":{}},{\"settings_json\":{}}]"};
 BnDoc d;for(size_t i=0;i<sizeof broken/sizeof *broken;i++){if(bnc_response(e,broken[i],&d)){fprintf(stderr,"accepted broken %zu\n",i);return 1;}assert(i==3||!bnc_legacy_j0(broken[i]));}
 /* A real profile document is unaffected and is not reported as legacy. */
 const char *real="[{\"settings_json\":{\"schema\":\"better-nuvio-profile\",\"version\":1,\"fields\":{\"testCount\":{\"type\":\"int\",\"value\":7,\"rev\":1,\"device\":\"peer\"}}}}]";
 size_t n=0;assert(bnc_response(e,real,&d)&&d.count==1&&!bnc_legacy_j0(real));
 /* First sync over a probe: nothing applied, nothing pending, so nothing to upload. */
 char *r=row(probes[0]);assert(bnc_response(e,r,&d)&&bn_pull(e,&d));free(r);BnDoc out;assert(!bn_prepare(e,&out));(void)n;
 BnValue v={.type=BN_INT,.number=5};assert(bn_edit(e,"testCount",v));r=row(probes[0]);assert(bnc_response(e,r,&d)&&bn_pull(e,&d));free(r);
 assert(bn_prepare(e,&out)&&out.count==1&&!strcmp(out.schema,"better-nuvio-profile"));
 printf("PASS: J0 probe and J0 clean ({}) read as uninitialised; %zu near-miss/unknown/malformed documents and other-profile rows refused; real documents unchanged; a probe causes no upload until a real local edit\n",sizeof bad/sizeof *bad+sizeof broken/sizeof *broken);
 free(e);return 0;
}

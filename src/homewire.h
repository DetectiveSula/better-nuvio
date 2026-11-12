/* J5's closed, portable field family. No URLs, names, list snapshots or assets. */
#ifndef NV_HOMEWIRE_H
#define NV_HOMEWIRE_H
#include "bnsettings.h"
#include "fileiras.h"
#include <string.h>
#define HOME_HERO "homeHeroSource"
#define HOME_MARKER "homePresentationVersion"
static inline int home_digest(const char *s){if(!s||strlen(s)!=32)return 0;for(int i=0;i<32;i++)if(!strchr("0123456789abcdef",s[i]))return 0;return 1;}
static inline int home_reference(const BnValue *v){return v->type==BN_STRING&&(!strcmp(v->text,"auto")||!strcmp(v->text,"random")||home_digest(v->text));}
static inline int home_field(const char *k,BnValue v){
 if(strlen(k)!=39||strncmp(k,"home_",5)||k[37]!='_')return 0;
 char id[33];memcpy(id,k+5,32);id[32]=0;if(!home_digest(id))return 0;
 switch(k[38]){
 case 's':return v.type==BN_ENUM&&v.number>=0&&v.number<FIL_TIPO_N;
 case 'z':return v.type==BN_ENUM&&v.number>=0&&v.number<FIL_TAM_N;
 case 'o':return v.type==BN_ENUM&&v.number>=0&&v.number<FIL_ORD_N;
 case 'v':return v.type==BN_BOOL&&(v.number==0||v.number==1);
 case 'a':return v.type==BN_STRING&&(!strcmp(v.text,"start")||home_digest(v.text));
 default:return 0;
 }
}
static inline void home_definition(BnDef *d){memset(d,0,sizeof *d);strcpy(d->key,HOME_MARKER);d->type=BN_INT;d->portable=1;d->min=d->max=1;d->initial.type=BN_INT;d->initial.number=1;}
#endif

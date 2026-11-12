#ifndef BN_ACCOUNT_H
#define BN_ACCOUNT_H
#include "bnsettings.h"
typedef struct BnAccount BnAccount;
/* Worker APIs. One object per captured session; reviewed definitions are supplied
 * by the Native adapter, with a closed optional J5 Home field family. */
BnAccount *bna_open(const BnDef *,size_t);
int bna_close(BnAccount *); /* refuses while a request is in flight */
int bna_edit(BnAccount *,const char *,BnValue);
int bna_get(BnAccount *,const char *,BnValue *);
int bna_writes(BnAccount *,int); /* explicit per-profile local opt-in; default Off */
int bna_writes_enabled(BnAccount *);
/* Synchronous worker cycle; 1 accepted, 0 retry/offline/invalid/stale. */
int bna_cycle(BnAccount *);
int bna_edit_many(BnAccount *,const char *const *,const BnValue *,size_t);
/* UI polling never waits on a worker commit when nonblocking is true. */
int bna_view(BnAccount *,const char *const *,size_t,BnValue *,unsigned char *,int nonblocking);
int bna_cached(BnAccount *);
/* Copy J5 overrides and private unobserved baselines; only known/dirty fields upload. */
int bna_home_seed(BnAccount *,const BnField *,size_t);
int bna_home_view(BnAccount *,BnField *,size_t,size_t *);
/* Local-only authorised profile editor; never dispatches RPCs for another profile. */
int bna_stage(int profile,const BnDef *,size_t,const char *const *,const BnValue *,size_t);
int bna_staged_view(int profile,const BnDef *,size_t,const char *const *,size_t,BnValue *,unsigned char *);
#endif

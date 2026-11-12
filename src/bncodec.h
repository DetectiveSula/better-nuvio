#ifndef BN_CODEC_H
#define BN_CODEC_H
#include "bnsettings.h"
// 1 when a pull response holds one of the historical J0 probe documents.
int bnc_legacy_j0(const char *raw);
int bnc_response(const BnEngine *,const char *,BnDoc *);
int bnc_save(const BnEngine *,int writes,char **);
int bnc_load(BnEngine *,const char *,int *writes);
#endif

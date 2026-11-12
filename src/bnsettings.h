#ifndef BN_SETTINGS_H
#define BN_SETTINGS_H
#include <stdint.h>
#include <stddef.h>
#define BN_MAX 4096 /* 90 controls + five overrides for each of 768 Home rows */
#define BN_TEXT 160
#define BN_SCHEMA "better-nuvio-profile"
#define BN_VERSION 1
#define BN_LIMIT (1024*1024)
typedef enum { BN_BOOL=1, BN_INT, BN_ENUM, BN_STRING } BnType;
typedef struct { BnType type; int64_t number; char text[BN_TEXT]; } BnValue;
typedef struct { char key[80]; BnValue value; uint64_t rev; char device[65]; } BnField;
/* Owner/profile are local request-envelope guards, not new remote fields. */
typedef struct { char schema[32], owner[80]; unsigned version; int profile; size_t count; BnField fields[BN_MAX]; } BnDoc;
typedef struct { char key[80]; BnType type; int portable; int64_t min,max; BnValue initial;
  int (*accept)(const BnValue *); /* optional pure conversion validator; never persisted */
} BnDef;
typedef struct { BnField current; int known,dirty; } BnSlot;
/* No pointers: caller-owned checkpoint, validated before restoring. A storage
 * adapter must encode these fields explicitly, never persist the C ABI bytes. */
typedef struct { unsigned version; char owner[80],device[65]; int profile; size_t count; BnSlot slots[BN_MAX]; } BnCheckpoint;
typedef struct { BnCheckpoint state; size_t definitions; BnDef defs[BN_MAX]; int fresh; BnDoc prepared; } BnEngine;
int bn_init(BnEngine *,const char *owner,int profile,const char *device,const BnDef *,size_t);
int bn_edit(BnEngine *,const char *key,BnValue);
const BnValue *bn_get(const BnEngine *,const char *key);
/* Consume a complete, successfully decoded mock/transport response. Failed
 * requests must call bn_failed; missing fields never mean reset. */
int bn_pull(BnEngine *,const BnDoc *);
int bn_prepare(BnEngine *,BnDoc *);
int bn_ack(BnEngine *,const BnDoc *sent);
void bn_failed(BnEngine *);
int bn_checkpoint(const BnEngine *,BnCheckpoint *);
int bn_restore(BnEngine *,const BnCheckpoint *);
/* JSON body only. No network, credentials, files, app preferences or callbacks. */
int bn_json(const BnEngine *,const BnDoc *,char **);
#endif

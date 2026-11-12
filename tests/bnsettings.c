#include "bnsettings.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static BnValue num(BnType t,int n) { BnValue v={0};v.type=t;v.number=n;return v; }
static BnValue str(const char *s) {BnValue v={0};v.type=BN_STRING;snprintf(v.text,sizeof v.text,"%s",s);return v;}
static BnDef defs[4];
static void init(BnEngine *e,const char *owner,int p,const char *device) {assert(bn_init(e,owner,p,device,defs,4));}
static BnDoc empty(const char *owner,int p) {BnDoc d={0};strcpy(d.schema,BN_SCHEMA);strcpy(d.owner,owner);d.version=1;d.profile=p;return d;}
int main(void) {
 const char *keys[]={"synthetic_toggle","synthetic_count","synthetic_choice","synthetic_text"};
 for(int i=0;i<4;i++){strcpy(defs[i].key,keys[i]);defs[i].type=(BnType)(i+1);defs[i].portable=1;defs[i].max=i?10:1;defs[i].initial=i==3?str("initial"):num((BnType)(i+1),0);}
 BnEngine *a=calloc(1,sizeof *a),*b=calloc(1,sizeof *b),*reboot=calloc(1,sizeof *reboot);assert(a&&b&&reboot);
 init(a,"owner1",1,"deviceA");init(b,"owner1",1,"deviceB");static BnDoc server,sent,other;server=empty("owner1",1);
 assert(!bn_prepare(a,&sent));assert(bn_pull(a,&server));assert(!bn_prepare(a,&sent));
 assert(bn_edit(a,keys[0],num(BN_BOOL,1)));assert(!bn_prepare(a,&sent));assert(bn_pull(a,&server));assert(bn_prepare(a,&sent));
 server=sent;assert(bn_ack(a,&sent));assert(!bn_ack(a,&sent));assert(bn_pull(b,&server));assert(bn_get(b,keys[0])->number==1);assert(!bn_prepare(b,&other));
 assert(bn_edit(b,keys[1],num(BN_INT,7)));assert(bn_edit(b,keys[2],num(BN_ENUM,3)));assert(bn_edit(b,keys[3],str("portable text")));
 assert(bn_pull(b,&server)&&bn_prepare(b,&other));server=other;assert(bn_ack(b,&other));assert(bn_pull(a,&server));assert(bn_get(a,keys[1])->number==7);
 for(int i=0;i<10;i++) {assert(bn_pull(a,&server));assert(!bn_prepare(a,&sent));}
 puts("PASS: versioned typed document, safe login, both devices/directions, defaults and no loops");
 assert(bn_edit(a,keys[1],num(BN_INT,8)));bn_failed(a);assert(!bn_prepare(a,&sent));
 static BnCheckpoint checkpoint;assert(bn_checkpoint(a,&checkpoint));init(reboot,"owner1",1,"deviceA");assert(bn_restore(reboot,&checkpoint));assert(bn_get(reboot,keys[1])->number==8);assert(!bn_prepare(reboot,&sent));
 assert(bn_pull(reboot,&server)&&bn_prepare(reboot,&sent));
 assert(bn_edit(reboot,keys[1],num(BN_INT,9)));assert(bn_ack(reboot,&sent));assert(bn_get(reboot,keys[1])->number==9);
 server=sent;assert(bn_pull(reboot,&server)&&bn_prepare(reboot,&sent));server=sent;bn_failed(reboot);
 assert(bn_pull(reboot,&server));assert(!bn_prepare(reboot,&sent));
 puts("PASS: offline checkpoint/restart, fresh-pull gate, edit during upload and ambiguous-ack recovery");
 init(a,"owner1",1,"deviceA");init(b,"owner1",1,"deviceB");assert(bn_pull(a,&server)&&bn_pull(b,&server));
 assert(bn_edit(a,keys[1],num(BN_INT,2)));assert(bn_edit(b,keys[1],num(BN_INT,4)));
 assert(bn_pull(a,&server)&&bn_prepare(a,&sent));assert(bn_pull(b,&server)&&bn_prepare(b,&other));
 assert(bn_pull(a,&other));assert(bn_pull(b,&sent));assert(bn_get(a,keys[1])->number==4&&bn_get(b,keys[1])->number==4);
 // Losing whole-document write: retained winner repairs next fresh snapshot.
 assert(bn_prepare(b,&other));assert(other.fields[1].rev>=sent.fields[1].rev);
 assert(bn_ack(b,&other));assert(bn_pull(a,&other));assert(!bn_prepare(a,&sent));
 puts("PASS: deterministic per-field revision/device conflicts and lower-revision snapshot repair");
 static BnDoc bad;bad=other;bad.profile=2;assert(!bn_pull(a,&bad));bad=other;strcpy(bad.owner,"owner2");assert(!bn_pull(a,&bad));
 bad=other;bad.version=2;assert(!bn_pull(a,&bad));bad=other;bad.fields[0].value.type=BN_STRING;assert(!bn_pull(a,&bad));
 bad=other;bad.fields[0].value.number=99;assert(!bn_pull(a,&bad));bad=other;bad.fields[bad.count++]=bad.fields[0];assert(!bn_pull(a,&bad));
 bad=other;strcpy(bad.fields[0].key,"api_key");assert(!bn_pull(a,&bad));assert(!bn_edit(a,"api_key",str("private")));
 assert(!bn_edit(a,keys[3],str("https://private.invalid/token")));
 bad=other;strcpy(bad.fields[0].key,"future_unknown");assert(!bn_pull(a,&bad));
 bad=other;bad.count=BN_MAX+1;assert(!bn_pull(a,&bad));
 bad=other;bad.fields[0].rev=UINT64_MAX;assert(!bn_pull(a,&bad));
 bad=other;bad.fields[0].value.number=!bad.fields[0].value.number;assert(!bn_pull(a,&bad));
 assert(!bn_edit(a,keys[3],str("\xff")));
 static BnDoc partial;partial=empty("owner1",1);assert(bn_pull(a,&partial));assert(bn_get(a,keys[1])->number==4);
 init(reboot,"owner1",2,"deviceA");assert(!bn_restore(reboot,&checkpoint));assert(bn_get(reboot,keys[1])->number==0);
 init(reboot,"owner2",1,"deviceA");assert(!bn_restore(reboot,&checkpoint));
 char *json=NULL;assert(bn_json(a,&other,&json));assert(strstr(json,"better-nuvio-profile")&&strstr(json,"\"type\":\"bool\""));assert(!strstr(json,"owner1")&&!strstr(json,"api_key"));puts(json);free(json);
 BnDef deny=defs[0];deny.portable=0;assert(!bn_init(reboot,"owner1",1,"deviceA",&deny,1));deny=defs[0];strcpy(deny.key,"decoder_mode");assert(!bn_init(reboot,"owner1",1,"deviceA",&deny,1));
 puts("PASS: account/profile/checkpoint isolation, malformed/partial/future documents and credential/device deny gates");
 BnDef *many=calloc(BN_MAX,sizeof *many);assert(many);
 for(int i=0;i<BN_MAX;i++) {many[i]=defs[3];snprintf(many[i].key,sizeof many[i].key,"portable_%070d",i);}
 assert(bn_init(reboot,"owner1",1,"deviceA",many,BN_MAX));
 static BnDoc large;large=empty("owner1",1);large.count=BN_MAX;
 for(int i=0;i<BN_MAX;i++) {
   strcpy(large.fields[i].key,many[i].key);large.fields[i].value=str("");
   memset(large.fields[i].value.text,'"',BN_TEXT-1);large.fields[i].rev=1;
   memset(large.fields[i].device,'A',64);large.fields[i].device[64]=0;
 }
 assert(bn_pull(reboot,&large));json=NULL;assert(!bn_json(reboot,&large,&json)&&!json);
 puts("PASS: bounded field count, revision overflow, UTF-8, equivocation and 1 MiB JSON limit");
 free(many);free(a);free(b);free(reboot);return 0;
}

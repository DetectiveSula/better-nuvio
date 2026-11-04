// Reuse the real Stage D pipeline and its transport, never a live provider.
#define main stageDReadOnlyMain
#include "settings_stage_d.c"
#undef main
#include "../src/debrid_add_ledger.h"
#include "../src/dados.h"
#include "../src/admin.h"
#include <sys/stat.h>
static char testHash[41];static int serial;
static void setup(void) {
  snprintf(testHash,sizeof testHash,"%040x",++serial+100);compatHash=testHash;
  compatFixture=compatOptIn=1;compatCreateMode=compatReadyMode=compatListMode=compatVisible=0;
  compatFreshResult=compatDisableFresh=compatDisablePost=compatSecondCached=0;
  compatCreateCalls=compatCacheCalls=compatLastApi=0;mode=linkMode=0;
  configure(1,1,0,0,1,0,0,0);assert(ajustes_torbox_adicionar_cache());
}
static int resolve(void) {char url[512];int outside=-1;int result=resolver_politica_resolver(-1,compatHash,0,url,sizeof url,&outside);assert(result?url[0]!=0:url[0]==0);return result;}
static void *concurrent(void *unused){(void)unused;assert(resolve());return NULL;}
int main(void) {
  char temp[1024];snprintf(temp,sizeof temp,"%s/nuvio-tb-optin-XXXXXX",nvt_dir());assert(mkdtemp(temp));dir=temp;
  setenv("NUVIO_DADOS",dir,1);dados_iniciar(dir);assert(SDL_Init(SDL_INIT_TIMER)==0);
  setup();assert(resolve());assert(compatCreateCalls==1&&compatCacheCalls>=2);
  // No collection visibility yet: a persisted ID still prevents duplicate POSTs.
  assert(resolve()&&compatCreateCalls==1);
  int id=0;assert(!debrid_tb_adicao_claim("synthetic-tb",compatHash,&id)&&id==99);
  char seed[300],digest[65],receipt[1400];snprintf(seed,sizeof seed,"synthetic-tb:%s",compatHash);
  admin_sha256_hex((unsigned char*)seed,strlen(seed),digest);snprintf(receipt,sizeof receipt,"%s/torbox-cached-additions/%s.state",dir,digest);
  FILE *corrupt=fopen(receipt,"w");assert(corrupt);fputs("item 99 junk\n",corrupt);fclose(corrupt);
  assert(!debrid_tb_adicao_claim("synthetic-tb",compatHash,&id)&&!id);assert(!resolve()&&compatCreateCalls==1);
  if(geteuid()!=0){setup();char folder[1400];snprintf(folder,sizeof folder,"%s/torbox-cached-additions",dir);assert(!chmod(folder,0500));assert(!resolve()&&!compatCreateCalls);assert(!chmod(folder,0700));}
  setup();compatOptIn=0;configure(1,1,0,0,1,0,0,0);assert(!resolve()&&!compatCreateCalls);
  setup();configure(1,1,1,0,1,0,0,0);assert(!ajustes_torbox_adicionar_cache()&&!resolve()&&!compatCreateCalls);
  setup();compatFreshResult=1;assert(!resolve()&&!compatCreateCalls); // fresh check became uncached
  setup();compatFreshResult=2;assert(!resolve()&&!compatCreateCalls); // fresh check failed
  for(mode=1;mode<=7;mode++){int selected=mode;setup();mode=selected;assert(!resolve()&&!compatCreateCalls);}mode=0;
  setup();compatHash="invalid";assert(!resolve()&&!compatCreateCalls);
  for(int failure=1;failure<=6;failure++) {
    setup();compatCreateMode=failure;
    int success=resolve();assert(success==(failure==5));assert(compatCreateCalls==1);
    assert(resolve()==success&&compatCreateCalls==1); // no blind retry, including ambiguous timeout
  }
  setup();compatCreateMode=1;assert(!resolve()&&compatCreateCalls==1);compatCreateMode=0;
  compatOptIn=0;configure(1,1,0,0,1,0,0,0);compatOptIn=1;configure(1,1,0,0,1,0,0,0);
  assert(!resolve()&&compatCreateCalls==1); // receipt survives settings reload, no process-memory lookup
  compatVisible=1;assert(resolve()&&compatCreateCalls==1); // discover a late accepted item, no POST
  setup();compatReadyMode=1;assert(!resolve()&&compatCreateCalls==1);
  compatReadyMode=0;assert(resolve()&&compatCreateCalls==1); // ready later, reuses saved ID
  setup();compatReadyMode=2;assert(!resolve()&&compatCreateCalls==1); // contradictory readiness
  setup();compatReadyMode=3;assert(!resolve()&&compatCreateCalls==1); // wrong hash
  setup();compatReadyMode=4;assert(resolve()&&compatCreateCalls==1); // optional hash, acknowledged ID verified
  for(int failure=1;failure<=3;failure++){setup();linkMode=failure;assert(!resolve()&&compatCreateCalls==1);assert(!resolve()&&compatCreateCalls==1);}linkMode=0;
  for(int failure=1;failure<=3;failure++){setup();compatListMode=failure;assert(!resolve()&&!compatCreateCalls);}
  setup();compatVisible=1;assert(resolve()&&!compatCreateCalls); // existing ready item reused
  setup();compatDisableFresh=1;assert(!resolve()&&!compatCreateCalls);
  setup();compatDisablePost=1;assert(!resolve()&&compatCreateCalls==1); // effect may have happened, never report stale success
  compatOptIn=1;configure(1,1,0,0,1,0,0,0);assert(resolve()&&compatCreateCalls==1);
  setup();configure(1,2,0,0,1,1,1,1);assert(!resolve()&&!compatCreateCalls); // prefer PM, no hidden TB add
  setup();configure(0,1,0,0,1,0,0,0);assert(!resolve()&&!compatCreateCalls);
  setup();pthread_t threads[6];for(int i=0;i<6;i++)assert(!pthread_create(&threads[i],NULL,concurrent,NULL));
  for(int i=0;i<6;i++)pthread_join(threads[i],NULL);assert(compatCreateCalls==1);
  // Real raw addon hash (no addon TorBox key) -> global cache -> addition -> autoplay URL.
  setup();compatHash=H1;compatSecondCached=1;configure(1,1,0,2,1,0,0,0);Stream *v=NULL;int count=sources(&v);assert(count==4);stream_definir_lista(v,count);free(v);
  assert(stream_primeira_boa_sessao(8,nv_perfil_sessao_ler())==0&&lista[0].url[0]);assert(compatCreateCalls==1);expectLabel(&lista[0],"Cached");
  puts("PASS: cached TorBox opt-in fresh checks/contract/readiness, autoplay, durable duplicate and timeout suppression, concurrency, provider/Off gates and manual-path separation");
  SDL_Quit();return 0;
}

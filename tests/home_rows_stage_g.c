#define _DARWIN_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "fileiras.h"
#include "perfis.h"
#include "catordem.h"
#include "catordemcache.h"
#include "cotacat.h"
#include "dados.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static char directory[1024];static int writes;
static ContaPerfil testProfile;static int testActive=1;
const ContaPerfil *perfis_item_ativo(void){return &testProfile;}
int perfis_ativo(void){return testActive;}
int ajustes_addons_do_principal(void){return 1;}
#include "stage-g-profile.inc"

char *dados_caminho(char *dst,unsigned size,const char *name){return snprintf(dst,size,"%s/%s",directory,name)<(int)size?dst:NULL;}
char *dados_ler(const char *name){char path[1200];if(!dados_caminho(path,sizeof path,name))return NULL;FILE*f=fopen(path,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);assert(n>=0);char*s=calloc((size_t)n+1,1);assert(s);assert(fread(s,1,(size_t)n,f)==(size_t)n);fclose(f);return s;}
int dados_gravar(const char *name,const char *text){char path[1200];assert(dados_caminho(path,sizeof path,name));FILE*f=fopen(path,"wb");if(!f)return 0;int ok=fwrite(text,1,strlen(text),f)==strlen(text);fclose(f);writes++;return ok;}
int dados_apagar(const char*name){char path[1200];dados_caminho(path,sizeof path,name);return !unlink(path);}
unsigned addons_versao(void){return 0;}
int addons_travar_lista(unsigned version,int profile){(void)version;(void)profile;return 0;}
void addons_destravar_lista(void){}
static int indexOf(const char*key){for(int i=0;i<fil_n();i++)if(!strcmp(fil_chave(i),key))return i;return -1;}
// Compile the actual editor partitioner from ajustes.c, with only drawing state stubbed.
#define AJ_FIL_DESTAQUE (-2)
static int filFoco,filListaN,filSep,filAba,filLista[FIL_MAX+1];
static int filCmpFora(const void*a,const void*b){return strcmp(fil_titulo(*(const int*)a),fil_titulo(*(const int*)b));}
#include "stage-g-editor.inc"
static char prefOff[2][192],prefOrdem[2][192];static int nPrefOff,nPrefOrdem;
static int wrapped;
static int dentroDeColecaoVisivelBase(const char*b,const char*t,const char*c){(void)b;(void)t;(void)c;return wrapped;}
#include "stage-g-quota.inc"
static void publish(void){const char*keys[FIL_MAX];int enabled[FIL_MAX],n=catordem_n();assert(n<=FIL_MAX);for(int i=0;i<n;i++){keys[i]=catordem_chave(i);enabled[i]=!catordem_oculta(keys[i],keys[i]);}fil_projecao_conta(keys,enabled,n);}
static void snapshot(int n,int disabled,char *json,unsigned cap){unsigned pos=snprintf(json,cap,"{\"items\":[");for(int i=n-1;i>=0;i--)pos+=snprintf(json+pos,cap-pos,"%s{\"addon_id\":\"fixture.addon\",\"type\":\"movie\",\"catalog_id\":\"row%d\",\"order\":%d,\"enabled\":%s}",i==n-1?"":",",i,n-1-i,i==disabled?"false":"true");snprintf(json+pos,cap-pos,"]}");}
static const char *sharedNames[]={"Streaming Services","Genres","Top 25 Movies","Top 25 Series","Trending Movies","Trending Series","Popular Movies","Popular Series"};
static void preferences(int profile,int n,int friendsOff){char name[80],text[24000];unsigned pos=snprintf(text,sizeof text,"limite 10\nordem 1\nmigracao 197\n");for(int i=0;i<n;i++){pos+=snprintf(text+pos,sizeof text-pos,"linha fixture.addon_movie_row%d\t%d\t0\t1\t0\t%s\n",i,i==n-1,i<8?sharedNames[i]:"Same display name");if(i==1)pos+=snprintf(text+pos,sizeof text-pos,"linha social_activity\t%d\t0\t2\t0\tFriends Watching\n",friendsOff);}
 pos+=snprintf(text+pos,sizeof text-pos,"linha fixture.addon_movie_row0\t1\t0\t1\t0\tDuplicate must not get its own slot\n");
 pos+=snprintf(text+pos,sizeof text-pos,"linha native.extras_series_custom\t0\t2\t2\t0\tSame display name\nlinha collection_native\t0\t0\t1\t0\tNative collection\n");
 snprintf(name,sizeof name,"fileirasui-p%d.txt",profile);assert(dados_gravar(name,text));}
// Regular Nuvio's order is primary: shared rows follow the synced order exactly,
// each Native-exclusive row stays after the shared row it followed natively
// (Friends Watching after row1, the Native extras row after the last row), and
// the unlisted shared collection is appended like an unsaved official collection.
static int at(const char *const *keys,const int *order,int q,const char *key){for(int i=0;i<q;i++)if(!strcmp(keys[order[i]],key))return i;return -1;}
static void checkOrder(int n){const char*keys[FIL_MAX];int order[FIL_MAX];for(int i=0;i<fil_n();i++)keys[i]=fil_chave(i);int q=fil_unir(keys,fil_n(),order,FIL_MAX),seen=0;assert(q==fil_n());
 char last[100];snprintf(last,sizeof last,"fixture.addon_movie_row%d",n-1);
 assert(at(keys,order,q,"social_activity")==at(keys,order,q,"fixture.addon_movie_row1")+1);
 assert(at(keys,order,q,"native.extras_series_custom")==at(keys,order,q,last)+1);
 assert(at(keys,order,q,"collection_native")==q-1);
 for(int i=0;i<q;i++)if(!strncmp(keys[order[i]],"fixture.addon_movie_row",23)){char expected[100];snprintf(expected,sizeof expected,"fixture.addon_movie_row%d",n-1-seen++);assert(!strcmp(keys[order[i]],expected));}assert(seen==n);}
int main(void){snprintf(directory,sizeof directory,"%s/nuvio-home-g-XXXXXX",getenv("TMPDIR")?getenv("TMPDIR"):"/tmp");assert(mkdtemp(directory));
 testActive=2;testProfile.primario=0;testProfile.usaAddonsDoPrimario=0;assert(perfis_home_config()==2);
 assert(perfis_ativo_addons()==1); // preserved device fallback
 const char *scopeKey[]={"fixture_movie_shared"};int scopeEnabled[]={1};fil_projecao_conta(scopeKey,scopeEnabled,1);
 assert(perfis_ativo_addons()==2); // valid profile rows override the device fallback
 fil_projecao_conta(NULL,NULL,0);assert(perfis_ativo_addons()==1);
 testProfile.usaAddonsDoPrimario=1;assert(perfis_home_config()==1);
 testProfile.primario=1;assert(perfis_home_config()==2);testActive=1;
 const int sizes[]={10,15,20,30};char json[16000];
 for(int t=0;t<4;t++){
  int n=sizes[t],profile=t+1;preferences(profile,n,0);fil_definir_perfil(profile);catordem_esquecer();fil_projecao_conta(NULL,NULL,0);
  assert(fil_n()==n+3);for(int k=0;k<8;k++){char key[80];snprintf(key,sizeof key,"fixture.addon_movie_row%d",k);assert(!strcmp(fil_titulo(indexOf(key)),sharedNames[k]));}char name[80];snprintf(name,sizeof name,"fileirasui-p%d.txt",profile);char*original=dados_ler(name);assert(original);
  assert(fil_limite()==FIL_LIMITE_MAX&&fil_limite_gravado()==10);
  assert(!strcmp(fil_chave(2),"social_activity"));int style=fil_tipo("native.extras_series_custom");
  snapshot(n,-1,json,sizeof json);assert(catordem_ler(json));publish();int beforeWrites=writes;
  assert(fil_n_na_home()==n+3&&fil_n_fila()==0);checkOrder(n);
  strcpy(prefOff[0],"fixture.addon_movie_row0");nPrefOff=1;int priority=0;wrapped=1;
  assert(prioCatalogo(prefOff[0],"","","movie","row0",&priority)==COTA_ORDEM_CONTA&&priority==n-1);nPrefOff=0;wrapped=0;
  fil_normalizar();fil_gravar_registro();assert(!fil_migrar_197(NULL,0));fil_conta_reconciliar(NULL,NULL,NULL,0);
  fil_colecoes_reconciliar(NULL,NULL,NULL,0,1);assert(indexOf("collection_native")>=0);
  const char *ordinary[]={"fixture.addon_movie_row0"};int off[]={0},inside[]={1};
  fil_conta_reconciliar(ordinary,off,inside,1);assert(fil_estado_chave(ordinary[0])==FIL_NA_HOME); // collection membership cannot suppress an enabled shared catalogue
  assert(writes==beforeWrites);char*unchanged=dados_ler(name);assert(!strcmp(original,unchanged));free(unchanged);
  filAba=0;filMontarLista();assert(filListaN==n+4&&filSep==-1);
  int friends=indexOf("social_activity"),seen=0;for(int i=0;i<filListaN;i++)if(filLista[i]==friends)seen++;assert(seen==1);
  snapshot(n,2,json,sizeof json);assert(catordem_ler(json));publish();assert(fil_estado_chave("fixture.addon_movie_row2")==FIL_FORA);assert(fil_estado_chave("social_activity")==FIL_NA_HOME);assert(fil_tipo("native.extras_series_custom")==style);
  assert(catordem_cache_gravar(profile,"fixture-account",json));unsigned rev=catordem_revisao();
  assert(!catordem_ler("{\"items\":[{"));assert(!catordem_ler("[]"));assert(!catordem_ler("{\"items\":[]}"));assert(catordem_revisao()==rev);publish();checkOrder(n);
  fil_definir_perfil(8);catordem_esquecer();publish();assert(fil_n()==0);
  fil_definir_perfil(profile);assert(catordem_cache_carregar(profile,"fixture-account"));publish();checkOrder(n);assert(fil_estado_chave("social_activity")==FIL_NA_HOME);
  // Repeated refreshes, restart/reload and account mismatch preserve Native rows.
  for(int i=0;i<3;i++){assert(!catordem_ler(json));publish();fil_recarregar();assert(fil_n()==n+3);checkOrder(n);}
  assert(!catordem_cache_carregar(profile,"another-fixture-account"));publish();assert(fil_estado_chave("fixture.addon_movie_row2")==FIL_NA_HOME);assert(fil_estado_chave("social_activity")==FIL_NA_HOME);
  assert(fil_estado_chave("fixture.addon_movie_row29")==(n==30?FIL_FORA:-1));
  // Explicit local edits can normalise duplicate identities only after backing up exact original bytes.
  fil_ciclar_tam(indexOf("native.extras_series_custom"));int nativeSize=fil_linha_tam(indexOf("native.extras_series_custom"));char backup[110];snprintf(backup,sizeof backup,"%s.pre-stage-g",name);char*recovery=dados_ler(backup);assert(recovery&&!strcmp(original,recovery));free(recovery);free(original);
  fil_recarregar();assert(fil_n()==n+3);assert(fil_linha_tam(indexOf("native.extras_series_custom"))==nativeSize);assert(fil_estado_chave("social_activity")==FIL_NA_HOME);
 }
 // A disabled exclusive preference stays disabled through sync, profile switch and restart.
 preferences(7,15,1);fil_definir_perfil(7);catordem_esquecer();snapshot(15,-1,json,sizeof json);catordem_ler(json);publish();assert(fil_estado_chave("social_activity")==FIL_FORA);fil_recarregar();assert(fil_estado_chave("social_activity")==FIL_FORA);
 // Inherited shared rows use primary's cache while exclusive rows stay profile-local.
 assert(catordem_cache_carregar(1,"fixture-account"));publish();assert(fil_estado_chave("social_activity")==FIL_FORA);
 fil_definir_perfil(1);assert(catordem_cache_carregar(1,"fixture-account"));publish();assert(fil_estado_chave("social_activity")==FIL_NA_HOME);
 fil_definir_perfil(7);assert(catordem_cache_carregar(1,"fixture-account"));publish();fil_recarregar();assert(fil_estado_chave("social_activity")==FIL_FORA);
 assert(fil_limite()==200);filAba=0;filMontarLista();
 int collection=indexOf("collection_native");assert(collection>=0);
 for(int i=0;i<filListaN;i++)if(filLista[i]==collection)filFoco=i;
 fil_definir_teto_sessao(12);assert(fil_limite()==12);filMontarLista();assert(filSep>0);
 assert(filLista[filFoco]==collection && filFoco<filSep);
 for(int i=1;i<filListaN;i++)assert(fil_estado(filLista[i])==(i<filSep?FIL_NA_HOME:FIL_NA_FILA));
 fil_definir_teto_sessao(0);
 // Legacy pre-profile file migration remains non-destructive, including duplicates.
 const char *legacy="limite 10\nordem 1\nlinha social_activity\t0\t0\t1\t0\tFriends Watching\nlinha social_activity\t1\t0\t1\t0\tDuplicate\n";
 assert(dados_gravar("fileirasui.txt",legacy));dados_apagar("fileirasui-p1.txt");dados_apagar("fileirasui-p1.txt.pre-stage-g");
 fil_definir_perfil(0);catordem_esquecer();publish();fil_definir_perfil(1);assert(fil_n()==1);fil_ciclar_tam(0);
 char *legacyAfter=dados_ler("fileirasui.txt"),*legacyBackup=dados_ler("fileirasui-p1.txt.pre-stage-g");assert(legacyAfter&&legacyBackup&&!strcmp(legacy,legacyAfter)&&!strcmp(legacy,legacyBackup));free(legacyAfter);free(legacyBackup);
 puts("PASS: Stage G 10/15/20/30 rows, regular Nuvio order primary, Friends Watching/exclusive rows anchored after their shared neighbour and states kept, real editor partition, duplicate identity recovery, profile/cache/restart/offline isolation and Native file preservation");return 0;
}

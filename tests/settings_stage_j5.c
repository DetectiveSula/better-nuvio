/* Two devices with different registered catalogues, real row engine + J2/J3.
 * The account fixture is file-backed and has no HTTP implementation. */
#define main j3_fixture_main
#include "bnaccount.c"
#undef main
#include "fileiras.h"
#include "homewire.h"
#include "addonurl.h"
#define NV_J7_IMPL 1
#include "j7arme.h"
/* Built as Android debug (NV_ANDROID): the J7 gate is the real declaration.
 * This unit test models an armed test account; tests/settings_stage_j7.sh
 * covers the gate itself. */
int j7arme_permite_j4(const char *owner,int profile){(void)owner;(void)profile;return 1;}
int j7arme_permite_j6(const char *owner,int profile){(void)owner;(void)profile;return 0;}
static BnDef registry[4];
unsigned addons_versao(void){return 0;}
int addons_travar_lista(unsigned v,int p){(void)v;(void)p;return 0;}
void addons_destravar_lista(void){}
char *dados_caminho(char *out,unsigned cap,const char *name){localpath(out,cap,name);return out;}
static int observe(const char *key,BnValue v){return bna_edit(active,key,v);}
static int row(const char *key){for(int i=0;i<fil_n();i++)if(!strcmp(key,fil_chave(i)))return i;return -1;}
static void project(void){BnField *f=calloc(BN_MAX,sizeof *f);size_t n=0;assert(f);fil_home_scope(owner,profile);assert(fil_home_baseline(owner,profile,f,BN_MAX,&n));assert(bna_home_seed(active,f,n));assert(bna_home_view(active,f,BN_MAX,&n));fil_home_apply(owner,profile,f,n);free(f);}
static void cycle(void){backoff=0;assert(bna_cycle(active));project();}
static void reg(const char *key){fil_registrar(key,key,"Fixture","movie",10);}
static char second[192],extra[192];
static const char *first="fixture.addon_movie_top",*param="fixture.addon_movie_top_genre=drama",*series="fixture.addon_series_top";
static const char *ids[]={"fixture.addon","fixture.addon","other.addon"};
static const char *bases[]={"https://mock.invalid/one?token=PRIVATE","https://mock.invalid/two?token=PRIVATE","https://mock.invalid/other"};
static void openDevice(int which){
 if(active){fil_gravar_registro();assert(bna_close(active));active=NULL;}
 fil_home_observer(NULL);fil_definir_perfil(0);side=which;strcpy(device,which?"deviceB":"deviceA");fil_definir_perfil(profile);
 fil_podar_catalogos(ids,bases,which?2:3,profile);
 reg("continue_watching");reg("social_activity");reg("last_session");reg(first);reg(param);reg(series);reg(second);reg("collection_fixture");if(!which)reg(extra);
 const char *order[]={first,param,series};int enabled[]={1,1,0};fil_projecao_conta(order,enabled,3);
 active=bna_open(registry,4);assert(active);fil_home_scope(owner,profile);fil_home_observer(observe);project();
}
static void property(const char *local,char prop,char key[40]){char id[33];assert(fil_home_identity(local,id));snprintf(key,40,"home_%s_%c",id,prop);}
static void anchors(void){const char *k[]={"continue_watching",first,param,"social_activity",second,"last_session"};int order[6];assert(fil_unir(k,6,order,6)==6);int f=-1,l=-1,a=-1;for(int i=0;i<6;i++){if(!strcmp(k[order[i]],first))a=i;if(!strcmp(k[order[i]],"social_activity"))f=i;if(!strcmp(k[order[i]],"last_session"))l=i;}assert(a>=0&&f==a+1&&l==f+1);}
int main(int argc,char **argv){
 assert(argc==2);snprintf(root,sizeof root,"%s",argv[1]);char path[800];snprintf(path,sizeof path,"%s/A",root);assert(!mkdir(path,0700));snprintf(path,sizeof path,"%s/B",root);assert(!mkdir(path,0700));
 schema();memcpy(registry,defs,sizeof defs);home_definition(&registry[3]);char fp[32];nv_addon_chave(bases[1],fp,sizeof fp);snprintf(second,sizeof second,"%s_movie_top",fp);strcpy(extra,"other.addon_movie_onlyA");
 openDevice(0);assert(!bna_writes_enabled(active));cycle();assert(!posts);char a[40],b[40];property(first,'s',a);property(second,'s',b);assert(strcmp(a,b));
 assert(fil_definir_tipo(first,FIL_TIPO_FICHA));assert(fil_definir_tipo(second,FIL_TIPO_PALCO));assert(fil_definir_tipo(param,FIL_TIPO_TOP10));fil_ciclar_tam(row(first));assert(fil_definir_ordem("collection_fixture",FIL_ORD_AZ));fil_remover(row(param));fil_remover(row("social_activity"));fil_adicionar(row("social_activity"),NULL);
 /* A stale remote Automatic overlay must not veto a local picker change
  * while the observer is detached (offline/read-only validation). */
 fil_home_observer(NULL);
 fil_definir_hero_fonte("*");assert(!strcmp(fil_hero_fonte(),"*"));
 fil_definir_hero_fonte(first);assert(!strcmp(fil_hero_fonte(),first));
 fil_home_observer(observe);
 assert(fil_definir_tipo(extra,FIL_TIPO_HEROI));fil_definir_hero_fonte(extra);
 /* Explicit anchors are sparse property edits, not a shared catalogue order. */
 char k[40],id[33];BnValue v={.type=BN_STRING};assert(fil_home_identity(first,id));strcpy(v.text,id);property("social_activity",'a',k);assert(observe(k,v));assert(fil_home_identity("social_activity",id));strcpy(v.text,id);property("last_session",'a',k);assert(observe(k,v));
 cycle();assert(!posts);assert(bna_writes(active,1));cycle();assert(posts==1);assert(lastWrite&&!strstr(lastWrite,"PRIVATE")&&!strstr(lastWrite,"https:")&&!strstr(lastWrite,"home_catalog_shared"));
 int nA=fil_n();openDevice(1);int nB=fil_n();assert(nB==nA-1);assert(!bna_writes_enabled(active));cycle();assert(posts==1&&fil_n()==nB&&row(extra)<0);
 assert(fil_tipo(first)==FIL_TIPO_FICHA&&fil_tipo(second)==FIL_TIPO_PALCO&&fil_tipo(param)==FIL_TIPO_TOP10);assert(fil_linha_tam(row(first))==FIL_TAM_GRANDE);assert(fil_ordem("collection_fixture")==FIL_ORD_AZ);assert(fil_oculta(param)&&!fil_oculta(first));assert(fil_oculta(series));assert(!*fil_hero_fonte());anchors();
 puts("PASS: two devices, distinct duplicate instances/type/parameters, row shape/size/item order/TV hides, exclusive anchors, unavailable hero fallback; no list replacement or credentials");
 /* A partial catalogue list must not erase A-only overrides when B uploads. */
 assert(bna_writes(active,1));assert(fil_definir_tipo(first,FIL_TIPO_CARTAZ));fil_ciclar_tam(row(second));fil_definir_hero_fonte(first);cycle();assert(posts==2);int prior=posts;cycle();cycle();assert(posts==prior);
 openDevice(0);char *nativeBefore=dados_ler("fileirasui-p1.txt");int focus=row(first);cycle();char *nativeAfter=dados_ler("fileirasui-p1.txt");assert((!nativeBefore&&!nativeAfter)||(nativeBefore&&nativeAfter&&!strcmp(nativeBefore,nativeAfter)));free(nativeBefore);free(nativeAfter);assert(row(first)==focus);assert(fil_tipo(first)==FIL_TIPO_CARTAZ&&fil_tipo(extra)==FIL_TIPO_HEROI&&fil_tipo(second)==FIL_TIPO_PALCO);assert(!strcmp(fil_hero_fonte(),first));
 /* Remote-selected default is an explicit edit even if the fallback is default. */
 fil_definir_hero_fonte("");cycle();openDevice(1);cycle();assert(!*fil_hero_fonte());assert(fil_definir_tipo(first,FIL_TIPO_AUTO));fil_adicionar(row(param),NULL);failPull=1;backoff=0;assert(!bna_cycle(active));project();assert(fil_tipo(first)==FIL_TIPO_AUTO&&!fil_oculta(param));
 openDevice(1);project();assert(fil_tipo(first)==FIL_TIPO_AUTO&&!fil_oculta(param));failPull=0;cycle();openDevice(0);cycle();assert(fil_tipo(first)==FIL_TIPO_AUTO&&!fil_oculta(param));assert(fil_oculta(series));
 /* Remote show overrides a receiving device's old personal hide too. */
 openDevice(1);fil_remover(row("social_activity"));cycle();openDevice(0);cycle();assert(fil_oculta("social_activity"));fil_adicionar(row("social_activity"),NULL);cycle();openDevice(1);cycle();assert(!fil_oculta("social_activity"));
 puts("PASS: explicit defaults, offline edits/restart/reconnect, no echo, original disabled rows authoritative, unavailable rows retained in subsequent uploads");
 /* Independent field conflicts converge; no default overwrites on first login. */
 char key[40];property(first,'s',key);assert(bna_edit(active,key,(BnValue){.type=BN_ENUM,.number=FIL_TIPO_HEROI}));openDevice(1);assert(bna_edit(active,key,(BnValue){.type=BN_ENUM,.number=FIL_TIPO_REDONDA}));cycle();openDevice(0);cycle();int winner=fil_tipo(first);openDevice(1);cycle();assert(fil_tipo(first)==winner);
 /* Different profile / account have no inherited portable overrides. */
 assert(bna_close(active));active=NULL;profile=2;generation++;openDevice(1);cycle();assert(!bna_writes_enabled(active)&&fil_tipo(first)==FIL_TIPO_AUTO&&!fil_oculta(param));assert(bna_close(active));active=NULL;strcpy(owner,"owner2");profile=1;generation++;openDevice(1);cycle();assert(!bna_writes_enabled(active)&&fil_tipo(first)==FIL_TIPO_AUTO&&fil_linha_tam(row(second))==FIL_TAM_PADRAO&&!fil_oculta("social_activity"));
 assert(bna_close(active));active=NULL;profile=1;strcpy(owner,"owner1");generation++;openDevice(0);cycle();assert(fil_tipo(first)==winner&&fil_tipo(extra)==FIL_TIPO_HEROI);
 puts("PASS: profile/account isolation, pending conflict convergence and per-profile default-Off permission");
 /* Many available rows; presentation never truncates or replaces the registry. */
 for(int i=0;i<40;i++){char local[192];snprintf(local,sizeof local,"fixture.addon_movie_extra-%d",i);reg(local);assert(fil_definir_tipo(local,FIL_TIPO_FICHA));fil_ciclar_tam(row(local));}
 cycle();int before=fil_n();openDevice(0);cycle();assert(fil_n()>=before);for(int i=0;i<40;i++){char local[192];snprintf(local,sizeof local,"fixture.addon_movie_extra-%d",i);reg(local);assert(fil_tipo(local)==FIL_TIPO_FICHA);}
 assert(!bna_edit(active,"home_ffffffffffffffffffffffffffffffff_x",val(1)));assert(!bna_edit(active,"home_ffffffffffffffffffffffffffffffff_s",(BnValue){.type=BN_ENUM,.number=FIL_TIPO_N}));assert(!bna_edit(active,HOME_HERO,(BnValue){.type=BN_STRING,.text="https://private.invalid"}));
 char beforeId[33],afterId[33],otherId[33];assert(fil_home_identity(second,beforeId));assert(fil_home_identity(first,otherId));
 const char *reordered[]={bases[1],bases[0],bases[2]};fil_podar_catalogos(ids,reordered,3,profile);assert(fil_home_identity(first,afterId)&&!strcmp(afterId,beforeId));
 /* A partial duplicate family must not apply the other instance's overrides. */
 fil_podar_catalogos(ids,bases,1,profile);assert(fil_home_identity(first,afterId)&&strcmp(afterId,otherId));
 char typed[40];property(first,'s',typed);for(int i=0;i<FIL_TIPO_N;i++)assert(bna_edit(active,typed,(BnValue){.type=BN_ENUM,.number=i}));assert(!bna_edit(active,typed,(BnValue){.type=BN_ENUM,.number=FIL_TIPO_N}));
 property(first,'z',typed);for(int i=0;i<FIL_TAM_N;i++)assert(bna_edit(active,typed,(BnValue){.type=BN_ENUM,.number=i}));assert(!bna_edit(active,typed,(BnValue){.type=BN_ENUM,.number=FIL_TAM_N}));
 property("collection_fixture",'o',typed);for(int i=0;i<FIL_ORD_N;i++)assert(bna_edit(active,typed,(BnValue){.type=BN_ENUM,.number=i}));
 assert(bna_close(active));free(lastWrite);puts("PASS: 40+ rows survive restarts, closed property schema rejects invalid and sensitive values");return 0;
}

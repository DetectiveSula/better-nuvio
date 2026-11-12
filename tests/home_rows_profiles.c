// Per-profile Home visibility across profile switches, through the real
// fileiras.c / catordem.c / catordemcache.c and the same order of operations as
// sync.c (fil_definir_perfil -> catordem_cache_carregar(perfis_home_config) ->
// projection publish -> account addon list). Payloads use regular NuvioTV's keys
// (homeCatalogKey: <manifest id>_<type>_<catalogue> for EVERY addon instance).
#define _DARWIN_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "fileiras.h"
#include "catordem.h"
#include "catordemcache.h"
#include "addonurl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static char directory[1024];
char *dados_caminho(char *dst,unsigned size,const char *name){return snprintf(dst,size,"%s/%s",directory,name)<(int)size?dst:NULL;}
char *dados_ler(const char *name){char path[1200];if(!dados_caminho(path,sizeof path,name))return NULL;FILE*f=fopen(path,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);char*s=calloc((size_t)n+1,1);assert(s&&fread(s,1,(size_t)n,f)==(size_t)n);fclose(f);return s;}
int dados_gravar(const char *name,const char *text){char path[1200];assert(dados_caminho(path,sizeof path,name));FILE*f=fopen(path,"wb");if(!f)return 0;int ok=fwrite(text,1,strlen(text),f)==strlen(text);fclose(f);return ok;}
int dados_apagar(const char*name){char path[1200];dados_caminho(path,sizeof path,name);return !unlink(path);}
unsigned addons_versao(void){return 0;}
int addons_travar_lista(unsigned version,int profile){(void)version;(void)profile;return 0;}
void addons_destravar_lista(void){}

#define OWNER "fixture-account"
static const char *US="https://streamcats.example/us-config", *IN="https://streamcats.example/in-config";
static const char *ADDON="org.streamcats", *CINE="com.linvo.cinemeta", *CINEB="https://v3-cinemeta.strem.io";
static char fpIn[32], hotstar[96], zee5[96], nfxIn[96];
static const char *nfxUs="org.streamcats_movie_nfx", *cineTop="com.linvo.cinemeta_movie_top";

// Regular Nuvio's push for a profile: India rows enabled or not (zee5 may differ).
static void blob(char *out,size_t cap,int hot,int zee) {
  snprintf(out,cap,"[{\"profile_id\":1,\"settings_json\":{\"items\":["
    "{\"addon_id\":\"%s\",\"type\":\"movie\",\"catalog_id\":\"top\",\"enabled\":true,\"order\":0},"
    "{\"addon_id\":\"%s\",\"type\":\"movie\",\"catalog_id\":\"nfx\",\"enabled\":true,\"order\":1},"
    "{\"addon_id\":\"%s\",\"type\":\"movie\",\"catalog_id\":\"hotstar\",\"enabled\":%s,\"order\":2},"
    "{\"addon_id\":\"%s\",\"type\":\"movie\",\"catalog_id\":\"zee5\",\"enabled\":%s,\"order\":3}]}}]",
    CINE,ADDON,ADDON,hot?"true":"false",ADDON,zee?"true":"false");
}
// Native rows file for a profile: Indian rows locally visible everywhere (the
// stale state that used to leak through on profiles that hide them).
static void nativeRows(int profile) {
  char name[64],text[2048];
  snprintf(text,sizeof text,"limite 10\nordem 1\nmigracao 197\n"
    "linha %s\t0\t0\t1\t0\tTop\nlinha social_activity\t0\t0\t1\t0\tFriends\n"
    "linha %s\t0\t0\t1\t0\tNetflix US\nlinha %s\t0\t0\t1\t0\tHotstar\n"
    "linha %s\t0\t0\t1\t0\tZee5\nlinha %s\t0\t0\t1\t0\tNetflix IN\n",cineTop,nfxUs,hotstar,zee5,nfxIn);
  snprintf(name,sizeof name,"fileirasui-p%d.txt",profile); assert(dados_gravar(name,text));
}
// sync.c's switch: profile rows file, cached snapshot of the effective home
// profile (primary for an inheriting profile), projection, then addon list.
static void abrir(int profile,int homeProfile) {
  fil_definir_perfil(profile);
  catordem_cache_carregar(homeProfile,OWNER);
  const char *k[FIL_MAX]; int en[FIL_MAX], n=catordem_n();
  for (int i=0;i<n;i++){k[i]=catordem_chave(i);en[i]=!catordem_oculta(k[i],k[i]);}
  fil_projecao_conta(k,en,n);
  const char *ids[]={CINE,ADDON,ADDON}, *bases[]={CINEB,US,IN};
  fil_podar_catalogos(ids,bases,3,profile);
}
static void expect(int profile,int hot,int zee) {
  assert(fil_tem_projecao());
  assert(fil_oculta(hotstar)==!hot);
  assert(fil_oculta(zee5)==!zee);
  assert(!fil_oculta(nfxUs) && !fil_oculta(nfxIn) && !fil_oculta(cineTop) && !fil_oculta("social_activity"));
  // Order follows the profile's synced order; Friends stays after its anchor.
  const char *c[]={cineTop,"social_activity",nfxUs,hotstar,zee5,nfxIn}; int o[6];
  assert(fil_unir(c,6,o,6)==6);
  assert(!strcmp(c[o[0]],cineTop) && !strcmp(c[o[1]],"social_activity") && !strcmp(c[o[2]],nfxUs) &&
         !strcmp(c[o[3]],hotstar) && !strcmp(c[o[4]],zee5) && !strcmp(c[o[5]],nfxIn));
  printf("  profile %d: hotstar %s, zee5 %s\n",profile,hot?"shown":"hidden",zee?"shown":"hidden");
}
int main(void) {
  snprintf(directory,sizeof directory,"%s/nuvio-home-profiles-XXXXXX",getenv("TMPDIR")?getenv("TMPDIR"):"/tmp");
  assert(mkdtemp(directory));
  nv_addon_chave(IN,fpIn,sizeof fpIn); assert(fpIn[0] && strcmp(fpIn,ADDON));
  // Native keys of the second instance (addons_catalogo_chave: fingerprint).
  snprintf(hotstar,sizeof hotstar,"%s_movie_hotstar",fpIn);
  snprintf(zee5,sizeof zee5,"%s_movie_zee5",fpIn);
  snprintf(nfxIn,sizeof nfxIn,"%s_movie_nfx",fpIn);
  char json[4096];
  blob(json,sizeof json,1,1); assert(catordem_cache_gravar(1,OWNER,json));   // primary: India on
  blob(json,sizeof json,0,0); assert(catordem_cache_gravar(2,OWNER,json));   // India hidden
  blob(json,sizeof json,1,0); assert(catordem_cache_gravar(4,OWNER,json));   // mixed
  blob(json,sizeof json,0,0); assert(catordem_cache_gravar(3,OWNER,json));   // own blob, unused while inheriting
  for (int p=1;p<=4;p++) nativeRows(p);
  // 1 -> 2 -> 3 (inherits primary) -> 4 -> 2 -> 1, each profile's own settings.
  abrir(1,1); expect(1,1,1);
  abrir(2,2); expect(2,0,0);
  abrir(3,1); expect(3,1,1);
  abrir(4,4); expect(4,1,0);
  abrir(2,2); expect(2,0,0);
  abrir(1,1); expect(1,1,1);
  // Before the account addon list arrives on a fresh switch, a disabled India
  // row is still hidden through the cached snapshot once the list is known;
  // no snapshot (never-synced profile) falls back to that profile's own file.
  fil_definir_perfil(5); catordem_cache_carregar(5,OWNER);
  fil_projecao_conta(NULL,NULL,0); nativeRows(5); fil_recarregar();
  assert(!fil_tem_projecao() && !fil_oculta(hotstar));
  // Native files keep their stored keys and local choices after all switches.
  char *p2=dados_ler("fileirasui-p2.txt"); assert(p2 && strstr(p2,hotstar) && strstr(p2,zee5)); free(p2);
  puts("PASS: each profile applies its own synced India-row visibility across 1->2->3->4->2->1 switches, "
       "including a second addon instance, primary inheritance and stale Native-local rows; nothing carries over");
  // Display names are resolved by the full catalogue identity, never title or
  // an exposed configured URL, and must not survive switching profile stores.
  char name[48];
  fil_registrar(nfxUs,"Netflix","US catalogue","movie",4);
  fil_registrar(nfxIn,"Netflix","India catalogue","movie",4);
  fil_catalogo_addon_copiar(nfxUs,name,sizeof name);assert(!strcmp(name,"US catalogue"));
  fil_catalogo_addon_copiar(nfxIn,name,sizeof name);assert(!strcmp(name,"India catalogue"));
  fil_registrar("continue_watching","Continue Watching","Not an addon","series",1);
  fil_catalogo_addon_copiar("continue_watching",name,sizeof name);assert(!name[0]);
  fil_catalogo_addon_copiar("collection_fixture",name,sizeof name);assert(!name[0]);
  fil_catalogo_addon_copiar("unknown",name,sizeof name);assert(!name[0]);
  fil_definir_perfil(6);
  fil_catalogo_addon_copiar(nfxUs,name,sizeof name);assert(!name[0]);
  puts("PASS addon display-name snapshots: duplicate titles/instances, app/collection/unknown exclusion and profile isolation");
  return 0;
}

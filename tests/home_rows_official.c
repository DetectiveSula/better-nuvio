// Better Nuvio's real Home pipeline pieces for tests/home_rows_official.sh:
// manifest eligibility (exigeBusca/foraDaHome sliced from descoberta.c), row
// identity (catordem_tipo_chave, as addons_catalogo_chave), the synced snapshot
// (catordem.c), the projection, pinToTop and follow order (fileiras.c), then the
// visible Home order printed one key per line. Only storage/addon locks stubbed.
#define _DARWIN_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "fileiras.h"
#include "catordem.h"
#include "js.h"
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
#include "official-eligibility.inc"

#define MAXK 128
static char ids[8][96], bases[8][256], keys[MAXK][192], natives[MAXK][192], members[16][192], colls[16][96];
static int nAddons, nKeys, nNatives, natOculta[MAXK], nMembers, nColls, collPin[16], follow;
static char *payload;

int main(int argc,char **argv){
  assert(argc==2);
  snprintf(directory,sizeof directory,"%s/nuvio-home-official-XXXXXX",getenv("TMPDIR")?getenv("TMPDIR"):"/tmp");
  assert(mkdtemp(directory));
  FILE *in=fopen(argv[1],"r"); assert(in);
  static char line[1<<16];
  while (fgets(line,sizeof line,in)) {
    line[strcspn(line,"\n")]=0;
    char *tab=strchr(line,'\t'); assert(tab); *tab++=0;
    if (!strcmp(line,"follow")) follow=atoi(tab);
    else if (!strcmp(line,"addon")) { char *b=strchr(tab,'\t'); *b++=0; snprintf(ids[nAddons],96,"%s",tab); snprintf(bases[nAddons++],256,"%s",b); }
    else if (!strcmp(line,"cat")) {
      char *json=strchr(tab,'\t'); *json++=0; int a=atoi(tab);
      const char *p=json,*f=json+strlen(json);
      if (exigeBusca(p,f) || foraDaHome(p,f)) continue;   // descoberta.c eligibility
      char tipo[32]="",id[96]="";
      js_texto_raiz_em(p,f,"type",tipo,sizeof tipo); catordem_tipo_chave(tipo,tipo,sizeof tipo);
      js_texto_raiz_em(p,f,"id",id,sizeof id);
      snprintf(keys[nKeys++],192,"%s_%s_%s",ids[a],tipo,id);  // single-instance addons_catalogo_chave
    }
    else if (!strcmp(line,"coll")) { char *pin=strchr(tab,'\t'); *pin++=0; snprintf(colls[nColls],96,"%s",tab); collPin[nColls++]=atoi(pin); }
    else if (!strcmp(line,"native")) { char *o=strchr(tab,'\t'); *o++=0; snprintf(natives[nNatives],192,"%s",tab); natOculta[nNatives++]=atoi(o); }
    else if (!strcmp(line,"member")) snprintf(members[nMembers++],192,"%s",tab);
    else if (!strcmp(line,"payload")) payload=strdup(tab);
  }
  fclose(in);
  // Home candidates: catalogues in addon x manifest order, collections, app rows.
  for (int i=0;i<nColls;i++) snprintf(keys[nKeys++],192,"collection_%s",colls[i]);
  const char *app[]={"continue_watching","social_activity","last_session"};
  for (int i=0;i<3;i++) snprintf(keys[nKeys++],192,"%s",app[i]);
  // This profile's Native rows file (storage order, personal hides).
  static char text[65536]; size_t pos=(size_t)snprintf(text,sizeof text,"limite 10\nordem 1\nmigracao 197\n");
  for (int i=0;i<nNatives;i++) pos+=(size_t)snprintf(text+pos,sizeof text-pos,"linha %s\t%d\t0\t1\t0\t%s\n",natives[i],natOculta[i],natives[i]);
  assert(dados_gravar("fileirasui-p1.txt",text));
  fil_definir_perfil(1);
  const char *idp[8],*basep[8]; for (int i=0;i<nAddons;i++){idp[i]=ids[i];basep[i]=bases[i];}
  fil_podar_catalogos(idp,basep,nAddons,1);           // the profile's account addon list
  // Collection membership from the account collections (formerly hid the row).
  { const char *mk[16]; int off[16],inside[16];
    for (int i=0;i<nMembers;i++){mk[i]=members[i];off[i]=0;inside[i]=1;}
    if (nMembers) fil_conta_reconciliar(mk,off,inside,nMembers); }
  assert(payload && catordem_ler(payload));
  { const char *pk[FIL_MAX]; int en[FIL_MAX], n=catordem_n();     // colfileiras projetarConta
    for (int i=0;i<n;i++){pk[i]=catordem_chave(i);en[i]=!catordem_oculta(pk[i],pk[i]);}
    fil_projecao_conta(pk,en,n); }
  { const char *fixas[16]; char buf[16][120]; int n=0;
    for (int i=0;i<nColls;i++) if (collPin[i]) { snprintf(buf[n],120,"collection_%s",colls[i]); fixas[n]=buf[n]; n++; }
    fil_definir_colecoes_fixas(fixas,n); }
  fil_definir_seguir_addons(follow);
  const char *cand[MAXK]; int order[MAXK];
  for (int i=0;i<nKeys;i++) cand[i]=keys[i];
  int q=fil_unir(cand,nKeys,order,MAXK);
  assert(q==nKeys);
  for (int i=0;i<q;i++) if (!fil_oculta(cand[order[i]])) printf("ROW\t%s\n",cand[order[i]]);
  return 0;
}

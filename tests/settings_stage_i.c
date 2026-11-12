// Actual production bridge and scoped JSON helpers; only Native UI/storage
// dependencies are replaced. No HTTP or provider library is linked.
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include "js.h"
#include "jsw.h"
#include "shared-test-enum.inc"
#include "shared-test-json.inc"
static int valor[AJ_N], edPerfil, profile=1;
static char owner[80]="account-one", root[512], contaTmdbLing[80],contaLegLing[80];
static void idiomaResolver(int n) { (void)n; }
static int perfis_ativo(void) { return profile; }
static const char *sessao_usuario(void) { return owner; }
static char *dados_ler(const char *name) {
  char path[700]; snprintf(path,sizeof path,"%s/%s",root,name);
  FILE *f=fopen(path,"r"); if (!f) return NULL;
  fseek(f,0,SEEK_END); long n=ftell(f); rewind(f);
  char *out=calloc((size_t)n+1,1); assert(out); assert(fread(out,1,(size_t)n,f)==(size_t)n); fclose(f); return out;
}
static int dados_gravar(const char *name,const char *text) {
  char path[700]; snprintf(path,sizeof path,"%s/%s",root,name);
  FILE *f=fopen(path,"w"); assert(f); fputs(text,f); return fclose(f)==0;
}
static void ajustesHashBytes(unsigned long long *h,const void *p,size_t n) {
  const unsigned char *s=p; while(n--) { *h^=*s++; *h*=1099511628211ULL; }
}
static void nop(const char *p) {(void)p;}
#define ling_conta_legenda nop
#define ling_conta_legenda2 nop
#define ling_conta_audio nop
#define ling_conta_audio2 nop
#define fonteregra_do_blob nop
#define selospacote_conta_do_blob nop
void fil_definir_seguir_addons(int ligado) { (void)ligado; }
void ajustes_compartilhados_observar(void);
static int gravar(void) { ajustes_compartilhados_observar(); return 1; }
#include "fixtures/settings-restoration/stage-j6-isolated.h"
#include "ajustes_shared.inc"
static const char *yes="{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":true}}}}";
static const char *no="{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":false}}}}";
static void edit(int op,int v) {valor[op]=v;gravar();}
int main(int argc,char **argv) {
  assert(argc==2 || argc==3); snprintf(root,sizeof root,"%s",argv[1]);
  if (argc==3) {
    ajustes_compartilhados_contexto();
    assert(valor[AJ_HERO]==1 && ajustes_compartilhados_pendentes()==1);
    puts("PASS: fresh process restores the selected profile's offline pending edit"); return 0;
  }
  ajustes_compartilhados_contexto();
  assert(!ajustes_compartilhados_pendentes()); // activation is not an edit
  for (size_t i=0;i<AJ_SHARED_N;i++) {
    char json[400];
    snprintf(json,sizeof json,"{\"version\":1,\"features\":{\"%s\":{\"%s\":{\"type\":\"boolean\",\"value\":false}}}}",ajSharedMap[i].feature,ajSharedMap[i].key);
    ajustes_compartilhados_aplicar(json); assert(valor[ajSharedMap[i].op]==1);
    edit(ajSharedMap[i].op,0); char *out=NULL;
    assert(ajustes_compartilhados_mesclar(json,&out)==1 && out);
    assert(ajSharedRemote(out,i)==1 && !strstr(out,"c_settings") && !strstr(out,"c_devices"));
    ajustes_compartilhados_confirmar(out); free(out); assert(!ajustes_compartilhados_pendentes());
  }
  puts("PASS: every shared control imports/exports the original typed boolean and acknowledges without echo");
  const char *badEnvelope[]={"[{\"nested\":{\"settings_json\":{}}}]","[{\"settings_json\":{}},{}]","[]junk","[{\"settings_json\":{},\"settings_json\":null}]","[{\"settings_json\":\"{}\"}]"};
  for (size_t i=0;i<sizeof badEnvelope/sizeof *badEnvelope;i++) {
    char *blob=NULL; assert(!ajustes_compartilhados_resposta(badEnvelope[i],&blob) && !blob);
  }
  char *blob=NULL;
  assert(ajustes_compartilhados_resposta("[]",&blob) && !strcmp(blob,"{}")); free(blob);
  assert(ajustes_compartilhados_resposta("[{\"settings_json\":null}]",&blob) && !strcmp(blob,"{}")); free(blob);
  ajustes_compartilhados_aplicar(yes); edit(AJ_HERO,1);
  const char *mixed="{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":true},\"poster_labels_enabled\":{\"type\":\"boolean\",\"value\":false}}}}";
  ajustes_compartilhados_aplicar(mixed);
  assert(valor[AJ_HERO]==1 && valor[AJ_ROTULOS]==1 && ajustes_compartilhados_pendentes()==1);
  char *mixedOut=NULL; assert(ajustes_compartilhados_mesclar(mixed,&mixedOut)==1);
  assert(strstr(mixedOut,"\"poster_labels_enabled\":{\"type\":\"boolean\",\"value\":false}")); free(mixedOut);
  // Missing values, malformed types and nested same-name impostors cannot reset.
  const char *invalid[]={"{", "{\"features\":{}}junk", "{\"version\":2}",
    "{\"features\":{},\"features\":{}}", "{\"x\":01}", "{\"x\":[1,]}",
    "{\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":false,\"value\":true}}}}"};
  for (size_t i=0;i<sizeof invalid/sizeof *invalid;i++) {
    assert(!ajustes_compartilhados_validar(invalid[i]));
    assert(!ajustes_compartilhados_aplicar(invalid[i])); assert(valor[AJ_HERO]==1);
    char *out=NULL; assert(!ajustes_compartilhados_mesclar(invalid[i],&out) && !out);
  }
  const char *wrong="{\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"string\",\"value\":\"true\"}},\"other\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":true}}}}";
  ajustes_compartilhados_aplicar(wrong); assert(valor[AJ_HERO]==1);
  char *out=NULL; assert(!ajustes_compartilhados_mesclar(wrong,&out));
  ajustes_compartilhados_aplicar("{}"); assert(valor[AJ_HERO]==1 && ajustes_compartilhados_pendentes()==1);
  puts("PASS: missing, partial, wrong-type, duplicate, malformed, unknown-version and wrong-feature data preserve preferences");
  // Retry/restart keeps offline pending edits, without touching Native files.
  dados_gravar("fileirasui-p1.txt","Friends Watching\n30+ rows\n");
  dados_gravar("debrid.txt","private-local-placeholder\n");
  pid_t child=fork(); assert(child>=0);
  if (!child) { execl(argv[0],argv[0],argv[1],"restart",(char *)NULL); _exit(127); }
  int status; assert(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
  ajustes_compartilhados_esquecer(); valor[AJ_HERO]=0;
  ajustes_compartilhados_contexto(); assert(valor[AJ_HERO]==1 && ajustes_compartilhados_pendentes()==1);
  assert(ajustes_compartilhados_mesclar(yes,&out)==1);
  edit(AJ_HERO,0); // edit during HTTP: ack must not clear newer value
  ajustes_compartilhados_confirmar(out); free(out); out=NULL;
  assert(ajustes_compartilhados_pendentes()==1);
  assert(ajustes_compartilhados_mesclar(no,&out)==1);
  ajustes_compartilhados_confirmar(out); ajustes_compartilhados_aplicar(out); free(out); out=NULL;
  assert(valor[AJ_HERO]==0 && !ajustes_compartilhados_pendentes());
  assert(!ajustes_compartilhados_mesclar(yes,&out));
  puts("PASS: durable offline edit, reconnect, in-flight edit acknowledgement, restart and upload-loop prevention");
  // No common baseline: conservatively accept existing remote on conflict.
  profile=2; ajustes_compartilhados_contexto(); edit(AJ_HERO,1);
  assert(!ajustes_compartilhados_mesclar(yes,&out));
  ajustes_compartilhados_aplicar(yes); assert(valor[AJ_HERO]==0 && !ajustes_compartilhados_pendentes());
  edit(AJ_HERO,1); profile=1;
  assert(!ajustes_compartilhados_aplicar(no)); // old state cannot cross profile
  ajustes_compartilhados_contexto(); assert(valor[AJ_HERO]==0 && !ajustes_compartilhados_pendentes());
  profile=2; ajustes_compartilhados_contexto(); assert(valor[AJ_HERO]==1 && ajustes_compartilhados_pendentes()==1);
  snprintf(owner,sizeof owner,"account-two"); ajustes_compartilhados_contexto();
  assert(!ajustes_compartilhados_pendentes()); ajustes_compartilhados_aplicar(yes); assert(valor[AJ_HERO]==0);
  snprintf(owner,sizeof owner,"account-one"); ajustes_compartilhados_contexto();
  assert(valor[AJ_HERO]==1 && ajustes_compartilhados_pendentes()==1);
  puts("PASS: account/profile isolation, per-field conflict handling and offline profile recovery");
  int before[AJ_N],after[AJ_N]; memcpy(before,valor,sizeof before); memcpy(after,valor,sizeof after);
  before[AJ_HERO]=0; after[AJ_HERO]=1;
  ajustes_compartilhados_edicao(3,before,after);
  assert(ajSharedProfile==2); // editor must not redirect active sync
  profile=3; ajustes_compartilhados_contexto();
  assert(valor[AJ_HERO]==1 && ajustes_compartilhados_pendentes()==1);
  profile=2; ajustes_compartilhados_contexto();
  puts("PASS: inactive-profile editor journals its own changes without rebinding active uploads");
  profile=4; ajustes_compartilhados_contexto();
  assert(!ajustes_compartilhados_mesclar("{}",&out) && !out);
  edit(AJ_HERO,!valor[AJ_HERO]);
  assert(ajustes_compartilhados_mesclar("{}",&out)==1 && out && strstr(out,"\"type\":\"boolean\"")); free(out); out=NULL;
  profile=2; ajustes_compartilhados_contexto();
  int native=valor[AJ_LEG2_COR]=3, device=valor[AJ_TUNNELING]=1;
  const char *base="{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":true}},\"future\":{\"keep\":42},\"player_settings\":{\"tunneling_enabled\":{\"type\":\"boolean\",\"value\":true}}},\"c_settings_v1\":{\"store1\":\"bad\"}}";
  ajustes_compartilhados_aplicar(base);
  assert(valor[AJ_LEG2_COR]==native && valor[AJ_TUNNELING]==device);
  assert(ajustes_compartilhados_mesclar(base,&out)==1);
  assert(strstr(out,"\"keep\":42") && strstr(out,"\"store1\":\"bad\""));
  assert(!strstr(out,"private-local-placeholder") && !strstr(out,"Friends Watching")); free(out);
  char *rows=dados_ler("fileirasui-p1.txt"),*cred=dados_ler("debrid.txt");
  assert(!strcmp(rows,"Friends Watching\n30+ rows\n") && !strcmp(cred,"private-local-placeholder\n")); free(rows);free(cred);
  puts("PASS: Native/device preferences, credentials, Friends Watching and home-row files preserved; unknown remote fields retained");
  return 0;
}

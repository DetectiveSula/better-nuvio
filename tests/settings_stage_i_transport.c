// Regression: missing portable keys and a push before first cloud apply lose settings.
#include "../src/ajustes.c"
#include "nvtmp.h"
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#define main conta_ajustes_main
#include "conta_ajustes.c"
#undef main

static char *cloud;
static int pushes, pushRequests, failPull, editDuringPull, failPush, stalePull;
static char *lastWrite;
int nuvem_configurar(const char *d) { (void)d; return 1; }
int nuvem_pronta(void) { return 1; }
const char *nuvem_url(void) { return "https://account.invalid"; }
const char *nuvem_anon(void) { return "test"; }
const char *nuvem_base_login(void) { return ""; }
const char *nuvem_trakt_cliente(void) { return ""; }
const char *nuvem_trakt_segredo(void) { return ""; }
const char *nuvem_simkl_cliente(void) { return ""; }
const char *nuvem_simkl_app(void) { return ""; }
const char *nuvem_ultimo_erro(void) { return ""; }
int nuvem_freio_ativo(void) { return 0; }
void nuvem_falhou(void) {}
void nuvem_ok(void) {}
int nuvem_erro_ausente(const char *s) { (void)s; return 0; }
void nuvem_url_escapar(const char *v, char *d, unsigned n) { snprintf(d, n, "%s", v); }
char *nuvem_post(const char *p, const char *c, const char *b, int *st) {
  (void)p; (void)c; (void)b; *st = 200; return strdup("{}");
}
char *nuvem_tabela(const char *t, const char *q, const char *b, int *st) {
  (void)t; (void)q; (void)b; *st = 200; return strdup("[]");
}
char *nuvem_addon_alternar(const char *q, int a, const char *b, unsigned p, int *st) {
  (void)q; (void)a; (void)b; (void)p; *st = 200; return strdup("[]");
}
char *nuvem_rpc_com(const char *f, const char *c, const char *b, int *st) {
  (void)b; *st = 200;
  if (!strcmp(f, "sync_pull_profiles"))
    return strdup("[{\"profile_index\":1,\"name\":\"Test\"}]");
  if (!strcmp(f, "sync_pull_profile_settings_blob")) {
    if (failPull) { *st = 503; return strdup("unavailable"); }
    if (stalePull) perfis_invalidar_sessao();
    if (editDuringPull) sync_proteger_ajustes_locais();
    if (!cloud) return strdup("[]");
    char *r = malloc(strlen(cloud) + 32); assert(r);
    sprintf(r, "[{\"settings_json\":%s}]", cloud); return r;
  }
  if (!strcmp(f, "sync_push_profile_settings_blob")) {
    pushRequests++;
    free(lastWrite); lastWrite=strdup(c);
    if (failPush==1) { *st=503; return strdup("offline"); }
    const char *p = strstr(c, "\"p_settings_json\":"); assert(p);
    p = strchr(p, ':') + 1;
    const char *e = js_fim(p); assert(e);
    free(cloud); cloud = strndup(p, (size_t)(e - p)); pushes++;
    if (failPush==2) { *st=503; return strdup("ambiguous timeout after server commit"); }
    return strdup("null");
  }
  return strdup("[]");
}
static void login(void) {
  // Payload: {"sub":"settings-test","exp":4102444800}; no network/auth service.
  dados_gravar("sessao.txt", "eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJzZXR0aW5ncy10ZXN0IiwiZXhwIjo0MTAyNDQ0ODAwfQ.test\nrefresh\n0\n");
  sessao_iniciar(); assert(sessao_logada());
  sync_reaplicar_ajustes();
}
static void cycle(void) {
  sync_iniciar();
  for (int i = 0; i < 5000 && sync_estado() == SYNC_RODANDO; i++) usleep(1000);
  assert(sync_estado() == SYNC_PRONTO);
  sync_passo(SDL_GetTicks());
}
static void check(int ok, const char *name) {
  if (!ok) { fprintf(stderr, "FAIL: %s\n", name); exit(1); }
}
int main(int argc,char **argv) {
  assert(argc==2); setenv("NUVIO_DADOS",argv[1],1);
  dados_iniciar(NULL); player_dir(argv[1]); ajustes_dir(argv[1]); fil_definir_perfil(1);
  login();
  for (size_t i=0;i<AJ_SHARED_N;i++)
    check(OPCOES[ajSharedMap[i].op].tipo==OP_ESCOLHA && OPCOES[ajSharedMap[i].op].n==2 &&
          (OPCOES[ajSharedMap[i].op].valores==V_LIGA || ajSharedMap[i].op==AJ_RAIL) &&
          !soNaTv(ajSharedMap[i].op),"shared allowlist uses actual two-choice profile controls");
  cloud=strdup("{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":false}},\"future\":{\"keep\":42}}}");
  cycle();
  check(valor[AJ_HERO]==1 && pushes==0,"initial official remote import without defaults upload");
  valor[AJ_HERO]=0; assert(gravar()); sync_proteger_ajustes_locais();
  failPull=1; cycle(); check(pushes==0 && valor[AJ_HERO]==0,"failed pull cannot upload stale base or erase pending edit");
  failPull=0; failPush=1; cycle();
  check(pushes==0 && ajustes_compartilhados_pendentes()==1,"failed push retains pending edit");
  failPush=0; cycle();
  check(pushes==1 && ajustes_compartilhados_pendentes()==0,"successful retry acknowledges exactly the sent setting");
  check(lastWrite && strstr(lastWrite,"\"p_profile_id\":1") && strstr(lastWrite,"\"p_platform\":\"tv\"") &&
        strstr(lastWrite,"\"p_origin_client_id\":") && strstr(lastWrite,dados_cliente_id()) &&
        strstr(lastWrite,"\"p_settings_json\"") && strstr(lastWrite,"\"type\":\"boolean\"") &&
        strstr(lastWrite,"\"value\":true") && strstr(lastWrite,"\"keep\":42") &&
        !strstr(lastWrite,"c_settings_v1") && !strstr(lastWrite,"c_devices_v1"),"exact original-format RPC payload, unknown features retained, no Native extensions");
  cycle(); check(pushes==1,"remote echo and repeated cycles do not upload again");
  free(cloud); cloud=strdup("{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":false}}}}");
  cycle(); check(valor[AJ_HERO]==1 && pushes==1,"later regular-Nuvio edit applies even after a local edit and push");
  int device=valor[AJ_TUNNELING], legacy=valor[AJ_LEG2_COR];
  dados_gravar("fileirasui-p1.txt","preserve-native-home-rows\n");
  free(cloud); cloud=strdup("{\"version\":1,\"features\":{},\"c_settings_v1\":{\"values\":{\"c_heroSectionEnabled\":0},\"store1\":\"626164\"},\"c_devices_v1\":{}}");
  cycle(); char *rows=dados_ler("fileirasui-p1.txt");
  check(valor[AJ_HERO]==1 && valor[AJ_TUNNELING]==device && valor[AJ_LEG2_COR]==legacy && rows && !strcmp(rows,"preserve-native-home-rows\n"),"partial pull and old Native extensions cannot reset settings or overwrite Stage G stores"); free(rows);
  free(cloud); cloud=strdup("{\"version\":1,\"features\":{\"player_settings\":{\"preferred_audio_language\":{\"type\":\"string\",\"value\":\"fr\"},\"subtitle_preferred_language\":{\"type\":\"string\",\"value\":\"en\"}}}}");
  ling_local_audio("de");
  cycle(); check(!strcmp(ling_conta_audio_valor(),"fr") && !strcmp(ling_conta_legenda_valor(),"en"),"existing typed account language imports remain functional");
  check(!strcmp(ling_audio(),"de"),"explicit Native audio-language choice survives account language import");
  free(cloud); cloud=strdup("{\"version\":1,\"features\":{\"player_settings\":{\"preferred_audio_language\":{\"type\":\"boolean\",\"value\":true}}}}");
  cycle(); check(!strcmp(ling_conta_audio_valor(),"fr") && !strcmp(ling_conta_legenda_valor(),"en"),"partial/malformed language fields retain current profile values");
  free(cloud); cloud=strdup("{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":false}}}}");
  cycle(); valor[AJ_HERO]=0; assert(gravar()); int requests=pushRequests;
  failPush=2; cycle(); check(ajustes_compartilhados_pendentes()==1,"timeout after server commit retains pending edit until confirmation");
  failPush=0; cycle();
  check(pushRequests==requests+1 && pushes==2 && !ajustes_compartilhados_pendentes(),"fresh pull confirms ambiguous write without duplicate upload");
  valor[AJ_HERO]=1; assert(gravar());
  free(cloud); cloud=strdup("{\"version\":1,\"features\":{\"layout_settings\":{\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":true}}}}");
  stalePull=1; cycle();
  check(valor[AJ_HERO]==1 && pushes==2,"old session response cannot publish settings after context invalidation");
  puts("PASS: Stage I real sync worker, mocked HTTP, remote/local changes, offline pulls, push failure/retry, original RPC and no loops");
  free(lastWrite); free(cloud); return 0;
}

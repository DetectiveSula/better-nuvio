// Stage J0 probe (src/j0sonda.c, NV_J0_TESTE) against a mock account server:
// one settings row per (profile, platform). No network, no account.
#define _DARWIN_C_SOURCE
#include "j0sonda.h"
#include "ilha.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { SRV_LIVRE, SRV_ENUM, SRV_SO_LEITURA, SRV_VAZA_TV };
static int modo, escritas, logado = 1, perfil = 2;
static char dono[64] = "test-account-owner", dir[512];
static struct { char ns[32]; int perfil; char json[256]; int versao; } linhas[16];
static int nLinhas;

static int achar(int p, const char *ns) {
  for (int i = 0; i < nLinhas; i++) if (linhas[i].perfil == p && !strcmp(linhas[i].ns, ns)) return i;
  return -1;
}
static void semear(int p, const char *ns, const char *json) {
  int i = achar(p, ns); if (i < 0) { i = nLinhas++; linhas[i].perfil = p; snprintf(linhas[i].ns, 32, "%s", ns); }
  snprintf(linhas[i].json, 256, "%s", json); linhas[i].versao++;
}
static int extrair(const char *corpo, const char *chave, char *dst, size_t cap) {
  const char *k = strstr(corpo, chave); if (!k) return 0;
  k += strlen(chave);
  if (*k == '"') { k++; size_t n = strcspn(k, "\""); if (n >= cap) n = cap - 1; memcpy(dst, k, n); dst[n] = 0; return 1; }
  size_t n = strcspn(k, ",}"); if (n >= cap) n = cap - 1; memcpy(dst, k, n); dst[n] = 0; return 1;
}
char *sessao_rpc(const char *funcao, const char *corpo, int *st) {
  char ns[32] = "", num[16] = "", r[600];
  extrair(corpo, "\"p_platform\":", ns, sizeof ns);
  extrair(corpo, "\"p_profile_id\":", num, sizeof num);
  int p = atoi(num);
  int restrito = strcmp(ns, "tv") && strcmp(ns, "mobile") && strcmp(ns, "home_catalog_shared");
  if (modo == SRV_ENUM && restrito) { *st = 400; return strdup("{\"code\":\"22P02\",\"message\":\"invalid input value for enum\"}"); }
  if (!strcmp(funcao, "sync_pull_profile_settings_blob")) {
    int i = achar(p, ns); *st = 200;
    if (i < 0) return strdup("[]");
    snprintf(r, sizeof r, "[{\"profile_id\":%d,\"settings_json\":%s,\"updated_at\":\"2026-11-04T12:00:0%d+00:00\"}]",
             p, linhas[i].json, linhas[i].versao);
    return strdup(r);
  }
  assert(!strcmp(funcao, "sync_push_profile_settings_blob"));
  assert(strstr(corpo, "\"p_origin_client_id\":\"client-fixture\""));
  if (modo == SRV_SO_LEITURA && restrito) { *st = 400; return strdup("{\"code\":\"23514\",\"message\":\"check violation\"}"); }
  const char *j = strstr(corpo, "\"p_settings_json\":"); assert(j); j += 18;
  char doc[256]; size_t n = strcspn(j, "}") + 1; if (j[n] == '}') n++; /* probe docs nest one level */
  if (!strncmp(j, "{}", 2)) n = 2;
  memcpy(doc, j, n); doc[n] = 0;
  semear(p, ns, doc); escritas++;
  if (modo == SRV_VAZA_TV) semear(p, "tv", "{\"version\":1,\"features\":{\"changed\":true}}");
  *st = 200; return strdup("null");
}
int sessao_logada(void) { return logado; }
const char *sessao_usuario(void) { return dono; }
int perfis_ativo(void) { return perfil; }
const char *dados_cliente_id(void) { return "client-fixture"; }
char *dados_ler(const char *nome) {
  char p[700]; snprintf(p, sizeof p, "%s/%s", dir, nome); FILE *f = fopen(p, "r"); if (!f) return NULL;
  char *b = calloc(1, 4096); size_t n = fread(b, 1, 4095, f); b[n] = 0; fclose(f); return b;
}
int dados_gravar(const char *nome, const char *texto) {
  char p[700]; snprintf(p, sizeof p, "%s/%s", dir, nome); FILE *f = fopen(p, "w"); assert(f); fputs(texto, f); fclose(f); return 1;
}
static int avisos; static char ultimo[600];
void ilha_avisar(const char *chave, int tipo, const char *icone, const char *texto, unsigned ms, int tecla) {
  (void)chave; (void)tipo; (void)icone; (void)ms; (void)tecla; avisos++; snprintf(ultimo, sizeof ultimo, "%s", texto);
}
static const char *executar(int fase, const char *token) {
  int antes = avisos;
  j0sonda_pedir(fase, token);
  for (int i = 0; i < 5000 && avisos == antes; i++) { usleep(1000); j0sonda_passo(); }
  assert(avisos == antes + 1);
  return ultimo;
}
static void token(const char *texto, char out[9]) {
  const char *t = strstr(texto, "token "); assert(t); memcpy(out, t + 6, 8); out[8] = 0;
}
static void reiniciar(int m) {
  modo = m; nLinhas = 0; escritas = 0;
  semear(2, "tv", "{\"version\":1,\"features\":{\"layout_settings\":{\"secret_tv_marker\":1}}}");
  semear(2, "home_catalog_shared", "{\"items\":[{\"addon_id\":\"secret_home_marker\"}]}");
  semear(2, "mobile", "{\"features\":{\"secret_mobile_marker\":true}}");
  char p[700]; snprintf(p, sizeof p, "%s/j0-sonda.txt", dir); unlink(p);
}
int main(void) {
  snprintf(dir, sizeof dir, "%s/nuvio-j0-XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  assert(mkdtemp(dir));
  char tk[9]; const char *r;
  fflush(stdout);
  char logPath[700]; snprintf(logPath, sizeof logPath, "%s/log.txt", dir);
  assert(freopen(logPath, "w", stdout));

  // Free-text namespace: read, write, isolation, cleanup.
  reiniciar(SRV_LIVRE);
  r = executar(J0_LER, ""); assert(strstr(r, "leitura aceita") && strstr(r, "vazio") && escritas == 0); token(r, tk);
  r = executar(J0_GRAVAR, "00000000"); assert(strstr(r, "token invalido") && escritas == 0);
  r = executar(J0_GRAVAR, tk); assert(strstr(r, "PASSOU") && escritas == 1);
  assert(strstr(linhas[achar(2, "better_nuvio")].json, tk));
  r = executar(J0_LIMPAR, tk); assert(strstr(r, "limpeza OK") && !strcmp(linhas[achar(2, "better_nuvio")].json, "{}"));
  r = executar(J0_LIMPAR, tk); assert(strstr(r, "limpeza recusada"));   // nothing left to clean
  // Another account or profile cannot use the token.
  reiniciar(SRV_LIVRE);
  r = executar(J0_LER, ""); token(r, tk);
  perfil = 3; r = executar(J0_GRAVAR, tk); assert(strstr(r, "token invalido") && escritas == 0); perfil = 2;
  snprintf(dono, sizeof dono, "another-owner"); r = executar(J0_GRAVAR, tk); assert(strstr(r, "token invalido") && escritas == 0);
  snprintf(dono, sizeof dono, "test-account-owner");
  // Restricted namespace: the read is refused and nothing is written.
  reiniciar(SRV_ENUM);
  r = executar(J0_LER, ""); assert(strstr(r, "leitura recusada") && strstr(r, "22P02") && !strstr(r, "token") && escritas == 0);
  // Reads accepted, writes refused.
  reiniciar(SRV_SO_LEITURA);
  r = executar(J0_LER, ""); token(r, tk);
  r = executar(J0_GRAVAR, tk); assert(strstr(r, "escrita recusada") && escritas == 0);
  // A server that touches "tv" on the probe write is caught.
  reiniciar(SRV_VAZA_TV);
  r = executar(J0_LER, ""); token(r, tk);
  r = executar(J0_GRAVAR, tk); assert(strstr(r, "FALHOU") && strstr(r, "MUDARAM"));
  // Signed out: nothing runs.
  logado = 0; r = executar(J0_LER, ""); assert(strstr(r, "entre numa conta")); logado = 1;
  fflush(stdout); freopen("/dev/tty", "w", stdout);
  char *log = dados_ler("log.txt"); assert(log);
  // Privacy: no document contents, owner id or client id in logs/notices.
  assert(!strstr(log, "secret_") && !strstr(log, "test-account-owner") && !strstr(log, "another-owner") &&
         !strstr(log, "client-fixture") && !strstr(ultimo, "secret_"));
  free(log);
  fprintf(stderr, "PASS: J0 probe - free-text namespace passes read/write/isolation/cleanup; wrong token, account or "
          "profile write nothing; enum and read-only servers reported unsupported; a server touching tv is caught; logs carry no contents\n");
  return 0;
}

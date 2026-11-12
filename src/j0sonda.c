#include "j0sonda.h"

#ifndef NV_J0_SONDA
void j0sonda_pedir(int fase, const char *token) { (void)fase; (void)token; }
void j0sonda_passo(void) {}
const char *j0sonda_resultado(void) { return ""; }
#else
#include "dados.h"
#include "ilha.h"
#include "jsw.h"
#include "perfis.h"
#include "sessao.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define J0_NS "better_nuvio"
#define J0_ARQ "j0-sonda.txt"
#define J0_PRAZO_GRAVAR_S (30 * 60)       // write must follow the read within 30 min
#define J0_PRAZO_LIMPAR_S (24 * 60 * 60)  // cleanup stays possible for a day

// The three regular-Nuvio namespaces whose rows must not change.
static const char *const OUTROS[3] = { "tv", "home_catalog_shared", "mobile" };

static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static int rodando, novo;
static char resultado[600];

// FNV-1a over the raw response (settings_json and updated_at): a change
// detector, not a cryptographic hash.
static unsigned long long impressao(const char *s) {
  unsigned long long h = 1469598103934665603ULL;
  for (; s && *s; s++) h = (h ^ (unsigned char)*s) * 1099511628211ULL;
  return h;
}

static char *puxar(int perfil, const char *ns, int *st) {
  char corpo[160];
  snprintf(corpo, sizeof corpo, "{\"p_profile_id\":%d,\"p_platform\":\"%s\"}", perfil, ns);
  *st = 0;
  return sessao_rpc("sync_pull_profile_settings_blob", corpo, st);
}

static int empurrar(int perfil, const char *json) {
  Jsw w; int st = 0; char *r;
  jsw_iniciar(&w); jsw_obj_ini(&w);
  jsw_ci(&w, "p_profile_id", perfil);
  jsw_cs(&w, "p_platform", J0_NS);
  jsw_chave(&w, "p_settings_json"); jsw_bruto(&w, json);
  jsw_cs(&w, "p_origin_client_id", dados_cliente_id());
  jsw_obj_fim(&w);
  r = sessao_rpc("sync_push_profile_settings_blob", jsw_texto_final(&w), &st);
  jsw_livre(&w); free(r);
  return st;
}

// PostgREST error code only ("PGRST202", "22P02", "23514", ...).
static void codigoErro(const char *r, char *dst, size_t cap) {
  const char *c = r ? strstr(r, "\"code\"") : NULL;
  dst[0] = 0;
  if (c && (c = strchr(c + 6, '"'))) {   // opening quote of the value
    size_t n = strcspn(c + 1, "\"");
    if (n >= cap) n = cap - 1;
    memcpy(dst, c + 1, n); dst[n] = 0;
  }
}

static int ok2xx(int st) { return st >= 200 && st < 300; }
static int vazio(const char *r) {
  while (r && (*r == ' ' || *r == '\n' || *r == '\r' || *r == '\t')) r++;
  return r && r[0] == '[' && r[1] == ']';
}

typedef struct { char dono[160], token[17]; int perfil, escrito; long long quando; unsigned long long fp[3]; } Base;

static int lerBase(Base *b) {
  char *t = dados_ler(J0_ARQ);
  int ok = t && sscanf(t, "v1 %159s %d %16s %lld %d %llx %llx %llx", b->dono, &b->perfil, b->token,
                       &b->quando, &b->escrito, &b->fp[0], &b->fp[1], &b->fp[2]) == 8;
  free(t);
  return ok;
}
static void gravarBase(const Base *b) {
  char t[400];
  snprintf(t, sizeof t, "v1 %s %d %s %lld %d %llx %llx %llx\n", b->dono, b->perfil, b->token,
           b->quando, b->escrito, b->fp[0], b->fp[1], b->fp[2]);
  dados_gravar(J0_ARQ, t);
}

static void novoToken(char out[17]) {
  unsigned char r[4] = {0};
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0 || read(fd, r, sizeof r) != (ssize_t)sizeof r) {
    unsigned long long x = impressao(sessao_usuario()) ^ (unsigned long long)time(NULL) ^ (unsigned long long)getpid();
    memcpy(r, &x, sizeof r);
  }
  if (fd >= 0) close(fd);
  snprintf(out, 17, "%02X%02X%02X%02X", r[0], r[1], r[2], r[3]);
}

static void publicar(const char *texto) {
  pthread_mutex_lock(&trava);
  snprintf(resultado, sizeof resultado, "%s", texto);
  novo = 1;
  pthread_mutex_unlock(&trava);
  printf("[j0] %s\n", texto);
  fflush(stdout);
}

// Fingerprints of the other namespaces; -1 when any read failed.
static int impressoes(int perfil, unsigned long long fp[3]) {
  for (int i = 0; i < 3; i++) {
    int st; char *r = puxar(perfil, OUTROS[i], &st);
    if (!ok2xx(st)) { free(r); return -1; }
    fp[i] = impressao(r);
    printf("[j0] %s: HTTP %d, %u bytes, impressao %016llx\n", OUTROS[i], st, r ? (unsigned)strlen(r) : 0u, fp[i]);
    free(r);
  }
  return 0;
}

typedef struct { int fase; char token[17]; } Pedido;

static void faseLer(int perfil, const char *dono) {
  int st; char *r = puxar(perfil, J0_NS, &st), cod[32], msg[600];
  Base b;
  codigoErro(r, cod, sizeof cod);
  printf("[j0] leitura %s perfil %d: HTTP %d, %u bytes%s%s\n", J0_NS, perfil, st,
         r ? (unsigned)strlen(r) : 0u, cod[0] ? ", codigo " : "", cod);
  if (!ok2xx(st)) {
    free(r);
    snprintf(msg, sizeof msg, "J0 %s: leitura recusada (HTTP %d%s%s) - namespace nao suportado; nada gravado",
             J0_NS, st, cod[0] ? ", " : "", cod);
    publicar(msg);
    return;
  }
  int jaExiste = !vazio(r);
  free(r);
  memset(&b, 0, sizeof b);
  if (impressoes(perfil, b.fp) < 0) {
    snprintf(msg, sizeof msg, "J0 %s: leitura OK (HTTP %d), mas a base de tv/home/mobile falhou; tente de novo", J0_NS, st);
    publicar(msg);
    return;
  }
  snprintf(b.dono, sizeof b.dono, "%s", dono);
  b.perfil = perfil; b.quando = (long long)time(NULL);
  novoToken(b.token);
  gravarBase(&b);
  snprintf(msg, sizeof msg, "J0 %s: leitura aceita (HTTP %d, %s). Base gravada. Para gravar: token %s",
           J0_NS, st, jaExiste ? "ja existe documento" : "vazio", b.token);
  publicar(msg);
}

static int baseValida(const Base *b, const char *dono, int perfil, const char *token, long long prazo) {
  return !strcmp(b->dono, dono) && b->perfil == perfil && !strcmp(b->token, token) &&
         (long long)time(NULL) - b->quando <= prazo;
}

static void faseGravar(int perfil, const char *dono, const char *token) {
  Base b; unsigned long long depois[3]; char doc[160], msg[600], *r; int st, lido, iguais = 1;
  if (!lerBase(&b) || !baseValida(&b, dono, perfil, token, J0_PRAZO_GRAVAR_S)) {
    publicar("J0 " J0_NS ": token invalido, vencido ou de outra conta/perfil - nada gravado");
    return;
  }
  snprintf(doc, sizeof doc, "{\"schema\":\"better-nuvio-probe\",\"version\":0,\"probe\":\"%s\"}", b.token);
  st = empurrar(perfil, doc);
  printf("[j0] escrita %s perfil %d: HTTP %d, %u bytes\n", J0_NS, perfil, st, (unsigned)strlen(doc));
  if (!ok2xx(st)) {
    snprintf(msg, sizeof msg, "J0 %s: escrita recusada (HTTP %d) - namespace nao suportado para gravar", J0_NS, st);
    publicar(msg);
    return;
  }
  b.escrito = 1; gravarBase(&b);
  r = puxar(perfil, J0_NS, &lido);
  int voltou = ok2xx(lido) && r && strstr(r, b.token) && strstr(r, "better-nuvio-probe");
  free(r);
  if (impressoes(perfil, depois) < 0) iguais = -1;
  else for (int i = 0; i < 3; i++) if (depois[i] != b.fp[i]) { iguais = 0; printf("[j0] %s MUDOU\n", OUTROS[i]); }
  if (voltou && iguais == 1)
    snprintf(msg, sizeof msg, "J0 %s: PASSOU - escrita HTTP %d, leitura de volta OK, tv/home/mobile inalterados. "
             "Limpeza: clean?token=%s", J0_NS, st, b.token);
  else
    snprintf(msg, sizeof msg, "J0 %s: FALHOU - escrita HTTP %d, leitura de volta %s, outros namespaces %s. "
             "Limpeza: clean?token=%s", J0_NS, st, voltou ? "OK" : "sem a prova",
             iguais == 1 ? "inalterados" : iguais == 0 ? "MUDARAM" : "sem leitura", b.token);
  publicar(msg);
}

static void faseLimpar(int perfil, const char *dono, const char *token) {
  Base b; int st, lido; char *r, msg[200];
  if (!lerBase(&b) || !b.escrito || !baseValida(&b, dono, perfil, token, J0_PRAZO_LIMPAR_S)) {
    publicar("J0 " J0_NS ": limpeza recusada - token invalido, vencido ou nada gravado");
    return;
  }
  st = empurrar(perfil, "{}");
  r = puxar(perfil, J0_NS, &lido);
  int limpo = ok2xx(lido) && r && !strstr(r, "better-nuvio-probe");
  free(r);
  if (ok2xx(st) && limpo) { b.escrito = 0; gravarBase(&b); }
  snprintf(msg, sizeof msg, ok2xx(st) && limpo ? "J0 %s: limpeza OK (HTTP %d): documento de prova trocado por {}"
                                             : "J0 %s: limpeza FALHOU (HTTP %d)", J0_NS, st);
  publicar(msg);
}

static void *rodar(void *u) {
  Pedido p = *(Pedido *)u;
  char dono[160];
  int perfil = perfis_ativo();
  free(u);
  snprintf(dono, sizeof dono, "%s", sessao_usuario() ? sessao_usuario() : "");
  if (!sessao_logada() || !dono[0]) publicar("J0: entre numa conta de teste antes da sonda");
  else if (p.fase == J0_LER) faseLer(perfil, dono);
  else if (p.fase == J0_GRAVAR) faseGravar(perfil, dono, p.token);
  else if (p.fase == J0_LIMPAR) faseLimpar(perfil, dono, p.token);
  pthread_mutex_lock(&trava); rodando = 0; pthread_mutex_unlock(&trava);
  return NULL;
}

void j0sonda_pedir(int fase, const char *token) {
  Pedido *p;
  pthread_t t;
  pthread_mutex_lock(&trava);
  if (rodando) { pthread_mutex_unlock(&trava); return; }
  rodando = 1;
  pthread_mutex_unlock(&trava);
  p = calloc(1, sizeof *p);
  if (p) { p->fase = fase; snprintf(p->token, sizeof p->token, "%s", token ? token : ""); }
  if (!p || pthread_create(&t, NULL, rodar, p) != 0) {
    free(p);
    pthread_mutex_lock(&trava); rodando = 0; pthread_mutex_unlock(&trava);
    return;
  }
  pthread_detach(t);
}

void j0sonda_passo(void) {
  char texto[600];
  pthread_mutex_lock(&trava);
  int mostrar = novo && !rodando;
  if (mostrar) { snprintf(texto, sizeof texto, "%s", resultado); novo = 0; }
  pthread_mutex_unlock(&trava);
  if (mostrar)
    ilha_avisar("j0-sonda", strstr(texto, "PASSOU") || strstr(texto, "aceita") || strstr(texto, "limpeza OK")
                ? ILHA_OK : ILHA_ERRO, NULL, texto, 60000u, 0);
}

const char *j0sonda_resultado(void) { return resultado; }
#endif

#define NV_J7_IMPL 1
#include "j7arme.h"
#include "validation_readonly.h"

#ifndef NV_J7_ARME
// Non-arming builds: everything is inline in j7arme.h.
typedef int j7arme_vazio;
#else
#include "ajustes.h"
#include "dados.h"
#include "ilha.h"
#include "perfis.h"
#include "sessao.h"
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define J7_ARQ "j7-arme.txt"
#define J7_CODIGO_PRAZO_S 600
#define J7_PERFIS 32
#define NOME(e) ((e) == J7_J6 ? "J6 (tv)" : "J4/J5 (better_nuvio)")
#define CURTO(e) ((e) == J7_J6 ? "J6" : "J4/J5")

static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static int carregado;
static unsigned long long dono;                      // enrolled owner hash, 0 = none
static struct { int perfil, escopo; long long ate; } armado[J7_PERFIS];   // escopo: J7_J4 or J7_J6
static int nArmado;
static char pendToken[9];
static unsigned long long pendDono;
static int pendPerfil, pendEscopo;
static long long pendQuando, desvio;
static char resultado[600];
static int novo, fotoRodando;

static long long agora(void) { return (long long)time(NULL) + desvio; }
static unsigned long long hashDono(const char *s) {
  unsigned long long h = 1469598103934665603ULL;
  for (; s && *s; s++) h = (h ^ (unsigned char)*s) * 1099511628211ULL;
  return h ? h : 1;
}

// Requires `trava`. A malformed file is treated as no enrolment (fail closed).
static void carregar(void) {
  char *t;
  if (carregado) return;
  carregado = 1;
  dono = 0; nArmado = 0; pendToken[0] = 0;
  t = dados_ler(J7_ARQ);
  if (!t) return;
  char tok[16] = "";
  int campos = sscanf(t, "v1 %llx %15s %llx %d %lld %d", &dono, tok, &pendDono, &pendPerfil, &pendQuando, &pendEscopo);
  if (campos != 5 && campos != 6) { dono = 0; free(t); return; }
  if (campos == 5 || (pendEscopo != J7_J4 && pendEscopo != J7_J6)) pendEscopo = J7_J4;   // J7a files: J4/J5 only
  if (strcmp(tok, "-")) snprintf(pendToken, sizeof pendToken, "%s", tok);
  for (const char *l = strchr(t, '\n'); l && *++l && nArmado < J7_PERFIS; l = strchr(l, '\n')) {
    int p, e = J7_J4, c; long long ate;
    // A J7a line without a scope arms J4/J5 only; an unknown scope is ignored (fails closed).
    if ((c = sscanf(l, "armed %d %lld %d", &p, &ate, &e)) >= 2 && p > 0 && (c == 2 || e == J7_J4 || e == J7_J6)) {
      armado[nArmado].perfil = p; armado[nArmado].escopo = c == 2 ? J7_J4 : e; armado[nArmado++].ate = ate;
    }
  }
  free(t);
}
static void salvar(void) {
  char t[1200];
  size_t n = (size_t)snprintf(t, sizeof t, "v1 %llx %s %llx %d %lld %d\n", dono, pendToken[0] ? pendToken : "-",
                              pendDono, pendPerfil, pendQuando, pendEscopo ? pendEscopo : J7_J4);
  for (int i = 0; i < nArmado && n < sizeof t; i++)
    n += (size_t)snprintf(t + n, sizeof t - n, "armed %d %lld %d\n", armado[i].perfil, armado[i].ate, armado[i].escopo);
  dados_gravar(J7_ARQ, t);
}
// An expiry beyond now + window (clock moved back or edited file) is invalid.
__attribute__((unused)) static int valido(int perfil, int escopo) {
  long long t = agora();
  for (int i = 0; i < nArmado; i++)
    if (armado[i].perfil == perfil && armado[i].escopo == escopo && armado[i].ate > t && armado[i].ate <= t + J7_JANELA_S + 60) return 1;
  return 0;
}
static int limparVencidos(void) {
  int w = 0, mudou = 0;
  long long t = agora();
  for (int i = 0; i < nArmado; i++) {
    if (armado[i].ate > t && armado[i].ate <= t + J7_JANELA_S + 60) armado[w++] = armado[i];
    else { printf("[j7] perfil %d: janela de uploads de teste %s expirou\n", armado[i].perfil, NOME(armado[i].escopo)); mudou = 1; }
  }
  nArmado = w;
  return mudou;
}

// Settings sync is automatic (owner decision, 12 Nov 2026): any signed-in
// account/profile may upload its J4/J5 and J6 edits. Enrolment and arming are
// kept only as debug diagnostics; the read-only validation build still refuses.
static int permite(const char *owner, int profile, int escopo) {
  (void)escopo;
  if (NV_VALIDATION_READONLY_ENABLED) return 0;
  return owner && *owner && profile > 0;
}
int j7arme_permite_j4(const char *owner, int profile) { return permite(owner, profile, J7_J4); }
int j7arme_permite_j6(const char *owner, int profile) { return permite(owner, profile, J7_J6); }

// The screen gets `texto`; the log gets `registro` (the activation code is
// shown on screen only, never written to logcat).
static void publicarComo(const char *texto, const char *registro) {
  pthread_mutex_lock(&trava);
  snprintf(resultado, sizeof resultado, "%s", texto);
  novo = 1;
  pthread_mutex_unlock(&trava);
  printf("[j7] %s\n", registro);
  fflush(stdout);
}
static void publicar(const char *texto) { publicarComo(texto, texto); }

static void novoToken(char out[9]) {
  unsigned char r[4] = {0};
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0 || read(fd, r, sizeof r) != (ssize_t)sizeof r) {
    unsigned long long x = hashDono(sessao_usuario()) ^ (unsigned long long)agora() ^ (unsigned long long)getpid();
    memcpy(r, &x, sizeof r);
  }
  if (fd >= 0) close(fd);
  snprintf(out, 9, "%02X%02X%02X%02X", r[0], r[1], r[2], r[3]);
}

static int reavaliar = 1;

static void codigo(int escopo) {
  char msg[300];
  const char *o = sessao_logada() ? sessao_usuario() : "";
  int p = perfis_ativo();
  if (!o || !*o) { publicar("J7: entre na conta de teste antes de ativar uploads"); return; }
  unsigned long long h = hashDono(o);
  pthread_mutex_lock(&trava);
  carregar();
  if (dono && dono != h) {
    pthread_mutex_unlock(&trava);
    snprintf(msg, sizeof msg, "J7: outra conta (%08llx) ja esta inscrita; use disarm antes", dono >> 32);
    publicar(msg);
    return;
  }
  novoToken(pendToken);
  pendDono = h; pendPerfil = p; pendQuando = agora(); pendEscopo = escopo;
  salvar();
  // The island shows one line and cuts the rest: the code goes first (STAGE-J.md 17.9).
  snprintf(msg, sizeof msg, "J7 %s codigo %s - conta %08llx perfil %d - so na CONTA DE TESTE (10 min)",
           CURTO(escopo), pendToken, h >> 32, p);
  char reg[300];
  snprintf(reg, sizeof reg, "J7 %s: codigo de ativacao mostrado na tela para a conta %08llx perfil %d (10 min)", NOME(escopo), h >> 32, p);
  pthread_mutex_unlock(&trava);
  publicarComo(msg, reg);
}

static void armar(const char *token, int escopo) {
  char msg[300];
  const char *o = sessao_logada() ? sessao_usuario() : "";
  int p = perfis_ativo();
  unsigned long long h = o && *o ? hashDono(o) : 0;
  pthread_mutex_lock(&trava);
  carregar();
  int ok = h && pendToken[0] && token && !strcmp(token, pendToken) && pendDono == h && pendPerfil == p && pendEscopo == escopo &&
           agora() - pendQuando >= 0 && agora() - pendQuando <= J7_CODIGO_PRAZO_S && (!dono || dono == h);
  if (!ok) {
    pthread_mutex_unlock(&trava);
    publicar("J7: codigo invalido, vencido ou de outra conta/perfil - uploads continuam desligados");
    return;
  }
  dono = h;
  int i;
  for (i = 0; i < nArmado && (armado[i].perfil != p || armado[i].escopo != escopo); i++);
  if (i == nArmado && nArmado < J7_PERFIS) nArmado++;
  if (i < J7_PERFIS) { armado[i].perfil = p; armado[i].escopo = escopo; armado[i].ate = agora() + J7_JANELA_S; }
  pendToken[0] = 0; pendDono = 0; pendPerfil = 0; pendQuando = 0; pendEscopo = 0;
  salvar();
  pthread_mutex_unlock(&trava);
  reavaliar = 1;
  snprintf(msg, sizeof msg, "J7: uploads de teste %s LIGADOS para a conta %08llx perfil %d por 4 h", NOME(escopo), h >> 32, p);
  publicar(msg);
}

static void desarmar(const char *motivo) {
  pthread_mutex_lock(&trava);
  carregar();
  dono = 0; nArmado = 0; pendToken[0] = 0; pendDono = 0; pendPerfil = 0; pendQuando = 0; pendEscopo = 0;
  salvar();
  pthread_mutex_unlock(&trava);
  reavaliar = 1;
  publicar(motivo);
}

// ---------------------------------------------------------------- snapshot (A3)
static unsigned long long impressao(const char *s) {
  unsigned long long h = 1469598103934665603ULL;
  for (; s && *s; s++) h = (h ^ (unsigned char)*s) * 1099511628211ULL;
  return h;
}
static int contemSemCaixa(const char *s, const char *agulha) {
  size_t n = strlen(agulha);
  int c = 0;
  for (; s && *s; s++) {
    size_t k = 0;
    while (k < n && s[k] && tolower((unsigned char)s[k]) == agulha[k]) k++;
    if (k == n) c++;
  }
  return c;
}
// Field names of better_nuvio's "fields" object outside the J4/J5 allowlist.
static int camposEstranhos(const char *r) {
  const char *f = r ? strstr(r, "\"fields\"") : NULL;
  int n = 0, prof = 0;
  if (!f || !(f = strchr(f, '{'))) return 0;
  for (const char *p = f; *p; p++) {
    if (*p == '"' ) {
      const char *q = p + 1;
      while (*q && *q != '"') { if (*q == '\\' && q[1]) q++; q++; }
      if (!*q) break;
      if (prof == 1) {   // a member name of "fields"
        const char *c = q + 1;
        while (*c == ' ') c++;
        if (*c == ':') {
          char chave[96]; size_t len = (size_t)(q - p - 1);
          if (len < sizeof chave) {
            memcpy(chave, p + 1, len); chave[len] = 0;
            if (!ajustes_better_chave_sincronizavel(chave)) n++;
          } else n++;
        }
      }
      p = q;
    } else if (*p == '{') prof++;
    else if (*p == '}') { if (--prof == 0) break; }
  }
  return n;
}
static void *foto(void *u) {
  static const char *const NS[4] = { "tv", "better_nuvio", "home_catalog_shared", "mobile" };
  char corpo[160], msg[600];
  int perfil = perfis_ativo(), falhou = 0, estranhos = 0, sensiveis = 0, j4NoTv = 0;
  (void)u;
  for (int i = 0; i < 4; i++) {
    int st = 0; char *r;
    snprintf(corpo, sizeof corpo, "{\"p_profile_id\":%d,\"p_platform\":\"%s\"}", perfil, NS[i]);
    r = sessao_rpc("sync_pull_profile_settings_blob", corpo, &st);
    printf("[j7] foto %s perfil %d: HTTP %d, %u bytes, impressao %016llx\n", NS[i], perfil, st,
           r ? (unsigned)strlen(r) : 0u, impressao(r));
    if (st < 200 || st >= 300) falhou++;
    if (i == 0 && r)
      for (size_t k = 0; k < ajustes_better_n(); k++) {
        char q[100]; snprintf(q, sizeof q, "\"%s\"", ajustes_better_chave(k));
        if (strstr(r, q)) j4NoTv++;
      }
    if (i == 1 && r) {
      static const char *const PAD[] = { "token", "password", "senha", "api_key", "apikey", "secret", "bearer", "http://", "https://" };
      estranhos = camposEstranhos(r);
      for (size_t k = 0; k < sizeof PAD / sizeof *PAD; k++) sensiveis += contemSemCaixa(r, PAD[k]);
    }
    free(r);
  }
  snprintf(msg, sizeof msg, "J7 foto perfil %d: %d leitura(s) falharam; better_nuvio campos fora da lista %d, "
           "padroes sensiveis %d; chaves Better no tv %d%s", perfil, falhou, estranhos, sensiveis, j4NoTv,
           estranhos || sensiveis || j4NoTv ? " - VAZAMENTO" : " - limpo");
  publicar(msg);
  pthread_mutex_lock(&trava); fotoRodando = 0; pthread_mutex_unlock(&trava);
  return NULL;
}

void j7arme_pedir(int fase, const char *token) {
  if (NV_VALIDATION_READONLY_ENABLED && fase != J7_DESARMAR && fase != J7_FOTO) {
    publicar("J7: read-only validation build; account uploads and arming disabled");
    return;
  }
  if (fase == J7_CODIGO) codigo(J7_J4);
  else if (fase == J7_ARMAR) armar(token, J7_J4);
  else if (fase == J7_CODIGO_J6) codigo(J7_J6);
  else if (fase == J7_ARMAR_J6) armar(token, J7_J6);
  else if (fase == J7_DESARMAR) desarmar("J7: uploads de teste DESLIGADOS e inscricao apagada");
  else if (fase == J7_FOTO) {
    pthread_t t;
    pthread_mutex_lock(&trava);
    int livre = !fotoRodando; if (livre) fotoRodando = 1;
    pthread_mutex_unlock(&trava);
    if (!sessao_logada()) { pthread_mutex_lock(&trava); fotoRodando = 0; pthread_mutex_unlock(&trava); publicar("J7: entre numa conta antes da foto"); return; }
    if (livre && pthread_create(&t, NULL, foto, NULL) == 0) pthread_detach(t);
    else if (livre) { pthread_mutex_lock(&trava); fotoRodando = 0; pthread_mutex_unlock(&trava); }
  }
}

void j7arme_passo(void) {
  static unsigned long long ultDono;
  static int ultPerfil = -1, ultAplicado = -1;   // bit 0: J4/J5, bit 1: J6
  static long long ultCheque;
  char texto[600];
  int mostrar;
  pthread_mutex_lock(&trava);
  mostrar = novo; if (mostrar) { snprintf(texto, sizeof texto, "%s", resultado); novo = 0; }
  pthread_mutex_unlock(&trava);
  if (mostrar)
    ilha_avisar("j7-arme", strstr(texto, "VAZAMENTO") || strstr(texto, "invalido") ? ILHA_ERRO : ILHA_OK,
                NULL, texto, 60000u, 0);

  long long t = agora();
  const char *o = sessao_logada() ? sessao_usuario() : "";
  unsigned long long h = o && *o ? hashDono(o) : 0;
  int p = perfis_ativo();
  if (!reavaliar && t == ultCheque && h == ultDono && p == ultPerfil) return;
  ultCheque = t;
  pthread_mutex_lock(&trava);
  carregar();
  int saiu = dono && h != dono, mudou = limparVencidos();
  if (saiu) {   // another account or signed out: nothing stays armed
    dono = 0; nArmado = 0; pendToken[0] = 0; pendDono = 0; mudou = 1;
    printf("[j7] a conta de teste inscrita saiu: uploads de teste desligados e inscricao apagada\n");
  }
  if (mudou) salvar();
  // Automatic sync: uploads follow the signed-in account, not the test arming.
  int q4 = !NV_VALIDATION_READONLY_ENABLED && h && p > 0;
  int q6 = q4, quer = q4 | (q6 << 1);
  pthread_mutex_unlock(&trava);
  if (quer != ultAplicado || h != ultDono || p != ultPerfil || reavaliar) {
    // Independent: arming J4/J5 never grants J6 (official "tv") writes.
    int a = h ? ajustes_better_writes(q4) : 1, b = h ? ajustes_oficiais_permitir_escritas(q6) : 1;
    if (a && b) {
      if (quer != ultAplicado) printf("[j7] uploads J4/J5 %s, J6 %s (perfil %d)\n", q4 ? "LIGADOS" : "desligados", q6 ? "LIGADOS" : "desligados", p);
      ultAplicado = quer; ultDono = h; ultPerfil = p; reavaliar = 0;
    } else ultAplicado = -1;   // context not ready: retry next second
  }
}

const char *j7arme_resultado(void) { return resultado; }

#ifdef NV_J7_TESTE
void j7arme_teste_avancar(long long segundos) { pthread_mutex_lock(&trava); desvio += segundos; pthread_mutex_unlock(&trava); }
void j7arme_teste_foto(void) { pthread_mutex_lock(&trava); fotoRodando = 1; pthread_mutex_unlock(&trava); foto(NULL); }
#endif
#endif

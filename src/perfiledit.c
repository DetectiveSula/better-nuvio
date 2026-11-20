// Ver perfiledit.h.
#include "perfiledit.h"
#include "js.h"
#include "jsw.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *const PE_CORES_HEX[PE_CORES] = {
  "#E53935", "#1E88E5", "#8E24AA", "#43A047", "#FFB300", "#D81B60", "#00ACC1", "#6D4C41" };

// --- pure rules ------------------------------------------------------------
// Byte length of the UTF-8 code point at s (1 for a stray byte).
static int cpLen(const unsigned char *s) {
  if (*s < 0x80) return 1;
  if ((*s & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) return 2;
  if ((*s & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) return 3;
  if ((*s & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) return 4;
  return 1;
}
static int espaco(unsigned char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }
static int controle(unsigned char c) { return c < 0x20 || c == 0x7F; }

// s without surrounding whitespace, as (start, length).
static void aparado(const char *s, const char **ini, size_t *n) {
  size_t l;
  while (*s && espaco((unsigned char)*s)) s++;
  l = strlen(s);
  while (l && espaco((unsigned char)s[l - 1])) l--;
  *ini = s; *n = l;
}

void perfiledit_nome_aparar(const char *in, char *out, size_t n) {
  const char *s;
  size_t len, i = 0, w = 0, fimTexto = 0;
  int chars = 0;
  if (!out || !n) return;
  aparado(in ? in : "", &s, &len);
  while (i < len) {
    const unsigned char *c = (const unsigned char *)s + i;
    int l = cpLen(c);
    if (i + (size_t)l > len) break;
    if (controle(*c)) { i++; continue; }   // control characters never enter
    if (chars + 1 > PE_NOME_MAX_CHARS || w + (size_t)l > PE_NOME_MAX_BYTES || w + (size_t)l + 1 > n) break;
    memcpy(out + w, c, (size_t)l); w += (size_t)l; chars++;
    if (!espaco(*c)) fimTexto = w;
    i += (size_t)l;
  }
  out[fimTexto] = 0;   // clamping may leave a trailing space: drop it
}
int perfiledit_nome_valido(const char *s) {
  const char *ini;
  size_t len, i;
  int chars = 0;
  if (!s) return 0;
  aparado(s, &ini, &len);
  if (!len || len > PE_NOME_MAX_BYTES) return 0;
  for (i = 0; i < len; ) {
    const unsigned char *c = (const unsigned char *)ini + i;
    if (controle(*c)) return 0;
    i += (size_t)cpLen(c); chars++;
  }
  return chars <= PE_NOME_MAX_CHARS;
}
int perfiledit_url_valida(const char *s) {
  if (!s) return 0;
  if (!*s) return 1;
  if (strncmp(s, "https://", 8) && strncmp(s, "http://", 7)) return 0;
  if (strlen(s) >= PE_URL_MAX) return 0;
  for (const unsigned char *p = (const unsigned char *)s; *p; p++)
    if (*p <= 0x20 || *p == 0x7F) return 0;
  return 1;
}
static int hex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
int perfiledit_cor_normal(const char *in, char out[8]) {
  const char *v;
  size_t n;
  char t[6];
  if (!in) return 0;
  aparado(in, &v, &n);
  if (n && *v == '#') { v++; n--; }
  if (n == 8) { v += 2; n = 6; }   // AARRGGBB: the alpha is dropped
  if (n != 6) return 0;
  for (int i = 0; i < 6; i++) {
    if (!hex(v[i])) return 0;
    t[i] = (char)(v[i] >= 'a' && v[i] <= 'f' ? v[i] - 32 : v[i]);
  }
  out[0] = '#'; memcpy(out + 1, t, 6); out[7] = 0;
  return 1;
}
int perfiledit_pin_valido(const char *s) {
  if (!s || strlen(s) != 4) return 0;
  for (int i = 0; i < 4; i++) if (s[i] < '0' || s[i] > '9') return 0;
  return 1;
}
static int caminhoSeguro(const char *p) {
  return p && *p && strlen(p) < 200 && !strstr(p, "..") && !strchr(p, ':') && !strchr(p, '?') && !strchr(p, '#');
}
static int idSeguro(const char *id) { return id && id[0] && strlen(id) < PE_ID_MAX; }

int perfiledit_ler_avatares(const char *json, const char *base, PEAvatar *out, int max) {
  int n = 0, lidos = 0;
  const char *e = json ? js_raiz_array(json) : NULL;
  char b[PE_URL_MAX];
  size_t lb;
  if (!base || !out) return 0;
  snprintf(b, sizeof b, "%s", base);
  lb = strlen(b);
  while (lb && b[lb - 1] == '/') b[--lb] = 0;
  for (; e && n < max && lidos < 64; e = js_prox(js_fim(e))) {
    const char *f = js_fim(e), *p;
    char id[PE_ID_MAX * 2] = "", path[PE_URL_MAX] = "", nome[96] = "", cor[32] = "";
    PEAvatar *a = &out[n];
    lidos++;
    if (*e != '{') continue;
    js_texto_raiz_em(e, f, "id", id, sizeof id);
    js_texto_raiz_em(e, f, "storage_path", path, sizeof path);
    js_texto_raiz_em(e, f, "display_name", nome, sizeof nome);
    js_texto_raiz_em(e, f, "bg_color", cor, sizeof cor);
    p = path; while (*p == '/') p++;
    if (!idSeguro(id) || !caminhoSeguro(p)) continue;
    memset(a, 0, sizeof *a);
    snprintf(a->id, sizeof a->id, "%s", id);
    { int w = snprintf(a->url, sizeof a->url, "%s/storage/v1/object/public/avatars/%s", b, p);
      if (w < 0 || (size_t)w >= sizeof a->url || !perfiledit_url_valida(a->url)) continue; }
    if (nome[0]) snprintf(a->nome, sizeof a->nome, "%s", nome);
    else {
      snprintf(a->nome, sizeof a->nome, "%s", !strncmp(id, "avatar_", 7) ? id + 7 : id);
      for (char *c = a->nome; *c; c++) if (*c == '_') *c = ' ';
    }
    if (!perfiledit_cor_normal(cor, a->cor)) a->cor[0] = 0;
    n++;
  }
  return n;
}
int perfiledit_ler_fundos(const char *json, PEFundo *out, int max) {
  int n = 0, lidos = 0;
  const char *e = json ? js_raiz_array(json) : NULL;
  if (!out) return 0;
  for (; e && n < max && lidos < 64; e = js_prox(js_fim(e))) {
    const char *f = js_fim(e), *p;
    char id[PE_ID_MAX * 2] = "", path[PE_URL_MAX] = "", nome[96] = "";
    lidos++;
    if (*e != '{') continue;
    js_texto_raiz_em(e, f, "id", id, sizeof id);
    js_texto_raiz_em(e, f, "storage_path", path, sizeof path);
    js_texto_raiz_em(e, f, "display_name", nome, sizeof nome);
    p = path; while (*p == '/') p++;
    if (!idSeguro(id) || !caminhoSeguro(p)) continue;
    memset(&out[n], 0, sizeof out[n]);
    snprintf(out[n].id, sizeof out[n].id, "%s", id);
    snprintf(out[n].nome, sizeof out[n].nome, "%s", nome[0] ? nome : id);
    n++;
  }
  return n;
}

static char *fechar(Jsw *w) {
  char *r = !w->erro && jsw_texto_final(w) ? strdup(jsw_texto_final(w)) : NULL;
  jsw_livre(w);
  return r;
}
// The photo fields both bodies send: catalogue avatar (id), no photo, or an
// image link (avatar_url with no id, as official Nuvio saves a custom photo).
static int fotoValida(int foto, const char *avatarId, const char *fotoUrl) {
  if (foto < 0 || foto > 3) return 0;
  if (foto == 1 && !idSeguro(avatarId)) return 0;
  if (foto == 3 && (!fotoUrl || !fotoUrl[0] || !perfiledit_url_valida(fotoUrl))) return 0;
  return 1;
}
static void fotoCampos(Jsw *w, const char *kId, const char *kUrl, int foto, const char *avatarId, const char *fotoUrl) {
  jsw_chave(w, kId); if (foto == 1) jsw_str(w, avatarId); else jsw_nulo(w);
  jsw_chave(w, kUrl); if (foto == 3) jsw_str(w, fotoUrl); else jsw_nulo(w);
}
char *perfiledit_corpo_patch(int perfil, const char *nome, int foto, const char *avatarId, const char *fotoUrl) {
  Jsw w;
  char t[PE_NOME_MAX_BYTES + 8];
  if (perfil < 1 || perfil > 8 || !perfiledit_nome_valido(nome)) return NULL;
  if (!fotoValida(foto, avatarId, fotoUrl)) return NULL;
  perfiledit_nome_aparar(nome, t, sizeof t);
  jsw_iniciar(&w); jsw_obj_ini(&w);
  jsw_ci(&w, "p_profile_id", perfil);
  jsw_cs(&w, "p_name", t);
  if (foto) {
    jsw_cb(&w, "p_avatar_id_provided", 1); jsw_cb(&w, "p_avatar_url_provided", 1);
    fotoCampos(&w, "p_avatar_id", "p_avatar_url", foto, avatarId, fotoUrl);
  }
  jsw_obj_fim(&w);
  return fechar(&w);
}

// A field copied exactly as the account returned it (JSON text), or null.
static void cru(Jsw *w, const char *e, const char *f, const char *chave) {
  char v[PE_URL_MAX * 2];
  jsw_chave(w, chave);
  if (js_bruto(e, f, chave, v, sizeof v)) jsw_bruto(w, v); else jsw_nulo(w);
}
static int verdade(const char *e, const char *f, const char *chave) {
  char b[16] = "";
  return js_bruto(e, f, chave, b, sizeof b) && !strcmp(b, "true");
}
char *perfiledit_corpo_push(const char *pull, int perfil, const char *nome, int foto,
                            const char *avatarId, const char *fotoUrl, const char *cor, int fundo,
                            const char *fundoId, const char *cliente) {
  Jsw w;
  char t[PE_NOME_MAX_BYTES + 8], c[8];
  const char *e = pull ? js_raiz_array(pull) : NULL;
  int vistos[9] = { 0 }, n = 0, achou = 0;
  if (perfil < 1 || perfil > 8 || !perfiledit_nome_valido(nome)) return NULL;
  if (cor && !perfiledit_cor_normal(cor, c)) return NULL;
  if (!fotoValida(foto, avatarId, fotoUrl)) return NULL;
  if (fundo < 0 || fundo > 2 || (fundo == 2 && !idSeguro(fundoId))) return NULL;
  perfiledit_nome_aparar(nome, t, sizeof t);
  jsw_iniciar(&w); jsw_obj_ini(&w);
  jsw_ci(&w, "p_client_max_profiles", 6);
  jsw_chave(&w, "p_profiles"); jsw_arr_ini(&w);
  for (; e; e = js_prox(js_fim(e))) {
    const char *f = js_fim(e);
    int idx;
    char v[96] = "";
    // An unexpected list (not objects, ids outside 1..8, duplicates, more than
    // 6) is refused whole: nothing is written.
    if (*e != '{') { jsw_livre(&w); return NULL; }
    idx = (int)js_num(e, f, "profile_index", 0);
    if (idx < 1 || idx > 8 || vistos[idx] || ++n > 6) { jsw_livre(&w); return NULL; }
    vistos[idx] = 1;
    jsw_obj_ini(&w);
    jsw_ci(&w, "profile_index", idx);
    if (idx == perfil) jsw_cs(&w, "name", t);
    else { js_texto_raiz_em(e, f, "name", v, sizeof v); jsw_cs(&w, "name", v); }
    v[0] = 0; js_texto_raiz_em(e, f, "avatar_color_hex", v, sizeof v);
    if (idx == perfil && cor) jsw_cs(&w, "avatar_color_hex", c);
    else jsw_cs(&w, "avatar_color_hex", v[0] ? v : "#1E88E5");
    jsw_cb(&w, "uses_primary_addons", verdade(e, f, "uses_primary_addons"));
    jsw_cb(&w, "uses_primary_plugins", verdade(e, f, "uses_primary_plugins"));
    if (idx == perfil && foto) fotoCampos(&w, "avatar_id", "avatar_url", foto, avatarId, fotoUrl);
    else { cru(&w, e, f, "avatar_id"); cru(&w, e, f, "avatar_url"); }
    if (idx == perfil && fundo) {
      jsw_chave(&w, "profile_background_id"); if (fundo == 2) jsw_str(&w, fundoId); else jsw_nulo(&w);
      jsw_chave(&w, "profile_background_url"); jsw_nulo(&w);
    } else { cru(&w, e, f, "profile_background_id"); cru(&w, e, f, "profile_background_url"); }
    jsw_obj_fim(&w);
    if (idx == perfil) achou = 1;
  }
  jsw_arr_fim(&w);
  if (cliente && cliente[0]) jsw_cs(&w, "p_origin_client_id", cliente);
  jsw_obj_fim(&w);
  if (!achou) { jsw_livre(&w); return NULL; }
  return fechar(&w);
}

#ifndef NV_PERFILEDIT_SO_REGRAS
// --- the screen --------------------------------------------------------------
#include "ajustes.h"
#include "anim.h"
#include "dados.h"
#include "gfx.h"
#include "layout.h"
#include "nuvem.h"
#include "perfis.h"
#include "sessao.h"
#include "tex_cache.h"
#include "text.h"
#include "teclado.h"
#include "validation_readonly.h"

// The views, as in official Nuvio TV: the options first, then the editor, and
// the confirmation for removing the PIN. Drawn with Settings' own modal pieces
// (ajustes_ui_*) so the editor reads as part of the app.
enum { V_OPCOES, V_EDITOR, V_CONFIRMA };
enum { OP_EDITAR, OP_PIN, OP_REMOVER, OP_FECHAR };
// Editor focus zones.
enum { Z_NOME, Z_CANCELAR, Z_ABAS, Z_GRADE, Z_EXTRA, Z_SALVAR };
enum { ABA_AVATAR, ABA_COR, ABA_FUNDO, ABA_N };
enum { JOB_NADA = 0, JOB_CATALOGOS, JOB_SALVAR, JOB_PIN };
enum { TEC_NADA = 0, TEC_NOME, TEC_PIN_NOVO, TEC_PIN_CONFIRMA, TEC_LINK };

static int aberto, mudouFlag, vista, opFoco, zona, aba, item, confFoco;
static int perfil, temPin;
static uint64_t geracao;
static char conta[128];
static char nomeOrig[64], nome[64], fotoOrig[PE_URL_MAX], corOrig[8], fundoOrig[PE_URL_MAX];
// Choices. foto: -1 as it was, 0 no photo (the initial), i = avatares[i-1],
// -2 an image link (fotoLink), saved as avatar_url like official Nuvio.
// fundo: -1 as it was, 0 standard, i = fundos[i-1]. cor: "" = as it was.
static int fotoSel, fundoSel;
static char corEscolhida[8], fotoLink[PE_URL_MAX];
static int extraFoco;   // under the avatars: 0 No photo, 1 Avatar link
static PEAvatar avatares[64];
static PEFundo fundos[64];
static int nAvatares, nFundos, catalogo;   // catalogo: 0 loading, 1 ready, 2 failed
static char msg[200];
static int msgErro;
static int tecladoPara;
static char pinAtual[8], pinPrimeiro[8];
// The profile's PIN, already checked by the picker before it opened a locked
// profile (it is the "current PIN" of a change), and a new PIN on its way to
// the account. All wiped on close.
static char pinConhecido[8], pinPendente[8];
static float entrada;

// --- worker: one job at a time; the token is copied in and wiped after use ---
typedef struct {
  int tipo, perfil, foto, fundo, pinNovo;   // pinNovo: 1 set, 0 clear
  unsigned serie;
  char nome[64], avatarId[PE_ID_MAX], fotoUrl[PE_URL_MAX], cor[8], fundoId[PE_ID_MAX], cliente[96];
  char pin[8], pinAtual[8], base[PE_URL_MAX];
  char jwt[4096];
} Job;
typedef struct {
  int tipo, ok, status;
  unsigned serie;
  char nome[64], foto[PE_URL_MAX], cor[8], fundo[PE_URL_MAX];
  int fotoVeio, temCor, temFundo, pin;
  PEAvatar av[64]; int nAv;
  PEFundo fu[64]; int nFu;
} Resultado;
static SDL_mutex *mtx;
static int ocupado, pronto;
static Resultado res;
static unsigned serie;   // reopening/closing the editor orphans a running job

static char *rpc(const Job *j, const char *funcao, const char *corpo, int *status) {
  int leitura = !strcmp(funcao, "get_avatar_catalog") || !strcmp(funcao, "sync_pull_profiles") ||
                !strcmp(funcao, "get_member_profile_background_catalog");
  if (NV_VALIDATION_READONLY_ENABLED && !leitura) { *status = 503; return NULL; }
  *status = 0;
  return nuvem_rpc_com(funcao, corpo, j->jwt, status);
}
static int ok2xx(const char *r, int st) { return r && st >= 200 && st < 300; }

// The edited profile's saved row, read back after a push.
static void linhaPerfil(const char *pull, int indice, Resultado *r) {
  for (const char *e = pull ? js_raiz_array(pull) : NULL; e; e = js_prox(js_fim(e))) {
    const char *f = js_fim(e);
    char v[32] = "";
    if (*e != '{' || (int)js_num(e, f, "profile_index", 0) != indice) continue;
    r->ok = 1;
    js_texto_raiz_em(e, f, "name", r->nome, sizeof r->nome);
    r->fotoVeio = 1; r->foto[0] = 0;
    js_texto_raiz_em(e, f, "avatar_url", r->foto, sizeof r->foto);   // null stays ""
    js_texto_raiz_em(e, f, "avatar_color_hex", v, sizeof v);
    r->temCor = perfiledit_cor_normal(v, r->cor);
    r->temFundo = 1; r->fundo[0] = 0;
    js_texto_raiz_em(e, f, "profile_background_url", r->fundo, sizeof r->fundo);
    return;
  }
}
static void trabalhoSalvar(Job *j, Resultado *r) {
  int st = 0;
  if (j->cor[0] || j->fundo) {
    char *pull = rpc(j, "sync_pull_profiles", "{}", &st), *corpo = NULL, *push = NULL, *de = NULL;
    if (!ok2xx(pull, st)) r->status = st;
    else if (!(corpo = perfiledit_corpo_push(pull, j->perfil, j->nome, j->foto, j->avatarId, j->fotoUrl,
                                              j->cor[0] ? j->cor : NULL, j->fundo, j->fundoId, j->cliente)))
      r->status = -1;
    else if (push = rpc(j, "sync_push_profiles", corpo, &st), !ok2xx(push, st)) r->status = st;
    else {
      de = rpc(j, "sync_pull_profiles", "{}", &st);
      if (ok2xx(de, st)) linhaPerfil(de, j->perfil, r);
      if (!r->ok) r->status = -2;
    }
    free(pull); free(corpo); free(push); free(de);
    return;
  }
  { char *corpo = perfiledit_corpo_patch(j->perfil, j->nome, j->foto, j->avatarId, j->fotoUrl), *resp;
    const char *row, *f;
    if (!corpo) { r->status = -1; return; }
    resp = rpc(j, "sync_patch_profile", corpo, &st);
    free(corpo);
    if (!ok2xx(resp, st)) { r->status = st; free(resp); return; }
    row = resp;
    while (*row == ' ' || *row == '\n' || *row == '\r' || *row == '\t') row++;
    if (*row == '[') row = js_raiz_array(resp);
    if (row && *row == '{' && (int)js_num(row, js_fim(row), "profile_index", 0) == j->perfil) {
      char b[16] = "";
      f = js_fim(row);
      r->ok = 1;
      snprintf(r->nome, sizeof r->nome, "%s", j->nome);
      js_texto_raiz_em(row, f, "name", r->nome, sizeof r->nome);
      // A null avatar_url leaves the photo to the app thread (the choice made).
      if (js_bruto(row, f, "avatar_url", b, sizeof b) && strcmp(b, "null"))
        r->fotoVeio = js_texto_raiz_em(row, f, "avatar_url", r->foto, sizeof r->foto);
    } else r->status = -2;
    free(resp); }
}
static int fioTrabalho(void *arg) {
  Job *j = arg;
  Resultado *r = calloc(1, sizeof *r);
  int st = 0;
  if (r) {
    r->tipo = j->tipo; r->serie = j->serie;
    if (j->tipo == JOB_CATALOGOS) {
      char *a = rpc(j, "get_avatar_catalog", "{}", &st);
      if (ok2xx(a, st)) { r->nAv = perfiledit_ler_avatares(a, j->base, r->av, 64); r->ok = 1; }
      free(a);
      // A non-supporter's call fails: that simply means no supporter backgrounds.
      a = rpc(j, "get_member_profile_background_catalog", "{}", &st);
      if (ok2xx(a, st)) r->nFu = perfiledit_ler_fundos(a, r->fu, 64);
      free(a);
    } else if (j->tipo == JOB_SALVAR) trabalhoSalvar(j, r);
    else if (j->tipo == JOB_PIN) {
      Jsw w;
      char *resp = NULL;
      jsw_iniciar(&w); jsw_obj_ini(&w);
      jsw_ci(&w, "p_profile_id", j->perfil);
      if (j->pinNovo) jsw_cs(&w, "p_pin", j->pin);
      if (j->pinAtual[0]) jsw_cs(&w, "p_current_pin", j->pinAtual);
      jsw_obj_fim(&w);
      if (!w.erro && jsw_texto_final(&w))
        resp = rpc(j, j->pinNovo ? "set_profile_pin" : "clear_profile_pin", jsw_texto_final(&w), &st);
      if (w.p) memset(w.p, 0, w.n);
      jsw_livre(&w);
      if (ok2xx(resp, st)) { r->ok = 1; r->pin = j->pinNovo; } else r->status = st;
      free(resp);
    }
  }
  memset(j->jwt, 0, sizeof j->jwt); memset(j->pin, 0, sizeof j->pin); memset(j->pinAtual, 0, sizeof j->pinAtual);
  SDL_LockMutex(mtx);
  if (r) { res = *r; pronto = 1; }
  ocupado = 0;
  SDL_UnlockMutex(mtx);
  free(r); free(j);
  return 0;
}
static int lancar(Job *j) {
  char usuario[128];
  SDL_Thread *t;
  if (!mtx) mtx = SDL_CreateMutex();
  if (!mtx || ocupado || !sessao_identidade_local_copiar(usuario, sizeof usuario, j->jwt, sizeof j->jwt) ||
      strcmp(usuario, conta)) {
    memset(j->jwt, 0, sizeof j->jwt); free(j); return 0;
  }
  snprintf(j->base, sizeof j->base, "%s", nuvem_url() ? nuvem_url() : "");
  snprintf(j->cliente, sizeof j->cliente, "%s", dados_cliente_id() ? dados_cliente_id() : "");
  j->perfil = perfil; j->serie = serie;
  SDL_LockMutex(mtx); ocupado = 1; pronto = 0; SDL_UnlockMutex(mtx);
  t = SDL_CreateThread(fioTrabalho, "nv-profile-edit", j);
  if (!t) {
    SDL_LockMutex(mtx); ocupado = 0; SDL_UnlockMutex(mtx);
    memset(j->jwt, 0, sizeof j->jwt); free(j); return 0;
  }
  SDL_DetachThread(t);
  return 1;
}

static void aviso(const char *s, int erro) { snprintf(msg, sizeof msg, "%s", s ? s : ""); msgErro = erro; }
static const char *mensagemFalha(int status, int pin) {
  if (status == 401) return "Sua sessão expirou. Entre de novo para salvar.";
  if (pin && status >= 400 && status <= 403) return "O PIN atual não foi aceito.";
  if (status == 404) return "Este servidor não permite esta mudança.";
  if (status == -2) return "A conta não confirmou a mudança. Tente de novo.";
  return "Não foi possível salvar. Tente de novo.";
}

#if defined(NV_SOCIALVIS_DEMO) || defined(NV_PERFILSEL_TEST)
static int demo;
static ContaPerfil demoPerfil = { .indice = 2, .nome = "Marina", .corHex = "#1E88E5", .temPin = 1 };
static int demoVista = -1, demoAba, demoZona, demoItem;
void perfiledit_demo(int liga) { demo = liga; }
void perfiledit_demo_estado(int v, int a, int z, int it) { demoVista = v; demoAba = a; demoZona = z; demoItem = it; }
#else
enum { demo = 0, demoVista = -1, demoAba = 0, demoZona = 0, demoItem = 0 };
#endif
int perfiledit_disponivel(void) { return demo || sessao_logada(); }

int perfiledit_abrir_perfil(int indice, const char *pinVerificado) {
  const ContaPerfil *p = NULL;
  char jwt[4096];
  Job *j;
  for (int i = 0; i < perfis_n(); i++)
    if (perfis_item(i) && perfis_item(i)->indice == indice) p = perfis_item(i);
#if defined(NV_SOCIALVIS_DEMO) || defined(NV_PERFILSEL_TEST)
  if (demo && !p) p = &demoPerfil;
#endif
  if (aberto || !p || (!demo && !sessao_logada())) return 0;
  // A locked profile opens only with its PIN already checked by the host.
  if (p->temPin && !perfiledit_pin_valido(pinVerificado) && !demo) return 0;
  if (!demo && !sessao_identidade_local_copiar(conta, sizeof conta, jwt, sizeof jwt)) return 0;
  memset(jwt, 0, sizeof jwt);
  perfil = p->indice; temPin = p->temPin;
  geracao = perfis_sessao_geracao();
  perfiledit_nome_aparar(p->nome, nomeOrig, sizeof nomeOrig);
  snprintf(nome, sizeof nome, "%s", nomeOrig);
  snprintf(fotoOrig, sizeof fotoOrig, "%s", p->avatarUrl);
  if (!perfiledit_cor_normal(p->corHex, corOrig)) snprintf(corOrig, sizeof corOrig, "#1E88E5");
  snprintf(fundoOrig, sizeof fundoOrig, "%s", p->fundoUrl);
  fotoSel = fundoSel = -1; corEscolhida[0] = 0; fotoLink[0] = 0; extraFoco = 0;
  nAvatares = nFundos = 0; catalogo = 0;
  vista = V_OPCOES; opFoco = 0; zona = Z_NOME; aba = ABA_AVATAR; item = 0; confFoco = 0;
  tecladoPara = TEC_NADA; memset(pinPrimeiro, 0, sizeof pinPrimeiro);
  memset(pinAtual, 0, sizeof pinAtual); memset(pinPendente, 0, sizeof pinPendente);
  memset(pinConhecido, 0, sizeof pinConhecido);
  if (perfiledit_pin_valido(pinVerificado)) snprintf(pinConhecido, sizeof pinConhecido, "%s", pinVerificado);
  msg[0] = 0; msgErro = 0; entrada = 0.0f;
  aberto = 1; serie++;
  if (demo) {
    static const char *const cores[] = { "#FF7043", "#7E57C2", "#26A69A", "#EC407A", "#5C6BC0",
                                         "#FFCA28", "#8D6E63", "#66BB6A", "#29B6F6", "#AB47BC" };
#define PE_DEMO_ARTE "deploy/app/art/poster"   // a folder, not interface text
    static const char *const nomes[] = { "Wanderer", "Night Owl", "Captain", "Rebel", "Drifter",
                                         "Sheriff", "Pilot", "Stranger", "Dreamer", "Scout" };
    for (int i = 0; i < 10; i++) {
      snprintf(avatares[i].id, sizeof avatares[i].id, "demo%d", i);
      snprintf(avatares[i].nome, sizeof avatares[i].nome, "%s", nomes[i]);
      snprintf(avatares[i].cor, sizeof avatares[i].cor, "%s", cores[i]);
      // Local art stands in for the account's avatar images.
      snprintf(avatares[i].url, sizeof avatares[i].url, "%s/%02d.jpg", PE_DEMO_ARTE, i);
    }
    nAvatares = 10; catalogo = 1;
    snprintf(fundos[0].nome, sizeof fundos[0].nome, "Aurora"); snprintf(fundos[1].nome, sizeof fundos[1].nome, "Neon");
    nFundos = 2;
    snprintf(pinConhecido, sizeof pinConhecido, "0000");
    if (demoVista >= 0) { vista = demoVista; aba = demoAba; zona = demoZona; item = demoItem; }
    return 1;
  }
  j = calloc(1, sizeof *j);
  if (j) { j->tipo = JOB_CATALOGOS; if (!lancar(j)) catalogo = 2; }
  else catalogo = 2;
  return 1;
}
int perfiledit_aberto(void) { return aberto; }
void perfiledit_fechar(void) {
  if (teclado_aberto()) {
    SDL_Event e;
    memset(&e, 0, sizeof e); e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_ESCAPE;
    teclado_evento(&e);
  }
  if (tecladoPara) { teclado_resultado(); teclado_esquecer(); }
  aberto = 0; tecladoPara = TEC_NADA;
  memset(pinAtual, 0, sizeof pinAtual); memset(pinConhecido, 0, sizeof pinConhecido);
  memset(pinPrimeiro, 0, sizeof pinPrimeiro);
  memset(pinPendente, 0, sizeof pinPendente);
  serie++;
}
int perfiledit_mudou(void) { int m = mudouFlag; mudouFlag = 0; return m; }

static const char *fotoEscolhida(void) {
  if (fotoSel == 0) return "";
  if (fotoSel == -2) return fotoLink;
  if (fotoSel > 0 && fotoSel <= nAvatares) return avatares[fotoSel - 1].url;
  return fotoOrig;
}
static const char *corAtual(void) { return corEscolhida[0] ? corEscolhida : corOrig; }
static int fotoMudou(void) { return fotoSel >= 0 && strcmp(fotoEscolhida(), fotoOrig); }
static int corMudou(void) { return corEscolhida[0] && strcmp(corEscolhida, corOrig); }
static int fundoMudou(void) { return fundoSel > 0 || (fundoSel == 0 && fundoOrig[0]); }
static int alterado(void) { return strcmp(nome, nomeOrig) || fotoMudou() || corMudou() || fundoMudou(); }

static void salvar(void) {
  Job *j;
  if (!perfiledit_nome_valido(nome)) { aviso("O nome não pode ficar vazio.", 1); return; }
  if (!alterado()) { perfiledit_fechar(); return; }
  if (ocupado || !(j = calloc(1, sizeof *j))) return;
  j->tipo = JOB_SALVAR;
  snprintf(j->nome, sizeof j->nome, "%s", nome);
  if (fotoMudou()) {
    if (fotoSel == 0) j->foto = 2;
    else if (fotoSel == -2) { j->foto = 3; snprintf(j->fotoUrl, sizeof j->fotoUrl, "%s", fotoLink); }
    else { j->foto = 1; snprintf(j->avatarId, sizeof j->avatarId, "%s", avatares[fotoSel - 1].id); }
  }
  if (corMudou()) snprintf(j->cor, sizeof j->cor, "%s", corEscolhida);
  if (fundoMudou()) {
    if (fundoSel == 0) j->fundo = 1;
    else { j->fundo = 2; snprintf(j->fundoId, sizeof j->fundoId, "%s", fundos[fundoSel - 1].id); }
  }
  if (lancar(j)) aviso("Salvando…", 0);
  else aviso("Entre na sua conta Nuvio para editar o perfil.", 1);
}
static void pedirPin(const char *novo) {
  Job *j = calloc(1, sizeof *j);
  if (!j) return;
  j->tipo = JOB_PIN;
  j->pinNovo = novo && novo[0];
  if (j->pinNovo) snprintf(j->pin, sizeof j->pin, "%s", novo);
  snprintf(pinPendente, sizeof pinPendente, "%s", j->pinNovo ? novo : "");
  snprintf(j->pinAtual, sizeof j->pinAtual, "%s", pinAtual);
  memset(pinAtual, 0, sizeof pinAtual);
  if (lancar(j)) aviso("Salvando…", 0);
  else aviso("Entre na sua conta Nuvio para editar o perfil.", 1);
}

// --- keyboard: the name and the new PIN (typed twice) -------------------------
static void abrirTecladoNome(void) {
  tecladoPara = TEC_NOME;
  teclado_abrir_com("Nome do perfil", "Até 20 caracteres.", PE_NOME_MAX_BYTES,
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .-_'&!", nome);
  teclado_contexto("Editar perfil");
  teclado_aceitar_utf8(1);
}
static void abrirTecladoLink(void) {
  tecladoPara = TEC_LINK;
  teclado_abrir_com("Link do avatar", "Cole ou digite um link de imagem https://.", PE_URL_MAX - 1,
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:/.-_~?=&%#+@!$()*,;",
                    fotoSel == -2 ? fotoLink : fotoOrig);
  teclado_contexto("Editar perfil");
}
static void abrirTecladoPin(int confirma) {
  tecladoPara = confirma ? TEC_PIN_CONFIRMA : TEC_PIN_NOVO;
  teclado_abrir_com(confirma ? "Confirme o novo PIN" : "Novo PIN",
                    confirma ? "Digite o mesmo PIN de novo." : "4 números.", 4, "0123456789", NULL);
  teclado_contexto(temPin ? "Mudar PIN" : "Definir PIN");
  teclado_tipo(TECLADO_TIPO_SENHA);
}
// The keyboard closed: take what it holds.
static void tecladoTerminou(void) {
  int r = teclado_resultado();
  const char *t = teclado_texto();
  int para = tecladoPara;
  if (r == TECLADO_NADA) return;
  tecladoPara = TEC_NADA;
  if (r == TECLADO_CANCELOU) { teclado_esquecer(); memset(pinPrimeiro, 0, sizeof pinPrimeiro); return; }
  if (para == TEC_NOME) {
    char a[PE_NOME_MAX_BYTES + 8];
    perfiledit_nome_aparar(t, a, sizeof a);
    if (a[0]) { snprintf(nome, sizeof nome, "%s", a); msg[0] = 0; }
    else aviso("O nome não pode ficar vazio.", 1);
  } else if (para == TEC_LINK) {
    const char *ini = t;
    size_t n;
    char l[PE_URL_MAX];
    while (*ini == ' ') ini++;
    n = strlen(ini);
    while (n && ini[n - 1] == ' ') n--;
    if (n && n < sizeof l) { memcpy(l, ini, n); l[n] = 0; } else l[0] = 0;
    if (l[0] && perfiledit_url_valida(l)) { snprintf(fotoLink, sizeof fotoLink, "%s", l); fotoSel = -2; msg[0] = 0; }
    else aviso("Digite um link http:// ou https:// válido.", 1);
  } else if (para == TEC_PIN_NOVO) {
    if (!perfiledit_pin_valido(t)) aviso("O PIN tem 4 números.", 1);
    else {
      snprintf(pinPrimeiro, sizeof pinPrimeiro, "%s", t);
      teclado_esquecer();
      abrirTecladoPin(1);
      return;
    }
  } else if (para == TEC_PIN_CONFIRMA) {
    if (strcmp(t, pinPrimeiro)) aviso("Os PINs não conferem. Tente de novo.", 1);
    else {
      snprintf(pinAtual, sizeof pinAtual, "%s", pinConhecido);
      pedirPin(t);
    }
    memset(pinPrimeiro, 0, sizeof pinPrimeiro);
  }
  teclado_esquecer();
}

// --- the options -------------------------------------------------------------
static int opLista(int *ids) {
  int n = 0;
  ids[n++] = OP_EDITAR;
  ids[n++] = OP_PIN;
  if (temPin) ids[n++] = OP_REMOVER;
  ids[n++] = OP_FECHAR;
  return n;
}
static const char *opRotulo(int id) {
  switch (id) {
    case OP_EDITAR: return "Editar perfil";
    case OP_PIN: return temPin ? "Mudar PIN" : "Definir PIN";
    case OP_REMOVER: return "Remover PIN";
    default: return "Fechar";
  }
}
static const char *opApoio(int id) {
  switch (id) {
    case OP_EDITAR: return "Nome, avatar, cor e fundo";
    case OP_PIN: return "4 números, digitados duas vezes";
    case OP_REMOVER: return "Abre sem PIN nesta TV";
    default: return "";
  }
}

// --- the editor's content ------------------------------------------------------
// Background rows: Standard, the current link (when there is one), supporter
// backgrounds. Returns the fundoSel value of row i.
static int fundoLinhas(void) { return 1 + (fundoOrig[0] ? 1 : 0) + nFundos; }
static int fundoValor(int i) {
  if (i == 0) return 0;
  if (fundoOrig[0]) return i == 1 ? -1 : i - 1;
  return i;
}
static int abaItens(void) {
  if (aba == ABA_AVATAR) return catalogo == 1 ? nAvatares : 0;
  if (aba == ABA_COR) return PE_CORES;
  return fundoLinhas();
}
static int abaColunas(void) { return aba == ABA_AVATAR ? 5 : aba == ABA_COR ? 4 : 1; }
static void escolherItem(void) {
  if (aba == ABA_AVATAR && item < nAvatares) {
    // OK again on the chosen avatar undoes the choice (official behaviour).
    if (fotoSel == item + 1) { fotoSel = -1; corEscolhida[0] = 0; }
    else {
      char c[8];
      fotoSel = item + 1;
      // Choosing an avatar also takes its colour.
      if (perfiledit_cor_normal(avatares[item].cor, c)) snprintf(corEscolhida, sizeof corEscolhida, "%s", c);
    }
  } else if (aba == ABA_COR && item < PE_CORES) snprintf(corEscolhida, sizeof corEscolhida, "%s", PE_CORES_HEX[item]);
  else if (aba == ABA_FUNDO && item < fundoLinhas()) fundoSel = fundoValor(item);
  msg[0] = 0;
}
static int abaEscolhido(int i) {
  if (aba == ABA_AVATAR) return fotoSel == i + 1 || (fotoSel == -1 && fotoOrig[0] && !strcmp(avatares[i].url, fotoOrig));
  if (aba == ABA_COR) return !strcmp(PE_CORES_HEX[i], corAtual());
  return fundoSel == fundoValor(i) || (fundoSel < 0 && fundoValor(i) == (fundoOrig[0] ? -1 : 0));
}

static void eventoEditor(SDL_Keycode k, int ok) {
  int n = abaItens(), cols = abaColunas();
  switch (zona) {
    case Z_NOME:
      if (ok) abrirTecladoNome();
      else if (k == SDLK_DOWN) zona = Z_CANCELAR;
      else if (k == SDLK_RIGHT) zona = Z_ABAS;
      else if (k == SDLK_UP) zona = Z_SALVAR;
      break;
    case Z_CANCELAR:
      if (ok) { vista = V_OPCOES; msg[0] = 0; }
      else if (k == SDLK_UP) zona = Z_NOME;
      else if (k == SDLK_RIGHT) zona = Z_ABAS;
      break;
    case Z_ABAS:
      // The tab under focus is the one shown, as in the Android panel.
      if (k == SDLK_LEFT) { if (aba > 0) { aba--; item = 0; } else zona = Z_NOME; }
      else if (k == SDLK_RIGHT) { if (aba < ABA_N - 1) { aba++; item = 0; } }
      else if (k == SDLK_UP) zona = Z_SALVAR;
      else if (k == SDLK_DOWN || ok) { if (n > 0) { zona = Z_GRADE; if (item >= n) item = n - 1; } else if (aba == ABA_AVATAR) zona = Z_EXTRA; }
      break;
    case Z_GRADE:
      if (ok) escolherItem();
      else if (k == SDLK_LEFT) { if (item % cols) item--; else zona = Z_NOME; }
      else if (k == SDLK_RIGHT) { if (item % cols < cols - 1 && item + 1 < n) item++; }
      else if (k == SDLK_UP) { if (item >= cols) item -= cols; else zona = Z_ABAS; }
      else if (k == SDLK_DOWN) {
        if (item + cols < n) item += cols;
        else if ((item / cols) < (n - 1) / cols) item = n - 1;
        else if (aba == ABA_AVATAR) zona = Z_EXTRA;
      }
      break;
    case Z_EXTRA:
      // Two buttons under the avatars: No photo | Avatar link.
      if (ok) { if (extraFoco == 0) { fotoSel = 0; msg[0] = 0; } else abrirTecladoLink(); }
      else if (k == SDLK_UP) zona = abaItens() > 0 ? Z_GRADE : Z_ABAS;
      else if (k == SDLK_RIGHT) extraFoco = 1;
      else if (k == SDLK_LEFT) { if (extraFoco) extraFoco = 0; else zona = Z_NOME; }
      break;
    case Z_SALVAR:
      if (ok) salvar();
      else if (k == SDLK_DOWN) zona = Z_ABAS;
      else if (k == SDLK_LEFT) zona = Z_NOME;
      break;
  }
}

void perfiledit_evento(const SDL_Event *e) {
  SDL_Keycode k;
  int ok;
  if (!aberto || !e) return;
  if (teclado_aberto()) { teclado_evento(e); return; }
  if (e->type != SDL_KEYDOWN) return;
  k = e->key.keysym.sym;
  ok = k == SDLK_RETURN || k == SDLK_KP_ENTER;
  // Back steps out one level: editor/confirmation -> options -> closed.
  if (k == SDLK_ESCAPE || k == SDLK_AC_BACK || k == SDLK_BACKSPACE) {
    if (vista != V_OPCOES && !ocupado) { vista = V_OPCOES; msg[0] = 0; }
    else perfiledit_fechar();
    return;
  }
  if (ocupado) return;   // saving: only Back
  if (vista == V_OPCOES) {
    int ids[4], n = opLista(ids);
    if (opFoco >= n) opFoco = n - 1;
    if (k == SDLK_UP) { if (opFoco > 0) opFoco--; }
    else if (k == SDLK_DOWN) { if (opFoco < n - 1) opFoco++; }
    else if (ok) {
      msg[0] = 0;
      switch (ids[opFoco]) {
        case OP_EDITAR: vista = V_EDITOR; zona = Z_NOME; break;
        case OP_PIN: abrirTecladoPin(0); break;
        case OP_REMOVER: vista = V_CONFIRMA; confFoco = 0; break;
        default: perfiledit_fechar(); break;
      }
    }
    return;
  }
  if (vista == V_CONFIRMA) {
    if (k == SDLK_UP) confFoco = 0;
    else if (k == SDLK_DOWN) confFoco = 1;
    else if (ok) {
      if (confFoco == 1) { snprintf(pinAtual, sizeof pinAtual, "%s", pinConhecido); pedirPin(""); }
      vista = V_OPCOES;
    }
    return;
  }
  eventoEditor(k, ok);
}

void perfiledit_atualizar(float dt, Uint32 agora) {
  Resultado *r = NULL;
  if (!aberto) return;
  entrada = anim_reduzida(anim_mola(entrada, 1.0f, dt, NV_MOLA_TELA), 1.0f, ajustes_animacoes_reduzidas());
  if (teclado_aberto()) { teclado_atualizar(dt, agora); return; }
  if (tecladoPara) tecladoTerminou();
  if (!demo && !perfis_sessao_atual(geracao)) { perfiledit_fechar(); return; }
  if (!mtx) return;
  SDL_LockMutex(mtx);
  if (pronto && res.serie == serie && (r = malloc(sizeof *r))) *r = res;
  pronto = 0;
  SDL_UnlockMutex(mtx);
  if (!r) return;
  if (r->tipo == JOB_CATALOGOS) {
    nAvatares = r->nAv; memcpy(avatares, r->av, sizeof avatares);
    nFundos = r->nFu; memcpy(fundos, r->fu, sizeof fundos);
    catalogo = r->ok ? 1 : 2;
  } else if (!r->ok) {
    aviso(mensagemFalha(r->status, r->tipo == JOB_PIN), 1);
  } else if (perfis_sessao_travar(geracao)) {
    if (r->tipo == JOB_SALVAR) {
      const char *foto = r->fotoVeio ? r->foto : fotoEscolhida();
      if (!perfiledit_url_valida(foto)) foto = fotoOrig;
      perfis_aplicar_edicao(perfil, r->nome[0] ? r->nome : nome, foto);
      if (r->temCor || r->temFundo)
        perfis_aplicar_aparencia(perfil, r->temCor ? r->cor : NULL,
                                 r->temFundo && perfiledit_url_valida(r->fundo) ? r->fundo : NULL);
      perfis_sessao_destravar();
      mudouFlag = 1;
      perfiledit_fechar();
    } else {
      perfis_aplicar_pin(perfil, r->pin);
      perfis_sessao_destravar();
      temPin = r->pin; mudouFlag = 1;
      memcpy(pinConhecido, pinPendente, sizeof pinConhecido);
      aviso(r->pin ? "PIN definido." : "PIN removido.", 0);
    }
  }
  free(r);
}

// --- drawing: Settings' modal language ------------------------------------------------
#define PE_TX 243, 242, 239, 255
#define PE_LH   56.0f    // a row of the menu (ajustes_ux_perfil_desenho.inc)
#define PE_GAP   6.0f
static void corDe(const char *hx, float *r, float *g, float *b) {
  unsigned v = 0;
  char c[8];
  if (!perfiledit_cor_normal(hx, c) || sscanf(c + 1, "%6x", &v) != 1) { *r = .12f; *g = .53f; *b = .9f; return; }
  *r = ((v >> 16) & 255) / 255.0f; *g = ((v >> 8) & 255) / 255.0f; *b = (v & 255) / 255.0f;
}
static TxtLinha tx(TxtEstilo e, const char *s, int foco) {
  int t = ajustes_tinta_foco();
  return foco ? txt_linha(e, s, t, t, t, 255) : txt_linha(e, s, PE_TX);
}
static void icone(const char *nomeIc, float x, float y, float lado, int foco, float a) {
  float t = foco ? ajustes_tinta_foco() / 255.0f : 0.953f;
  gfx_icone((GfxRect){ x, y, lado, lado }, nomeIc, t, t, t, a);
}
// A round avatar: the photo, or the coloured circle with the name's initial.
static void avatarDesenhar(GfxRect r, const char *url, const char *cor, TxtEstilo estilo, float a) {
  GLuint tex = url && url[0] ? tex_obter_larg(url, r.w) : 0;
  float cr, cg, cb;
  const char *s = nome[0] ? nome : "?";
  char ini[8];
  int l;
  TxtLinha t;
  corDe(cor, &cr, &cg, &cb);
  gfx_cor(r, 0.5f, cr, cg, cb, a);
  if (tex) {
    gfx_tex_aspect_atual = tex_aspecto(url);
    gfx_rect(r, tex, GFX_CARD, 0, 0, 0, 0.5f, 0, 0, 0, a);
    gfx_tex_aspect_atual = 0;
    return;
  }
  l = cpLen((const unsigned char *)s);
  memcpy(ini, s, (size_t)l); ini[l] = 0;
  t = txt_linha(estilo, ini, 255, 255, 255, 255);
  txt_desenhar_alpha(t, r.x + (r.w - (float)t.w) * 0.5f, r.y + (r.h - (float)t.h) * 0.5f, a);
}
// A full-width menu row: label left, detail right (the profile menu's row).
static void linhaMenu(GfxRect r, const char *rot, const char *apoio, int foco, int perigo, float a) {
  TxtLinha t = tx(TXT_ILHA_ITEM, rot, foco);
  if (foco) ajustes_ui_pilula_acento(r, r.h * 0.5f, a);
  else ajustes_ui_chip(r, r.h * 0.5f, 0.07f, a);
  if (perigo && !foco) t = txt_linha(TXT_ILHA_ITEM, rot, 255, 150, 150, 255);
  txt_desenhar_alpha(t, r.x + 26, r.y + (r.h - t.h) * 0.5f, a);
  if (apoio && apoio[0]) {
    TxtLinha p = tx(TXT_V2_24, apoio, foco);
    if (p.w <= r.w - 26 - (26 + t.w + 24))
      txt_desenhar_alpha(p, r.x + r.w - 26 - p.w, r.y + (r.h - p.h) * 0.5f, (foco ? 0.78f : 0.5f) * a);
  }
}
// Kicker, title and (optional) body; returns the y under them.
static float cabecalho(float x, float y, float w, const char *kick, const char *tit, const char *corpo, float a) {
  TxtLinha k = txt_linha(TXT_G22M, kick, PE_TX);
  txt_desenhar_alpha(k, x, y, a);
  y += 22 + 8;
  { TxtLinha t = txt_linha_corta(TXT_ILHA_PERGUNTA, tit, PE_TX, w); txt_desenhar_alpha(t, x, y, a); y += 44 + 10; }
  if (corpo && corpo[0]) y += txt_bloco(TXT_AJ_TEXTO, corpo, 243, 242, 239, x, y, w, 30, 0.62f * a, 4) + 22;
  return y;
}
static float alturaCorpo(const char *corpo, float w) {
  return corpo && corpo[0] ? txt_bloco(TXT_AJ_TEXTO, corpo, 0, 0, 0, 0, -4000, w, 30, 0, 4) + 22 : 0;
}
static void avisoLinha(float x, float y, float w, float a) {
  if (!msg[0]) return;
  { TxtLinha t = msgErro ? txt_linha_corta(TXT_V2_24, msg, 255, 170, 170, 255, w) : txt_linha_corta(TXT_V2_24, msg, PE_TX, w);
    txt_desenhar_alpha(t, x, y, msgErro ? a : 0.72f * a); }
}
static void dicas(float x, float yc, const char *ok, const char *voltar, float a) {
  const char *k[2] = { "OK", "Voltar" }, *l[2] = { ok, voltar };
  ajustes_ui_dicas_modal(k, l, 2, x, yc, a);
}

static void desenharOpcoes(float a) {
  int ids[4], n = opLista(ids);
  const char *corpo = "As mudanças vão para a sua conta Nuvio e aparecem em todos os aparelhos.";
  GfxRect m = { (NV_TELA_W - 1000) * 0.5f, 0, 1000, 0 };
  float x, y;
  m.h = 44 + 30 + 54 + alturaCorpo(corpo, m.w - 88) + (float)n * (PE_LH + PE_GAP) + (msg[0] ? 40 : 0) + 18 + 44 + 4;
  m.y = (NV_TELA_H - m.h) * 0.5f;
  ajustes_ui_ilha_a(m, 36, 1, a);
  x = m.x + 44;
  y = cabecalho(x, m.y + 44, m.w - 88, temPin ? "Perfil · Com PIN" : "Perfil", nome, corpo, a);
  for (int i = 0; i < n; i++) {
    linhaMenu((GfxRect){ x, y, m.w - 88, PE_LH }, opRotulo(ids[i]), opApoio(ids[i]), i == opFoco, 0, a);
    y += PE_LH + PE_GAP;
  }
  if (msg[0]) { avisoLinha(x + 4, y + 8, m.w - 96, a); y += 40; }
  dicas(x, y + 12 + 20, "Escolher", "Fechar", a);
}

static void desenharConfirma(float a) {
  const char *corpo = "Qualquer pessoa usando esta TV poderá abrir este perfil sem PIN.";
  GfxRect m = { (NV_TELA_W - 1000) * 0.5f, 0, 1000, 0 };
  float x, y;
  m.h = 44 + 30 + 54 + alturaCorpo(corpo, m.w - 88) + 2 * (PE_LH + PE_GAP) + 18 + 44 + 4;
  m.y = (NV_TELA_H - m.h) * 0.5f;
  ajustes_ui_ilha_a(m, 36, 1, a);
  x = m.x + 44;
  y = cabecalho(x, m.y + 44, m.w - 88, "Remover PIN", nome, corpo, a);
  linhaMenu((GfxRect){ x, y, m.w - 88, PE_LH }, "Cancelar", NULL, confFoco == 0, 0, a); y += PE_LH + PE_GAP;
  linhaMenu((GfxRect){ x, y, m.w - 88, PE_LH }, "Remover PIN", NULL, confFoco == 1, 1, a); y += PE_LH + PE_GAP;
  dicas(x, y + 12 + 20, "Escolher", "Voltar", a);
}

// The segmented Avatar | Colour | Background, like Settings' "Panel | List".
static void desenharAbas(float cx, float y, float a) {
  static const char *const rot[ABA_N] = { "Avatar", "Cor", "Fundo" };
  static const char *const ico[ABA_N] = { "aj_user-round", "aj_palette", "aj_image" };
  float lw[ABA_N], w = 12, x;
  int foco = zona == Z_ABAS;
  for (int i = 0; i < ABA_N; i++) { lw[i] = 22 + 26 + 10 + (float)txt_largura(TXT_V2_SEG, rot[i]) + 24; w += lw[i]; }
  x = cx - w * 0.5f;
  { GfxRect r = { x, y, w, 64 };
    if (foco) ajustes_ui_pilula_acento(r, 32, a); else ajustes_ui_chip(r, 32, 0.08f, a); }
  x += 6;
  for (int i = 0; i < ABA_N; i++) {
    GfxRect q = { x, y + 6, lw[i], 52 };
    TxtLinha t = tx(TXT_V2_SEG, rot[i], foco);
    if (i == aba) {
      float ti = ajustes_tinta_foco() / 255.0f;
      if (foco) gfx_cor(q, 0.5f, ti, ti, ti, 0.22f * a); else gfx_cor(q, 0.5f, 1, 1, 1, 0.14f * a);
    }
    icone(ico[i], x + 22, y + 19, 26, foco, (i == aba ? 1.0f : 0.55f) * a);
    txt_desenhar_alpha(t, x + 22 + 26 + 10, y + (64 - t.h) * 0.5f, (i == aba ? 1.0f : 0.55f) * a);
    x += lw[i];
  }
}

static void desenharEditor(float a) {
  GfxRect m = { (NV_TELA_W - 1440) * 0.5f, (NV_TELA_H - 880) * 0.5f, 1440, 880 };
  float ar, ag, ab, x = m.x + 44, y, lx, lw = 340, rx, rw, top;
  ajustes_acento(&ar, &ag, &ab);
  ajustes_ui_ilha_a(m, 36, 1, a);
  y = cabecalho(x, m.y + 44, 900, "Editar perfil", nomeOrig, NULL, a);
  // Save, top right (the Android panel's place for it).
  { float bw = 28 + 32 + (float)txt_largura(TXT_ILHA_ITEM, "Salvar") + 28;
    ajustes_ui_botao("Salvar", "aj_check", m.x + m.w - 44 - bw, m.y + 50, zona == Z_SALVAR); }
  top = y + 10;
  // Left: the profile as it will be saved, the name, Cancel.
  lx = x;
  avatarDesenhar((GfxRect){ lx + (lw - 200) * 0.5f, top, 200, 200 }, fotoEscolhida(), corAtual(), TXT_TITULO1, a);
  { GfxRect r = { lx, top + 232, lw, 64 };
    int foco = zona == Z_NOME;
    TxtLinha t = foco ? txt_linha_corta(TXT_ILHA_ITEM, nome, ajustes_tinta_foco(), ajustes_tinta_foco(), ajustes_tinta_foco(), 255, lw - 90)
                      : txt_linha_corta(TXT_ILHA_ITEM, nome, PE_TX, lw - 90);
    if (foco) ajustes_ui_pilula_acento(r, 32, a); else ajustes_ui_chip(r, 32, 0.08f, a);
    txt_desenhar_alpha(t, r.x + 26, r.y + (r.h - t.h) * 0.5f, a);
    icone("aj_pencil", r.x + r.w - 26 - 24, r.y + 20, 24, foco, 0.9f * a); }
  { TxtLinha t = txt_linha(TXT_V2_24, "Até 20 caracteres.", PE_TX);
    txt_desenhar_alpha(t, lx + (lw - t.w) * 0.5f, top + 306, 0.5f * a); }
  { float bw = 28 + 32 + (float)txt_largura(TXT_ILHA_ITEM, "Cancelar") + 28;
    ajustes_ui_botao("Cancelar", "aj_x", lx + (lw - bw) * 0.5f, top + 360, zona == Z_CANCELAR); }
  // Divider, then the tabs and their content.
  gfx_cor((GfxRect){ lx + lw + 44, top, 2, m.y + m.h - 110 - top }, 0, 1, 1, 1, 0.10f * a);
  rx = lx + lw + 90; rw = m.x + m.w - 44 - rx;
  desenharAbas(rx + rw * 0.5f, top, a);
  y = top + 64 + 26;
  { static const char *const nota[ABA_N] = { "Escolha um avatar", "Escolha a cor do perfil", "Escolha o fundo deste perfil" };
    TxtLinha t = txt_linha(TXT_V2_24, nota[aba], PE_TX);
    txt_desenhar_alpha(t, rx + (rw - t.w) * 0.5f, y, 0.62f * a); y += 44; }
  if (aba == ABA_AVATAR) {
    float d = 116, gap = 30, gx = rx + (rw - (5 * d + 4 * gap)) * 0.5f;
    int linhas = 2, primeira = 0, n = nAvatares;
    if (catalogo == 0) { TxtLinha t = txt_linha(TXT_ILHA_ITEM, "Carregando avatares…", PE_TX); txt_desenhar_alpha(t, rx + (rw - t.w) * 0.5f, y + 90, 0.6f * a); }
    else if (catalogo == 2 || n == 0) { TxtLinha t = txt_linha(TXT_ILHA_ITEM, catalogo == 2 ? "Não foi possível carregar os avatares." : "Nenhum avatar nesta conta.", PE_TX); txt_desenhar_alpha(t, rx + (rw - t.w) * 0.5f, y + 90, 0.6f * a); }
    else {
      if (zona == Z_GRADE && item / 5 >= linhas) primeira = item / 5 - linhas + 1;
      for (int i = primeira * 5; i < n && i < (primeira + linhas) * 5; i++) {
        GfxRect c = { gx + (float)(i % 5) * (d + gap), y + (float)(i / 5 - primeira) * (d + gap), d, d };
        avatarDesenhar(c, avatares[i].url, avatares[i].cor[0] ? avatares[i].cor : corAtual(), TXT_TITULO3, a);
        if (zona == Z_GRADE && i == item) gfx_anel_fora(c, 0.5f, 5.0f, 4.0f, ar, ag, ab, a);
        if (abaEscolhido(i)) {
          GfxRect b = { c.x + d - 40, c.y + d - 40, 40, 40 };
          gfx_cor((GfxRect){ b.x - 3, b.y - 3, 46, 46 }, 0.5f, .086f, .09f, .106f, a);
          gfx_cor(b, 0.5f, ar, ag, ab, a);
          icone("aj_check", b.x + 9, b.y + 9, 22, 1, a);
        }
      }
    }
    y += linhas * (d + gap) + 4;
    { const char *nm = zona == Z_GRADE && item < n ? avatares[item].nome : "";
      if (nm[0]) { TxtLinha t = txt_linha(TXT_ILHA_ITEM, nm, PE_TX); txt_desenhar_alpha(t, rx + (rw - t.w) * 0.5f, y, a); } }
    y += 50;
    { float b0 = 28 + 32 + (float)txt_largura(TXT_ILHA_ITEM, "Sem foto") + 28;
      float b1 = 28 + 32 + (float)txt_largura(TXT_ILHA_ITEM, "Link do avatar") + 28, bx = rx + (rw - (b0 + 16 + b1)) * 0.5f;
      ajustes_ui_botao("Sem foto", fotoSel == 0 ? "aj_check" : "aj_user-round", bx, y, zona == Z_EXTRA && extraFoco == 0);
      ajustes_ui_botao("Link do avatar", fotoSel == -2 ? "aj_check" : "aj_link", bx + b0 + 16, y, zona == Z_EXTRA && extraFoco == 1);
      // The link in use, under the buttons, so a typo is visible before Save.
      if (fotoSel == -2) {
        TxtLinha t = txt_linha_corta(TXT_V2_24, fotoLink, PE_TX, rw - 40);
        txt_desenhar_alpha(t, rx + (rw - t.w) * 0.5f, y + 72, 0.5f * a);
      } }
  } else if (aba == ABA_COR) {
    float d = 104, gap = 34, gx = rx + (rw - (4 * d + 3 * gap)) * 0.5f;
    for (int i = 0; i < PE_CORES; i++) {
      float cr, cg, cb;
      GfxRect c = { gx + (float)(i % 4) * (d + gap), y + 8 + (float)(i / 4) * (d + gap), d, d };
      corDe(PE_CORES_HEX[i], &cr, &cg, &cb);
      gfx_cor(c, 0.5f, cr, cg, cb, a);
      if (zona == Z_GRADE && i == item) gfx_anel_fora(c, 0.5f, 5.0f, 4.0f, ar, ag, ab, a);
      if (abaEscolhido(i)) icone("aj_check", c.x + (d - 40) * 0.5f, c.y + (d - 40) * 0.5f, 40, 0, a);
    }
    y += 8 + 2 * (d + gap) + 10;
    { TxtLinha t = txt_linha(TXT_V2_24, "Escolher um avatar também escolhe a cor.", PE_TX);
      txt_desenhar_alpha(t, rx + (rw - t.w) * 0.5f, y, 0.5f * a); }
  } else {
    int n = fundoLinhas();
    for (int i = 0; i < n && i < 5; i++) {
      int v = fundoValor(i), foco = zona == Z_GRADE && i == item;
      const char *rot = v == 0 ? "Padrão" : v < 0 ? "Atual" : fundos[v - 1].nome;
      GfxRect r = { rx + 40, y, rw - 80, PE_LH };
      linhaMenu(r, rot, NULL, foco, 0, a);
      if (abaEscolhido(i)) icone("aj_check", r.x + r.w - 26 - 26, r.y + 15, 26, foco, a);
      y += PE_LH + PE_GAP;
    }
    if (nFundos > 0) {
      TxtLinha t = txt_linha(TXT_V2_24, "Fundos de apoiador aparecem nos apps oficiais do Nuvio.", PE_TX);
      txt_desenhar_alpha(t, rx + (rw - t.w) * 0.5f, y + 10, 0.5f * a);
    }
  }
  // Footer: the status, then the key hints.
  { float fy = m.y + m.h - 44 - 20;
    if (msg[0]) avisoLinha(x, fy - 52, m.w - 88, a);
    else { TxtLinha t = txt_linha(TXT_V2_24, "As mudanças vão para a sua conta Nuvio.", PE_TX); txt_desenhar_alpha(t, x, fy - 52, 0.5f * a); }
    dicas(x, fy, zona == Z_NOME ? "Editar nome" : "Escolher", "Opções", a); }
}

void perfiledit_desenhar(Uint32 agora) {
  float a = entrada;
  if (!aberto) return;
  gfx_cor((GfxRect){ 0, 0, NV_TELA_W, NV_TELA_H }, 0, 0, 0, 0, 0.55f * a);   // Settings' modal veil, a step darker over the poster wall
  if (!teclado_aberto() || tecladoPara == TEC_NOME) {
    if (vista == V_OPCOES) desenharOpcoes(a);
    else if (vista == V_CONFIRMA) desenharConfirma(a);
    else desenharEditor(a);
  }
  if (teclado_aberto()) teclado_desenhar(agora);
}
#endif

// Stage H Family Watching: app state (see familia.h). The main thread
// snapshots the profiles and settings; the Home catalogue worker reads through
// the registered transport (none in this build) and keeps the last cards.
#include "familia.h"
#include "ajustes.h"
#include "perfis.h"
#include "perfilinfantil.h"
#include "descoberta.h"
#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define FAM_PERFIS_MAX 16
#define FAM_PASSO_MS   5000

static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static FamPerfil perfis[FAM_PERFIS_MAX];
static int nPerfis, ativo, ativoInf, ligado;
static FamBuscar busca;
static void *buscaU;
static FamCartao cartoes[FAM_MAX];
static int nCartoes, pedido = 1;
static Uint32 lidoEm, ultimoPasso;
static int lido;   // lidoEm is meaningful (never encode it in the time itself)
static unsigned rev;

void familia_definir_busca(FamBuscar fn, void *u) {
  pthread_mutex_lock(&trava);
  busca = fn; buscaU = u; pedido = 1;
  pthread_mutex_unlock(&trava);
}
int familia_tem_busca(void) {
  int b;
  pthread_mutex_lock(&trava); b = busca != NULL; pthread_mutex_unlock(&trava);
  return b;
}

// HOME OPEN / SETTINGS CHANGE -> A FAMILY-ONLY READ. The Home catalogue cycle
// may have run before the first snapshot (row Off at that instant), so waiting
// for the next cycle could leave the row empty until a profile switch. The
// read runs on its own thread (throttled, at most one at a time); Home is
// rebuilt only when the cards actually change (familia_revisao).
static volatile int fioRodando;
static unsigned revPublicada;
static int revConhecida;
int familia_quer_ler(void) {
  int quer, eleg = 0;
  Uint32 t = SDL_GetTicks();
  pthread_mutex_lock(&trava);
  for (int i = 0; i < nPerfis; i++) eleg += familia_perfil_elegivel(&perfis[i], ativo);
  quer = busca && familia_linha_visivel(ligado, ativoInf) && eleg > 0 && !fioRodando &&
         (pedido || !lido || t - lidoEm >= FAM_REFAZ_MS);
  pthread_mutex_unlock(&trava);
  return quer;
}
static void *fioLeitura(void *u) {
  FamCartao c[FAM_MAX];
  (void)u;
  familia_atualizar(c, FAM_MAX);
  __atomic_store_n(&fioRodando, 0, __ATOMIC_RELEASE);
  return NULL;
}
static void lancarLeitura(void) {
#ifndef NV_FAMILIA_SEM_FIO
  pthread_t f;
  fioRodando = 1;
  if (pthread_create(&f, NULL, fioLeitura, NULL) == 0) pthread_detach(f);
  else fioRodando = 0;
#endif
}

void familia_passo(void) {
  FamPerfil tmp[FAM_PERFIS_MAX];
  int i, k = 0, lig, at, inf;
  Uint32 t = SDL_GetTicks();
  // PROFILE SWITCH: the cards were read for the previous viewer and may hold
  // the NEW viewer's own activity. Drop them now and re-snapshot at once;
  // never wait for the throttle.
  { int atNow = perfis_ativo(), trocou;
    pthread_mutex_lock(&trava);
    trocou = ativo != atNow;
    if (trocou) {
      if (nCartoes) rev++;
      nCartoes = 0; nPerfis = 0; ativo = atNow; lido = 0; pedido = 1; ultimoPasso = 0;
    }
    pthread_mutex_unlock(&trava);
    if (trocou) printf("[familia] profile changed: cards cleared\n"); }
  { unsigned r = familia_revisao();
    if (!revConhecida) { revPublicada = r; revConhecida = 1; }
    else if (r != revPublicada && !fioRodando) {
      revPublicada = r;
      printf("[familia] cards changed: rebuilding Home\n");
      desc_repetir();
    } }
  if (ultimoPasso && t - ultimoPasso < FAM_PASSO_MS) return;
  ultimoPasso = t | 1;
  lig = ajustes_familia();
  at = perfis_ativo();
  inf = perfil_infantil_ativo();
  // Row off or child viewer: no per-profile reads of anything at all.
  if (familia_linha_visivel(lig, inf)) {
    for (i = 0; i < perfis_n() && k < FAM_PERFIS_MAX; i++) {
      const ContaPerfil *c = perfis_item(i);
      FamPerfil *p = &tmp[k];
      if (!c || c->indice <= 0 || c->indice == at) continue;
      memset(p, 0, sizeof *p);
      p->perfil = c->indice;
      snprintf(p->nome, sizeof p->nome, "%s", c->nome);
      snprintf(p->avatar, sizeof p->avatar, "%s", c->avatarUrl);
      snprintf(p->cor, sizeof p->cor, "%s", c->corHex);
      p->temPin = c->temPin;
      p->infantil = perfil_infantil(c->indice);
      p->partilha = ajustes_familia_partilha_perfil(c->indice);
      k++;
    }
  }
  pthread_mutex_lock(&trava);
  memcpy(perfis, tmp, sizeof *tmp * (size_t)k);
  nPerfis = k; ativo = at; ativoInf = inf; ligado = lig;
  pthread_mutex_unlock(&trava);
  { static int ultimo = -1;
    int eleg = 0;
    for (i = 0; i < k; i++) eleg += familia_perfil_elegivel(&tmp[i], at);
    int estado = (lig ? 1 : 0) | (inf ? 2 : 0) | (eleg << 2);
    if (estado != ultimo) {
      // Eligibility changed (sharing off, PIN, child...): drop stale cards now.
      if (ultimo >= 0) { pthread_mutex_lock(&trava); if (nCartoes) rev++; pedido = 1; pthread_mutex_unlock(&trava); }
      ultimo = estado;
      printf("[familia] row %s, viewer child %d, %d other profile(s), %d sharing/eligible\n",
             lig ? "on" : "off", inf, k, eleg);
    } }
  if (familia_quer_ler()) lancarLeitura();
}

// A card stays only while its profile is still eligible NOW (it may have been
// PIN-locked, marked child or stopped sharing since the read).
static int aindaElegivel(int perfil) {
  // Never the viewer's own card, whatever the snapshot says.
  if (perfil == perfis_ativo() || ativo != perfis_ativo()) return 0;
  for (int i = 0; i < nPerfis; i++)
    if (perfis[i].perfil == perfil) return familia_perfil_elegivel(&perfis[i], ativo);
  return 0;
}

int familia_atualizar(FamCartao *out, int max) {
  FamPerfil snap[FAM_PERFIS_MAX];
  int n, k = 0, lig, at, inf, ler;
  FamBuscar b;
  void *u;
  Uint32 t = SDL_GetTicks();
  pthread_mutex_lock(&trava);
  memcpy(snap, perfis, sizeof *perfis * (size_t)nPerfis); n = nPerfis;
  lig = ligado; at = ativo; inf = ativoInf; b = busca; u = buscaU;
  ler = pedido || !lido || t - lidoEm >= FAM_REFAZ_MS;
  pthread_mutex_unlock(&trava);
  if (!familia_linha_visivel(lig, inf) || !b) {
    pthread_mutex_lock(&trava);
    if (nCartoes) { nCartoes = 0; rev++; }
    pedido = 0;
    pthread_mutex_unlock(&trava);
    return 0;
  }
  if (ler) {
    FamCartao novo[FAM_MAX];
    int nn = familia_montar(snap, n, at, inf, lig, b, u, novo, FAM_MAX);
    pthread_mutex_lock(&trava);
    if (nn != nCartoes || memcmp(novo, cartoes, sizeof *novo * (size_t)nn)) rev++;
    memcpy(cartoes, novo, sizeof *novo * (size_t)nn); nCartoes = nn;
    lidoEm = t; lido = 1; pedido = 0;
    pthread_mutex_unlock(&trava);
    printf("[familia] %d card(s)\n", nn);
  }
  pthread_mutex_lock(&trava);
  for (int i = 0; i < nCartoes && k < max; i++)
    if (aindaElegivel(cartoes[i].perfil)) out[k++] = cartoes[i];
  pthread_mutex_unlock(&trava);
  return k;
}

// Cache only, never a read: what the Home catalogue cycle uses. Reads happen
// on the Family thread alone (familia_passo), so a settings change or a Home
// open costs one read per eligible profile, not one per caller.
int familia_cartoes(FamCartao *out, int max) {
  int k = 0;
  pthread_mutex_lock(&trava);
  if (familia_linha_visivel(ligado, ativoInf))
    for (int i = 0; i < nCartoes && k < max; i++)
      if (aindaElegivel(cartoes[i].perfil)) out[k++] = cartoes[i];
  pthread_mutex_unlock(&trava);
  return k;
}

int familia_cartao_do_perfil(int perfil, FamCartao *out) {
  int ok = 0;
  pthread_mutex_lock(&trava);
  for (int i = 0; i < nCartoes; i++)
    if (cartoes[i].perfil == perfil && aindaElegivel(perfil)) {
      if (out) *out = cartoes[i];
      ok = 1; break;
    }
  pthread_mutex_unlock(&trava);
  return ok;
}

// Home opened: snapshot now; a read follows only if the cards are stale (2 min).
void familia_home_aberta(void) {
  pthread_mutex_lock(&trava);
  ultimoPasso = 0;
  pthread_mutex_unlock(&trava);
}

void familia_ajuste_mudou(void) {
  pthread_mutex_lock(&trava);
  pedido = 1; ultimoPasso = 0;
  pthread_mutex_unlock(&trava);
  familia_passo();
  desc_repetir();
}

void familia_esquecer(void) {
  pthread_mutex_lock(&trava);
  nPerfis = 0; nCartoes = 0; lidoEm = 0; lido = 0; pedido = 1; ultimoPasso = 0; rev++;
  pthread_mutex_unlock(&trava);
}

unsigned familia_revisao(void) {
  unsigned r;
  pthread_mutex_lock(&trava); r = rev; pthread_mutex_unlock(&trava);
  return r;
}

// Stage H Family Watching: the pure rules and parsers (see familia.h). No
// network, no globals: the app state lives in familia_app.c.
#include "familia.h"
#include "idioma.h"
#include "js.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

int familia_linha_visivel(int ligado, int ativoInfantil) {
  return ligado && !ativoInfantil;
}

int familia_perfil_elegivel(const FamPerfil *p, int ativo) {
  return p && p->perfil > 0 && p->perfil != ativo && !p->temPin && !p->infantil &&
         p->partilha;
}

// An in-progress row near the end is a finished watch, as Continue Watching
// treats it (>= 90 %); an empty or zero-length row is not activity.
static int estadoDe(const FamAtividade *a) {
  if (a->visto) return FAM_VISTO;
  if (!(a->durMs > 0) || !(a->posMs > 0)) return 0;
  return a->posMs * 100.0 >= a->durMs * FAM_VISTO_PCT ? FAM_VISTO : FAM_EM_CURSO;
}

int familia_escolher(const FamAtividade *a, int n, FamAtividade *out) {
  int i, melhor = -1, melhorEst = 0;
  for (i = 0; i < n; i++) {
    int e = estadoDe(&a[i]);
    if (!e || !a[i].id[0]) continue;
    // Equal times are common: "mark season/series watched" stamps every
    // episode with one instant. Then in-progress wins, and otherwise the
    // furthest episode (S2 E8, not S1 E1).
    int empata = a[i].quandoMs == (melhor < 0 ? 0 : a[melhor].quandoMs);
    if (melhor < 0 || a[i].quandoMs > a[melhor].quandoMs ||
        (empata && e == FAM_EM_CURSO && melhorEst != FAM_EM_CURSO) ||
        (empata && (e == FAM_EM_CURSO) == (melhorEst == FAM_EM_CURSO) &&
         (a[i].temporada > a[melhor].temporada ||
          (a[i].temporada == a[melhor].temporada && a[i].episodio > a[melhor].episodio)))) {
      melhor = i; melhorEst = e;
    }
  }
  if (melhor < 0) return 0;
  if (out) *out = a[melhor];
  return 1;
}

int familia_cartao_de(const FamPerfil *p, const FamAtividade *a, FamCartao *out) {
  int e = estadoDe(a);
  if (!p || !a || !out || !e) return 0;
  memset(out, 0, sizeof *out);
  out->perfil = p->perfil;
  snprintf(out->nome, sizeof out->nome, "%s", p->nome);
  snprintf(out->avatar, sizeof out->avatar, "%s", p->avatar);
  snprintf(out->cor, sizeof out->cor, "%s", p->cor);
  snprintf(out->id, sizeof out->id, "%s", a->id);
  snprintf(out->tipo, sizeof out->tipo, "%s", a->tipo[0] ? a->tipo : "movie");
  out->temporada = a->temporada; out->episodio = a->episodio;
  out->estado = e; out->quandoMs = a->quandoMs; out->restanteMin = -1;
  if (e == FAM_EM_CURSO) {
    double p01 = a->posMs / a->durMs;
    out->pct = (int)(p01 * 100.0);
    if (out->pct < 1) out->pct = 1;
    out->restanteMin = (int)((a->durMs - a->posMs) / 60000.0 + 0.5);
  }
  return 1;
}

int familia_montar(const FamPerfil *perfis, int n, int ativo, int ativoInfantil,
                   int ligado, FamBuscar buscar, void *u, FamCartao *out, int max) {
  int i, k = 0;
  if (!familia_linha_visivel(ligado, ativoInfantil) || !buscar || !out || max <= 0) return 0;
  for (i = 0; i < n && k < max; i++) {
    FamAtividade at[FAM_ATIV_MAX], esc;
    int na;
    if (!familia_perfil_elegivel(&perfis[i], ativo)) continue;
    na = buscar(perfis[i].perfil, at, FAM_ATIV_MAX, u);
    if (na <= 0 || !familia_escolher(at, na, &esc)) continue;
    if (familia_cartao_de(&perfis[i], &esc, &out[k])) k++;
  }
  // Newest first: the row reads like a family feed; stable for equal times.
  for (i = 1; i < k; i++) {
    FamCartao v = out[i];
    int j = i;
    while (j > 0 && out[j - 1].quandoMs < v.quandoMs) { out[j] = out[j - 1]; j--; }
    out[j] = v;
  }
  return k;
}

// ---- parsers ----------------------------------------------------------------
static int coord(const char *p, const char *f, const char *chave) {
  char raw[12];
  double v;
  if (!js_tem(p, f, chave) || (js_bruto(p, f, chave, raw, sizeof raw) && !strcmp(raw, "null")))
    return 0;
  v = js_num(p, f, chave, NAN);
  return isfinite(v) && v >= 0 && v < 100000 && (int)v == v ? (int)v : -1;
}
static long long instanteDe(const char *p, const char *f, const char *const *chaves, int n) {
  for (int i = 0; i < n; i++) {
    char t[48];
    if (js_texto(p, f, chaves[i], t, sizeof t) && t[0]) {
      long long ms = js_ms_iso(t);
      if (ms > 0) return ms;
    }
    double v = js_num(p, f, chaves[i], -1.0);
    if (v > 0) return (long long)v;
  }
  return 0;
}
static void tipoDe(const char *p, const char *f, int ep, char *dst, size_t tam) {
  char t[24];
  int serie = ep > 0;
  if (js_texto(p, f, "content_type", t, sizeof t))
    serie = !strcmp(t, "series") || !strcmp(t, "tv") || !strcmp(t, "show") || !strcmp(t, "anime");
  snprintf(dst, tam, "%s", serie ? "series" : "movie");
}
static int validoArray(const char *json) {
  const char *p = json;
  if (!p) return 0;
  while (*p && (unsigned char)*p <= 32) p++;
  return *p == '[';
}

int familia_ler_progresso(const char *json, FamAtividade *out, int max) {
  static const char *const quando[] = { "last_watched", "last_watched_at", "updated_at" };
  const char *p;
  int k = 0;
  if (!validoArray(json) || !out) return -1;
  for (p = js_raiz_array(json); p && k < max; p = js_prox(js_fim(p))) {
    const char *f = js_fim(p);
    FamAtividade *d = &out[k];
    double pos, dur;
    memset(d, 0, sizeof *d);
    if (!js_texto(p, f, "content_id", d->id, sizeof d->id) || !d->id[0]) continue;
    // position_ms/duration_ms win: the plain fields are already ms in this RPC.
    pos = js_num(p, f, "position_ms", NAN); if (!isfinite(pos)) pos = js_num(p, f, "position", NAN);
    dur = js_num(p, f, "duration_ms", NAN); if (!isfinite(dur)) dur = js_num(p, f, "duration", NAN);
    if (!isfinite(pos) || !isfinite(dur) || pos < 0 || dur <= 0) continue;
    d->temporada = coord(p, f, "season"); d->episodio = coord(p, f, "episode");
    if (d->temporada < 0 || d->episodio < 0) continue;
    tipoDe(p, f, d->episodio, d->tipo, sizeof d->tipo);
    d->posMs = pos; d->durMs = dur;
    d->quandoMs = instanteDe(p, f, quando, 3);
    k++;
  }
  return k;
}

int familia_ler_vistos(const char *json, FamAtividade *out, int max) {
  static const char *const quando[] = { "watched_at", "updated_at" };
  const char *p;
  int k = 0;
  if (!validoArray(json) || !out) return -1;
  for (p = js_raiz_array(json); p && k < max; p = js_prox(js_fim(p))) {
    const char *f = js_fim(p);
    FamAtividade *d = &out[k];
    memset(d, 0, sizeof *d);
    if (!js_texto(p, f, "content_id", d->id, sizeof d->id) || !d->id[0]) continue;
    d->temporada = coord(p, f, "season"); d->episodio = coord(p, f, "episode");
    if (d->temporada < 0 || d->episodio < 0) continue;
    tipoDe(p, f, d->episodio, d->tipo, sizeof d->tipo);
    d->visto = 1;
    d->quandoMs = instanteDe(p, f, quando, 2);
    k++;
  }
  return k;
}

// ---- labels -----------------------------------------------------------------
void familia_rotulos(const FamCartao *c, long long agoraMs, char *pilula, size_t tp,
                     char *linha, size_t tl) {
  char cab[48], cauda[48];
  if (pilula && tp) pilula[0] = 0;
  if (linha && tl) linha[0] = 0;
  if (!c) return;
  if (pilula && tp)
    snprintf(pilula, tp, "%s", i18n(c->estado == FAM_EM_CURSO ? "Em andamento" : "Assistido recentemente"));
  if (!strcmp(c->tipo, "series") && c->temporada > 0 && c->episodio > 0)
    snprintf(cab, sizeof cab, i18n("T%d E%d"), c->temporada, c->episodio);
  else if (!strcmp(c->tipo, "series")) cab[0] = 0;
  else snprintf(cab, sizeof cab, "%s", i18n("Filme"));
  cauda[0] = 0;
  if (c->estado == FAM_EM_CURSO && c->restanteMin > 0) {
    if (c->restanteMin >= 60)
      snprintf(cauda, sizeof cauda, i18n("%dh %dm restantes"), c->restanteMin / 60, c->restanteMin % 60);
    else snprintf(cauda, sizeof cauda, i18n("%dm restantes"), c->restanteMin);
  } else if (c->estado == FAM_VISTO && c->quandoMs > 0 && agoraMs >= c->quandoMs) {
    long long min = (agoraMs - c->quandoMs) / 60000;
    if (min < 1) snprintf(cauda, sizeof cauda, "%s", i18n("Agora"));
    else if (min < 60) snprintf(cauda, sizeof cauda, i18n("há %d min"), (int)min);
    else if (min < 48 * 60) snprintf(cauda, sizeof cauda, i18n("há %d h"), (int)(min / 60));
    else snprintf(cauda, sizeof cauda, i18n("há %d dias"), (int)(min / (24 * 60)));
  }
  if (linha && tl)
    snprintf(linha, tl, "%s%s%s", cab, cab[0] && cauda[0] ? " · " : "", cauda);
}

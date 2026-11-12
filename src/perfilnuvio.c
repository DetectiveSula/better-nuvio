// Painel de 30 dias do perfil a partir de Nuvio sync. Regras em perfilnuvio.h.
#include "perfilnuvio.h"
#include "js.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

typedef struct {
  const char *id, *tipo;
  int filme, temporada, episodio;
  long long fimMs, durMs;
  int estimado, parcial, lote;
  int agg;                    // indice na agregacao por obra
} Ev;

typedef struct {
  const char *id, *tipo;
  int filme;
  long long ms;               // minutos na janela, em ms
  int episodios;
} Agg;

typedef struct { long long mid[PERFIL_DIAS + 1]; long long agora; } Janela;

static int ehFilme(const char *tipo) { return tipo && !strcmp(tipo, "movie"); }

static long long meiaNoite(long long ms, int somaDias) {
  time_t t = (time_t)(ms / 1000);
  struct tm tm;
  localtime_r(&t, &tm);
  tm.tm_mday += somaDias;
  tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
  tm.tm_isdst = -1;
  return (long long)mktime(&tm) * 1000;
}

// mid[i] e a meia-noite local do dia i; mid[PERFIL_DIAS] a de amanha. Cada
// dia vem do mktime do proprio dia, entao a semana do horario de verao tem um
// dia de 23 ou 25 h e nenhum indice escorrega.
static void janelaMontar(Janela *j, long long agora) {
  for (int i = 0; i <= PERFIL_DIAS; i++) j->mid[i] = meiaNoite(agora, i - (PERFIL_DIAS - 1));
  j->agora = agora;
}

static int diaDe(const Janela *j, long long ms) {
  if (ms < j->mid[0] || ms > j->agora) return -1;
  for (int i = PERFIL_DIAS - 1; i >= 0; i--) if (ms >= j->mid[i]) return i;
  return -1;
}

static int mesmoItem(const char *id, int filme, int t, int e,
                     const char *id2, int filme2, int t2, int e2) {
  if (strcmp(id, id2) || filme != filme2) return 0;
  return filme || (t == t2 && e == e2);
}

static const ProgRegistro *progDe(const ProgRegistro *r, int nr, int perfil,
                                  const char *id, int filme, int t, int e) {
  for (int i = 0; i < nr; i++)
    if (r[i].perfil == perfil &&
        mesmoItem(id, filme, t, e, r[i].contentId, ehFilme(r[i].tipo), r[i].temporada, r[i].episodio))
      return &r[i];
  return NULL;
}

static int vistoExiste(const ContaVisto *v, int nv, const char *id, int filme, int t, int e) {
  for (int i = 0; i < nv; i++)
    if (v[i].vistoMs > 0 && mesmoItem(id, filme, t, e, v[i].id, ehFilme(v[i].tipo), v[i].temporada, v[i].episodio))
      return 1;
  return 0;
}

static const PerfilMeta *metaDe(const PerfilMeta *m, int nm, const char *id) {
  for (int i = 0; i < nm; i++) if (!strcmp(m[i].id, id)) return &m[i];
  return NULL;
}

static long long duracaoMs(const char *id, int filme, int t, int e,
                           const ProgRegistro *r, int nr, int perfil,
                           const PerfilMeta *m, int nm, int *estimado) {
  const ProgRegistro *p = progDe(r, nr, perfil, id, filme, t, e);
  // durSeg < 60 e o marcador de 1 ms do Android para "duracao desconhecida".
  if (p && p->durSeg >= 60) { *estimado = 0; return (long long)(p->durSeg * 1000.0); }
  const PerfilMeta *mm = metaDe(m, nm, id);
  if (mm && mm->duracaoMin > 0) { *estimado = 0; return mm->duracaoMin * 60000LL; }
  *estimado = 1;
  return (filme ? PN_FILME_PADRAO_MIN : PN_EP_PADRAO_MIN) * 60000LL;
}

static int cmpLote(const void *a, const void *b) {
  const Ev *x = *(const Ev *const *)a, *y = *(const Ev *const *)b;
  int c = strcmp(x->id, y->id);
  if (c) return c;
  return x->fimMs < y->fimMs ? -1 : x->fimMs > y->fimMs;
}

static int cmpFim(const void *a, const void *b) {
  const Ev *x = *(const Ev *const *)a, *y = *(const Ev *const *)b;
  return x->fimMs < y->fimMs ? -1 : x->fimMs > y->fimMs;
}

// Os itens da janela: terminados (vistos) e parciais (progresso sem visto).
// Marca o lote; `*marcados` recebe quantos lotes caem na janela.
static Ev *coletar(const ContaVisto *v, int nv, const ProgRegistro *r, int nr, int perfil,
                   const PerfilMeta *m, int nm, const Janela *j, int *nEv, int *marcados) {
  Ev *ev = calloc((size_t)(nv + nr + 1), sizeof *ev);
  Ev **ord = calloc((size_t)(nv + 1), sizeof *ord);
  int n = 0, ns = 0;
  *nEv = 0; if (marcados) *marcados = 0;
  if (!ev || !ord) { free(ev); free(ord); return NULL; }
  // Um dia de folga antes da janela: o lote que comeca na vespera ainda conta
  // como lote para o pedaco que cai dentro.
  long long desde = j->mid[0] - 86400000LL;
  for (int i = 0; i < nv; i++) {
    const ContaVisto *x = &v[i];
    int filme = ehFilme(x->tipo);
    if (x->vistoMs <= 0 || x->vistoMs < desde || x->vistoMs > j->agora) continue;
    if (!filme && x->episodio <= 0) continue;      // linha da serie inteira
    Ev *e = &ev[n++];
    e->id = x->id; e->tipo = x->tipo; e->filme = filme;
    e->temporada = x->temporada; e->episodio = x->episodio; e->fimMs = x->vistoMs;
    e->durMs = duracaoMs(x->id, filme, x->temporada, x->episodio, r, nr, perfil, m, nm, &e->estimado);
    if (!filme) ord[ns++] = e;
  }
  qsort(ord, (size_t)ns, sizeof *ord, cmpLote);
  for (int i = 0; i < ns;) {
    int k = i + 1;
    while (k < ns && !strcmp(ord[k]->id, ord[i]->id) &&
           ord[k]->fimMs - ord[k - 1]->fimMs <= PN_LOTE_JANELA_MS) k++;
    if (k - i >= PN_LOTE_MIN) for (int q = i; q < k; q++) ord[q]->lote = 1;
    i = k;
  }
  free(ord);
  // Fora da janela (a folga do lote) sai agora; o lote da janela vira contagem.
  int w = 0;
  for (int i = 0; i < n; i++) {
    if (diaDe(j, ev[i].fimMs) < 0) continue;
    if (ev[i].lote) { if (marcados) (*marcados)++; continue; }
    ev[w++] = ev[i];
  }
  n = w;
  for (int i = 0; i < nr; i++) {
    const ProgRegistro *p = &r[i];
    int filme = ehFilme(p->tipo);
    if (p->perfil != perfil || p->posSeg <= 0 || diaDe(j, p->lastWatchedMs) < 0) continue;
    if (!p->contentId[0] || (!filme && p->episodio <= 0)) continue;
    if (vistoExiste(v, nv, p->contentId, filme, p->temporada, p->episodio)) continue;
    Ev *e = &ev[n++];
    e->id = p->contentId; e->tipo = p->tipo; e->filme = filme;
    e->temporada = p->temporada; e->episodio = p->episodio; e->fimMs = p->lastWatchedMs;
    if (p->durSeg >= 60 && p->posSeg / p->durSeg >= PN_CONCLUIDO) {
      e->durMs = (long long)(p->durSeg * 1000.0);     // terminou, sem visto
    } else {
      double pos = p->durSeg >= 60 && p->posSeg > p->durSeg ? p->durSeg : p->posSeg;
      e->durMs = (long long)(pos * 1000.0);
      if (e->durMs < 1000) e->durMs = 1000;
      e->parcial = 1;
    }
  }
  *nEv = n;
  return ev;
}

static int aggDe(Agg *a, int *na, const Ev *e) {
  for (int i = 0; i < *na; i++) if (!strcmp(a[i].id, e->id)) return i;   // obra = content_id
  Agg *x = &a[(*na)++];
  memset(x, 0, sizeof *x);
  x->id = e->id; x->tipo = e->tipo; x->filme = e->filme;
  return *na - 1;
}

static int cmpAggMs(const void *a, const void *b) {
  const Agg *x = a, *y = b;
  if (x->ms != y->ms) return x->ms > y->ms ? -1 : 1;
  return strcmp(x->id, y->id);
}

static int minutosDe(long long ms) { return (int)((ms + 30000) / 60000); }

typedef struct {
  long long dia[PERFIL_DIAS], filmeDia[PERFIL_DIAS], hora[24], semana[7], total;
} Somas;

// Espalha [ini, fim] pelas horas locais, cortado no inicio da janela.
static void espalhar(Somas *s, const Janela *j, long long ini, long long fim, int filme) {
  long long t = ini < j->mid[0] ? j->mid[0] : ini;
  while (t < fim) {
    time_t tt = (time_t)(t / 1000);
    struct tm tm;
    localtime_r(&tt, &tm);
    long long prox = t - (t % 1000) + ((59 - tm.tm_min) * 60 + (60 - tm.tm_sec)) * 1000LL;
    if (prox <= t) prox = t + 1000;
    if (prox > fim) prox = fim;
    long long q = prox - t;
    int di = diaDe(j, t);
    if (di >= 0) { s->dia[di] += q; if (filme) s->filmeDia[di] += q; }
    s->hora[tm.tm_hour] += q;
    s->semana[tm.tm_wday] += q;
    s->total += q;
    t = prox;
  }
}

static int ehLoteAntigo(const ContaVisto *v, int nv, const ContaVisto *x) {
  int perto = 0;
  if (ehFilme(x->tipo)) return 0;
  for (int i = 0; i < nv; i++) {
    const ContaVisto *y = &v[i];
    if (y == x || strcmp(y->id, x->id) || ehFilme(y->tipo)) continue;
    long long dt = y->vistoMs - x->vistoMs;
    if (dt < 0) dt = -dt;
    if (dt <= PN_LOTE_JANELA_MS) perto++;
  }
  return perto + 1 >= PN_LOTE_MIN;
}

// "Neste dia": o visto mais novo com o mesmo dia e mes locais em ano anterior.
static const ContaVisto *nesteDia(const ContaVisto *v, int nv, long long agora) {
  time_t ta = (time_t)(agora / 1000);
  struct tm hoje;
  const ContaVisto *melhor = NULL;
  localtime_r(&ta, &hoje);
  for (int i = 0; i < nv; i++) {
    const ContaVisto *x = &v[i];
    if (x->vistoMs <= 0 || (!ehFilme(x->tipo) && x->episodio <= 0)) continue;
    time_t tx = (time_t)(x->vistoMs / 1000);
    struct tm tm;
    localtime_r(&tx, &tm);
    if (tm.tm_mon != hoje.tm_mon || tm.tm_mday != hoje.tm_mday || tm.tm_year >= hoje.tm_year) continue;
    if (melhor && x->vistoMs <= melhor->vistoMs) continue;
    if (ehLoteAntigo(v, nv, x)) continue;
    melhor = x;
  }
  return melhor;
}

static int obraAdicionar(PerfilDados *d, const char *id, const char *tipo,
                         const PerfilMeta *m, int nm) {
  for (int i = 0; i < d->nObras; i++) if (!strcmp(d->obras[i].id, id)) return i;
  if (d->nObras >= PERFIL_MAX_OBRAS) return -1;
  PerfilObra *o = &d->obras[d->nObras];
  snprintf(o->id, sizeof o->id, "%s", id);
  snprintf(o->tipo, sizeof o->tipo, "%s", tipo ? tipo : "");
  const PerfilMeta *mm = metaDe(m, nm, id);
  if (mm) {
    snprintf(o->titulo, sizeof o->titulo, "%s", mm->titulo);
    snprintf(o->poster, sizeof o->poster, "%s", mm->poster);
  }
  if (!o->poster[0] && !strncmp(id, "tt", 2))
    snprintf(o->poster, sizeof o->poster, "https://images.metahub.space/poster/medium/%s/img", id);
  return d->nObras++;
}

// Rotulos de TIPO que catalogos misturam nos generos ("TV Show" veio do meta
// do Nuvio na conta de teste, 07/11): nao sao genero, nao entram no mix.
static int ehRotuloDeTipo(const char *g) {
  static const char *const tipos[] = { "Filme", "Filmes", "Movie", "Movies", "Film", "Série", "Séries",
                                       "Serie", "Series", "TV", "TV Show", "TV Shows", "TV Series",
                                       "Show", "Shows", "Série de TV", "Anime", NULL };
  for (int i = 0; tipos[i]; i++) if (!strcasecmp(g, tipos[i])) return 1;
  return 0;
}

#define PN_GEN_MAX 32
typedef struct { char nome[40]; long long ms; } GenSoma;

static void generosSomar(GenSoma *g, int *ng, const char *lista, long long ms) {
  const char *p = lista;
  while (p && *p) {
    char nome[40];
    size_t k = 0;
    while (*p == ' ' || *p == ',' || *p == '|' || *p == '/') p++;
    if (!strncmp(p, "\xC2\xB7", 2)) { p += 2; continue; }   // "·"
    while (*p && *p != ',' && *p != '|' && *p != '/' && strncmp(p, "\xC2\xB7", 2)) {
      if (k + 1 < sizeof nome) nome[k++] = *p;
      p++;
    }
    while (k && nome[k - 1] == ' ') k--;
    nome[k] = 0;
    if (!k || ehRotuloDeTipo(nome)) continue;
    int i;
    for (i = 0; i < *ng; i++) if (!strcmp(g[i].nome, nome)) break;
    if (i == *ng) { if (*ng >= PN_GEN_MAX) continue; snprintf(g[i].nome, sizeof g[i].nome, "%s", nome); g[i].ms = 0; (*ng)++; }
    g[i].ms += ms;
  }
}

static int cmpGen(const void *a, const void *b) {
  const GenSoma *x = a, *y = b;
  if (x->ms != y->ms) return x->ms > y->ms ? -1 : 1;
  return strcmp(x->nome, y->nome);
}

int perfilnuvio_montar(const ContaVisto *v, int nv,
                       const ProgRegistro *r, int nr, int perfil,
                       const PerfilMeta *m, int nm,
                       long long agoraMs, PerfilDados *d) {
  Janela j;
  Somas s;
  int nEv = 0, na = 0;
  long long estMs = 0;
  if (!d) return 0;
  memset(d, 0, sizeof *d);
  if (nv < 0 || !v) nv = 0;
  if (nr < 0 || !r) nr = 0;
  if (nm < 0 || !m) nm = 0;
  memset(&s, 0, sizeof s);
  janelaMontar(&j, agoraMs);
  d->janelaInicioMs = j.mid[0];
  d->maratona.obra = -1;
  d->hojeAnos.obra = -1;
  d->diaMaisAtivo = -1;
  d->cobertura = PF_COB_NUVIO;
  d->nDias = PERFIL_DIAS;
  d->streakCompleto = 1;
  snprintf(d->periodo, sizeof d->periodo, "%s", "Últimos 30 dias");
  { time_t t0 = (time_t)(j.mid[0] / 1000); struct tm tm; localtime_r(&t0, &tm); d->primeiroDiaSemana = tm.tm_wday; }

  Ev *ev = coletar(v, nv, r, nr, perfil, m, nm, &j, &nEv, &d->marcados);
  Agg *agg = calloc((size_t)nEv + 1, sizeof *agg);
  if (!ev || !agg) { free(ev); free(agg); return 0; }

  // Somas, contagens e agregacao por obra.
  for (int i = 0; i < nEv; i++) {
    Ev *e = &ev[i];
    long long antes = s.total;
    espalhar(&s, &j, e->fimMs - e->durMs, e->fimMs, e->filme);
    long long dentro = s.total - antes;
    if (e->estimado) estMs += dentro;
    e->agg = aggDe(agg, &na, e);
    agg[e->agg].ms += dentro;
    if (!e->parcial) {
      if (e->filme) d->filmes++;
      else { d->episodios++; agg[e->agg].episodios++; }
    }
    int di = diaDe(&j, e->fimMs);
    if (di >= 0 && d->atividade[di] < 0xFFFF) d->atividade[di]++;
  }
  d->plays = d->filmes + d->episodios;
  d->titulos = na;
  d->minutos = minutosDe(s.total);
  {
    long long filmeMs = 0;
    for (int i = 0; i < PERFIL_DIAS; i++) filmeMs += s.filmeDia[i];
    d->minutosFilmes = minutosDe(filmeMs);
    d->minutosSeries = d->minutos - d->minutosFilmes;
  }
  for (int i = 0; i < PERFIL_DIAS; i++) {
    int md = minutosDe(s.dia[i]), mf = minutosDe(s.filmeDia[i]);
    d->minutosDia[i] = (unsigned short)(md > 0xFFFF ? 0xFFFF : md);
    d->minutosFilmeDia[i] = (unsigned short)(mf > 0xFFFF ? 0xFFFF : mf);
    if (d->minutosDia[i] || d->atividade[i]) d->diasAtivosMes++;
  }
  for (int h = 0; h < 24; h++) d->minutosHora[h] = minutosDe(s.hora[h]);
  {
    long long mx = 0;
    for (int w = 0; w < 7; w++) {
      d->minutosSemana[w] = minutosDe(s.semana[w]);
      if (s.semana[w] > mx) { mx = s.semana[w]; d->diaMaisAtivo = w; }
    }
  }
  if (s.total > 0 && estMs * 5 > s.total) d->cobertura |= PF_COB_ESTIMADO;

  // Sequencia: hoje conta; se hoje ainda nao teve nada, conta a partir de ontem.
  {
    int i = PERFIL_DIAS - 1;
    if (!d->minutosDia[i] && !d->atividade[i]) i--;
    for (; i >= 0 && (d->minutosDia[i] || d->atividade[i]); i--) d->streakAtual++;
  }

  // Maratona: episodios terminados da mesma serie, vao < 30 min entre o fim
  // de um e o inicio do proximo, nada de outra obra no meio.
  {
    Ev **ord = calloc((size_t)nEv + 1, sizeof *ord);
    int no = 0;
    if (ord) {
      for (int i = 0; i < nEv; i++) if (!ev[i].filme && !ev[i].parcial) ord[no++] = &ev[i];
      qsort(ord, (size_t)no, sizeof *ord, cmpFim);
      int melhorIni = -1, melhorN = 0;
      long long melhorMs = 0;
      for (int i = 0; i < no;) {
        int k = i + 1;
        long long ms = ord[i]->durMs;
        while (k < no && !strcmp(ord[k]->id, ord[i]->id) &&
               (ord[k]->fimMs - ord[k]->durMs) - ord[k - 1]->fimMs <= PN_MARATONA_VAO_MS) {
          ms += ord[k]->durMs; k++;
        }
        int n = k - i;
        if (n >= 2 && (n > melhorN || (n == melhorN && ms > melhorMs))) {
          melhorIni = i; melhorN = n; melhorMs = ms;
        }
        i = k;
      }
      if (melhorIni >= 0) {
        const Ev *a = ord[melhorIni], *b = ord[melhorIni + melhorN - 1];
        d->maratona.episodios = melhorN;
        d->maratona.minutos = minutosDe(melhorMs);
        d->maratona.temporada = a->temporada;
        d->maratona.epIni = a->episodio;
        d->maratona.epFim = b->episodio;
        d->maratona.inicioMs = a->fimMs - a->durMs;
        d->maratona.fimMs = b->fimMs;
        d->maratona.obra = a->agg;          // provisorio: indice em agg
      }
      free(ord);
    }
  }

  // Obras: as de mais minutos primeiro; maratona e "neste dia" garantem vaga.
  const ContaVisto *hoje = nesteDia(v, nv, agoraMs);
  int *aggObra = malloc(sizeof(int) * ((size_t)na + 1));
  int *ordem = malloc(sizeof(int) * ((size_t)na + 1));
  if (!aggObra || !ordem) { free(aggObra); free(ordem); free(ev); free(agg); return nEv; }
  {
    Agg *cp = malloc(sizeof *cp * ((size_t)na + 1));
    if (cp) {
      memcpy(cp, agg, sizeof *cp * (size_t)na);
      qsort(cp, (size_t)na, sizeof *cp, cmpAggMs);
      for (int i = 0; i < na; i++)
        for (int q = 0; q < na; q++)
          if (agg[q].id == cp[i].id) { ordem[i] = q; break; }
      free(cp);
    } else for (int i = 0; i < na; i++) ordem[i] = i;
  }
  for (int i = 0; i < na; i++) aggObra[i] = -1;
  {
    int reserva = (d->maratona.obra >= 0) + (hoje != NULL);
    for (int i = 0; i < na && d->nObras < PERFIL_MAX_OBRAS - reserva; i++) {
      const Agg *a = &agg[ordem[i]];
      int o = obraAdicionar(d, a->id, a->tipo, m, nm);
      if (o < 0) break;
      aggObra[ordem[i]] = o;
      d->obras[o].minutos = minutosDe(a->ms);
      d->obras[o].episodios = a->episodios;
      if (d->nTopo < PERFIL_MAX_TOPO && a->ms > 0) d->topo[d->nTopo++] = o;
    }
    if (d->maratona.obra >= 0) {
      int ai = d->maratona.obra;
      if (aggObra[ai] < 0) {
        aggObra[ai] = obraAdicionar(d, agg[ai].id, agg[ai].tipo, m, nm);
        if (aggObra[ai] >= 0) {
          d->obras[aggObra[ai]].minutos = minutosDe(agg[ai].ms);
          d->obras[aggObra[ai]].episodios = agg[ai].episodios;
        }
      }
      d->maratona.obra = aggObra[ai];
    }
    if (hoje) {
      d->hojeAnos.obra = obraAdicionar(d, hoje->id, hoje->tipo, m, nm);
      d->hojeAnos.ms = hoje->vistoMs;
    }
  }

  // Itens de cada dia: por obra, os 3 de mais minutos (o item mora no dia em
  // que terminou).
  for (int di = 0; di < PERFIL_DIAS; di++) {
    PerfilDiaItem it[PERFIL_ITENS_DIA + 1];
    int n = 0;
    // Agregacao do dia num vetor pequeno, por obra, mantendo os maiores.
    struct { int agg; long long ms; int t, eIni, eFim; long long ini; } tmp[64];
    int nt = 0;
    for (int i = 0; i < nEv; i++) {
      const Ev *e = &ev[i];
      if (diaDe(&j, e->fimMs) != di) continue;
      int q;
      for (q = 0; q < nt; q++) if (tmp[q].agg == e->agg) break;
      if (q == nt) {
        if (nt >= 64) continue;
        tmp[q].agg = e->agg; tmp[q].ms = 0; tmp[q].t = e->temporada;
        tmp[q].eIni = tmp[q].eFim = e->episodio; tmp[q].ini = e->fimMs - e->durMs; nt++;
      }
      tmp[q].ms += e->durMs;
      if (!e->filme) {
        if (e->episodio < tmp[q].eIni || !tmp[q].eIni) tmp[q].eIni = e->episodio;
        if (e->episodio > tmp[q].eFim) tmp[q].eFim = e->episodio;
      }
      if (e->fimMs - e->durMs < tmp[q].ini) tmp[q].ini = e->fimMs - e->durMs;
    }
    for (int k = 0; k < PERFIL_ITENS_DIA; k++) {
      int best = -1;
      for (int q = 0; q < nt; q++) if (tmp[q].ms > 0 && (best < 0 || tmp[q].ms > tmp[best].ms)) best = q;
      if (best < 0) break;
      memset(&it[n], 0, sizeof it[n]);
      it[n].obra = aggObra[tmp[best].agg];
      it[n].temporada = agg[tmp[best].agg].filme ? 0 : tmp[best].t;
      it[n].epIni = agg[tmp[best].agg].filme ? 0 : tmp[best].eIni;
      it[n].epFim = agg[tmp[best].agg].filme ? 0 : tmp[best].eFim;
      it[n].minutos = minutosDe(tmp[best].ms);
      it[n].inicioMs = tmp[best].ini;
      tmp[best].ms = 0;
      n++;
    }
    d->nDiaItens[di] = (unsigned char)n;
    memcpy(d->diaItens[di], it, sizeof it[0] * (size_t)n);
  }

  // Completude: linhas de progresso tocadas na janela.
  for (int i = 0; i < nr; i++) {
    const ProgRegistro *p = &r[i];
    int filme = ehFilme(p->tipo);
    if (p->perfil != perfil || p->posSeg <= 0 || diaDe(&j, p->lastWatchedMs) < 0) continue;
    d->iniciadas++;
    if ((p->durSeg >= 60 && p->posSeg / p->durSeg >= PN_CONCLUIDO) ||
        vistoExiste(v, nv, p->contentId, filme, p->temporada, p->episodio))
      d->concluidas++;
  }

  // Generos ponderados por minutos (biblioteca ou meta, o que o chamador deu).
  {
    GenSoma g[PN_GEN_MAX];
    int ng = 0;
    for (int i = 0; i < na; i++) {
      const PerfilMeta *mm = metaDe(m, nm, agg[i].id);
      if (mm && mm->genero[0] && agg[i].ms > 0) generosSomar(g, &ng, mm->genero, agg[i].ms);
    }
    qsort(g, (size_t)ng, sizeof *g, cmpGen);
    for (int i = 0; i < ng && d->nGeneros < PERFIL_MAX_GENEROS; i++) {
      PerfilGenero *pg = &d->generos[d->nGeneros++];
      snprintf(pg->nome, sizeof pg->nome, "%s", g[i].nome);
      pg->quantidade = minutosDe(g[i].ms);
    }
    if (d->nGeneros) d->cobertura |= PF_COB_GENEROS;
  }

  // Compatibilidade: os quatro destaques da tela atual saem do topo.
  for (int i = 0; i < d->nTopo && d->nDestaques < PERFIL_MAX_DESTAQUES; i++) {
    const PerfilObra *o = &d->obras[d->topo[i]];
    PerfilDestaque *h = &d->destaques[d->nDestaques++];
    snprintf(h->id, sizeof h->id, "%s", o->id);
    snprintf(h->titulo, sizeof h->titulo, "%s", o->titulo[0] ? o->titulo : o->id);
    snprintf(h->detalhe, sizeof h->detalhe, "%s", ehFilme(o->tipo) ? "Filme" : "Série");
    snprintf(h->poster, sizeof h->poster, "%s", o->poster);
    if (!strncmp(o->id, "tt", 2))
      snprintf(h->backdrop, sizeof h->backdrop, "https://images.metahub.space/background/medium/%s/img", o->id);
    h->plays = o->episodios > 0 ? o->episodios : 1;
    h->minutos = o->minutos;
  }

  free(aggObra); free(ordem); free(ev); free(agg);
  return nEv;
}

int perfilnuvio_ids_meta(const ContaVisto *v, int nv,
                         const ProgRegistro *r, int nr, int perfil,
                         long long agoraMs,
                         char ids[][24], char tipos[][8], int max) {
  Janela j;
  int nEv = 0, na = 0, n = 0;
  if (max <= 0 || !ids || !tipos) return 0;
  if (nv < 0 || !v) nv = 0;
  if (nr < 0 || !r) nr = 0;
  janelaMontar(&j, agoraMs);
  Ev *ev = coletar(v, nv, r, nr, perfil, NULL, 0, &j, &nEv, NULL);
  Agg *agg = calloc((size_t)nEv + 1, sizeof *agg);
  if (!ev || !agg) { free(ev); free(agg); return 0; }
  for (int i = 0; i < nEv; i++) { int a = aggDe(agg, &na, &ev[i]); agg[a].ms += ev[i].durMs; }
  qsort(agg, (size_t)na, sizeof *agg, cmpAggMs);
  const ContaVisto *hoje = nesteDia(v, nv, agoraMs);
  int lim = hoje ? max - 1 : max;
  for (int i = 0; i < na && n < lim; i++) {
    snprintf(ids[n], 24, "%s", agg[i].id);
    snprintf(tipos[n], 8, "%s", agg[i].tipo ? agg[i].tipo : "");
    n++;
  }
  if (hoje && n < max) {
    int ja = 0;
    for (int i = 0; i < n; i++) if (!strcmp(ids[i], hoje->id)) ja = 1;
    if (!ja) {
      snprintf(ids[n], 24, "%s", hoje->id);
      snprintf(tipos[n], 8, "%s", hoje->tipo);
      n++;
    }
  }
  free(ev); free(agg);
  return n;
}

// "45 min" -> 45, "1h 50min" -> 110, "2h" -> 120, "95" -> 95.
static int runtimeMin(const char *s) {
  int h = 0, m = 0, n = 0, tem = 0;
  for (const char *p = s; *p; p++) {
    if (isdigit((unsigned char)*p)) { n = n * 10 + (*p - '0'); tem = 1; continue; }
    if (tem && (*p == 'h' || *p == 'H')) { h = n; n = 0; tem = 0; }
    else if (tem && (*p == 'm' || *p == 'M')) { m = n; n = 0; tem = 0; }
  }
  if (tem) m += n;
  return h * 60 + m;
}

int perfilnuvio_meta_ler(const char *corpo, PerfilMeta *m) {
  const char *o, *fim;
  char s[64];
  if (!corpo || !m || !(o = strstr(corpo, "\"meta\"")) || !(o = strchr(o, '{'))) return 0;
  fim = js_fim(o);
  if (!fim) return 0;
  if (!m->titulo[0]) js_texto_raiz_em(o, fim, "name", m->titulo, sizeof m->titulo);
  if (!m->poster[0]) js_texto_raiz_em(o, fim, "poster", m->poster, sizeof m->poster);
  if (!m->genero[0]) {
    const char *g = js_array(o, fim, "genres");
    size_t n = 0;
    while (g && g < fim && *g == '"') {
      const char *q = g + 1;
      if (n && n + 2 < sizeof m->genero) { m->genero[n++] = ','; m->genero[n++] = ' '; }
      while (*q && *q != '"' && n + 1 < sizeof m->genero) m->genero[n++] = *q++;
      while (*q && *q != '"') q++;
      if (*q) q++;
      while (*q == ' ' || *q == ',') q++;
      g = q;
    }
    m->genero[n] = 0;
  }
  if (m->duracaoMin <= 0) {
    s[0] = 0;
    if (js_texto_raiz_em(o, fim, "runtime", s, sizeof s) && s[0]) m->duracaoMin = runtimeMin(s);
    else m->duracaoMin = (int)js_num(o, fim, "runtime", 0.0);
  }
  return m->titulo[0] != 0;
}

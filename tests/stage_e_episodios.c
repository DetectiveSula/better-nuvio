/* Stage E: regular Nuvio's episode release rules, Continue Watching badge,
 * episode line and release alert (epdata.c, cwordem.c, continuar.c).
 * Frozen clocks and explicit time zones; no UI, network or sync. */
#include "../src/ajustes.c"
#include "epdata.h"
#include "cwordem.h"
#include "continuar.h"
#include "descoberta.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void fuso(const char *tz) { setenv("TZ", tz, 1); tzset(); }
static long long ms(const char *iso) {
  long long v = 0;
  assert(ep_instante_ms(iso, &v));
  return v;
}
static void dia(const char *raw, int a, int m, int d) {
  EpDia x;
  assert(ep_dia_local(raw, &x));
  if (x.ano != a || x.mes != m || x.dia != d) {
    fprintf(stderr, "FAIL dia %s -> %d-%d-%d (want %d-%d-%d)\n", raw, x.ano, x.mes, x.dia, a, m, d);
    exit(1);
  }
}
static void rot(const char *raw, long long agora, int curto, const char *esperado) {
  char b[96];
  ep_rotulo_estreia(raw, agora, curto, IDIOMA_EN, b, sizeof b);
  if (strcmp(b, esperado)) {
    fprintf(stderr, "FAIL rotulo %s curto=%d -> \"%s\" (want \"%s\")\n", raw, curto, b, esperado);
    exit(1);
  }
}
static void selo(const CatItem *ci, int curto, long long agora, const char *esperado, BadgeEstilo est) {
  char b[96];
  BadgeEstilo e = BADGE_APAGADO;
  continuar_selo(ci, curto, agora, b, sizeof b, &e);
  if (strcmp(b, esperado) || e != est) {
    fprintf(stderr, "FAIL selo %s curto=%d -> \"%s\"/%d (want \"%s\"/%d)\n",
            ci->imdb, curto, b, (int)e, esperado, (int)est);
    exit(1);
  }
}

int main(void) {
  const long long DIA = 86400000LL;
  valor[AJ_IDIOMA] = IDIOMA_EN + 1;

  /* 1. parseEpisodeReleaseLocalDate */
  fuso("Europe/London");
  dia("2026-11-20", 2026, 11, 20);                       /* plain date as supplied */
  dia("  2026-11-20  ", 2026, 11, 20);                   /* trimmed */
  dia("2026-11-20T23:30:00Z", 2026, 11, 20);             /* GMT in November */
  dia("2026-11-20T23:30:00-05:00", 2026, 11, 21);        /* instant -> local date */
  dia("2026-11-20T23:30:00.000+00:00[Europe/London]", 2026, 11, 20);
  dia("2026-11-20T10:00", 2026, 11, 20);                 /* local date-time */
  dia("Released 2026-11-20 worldwide", 2026, 11, 20);    /* embedded */
  { EpDia x;
    assert(!ep_dia_local("", &x) && !ep_dia_local(NULL, &x) && !ep_dia_local("TBA", &x));
    assert(!ep_dia_local("12026-11-20", &x));             /* digit before the match */
    assert(!ep_dia_local("2026-13-01 or 2026-11-20", &x)); /* invalid first match is final */
    assert(!ep_dia_local("2026-02-30", &x)); }
  fuso("Asia/Tokyo");
  dia("2026-11-20T23:30:00Z", 2026, 11, 21);             /* same instant, later local day */
  dia("2026-11-20", 2026, 11, 20);                       /* plain date never shifts */

  /* 2. isEpisodeReleaseAired: plain date = UTC midnight, instants exact */
  fuso("America/New_York");
  { long long m = ms("2026-11-20T00:00:00Z");
    assert(ep_ja_saiu("2026-11-20", m - 1) == 0 && ep_ja_saiu("2026-11-20", m) == 1);
    assert(ep_ja_saiu("2026-11-20T15:00:00+01:00", ms("2026-11-20T13:59:59Z")) == 0);
    assert(ep_ja_saiu("2026-11-20T15:00:00+01:00", ms("2026-11-20T14:00:00Z")) == 1);
    assert(ep_ja_saiu("soon", m) == -1 && ep_ja_saiu(NULL, m) == -1);
    /* local date-time is in the viewer's zone (New York, UTC-5 in November) */
    assert(ep_ja_saiu("2026-11-20T10:00", ms("2026-11-20T14:59:00Z")) == 0);
    assert(ep_ja_saiu("2026-11-20T10:00", ms("2026-11-20T15:00:00Z")) == 1); }

  /* 3. computeAirDateBadgeText / Short, London, now = 5 Nov 2026 12:00 GMT */
  fuso("Europe/London");
  { long long agora = ms("2026-11-05T12:00:00Z");
    rot("2026-11-05T20:00:00Z", agora, 0, "Airs Today");
    rot("2026-11-05T20:00:00Z", agora, 1, "Today");
    rot("2026-11-06", agora, 0, "Airs Tomorrow");
    rot("2026-11-06", agora, 1, "Tomorrow");
    rot("2026-11-08", agora, 0, "Airs in 3 Days");
    rot("2026-11-08", agora, 1, "In 3 Days");
    rot("2026-11-12", agora, 0, "Airs in 7 Days");
    rot("2026-11-13", agora, 0, "Airs Nov 13");         /* 8 days: the date */
    rot("2026-11-13", agora, 1, "Nov 13");
    rot("2027-01-03", agora, 0, "Airs Jan 3, 2027");    /* year when not this year */
    rot("2026-11-05", agora, 0, "");                    /* plain date today: aired at UTC midnight */
    rot("2026-11-01", agora, 0, "");                    /* aired */
    rot("", agora, 0, "");                              /* no date: caller says Upcoming */
    rot("TBA", agora, 0, ""); }
  /* BST boundaries (Europe/London) */
  { long long agora = ms("2026-03-28T23:30:00Z");       /* Sat 23:30 GMT */
    rot("2026-03-29T12:00:00Z", agora, 0, "Airs Tomorrow"); /* Sun 13:00 BST */
    agora = ms("2026-10-24T23:30:00Z");                 /* Sun 00:30 BST */
    rot("2026-10-25T23:30:00Z", agora, 0, "Airs Today"); }  /* Sun 23:30 GMT */

  /* 4. resolveNextUpReleaseState */
  { long long agora = ms("2026-11-05T12:00:00Z"), visto = agora - 90 * DIA;
    assert(ep_alerta_lancamento(agora - 59 * DIA, visto, agora));
    assert(!ep_alerta_lancamento(agora - 60 * DIA, visto, agora));    /* < 60 days only */
    assert(!ep_alerta_lancamento(agora - 61 * DIA, visto, agora));
    assert(!ep_alerta_lancamento(agora - 5 * DIA, agora - 4 * DIA, agora)); /* watched after */
    assert(!ep_alerta_lancamento(agora + DIA, visto, agora));         /* not aired */
    assert(ep_alerta_temporada(1, 1, 2) && !ep_alerta_temporada(1, 2, 2));
    assert(!ep_alerta_temporada(0, 1, 2) && !ep_alerta_temporada(1, -1, 2));
    assert(ep_alerta_temporada(1, 0, 1)); }             /* specials seed, as `seed.season != null` */

  /* 5. formatContinueWatchingProgressLabel */
  { char b[48];
    ep_rotulo_progresso(65, 50, b, sizeof b); assert(!strcmp(b, "1h 5m left"));
    ep_rotulo_progresso(120, 10, b, sizeof b); assert(!strcmp(b, "2h 0m left"));
    ep_rotulo_progresso(5, 90, b, sizeof b); assert(!strcmp(b, "5m left"));
    ep_rotulo_progresso(0, 42, b, sizeof b); assert(!strcmp(b, "42% Watched"));
    ep_rotulo_progresso(0, 0, b, sizeof b); assert(!strcmp(b, "Resume")); }

  /* 6. The card badge and episode line (continuar.c), account next-ups */
  { const char *ids[] = { "tt1:2:1", "tt2:3:4", "tt3:1:5", "tt4:1:2", "tt5:1:3" };
    long long agora = ms("2026-11-05T12:00:00Z");
    CatItem a, b, c, d, e, prog;
    char ep[96];
    cwo_conta_definir(ids, 5);
    memset(&a, 0, sizeof a); snprintf(a.tipo, sizeof a.tipo, "series");
    b = c = d = e = prog = a;
    /* a: S2E1, upcoming in 3 days, untitled; seed watched long ago in season 1 */
    snprintf(a.imdb, sizeof a.imdb, "tt1:2:1"); a.temporada = 2; a.episodio = 1;
    cwo_marcar_estreia_cru(a.imdb, "2026-11-08");
    cwo_marcar_semente(a.imdb, agora - 100 * DIA, 1);
    selo(&a, 0, agora, "Airs in 3 Days", BADGE_NEUTRO);
    selo(&a, 1, agora, "In 3 Days", BADGE_NEUTRO);
    assert(continuar_proximo_inedito(&a, agora));
    continuar_titulo_episodio(&a, agora, ep, sizeof ep);
    assert(!strcmp(ep, "Airs in 3 Days"));               /* untitled + unaired */
    /* Regression (BlueStacks, Abbott Elementary S6E6): a next-up that carries
     * the episode runtime in restanteMin is still a NextUp — countdown, never
     * "22m left". */
    a.restanteMin = 22;
    selo(&a, 0, agora, "Airs in 3 Days", BADGE_NEUTRO);
    selo(&a, 1, agora, "In 3 Days", BADGE_NEUTRO);
    assert(continuar_e_a_seguir(&a));
    a.restanteMin = 0;
    /* the same item once it airs: recomputed, New Season (season changed) */
    selo(&a, 0, ms("2026-11-09T09:00:00Z"), "New Season", BADGE_NOVA_TEMPORADA);
    selo(&a, 1, ms("2026-11-09T09:00:00Z"), "New Season", BADGE_NOVA_TEMPORADA);
    assert(!continuar_proximo_inedito(&a, ms("2026-11-09T09:00:00Z")));
    /* b: S3E4 aired 2 days ago after the seed was watched: New Episode */
    snprintf(b.imdb, sizeof b.imdb, "tt2:3:4"); b.temporada = 3; b.episodio = 4;
    snprintf(b.nomeEpisodio, sizeof b.nomeEpisodio, "The Return");
    cwo_marcar_estreia_cru(b.imdb, "2026-11-03T02:00:00Z");
    cwo_marcar_semente(b.imdb, agora - 30 * DIA, 3);
    selo(&b, 0, agora, "New Episode", BADGE_NOVO_EPISODIO);
    selo(&b, 1, agora, "New", BADGE_NOVO_EPISODIO);
    continuar_titulo_episodio(&b, agora, ep, sizeof ep);
    assert(!strcmp(ep, "The Return"));
    /* c: aired 90 days ago: no alert, plain Next Up */
    snprintf(c.imdb, sizeof c.imdb, "tt3:1:5"); c.temporada = 1; c.episodio = 5;
    cwo_marcar_estreia_cru(c.imdb, "2026-08-07");
    cwo_marcar_semente(c.imdb, agora - 200 * DIA, 1);
    selo(&c, 0, agora, "Next Up", BADGE_NEUTRO);
    selo(&c, 1, agora, "Next", BADGE_NEUTRO);
    c.restanteMin = 40;                                  /* aired next-up with runtime */
    selo(&c, 0, agora, "Next Up", BADGE_NEUTRO);
    c.restanteMin = 0;
    /* d: no date at all: counts as aired (official fallback), Next Up */
    snprintf(d.imdb, sizeof d.imdb, "tt4:1:2"); d.temporada = 1; d.episodio = 2;
    selo(&d, 0, agora, "Next Up", BADGE_NEUTRO);
    assert(!continuar_proximo_inedito(&d, agora));
    continuar_titulo_episodio(&d, agora, ep, sizeof ep);
    assert(!ep[0]);
    /* e: future but the date does not parse into a label: Upcoming */
    snprintf(e.imdb, sizeof e.imdb, "tt5:1:3"); e.temporada = 1; e.episodio = 3;
    cwo_marcar_estreia(e.imdb, agora + 3 * DIA);       /* instant only, no raw value */
    selo(&e, 0, agora, "Upcoming", BADGE_NEUTRO);
    /* in progress: the progress label, never a release badge */
    snprintf(prog.imdb, sizeof prog.imdb, "tt9:1:1"); prog.temporada = 1; prog.episodio = 1;
    prog.progresso = 40; prog.restanteMin = 33;
    selo(&prog, 0, agora, "33m left", BADGE_NEUTRO);
    prog.restanteMin = 0;
    selo(&prog, 0, agora, "40% Watched", BADGE_NEUTRO);
    /* not a next-up and not in progress: no badge (nothing invented) */
    { CatItem z = a; snprintf(z.imdb, sizeof z.imdb, "tt8:1:1"); selo(&z, 0, agora, "", BADGE_NEUTRO);
      assert(!continuar_e_a_seguir(&z));
      z.restanteMin = 22;                                /* not a next-up, remaining known */
      selo(&z, 0, agora, "22m left", BADGE_NEUTRO); }
    /* episode code wording */
    { char te[32]; snprintf(te, sizeof te, i18n("T%d E%d"), 1, 2); assert(!strcmp(te, "S1 E2")); } }

  /* 7. Episode list: unaired episode uses the title's landscape backdrop when
   * its "still" is missing, is the show's poster, or is portrait. */
  { const char *bd = "https://img/bd.jpg", *po = "https://img/poster.jpg", *st = "https://img/still.jpg";
    assert(ep_arte_usa_fundo(1, "", po, bd, 0.0f));          /* no still */
    assert(ep_arte_usa_fundo(1, po, po, bd, 0.0f));          /* the poster as thumbnail */
    assert(ep_arte_usa_fundo(1, st, po, bd, 0.667f));        /* portrait image */
    assert(!ep_arte_usa_fundo(1, st, po, bd, 1.778f));       /* real landscape still kept */
    assert(!ep_arte_usa_fundo(1, st, po, bd, 0.0f));         /* not loaded yet: keep asking */
    assert(!ep_arte_usa_fundo(0, po, po, bd, 0.667f));       /* aired: unchanged */
    assert(!ep_arte_usa_fundo(1, po, po, "", 0.667f)); }     /* no backdrop: unchanged */

  /* 7b. Artwork provider: the show's LANDSCAPE art for a landscape card. */
  { const char *prov = "https://tv.apple/key.jpg", *mh = "https://images.metahub.space/background/large/tt1/img",
                *cat = "https://cinemeta/bd.jpg";
    assert(!strcmp(ep_arte_paisagem(prov, 1.778f, mh, cat), prov));   /* provider landscape wins */
    assert(!strcmp(ep_arte_paisagem(prov, 0.0f, mh, cat), prov));     /* not loaded yet: provider */
    assert(!strcmp(ep_arte_paisagem(prov, 0.667f, mh, cat), mh));     /* provider portrait -> metahub */
    assert(!strcmp(ep_arte_paisagem(prov, 0.667f, NULL, cat), prov)); /* only portrait: kept, cropped */
    assert(!strcmp(ep_arte_paisagem(NULL, 0.0f, mh, cat), mh));       /* no provider art */
    assert(!strcmp(ep_arte_paisagem(NULL, 0.0f, NULL, cat), cat));    /* catalog backdrop last */
    assert(ep_arte_paisagem(NULL, 0.0f, NULL, "") == NULL);
    assert(ep_arte_cobrir(0.667f) && !ep_arte_cobrir(1.778f) && !ep_arte_cobrir(0.0f) && !ep_arte_cobrir(1.0f)); }

  /* 8. Air-date source: the user's metadata add-on (regular Nuvio). */
  { /* candidate rule (MetaRepositoryImpl first pass): enabled, manifest read,
     * serves "meta", and this type + id prefix or no prefixes (-1). */
    assert(desc_meta_addon_candidato(1, 1, 1, 1));
    assert(desc_meta_addon_candidato(1, 1, 1, -1));     /* no idPrefixes: accepts all */
    assert(!desc_meta_addon_candidato(1, 1, 1, 0));     /* other type / prefix */
    assert(!desc_meta_addon_candidato(0, 1, 1, 1));     /* switched off */
    assert(!desc_meta_addon_candidato(1, 0, 1, 1));     /* manifest not read */
    assert(!desc_meta_addon_candidato(1, 1, 0, 1)); }   /* no meta resource */
  { /* two add-on styles: AIOMetadata-like (zoned `released`) and a
     * Cinemeta-like one (`firstAired`), plus a listed episode without a date */
    const char *aio = "{\"meta\":{\"name\":\"Abbott Elementary\",\"videos\":["
      "{\"season\":6,\"episode\":6,\"released\":\"2026-11-11T01:00:00.000Z\"},"
      "{\"season\":6,\"episode\":7,\"released\":\"\"},"
      "{\"season\":6,\"episode\":8}]}}";
    const char *outro = "{\"meta\":{\"name\":\"Show\",\"videos\":["
      "{\"season\":2,\"episode\":1,\"firstAired\":\"2026-11-11\"}]}}";
    char d[48];
    assert(desc_lancamento_do_corpo(aio, 6, 6, d, sizeof d) == 1 &&
           !strcmp(d, "2026-11-11T01:00:00.000Z"));   /* the add-on's value, untouched */
    assert(desc_lancamento_do_corpo(aio, 6, 7, d, sizeof d) == 0);   /* listed, no date */
    assert(desc_lancamento_do_corpo(aio, 6, 8, d, sizeof d) == 0);
    assert(desc_lancamento_do_corpo(aio, 6, 9, d, sizeof d) == -1);  /* not listed: fallback */
    assert(desc_lancamento_do_corpo(NULL, 6, 6, d, sizeof d) == -1); /* no add-on */
    assert(desc_lancamento_do_corpo(outro, 2, 1, d, sizeof d) == 1 && !strcmp(d, "2026-11-11"));
    /* the TMDB "Release dates" setting never changes episode dates */
    valor[AJ_TMDB_DATAS] = 1;
    assert(desc_lancamento_do_corpo(aio, 6, 6, d, sizeof d) == 1 &&
           !strcmp(d, "2026-11-11T01:00:00.000Z"));
    valor[AJ_TMDB_DATAS] = 0;
    /* no automatic US shift: a plain US date stays that date everywhere; a
     * zoned instant becomes the viewer's local date (official rule) */
    fuso("America/New_York"); dia("2026-11-11", 2026, 11, 11);
    dia("2026-11-11T01:00:00.000Z", 2026, 11, 10);       /* 8 pm on the 10th in New York */
    fuso("Europe/London");    dia("2026-11-11", 2026, 11, 11);
    dia("2026-11-11T01:00:00.000Z", 2026, 11, 11);
    rot("2026-11-11T01:00:00.000Z", ms("2026-11-05T12:00:00Z"), 0, "Airs in 6 Days"); }
  { /* episode list overlay: the add-on's date (or its "no date") wins for the
     * episodes it lists; others keep theirs */
    CatEp eps[3], ad[2];
    memset(eps, 0, sizeof eps); memset(ad, 0, sizeof ad);
    eps[0].temporada = 6; eps[0].episodio = 5; snprintf(eps[0].dataIso, sizeof eps[0].dataIso, "2026-11-03");
    eps[1].temporada = 6; eps[1].episodio = 6; snprintf(eps[1].dataIso, sizeof eps[1].dataIso, "2026-11-12");
    eps[2].temporada = 6; eps[2].episodio = 7; snprintf(eps[2].dataIso, sizeof eps[2].dataIso, "2026-11-19");
    snprintf(eps[1].data, sizeof eps[1].data, "12 November 2026");
    ad[0].temporada = 6; ad[0].episodio = 6; snprintf(ad[0].dataIso, sizeof ad[0].dataIso, "2026-11-11");
    snprintf(ad[0].data, sizeof ad[0].data, "11 November 2026");
    ad[1].temporada = 6; ad[1].episodio = 7;              /* listed without a date */
    assert(desc_datas_do_addon(eps, 3, ad, 2) == 2);
    assert(!strcmp(eps[0].dataIso, "2026-11-03"));        /* not listed: kept */
    assert(!strcmp(eps[1].dataIso, "2026-11-11") && !strcmp(eps[1].data, "11 November 2026"));
    assert(!eps[2].dataIso[0] && !eps[2].data[0]);        /* add-on's "no date" */
    assert(desc_datas_do_addon(eps, 3, ad, 2) == 0); }    /* idempotent */

  puts("PASS: Stage E release dates, countdown, release alert, CW badge, episode line and unaired episode art");
  return 0;
}

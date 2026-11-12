/* Stage H Family Watching: rules, parsers, labels and the app state, with
 * fixtures only (no account, no network). The live transport is not
 * registered in this build, so the first app-state case proves no read happens
 * without it; the rest register a FIXTURE transport. */
#include "familia.h"
#include "perfis.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- stubs ---------------------------------------------------------------- */
const char *i18n(const char *s) {
  static const char *const m[][2] = {
    { "Em andamento", "In Progress" }, { "Assistido recentemente", "Recently Watched" },
    { "T%d E%d", "S%d E%d" }, { "Filme", "Movie" }, { "%dm restantes", "%dm left" },
    { "%dh %dm restantes", "%dh %dm left" }, { "há %d h", "%d h ago" },
    { "há %d min", "%d min ago" }, { "há %d dias", "%d days ago" }, { "Agora", "Now" } };
  for (size_t i = 0; i < sizeof m / sizeof *m; i++) if (!strcmp(s, m[i][0])) return m[i][1];
  return s;
}
static Uint32 relogio = 1000;
Uint32 SDL_GetTicks(void) { return relogio; }
static int stLigado, stAtivo = 1, stPartilha[8], stInfantil[8], nDescRepetir, nPartilhaLida;
static ContaPerfil stPerfis[6];
int ajustes_familia(void) { return stLigado; }
int ajustes_familia_partilha_perfil(int p) { nPartilhaLida++; return p > 0 && p < 8 && stPartilha[p]; }
int perfis_n(void) { return 6; }
const ContaPerfil *perfis_item(int i) { return i >= 0 && i < 6 ? &stPerfis[i] : NULL; }
int perfis_ativo(void) { return stAtivo; }
int perfil_infantil(int p) { return p > 0 && p < 8 && stInfantil[p]; }
int perfil_infantil_ativo(void) { return perfil_infantil(stAtivo); }
void desc_repetir(void) { nDescRepetir++; }

/* ---- fixtures (shapes of the two existing per-profile RPC bodies) --------- */
static const char *PROG2 =
  "[{\"content_id\":\"tt0903747\",\"content_type\":\"series\",\"video_id\":\"tt0903747:2:5\","
  "\"season\":2,\"episode\":5,\"position_ms\":1500000,\"duration_ms\":2820000,"
  "\"last_watched\":\"2026-11-06T18:00:00Z\"},"
  " {\"content_id\":\"tt1375666\",\"content_type\":\"movie\",\"season\":null,\"episode\":null,"
  "\"position_ms\":100,\"duration_ms\":0,\"last_watched\":\"2026-11-06T19:00:00Z\"}]";
static const char *VIST2 =
  "[{\"content_id\":\"tt0111161\",\"content_type\":\"movie\",\"season\":null,\"episode\":null,"
  "\"watched_at\":\"2026-11-05T10:00:00Z\"}]";
static const char *PROG5 =
  "[{\"content_id\":\"tt0816692\",\"content_type\":\"movie\",\"position_ms\":5000000,"
  "\"duration_ms\":5100000,\"last_watched\":\"2026-11-06T20:00:00Z\"}]";

static int chamadas[8];
static int busca(int perfil, FamAtividade *out, int max, void *u) {
  int n = 0, k;
  (void)u;
  chamadas[perfil]++;
  if (perfil == 2) {
    n = familia_ler_progresso(PROG2, out, max);
    k = familia_ler_vistos(VIST2, out + n, max - n);
    return n + k;
  }
  if (perfil == 5) return familia_ler_progresso(PROG5, out, max);
  if (perfil == 3 || perfil == 4) return familia_ler_progresso(PROG5, out, max);
  return 0;   /* profile 6: shares, but has no activity */
}

static void perfisFixos(void) {
  static const char *nomes[] = { "Owner", "Sam", "Locked", "Kid", "Alex", "Quiet" };
  for (int i = 0; i < 6; i++) {
    memset(&stPerfis[i], 0, sizeof stPerfis[i]);
    stPerfis[i].indice = i + 1;
    snprintf(stPerfis[i].nome, sizeof stPerfis[i].nome, "%s", nomes[i]);
    snprintf(stPerfis[i].corHex, sizeof stPerfis[i].corHex, "#1E88E5");
  }
  stPerfis[2].temPin = 1;           /* profile 3: PIN-locked */
  memset(stPartilha, 0, sizeof stPartilha); memset(stInfantil, 0, sizeof stInfantil);
  stPartilha[2] = stPartilha[3] = stPartilha[4] = stPartilha[5] = stPartilha[6] = 1;
  stInfantil[4] = 1;                /* profile 4: child */
}

int main(void) {
  /* 1. Visibility and eligibility. */
  assert(!familia_linha_visivel(0, 0));            /* Off by default */
  assert(!familia_linha_visivel(1, 1));            /* child viewer: whole row hidden */
  assert(familia_linha_visivel(1, 0));
  { FamPerfil p = { .perfil = 2, .partilha = 1 };
    assert(familia_perfil_elegivel(&p, 1));
    assert(!familia_perfil_elegivel(&p, 2));        /* never yourself */
    p.temPin = 1; assert(!familia_perfil_elegivel(&p, 1));
    p.temPin = 0; p.infantil = 1; assert(!familia_perfil_elegivel(&p, 1));
    p.infantil = 0; p.partilha = 0; assert(!familia_perfil_elegivel(&p, 1)); }

  /* 2. Parsers on the RPC shapes. */
  { FamAtividade a[8];
    int n = familia_ler_progresso(PROG2, a, 8);
    /* the zero-length movie row is not activity: dropped by the parser */
    assert(n == 1 && !strcmp(a[0].id, "tt0903747") && !strcmp(a[0].tipo, "series"));
    assert(a[0].temporada == 2 && a[0].episodio == 5 && a[0].durMs == 2820000 && a[0].quandoMs > 0);
    n = familia_ler_progresso(PROG5, a, 8);
    assert(n == 1 && !strcmp(a[0].tipo, "movie") && a[0].temporada == 0 && a[0].episodio == 0);
    n = familia_ler_vistos(VIST2, a, 8);
    assert(n == 1 && a[0].visto && a[0].quandoMs > 0);
    assert(familia_ler_progresso("{\"x\":1}", a, 8) == -1 && familia_ler_vistos(NULL, a, 8) == -1); }

  /* 3. Choice: newest usable activity; zero-length rows are not activity;
   *    >= 90 % is a finished watch; equal times prefer in-progress. */
  { FamAtividade a[8], e;
    int n = familia_ler_progresso(PROG2, a, 8);
    n += familia_ler_vistos(VIST2, a + n, 8 - n);
    assert(familia_escolher(a, n, &e) && !strcmp(e.id, "tt0903747"));  /* newer than the watched movie */
    FamAtividade b[2] = { { .id = "x", .tipo = "movie", .posMs = 95, .durMs = 100, .quandoMs = 5 },
                          { .id = "y", .tipo = "movie", .visto = 1, .quandoMs = 4 } };
    assert(familia_escolher(b, 2, &e) && !strcmp(e.id, "x"));
    FamCartao c; FamPerfil p = { .perfil = 2, .nome = "Sam", .partilha = 1 };
    assert(familia_cartao_de(&p, &e, &c) && c.estado == FAM_VISTO && c.pct == 0);
    FamAtividade t[2] = { { .id = "w", .tipo = "movie", .visto = 1, .quandoMs = 7 },
                          { .id = "p", .tipo = "movie", .posMs = 10, .durMs = 100, .quandoMs = 7 } };
    assert(familia_escolher(t, 2, &e) && !strcmp(e.id, "p"));
    assert(!familia_escolher(t, 0, &e));
    /* "Mark series watched": one instant for every episode -> the furthest one. */
    FamAtividade s[4] = { { .id = "m", .tipo = "series", .temporada = 1, .episodio = 1, .visto = 1, .quandoMs = 9 },
                          { .id = "m", .tipo = "series", .temporada = 2, .episodio = 8, .visto = 1, .quandoMs = 9 },
                          { .id = "m", .tipo = "series", .temporada = 2, .episodio = 3, .visto = 1, .quandoMs = 9 },
                          { .id = "m", .tipo = "series", .temporada = 1, .episodio = 9, .visto = 1, .quandoMs = 9 } };
    assert(familia_escolher(s, 4, &e) && e.temporada == 2 && e.episodio == 8); }

  /* 4. Labels (English UI). */
  { FamCartao c = { .tipo = "series", .temporada = 2, .episodio = 5, .estado = FAM_EM_CURSO,
                    .pct = 53, .restanteMin = 22 };
    char pl[64], ln[96];
    familia_rotulos(&c, 0, pl, sizeof pl, ln, sizeof ln);
    assert(!strcmp(pl, "In Progress") && !strcmp(ln, "S2 E5 · 22m left"));
    FamCartao m = { .tipo = "movie", .estado = FAM_EM_CURSO, .restanteMin = 65 };
    familia_rotulos(&m, 0, pl, sizeof pl, ln, sizeof ln);
    assert(!strcmp(ln, "Movie · 1h 5m left"));
    FamCartao v = { .tipo = "series", .temporada = 1, .episodio = 3, .estado = FAM_VISTO,
                    .quandoMs = 1000000 };
    familia_rotulos(&v, 1000000 + 3 * 3600000LL, pl, sizeof pl, ln, sizeof ln);
    assert(!strcmp(pl, "Recently Watched") && !strcmp(ln, "S1 E3 · 3 h ago"));
    v.quandoMs = 0; familia_rotulos(&v, 5, pl, sizeof pl, ln, sizeof ln);
    assert(!strcmp(ln, "S1 E3")); }

  /* 5. Building the row from fixture profiles: only eligible profiles are
   *    asked for anything; newest first; cards carry no copy of anything. */
  { FamPerfil ps[6]; FamCartao c[FAM_MAX];
    perfisFixos();
    for (int i = 0; i < 6; i++) {
      memset(&ps[i], 0, sizeof ps[i]);
      ps[i].perfil = i + 1; snprintf(ps[i].nome, sizeof ps[i].nome, "%s", stPerfis[i].nome);
      ps[i].temPin = stPerfis[i].temPin; ps[i].infantil = stInfantil[i + 1]; ps[i].partilha = stPartilha[i + 1];
    }
    memset(chamadas, 0, sizeof chamadas);
    int n = familia_montar(ps, 6, 1, 0, 1, busca, NULL, c, FAM_MAX);
    assert(n == 2);
    assert(chamadas[1] == 0 && chamadas[3] == 0 && chamadas[4] == 0);   /* self, PIN, child: never read */
    assert(chamadas[2] == 1 && chamadas[5] == 1 && chamadas[6] == 1);
    assert(c[0].perfil == 5 && c[0].estado == FAM_VISTO);                /* newest first (>=90 %) */
    assert(c[1].perfil == 2 && c[1].estado == FAM_EM_CURSO && c[1].temporada == 2 &&
           c[1].episodio == 5 && c[1].restanteMin == 22 && !strcmp(c[1].nome, "Sam"));
    memset(chamadas, 0, sizeof chamadas);
    assert(familia_montar(ps, 6, 1, 0, 0, busca, NULL, c, FAM_MAX) == 0);  /* row off */
    assert(familia_montar(ps, 6, 1, 1, 1, busca, NULL, c, FAM_MAX) == 0);  /* child viewer */
    for (int i = 0; i < 8; i++) assert(chamadas[i] == 0); }

  /* 6. App state. No transport is registered in this build: nothing is read. */
  { FamCartao c[FAM_MAX];
    perfisFixos(); stLigado = 1; stAtivo = 1;
    familia_passo();
    memset(chamadas, 0, sizeof chamadas);
    assert(!familia_tem_busca() && familia_atualizar(c, FAM_MAX) == 0);
    for (int i = 0; i < 8; i++) assert(chamadas[i] == 0);

    /* Row Off (the default): the per-profile consent is not even looked up. */
    stLigado = 0; nPartilhaLida = 0; relogio += 6000; familia_passo();
    assert(nPartilhaLida == 0);
    familia_definir_busca(busca, NULL);
    assert(familia_atualizar(c, FAM_MAX) == 0);
    for (int i = 0; i < 8; i++) assert(chamadas[i] == 0);

    /* On, with the fixture transport. */
    stLigado = 1; relogio += 6000; familia_passo();
    assert(familia_atualizar(c, FAM_MAX) == 2 && chamadas[2] == 1 && chamadas[5] == 1);
    FamCartao um;
    assert(familia_cartao_do_perfil(2, &um) && um.estado == FAM_EM_CURSO);
    /* The Home catalogue cycle reads the cache only: no request. */
    { int antes2 = chamadas[2]; FamCartao cc[FAM_MAX];
      assert(familia_cartoes(cc, FAM_MAX) == 2 && chamadas[2] == antes2); }
    assert(!familia_cartao_do_perfil(3, &um) && !familia_cartao_do_perfil(4, &um));
    /* Throttled: the next cycle inside 2 min reuses the cards. */
    relogio += 30000;
    assert(familia_atualizar(c, FAM_MAX) == 2 && chamadas[2] == 1);
    /* Home opened inside 2 min: no new read; after 2 min it wants one. */
    familia_home_aberta(); familia_passo();
    assert(!familia_quer_ler() && familia_atualizar(c, FAM_MAX) == 2 && chamadas[2] == 1);
    relogio += FAM_REFAZ_MS; familia_home_aberta(); familia_passo();
    assert(familia_quer_ler());
    assert(familia_atualizar(c, FAM_MAX) == 2 && chamadas[2] == 2 && !familia_quer_ler());
    /* Profile 2 stops sharing: its card goes at the next snapshot, without a read. */
    stPartilha[2] = 0; relogio += 6000; familia_passo();
    assert(familia_atualizar(c, FAM_MAX) == 1 && c[0].perfil == 5 && chamadas[2] == 2);
    assert(!familia_cartao_do_perfil(2, &um));
    /* Profile 5 gets a PIN: gone too. */
    stPerfis[4].temPin = 1; relogio += 6000; familia_passo();
    assert(familia_atualizar(c, FAM_MAX) == 0);
    /* The active profile becomes a child profile: the row is hidden. */
    stPerfis[4].temPin = 0; stPartilha[2] = 1; stInfantil[1] = 1; relogio += 6000; familia_passo();
    memset(chamadas, 0, sizeof chamadas);
    { FamCartao cc[FAM_MAX]; assert(familia_cartoes(cc, FAM_MAX) == 0); }   /* hidden at once */
    assert(!familia_quer_ler() && familia_atualizar(c, FAM_MAX) == 0);
    for (int i = 0; i < 8; i++) assert(chamadas[i] == 0);
    /* Profile switch: the cards read for profile 1 (here: profile 2's and 5's)
     * are dropped at once; the new viewer never gets its own card. */
    stInfantil[1] = 0; familia_ajuste_mudou(); familia_atualizar(c, FAM_MAX);
    assert(familia_cartoes(c, FAM_MAX) == 2 && familia_cartao_do_perfil(2, &um));
    stAtivo = 2; nDescRepetir = 0;
    { FamCartao cc[FAM_MAX];
      assert(familia_cartoes(cc, FAM_MAX) == 0 && !familia_cartao_do_perfil(2, &um));  /* before any step */
      familia_passo();
      assert(familia_cartoes(cc, FAM_MAX) == 0 && familia_quer_ler());
      stPartilha[1] = 1;   /* profile 1 shares now */
      relogio += 6000; familia_passo();
      assert(familia_atualizar(cc, FAM_MAX) >= 1);
      for (int i = 0; i < familia_cartoes(cc, FAM_MAX); i++) assert(cc[i].perfil != 2); }
    stAtivo = 1; stPartilha[1] = 0; relogio += 6000; familia_passo();
    /* A settings change rebuilds Home once; account change forgets the cards. */
    stInfantil[1] = 0; nDescRepetir = 0; familia_ajuste_mudou();
    assert(nDescRepetir == 1);
    assert(familia_atualizar(c, FAM_MAX) == 2);
    unsigned r = familia_revisao(); familia_esquecer();
    assert(familia_revisao() != r && !familia_cartao_do_perfil(2, &um)); }

  puts("ok Family Watching: off by default, opt-in sharing, PIN/child excluded, child viewer hides row, no reads without transport, throttled refresh, newest first, neutral cards");
  return 0;
}

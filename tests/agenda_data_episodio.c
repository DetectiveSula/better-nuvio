// SCHEDULE / DETAIL TAG / EPISODE LIST MUST SAY THE SAME DAY (owner, 12 Nov 2026).
//
//   bash tests/agenda_data_episodio.sh
//
// The owner's BlueStacks: the title page said "Next episode S6 E3 · 10 December
// 2026" and the Schedule agreed, but the episode card disagreed. Cause: the
// tag and the Schedule read TMDB `next_episode_to_air.air_date` (the network's
// LOCAL day), while the episode list prints the first 10 characters of the
// add-on's `released` for the same episode (Cinemeta/TVDB, a UTC instant).
// Live data on 12 Nov 2026, Abbott Elementary S6E7: TMDB air_date 2026-12-02,
// Cinemeta released 2026-12-03T04:30:00.000Z -> the card says "3 December".
//
// The fixture below is that pair. BEFORE the fix the tag says 2 December and
// the card 3 December; AFTER, all three say the card's day.
//
// Includes src/agenda.c (the static parsers are the real code), as
// tests/agenda.c does; the .sh compiles everything else.
#include "../src/agenda.c"
#include "idioma.h"
#include <unistd.h>

static int falhas = 0;
#define CONFERE(c, ...) do { if (!(c)) { falhas++; printf("FAIL: " __VA_ARGS__); printf("\n"); } \
                             else { printf("ok:   " __VA_ARGS__); printf("\n"); } } while (0)

// TMDB /tv/<id> for the series, cut to what tmdbLer reads.
static const char *TMDB_S6E7 =
  "{\"id\":95480,\"name\":\"Abbott Elementary\",\"status\":\"Returning Series\","
  "\"number_of_seasons\":6,"
  "\"last_episode_to_air\":{\"air_date\":\"2026-11-11\",\"season_number\":6,\"episode_number\":6,"
  "\"name\":\"Competition\"},"
  "\"next_episode_to_air\":{\"air_date\":\"2026-12-02\",\"season_number\":6,\"episode_number\":7,"
  "\"name\":\"One Hundred\",\"episode_type\":\"standard\"}}";
// Same body a week later: TMDB moved on to S6E8.
static const char *TMDB_S6E8 =
  "{\"id\":95480,\"name\":\"Abbott Elementary\",\"status\":\"Returning Series\","
  "\"next_episode_to_air\":{\"air_date\":\"2026-12-09\",\"season_number\":6,\"episode_number\":8,"
  "\"name\":\"Parents\"}}";
// The /meta the episode list is built from (Cinemeta form).
static const char *META =
  "{\"meta\":{\"id\":\"tt14218830\",\"name\":\"Abbott Elementary\",\"status\":\"Continuing\","
  "\"videos\":["
  "{\"id\":\"tt14218830:6:6\",\"name\":\"Competition\",\"season\":6,\"episode\":6,"
  "\"released\":\"2026-11-12T04:30:00.000Z\"},"
  "{\"id\":\"tt14218830:6:7\",\"name\":\"One Hundred\",\"season\":6,\"episode\":7,"
  "\"released\":\"2026-12-03T04:30:00.000Z\"},"
  "{\"id\":\"tt14218830:6:8\",\"name\":\"Parents\",\"season\":6,\"episode\":8,"
  "\"released\":\"2026-12-10T04:30:00.000Z\"}]}}";

#define ID "tt14218830"

static void episodio(CatEp *e, int t, int n, const char *released) {
  memset(e, 0, sizeof *e);
  e->temporada = t; e->episodio = n;
  snprintf(e->dataIso, sizeof e->dataIso, "%s", released);
  desc_data_extenso(released, e->data, sizeof e->data);   // what the card prints
}

static void reiniciar(void) {
  perfilCarregado = -1; nCache = 0; nLembretes = 0; nLista = 0; cacheSujo = 0;
  agenda_iniciar();
}

int main(void) {
  const char *dir = getenv("NUVIO_DADOS");
  char tag[200], cartao[48];
  CatEp lista[3];
  const AgItem *r;
  int ingles;

  dados_iniciar(".");
  if (!dir || !dir[0] || strcmp(dados_dir(), dir)) {
    printf("FAIL: data dir is [%s], expected NUVIO_DADOS [%s]. Aborting before writing.\n",
           dados_dir(), dir ? dir : "");
    return 1;
  }
  setenv("TZ", "Europe/London", 1); tzset();   // the owner's BlueStacks
  agenda_definir_hoje("2026-11-12");
  ingles = idioma_ingles_entrar();

  episodio(&lista[0], 6, 6, "2026-11-12T04:30:00.000Z");
  episodio(&lista[1], 6, 7, "2026-12-03T04:30:00.000Z");
  episodio(&lista[2], 6, 8, "2026-12-10T04:30:00.000Z");
  snprintf(cartao, sizeof cartao, "%s", lista[1].data);
  printf("episode card S6E7: \"%s\"\n", cartao);

  // --- 1. the mismatch as it was ------------------------------------------
  lerCorpoTv(ID, "Abbott Elementary", TMDB_S6E7);
  agenda_frase(ID, tag, sizeof tag);
  printf("BEFORE (TMDB only): tag \"%s\" | Schedule day %s\n", tag, agenda_registro(ID)->dataProx);
  CONFERE(!strstr(tag, cartao), "before: the tag does NOT carry the card's day (the bug)");

  // --- 2. the list arrives: the card's day wins everywhere -----------------
  CONFERE(agenda_alinhar_episodios(ID, lista, 3) == 1, "list aligned the record");
  agenda_frase(ID, tag, sizeof tag);
  r = agenda_registro(ID);
  printf("AFTER  (list wins): tag \"%s\" | Schedule day %s\n", tag, r->dataProx);
  CONFERE(strstr(tag, cartao) != NULL, "tag carries the card's day: [%s] vs [%s]", tag, cartao);
  CONFERE(strstr(tag, "S6 E7") != NULL, "tag keeps the episode code S6 E7: [%s]", tag);
  CONFERE(!strcmp(r->dataProx, "2026-12-03"), "Schedule record day = card day: [%s]", r->dataProx);
  CONFERE(agenda_alinhar_episodios(ID, lista, 3) == 0, "second pass changes nothing");
  { int i, achou = 0;
    agenda_montar();
    // The Schedule list may not include it (not followed in this catalog), so
    // the record is what both screens read; check the list copy when present.
    for (i = 0; i < agenda_n(); i++)
      if (!strcmp(agenda_lista(i)->imdb, ID)) {
        achou = 1;
        CONFERE(!strcmp(agenda_lista(i)->dataProx, "2026-12-03"), "Schedule row day: [%s]",
                agenda_lista(i)->dataProx);
      }
    if (!achou) printf("(Schedule row not mounted in this catalog; record checked above)\n"); }

  // --- 3. TMDB refresh of the SAME episode keeps the card's day -------------
  lerCorpoTv(ID, "Abbott Elementary", TMDB_S6E7);
  CONFERE(!strcmp(agenda_registro(ID)->dataProx, "2026-12-03"),
          "TMDB refresh keeps the pinned day: [%s]", agenda_registro(ID)->dataProx);

  // --- 4. it survives a restart (fields at the end of the TSV line) ---------
  reiniciar();
  r = agenda_registro(ID);
  CONFERE(r && !strcmp(r->dataProx, "2026-12-03"), "restart keeps the day: [%s]", r ? r->dataProx : "");
  lerCorpoTv(ID, "Abbott Elementary", TMDB_S6E7);
  CONFERE(!strcmp(agenda_registro(ID)->dataProx, "2026-12-03"), "and the pin after restart");

  // --- 5. a DIFFERENT next episode drops the pin ----------------------------
  lerCorpoTv(ID, "Abbott Elementary", TMDB_S6E8);
  r = agenda_registro(ID);
  CONFERE(r->temporada == 6 && r->episodio == 8 && !strcmp(r->dataProx, "2026-12-09"),
          "next is S6E8 with TMDB's day until the list says otherwise: S%dE%d %s",
          r->temporada, r->episodio, r->dataProx);
  CONFERE(agenda_alinhar_episodio(ID, 6, 8, lista[2].dataIso) == 1, "list aligns S6E8");
  CONFERE(!strcmp(agenda_registro(ID)->dataProx, "2026-12-10"), "S6E8 = card day 2026-12-10");
  // Other episodes or empty dates never touch the record.
  CONFERE(agenda_alinhar_episodio(ID, 6, 7, lista[1].dataIso) == 0, "not the next episode: ignored");
  CONFERE(agenda_alinhar_episodio(ID, 6, 8, "") == 0, "no released: ignored");
  CONFERE(agenda_alinhar_episodio("tt0000001", 6, 8, lista[2].dataIso) == 0, "no record: nothing created");
  CONFERE(agenda_registro("tt0000001") == NULL, "still no record");

  // --- 6. background Schedule fetch: the /meta it reads wins too -----------
  { AgBusca b;
    memset(&b, 0, sizeof b);
    CONFERE(tmdbLer(&b, TMDB_S6E7) == 1, "TMDB body read");
    CONFERE(!strcmp(b.dataProx, "2026-12-02"), "TMDB alone says 2026-12-02");
    cinemetaLer(&b, META, "2026-11-12");
    CONFERE(b.temp == 6 && b.ep == 7 && !strcmp(b.dataProx, "2026-12-03"),
            "with the list's /meta the fetch says S%dE%d %s", b.temp, b.ep, b.dataProx);
    CONFERE(!strcmp(b.fonte, "tmdb"), "TMDB still chose the episode"); }

  idioma_ingles_sair(ingles);
  printf(falhas ? "\n%d FAILURE(S)\n" : "\nagenda_data_episodio: all checks passed\n", falhas);
  return falhas ? 1 : 0;
}

#include "../src/extras.h"
#include <stdio.h>
#include <string.h>

static int check(const char *nome, int ok) {
  if (!ok) { fprintf(stderr, "FALHOU: %s\n", nome); return 1; }
  printf("ok %s\n", nome);
  return 0;
}

int main(void) {
  const char *json =
    "{\"results\":["
      "{\"site\":\"Vimeo\",\"type\":\"Trailer\",\"key\":\"bad-vimeo\"},"
      "{\"site\":\"YouTube\",\"type\":\"Behind the Scenes\",\"key\":\"badtype\"},"
      "{\"site\":\"YouTube\",\"type\":\"Trailer\",\"key\":\"AbC_123-xYz\"},"
      "{\"site\":\"YouTube\",\"type\":\"Teaser\",\"key\":\"bad/key\"},"
      "{\"site\":\"YouTube\",\"type\":\"Teaser\",\"key\":\"SecondValid\"}"
    "]}";
  char id[32] = "stale";
  int rc = 0;
  rc |= check("parser filtra site e tipo e preserva ordem", 
              extras_hero_trailer_parse(json, id, sizeof id) == 2 &&
              !strcmp(id, "AbC_123-xYz"));
  strcpy(id, "stale");
  rc |= check("parser rejeita resposta sem video elegivel",
              extras_hero_trailer_parse("{\"results\":[{\"site\":\"Vimeo\",\"type\":\"Trailer\",\"key\":\"abc123\"}]}",
                                        id, sizeof id) == 0 && !id[0]);
  rc |= check("parser tolera corpo ausente", extras_hero_trailer_parse(NULL, id, sizeof id) == 0 && !id[0]);
  // A PROPER TRAILER FIRST (19/11/2026), from what BlueStacks played: a TV
  // spot listed first by TMDB, a home-release promo, a teaser and a fan upload
  // all lose to the studio's own trailer; with only promos, one still plays.
  { const char *mix =
      "{\"results\":["
        "{\"site\":\"YouTube\",\"type\":\"Trailer\",\"key\":\"CinemasNow1\",\"name\":\"Resident Evil - In Cinemas Now\",\"official\":true,\"size\":1080,\"iso_639_1\":\"en\"},"
        "{\"site\":\"YouTube\",\"type\":\"Trailer\",\"key\":\"WatchAtHome\",\"name\":\"The Odyssey | Watch at Home November 17\",\"official\":true,\"size\":1080,\"iso_639_1\":\"en\"},"
        "{\"site\":\"YouTube\",\"type\":\"Teaser\",\"key\":\"TeaserOnly1\",\"name\":\"Official Teaser\",\"official\":true,\"size\":2160,\"iso_639_1\":\"en\"},"
        "{\"site\":\"YouTube\",\"type\":\"Trailer\",\"key\":\"FanUpload01\",\"name\":\"Trailer\",\"official\":false,\"size\":1080,\"iso_639_1\":\"en\"},"
        "{\"site\":\"YouTube\",\"type\":\"Trailer\",\"key\":\"OfficialTr1\",\"name\":\"Official Trailer\",\"official\":true,\"size\":1080,\"iso_639_1\":\"en\"}"
      "]}";
    const char *soPromo =
      "{\"results\":["
        "{\"site\":\"YouTube\",\"type\":\"Trailer\",\"key\":\"TvSpot00001\",\"name\":\"TV Spot\",\"official\":true}"
      "]}";
    rc |= check("proper trailer: the studio's Official Trailer beats spots, promos, teaser and fan upload",
                extras_hero_trailer_parse(mix, id, sizeof id) == 5 && !strcmp(id, "OfficialTr1"));
    rc |= check("proper trailer: with only a spot, the spot still plays",
                extras_hero_trailer_parse(soPromo, id, sizeof id) == 1 && !strcmp(id, "TvSpot00001")); }
  // End-of-film logos: UI language first, then English, then textless; .svg skipped.
  { const char *imgs =
      "{\"id\":1,\"logos\":["
        "{\"iso_639_1\":\"en\",\"file_path\":\"/en.svg\"},"
        "{\"iso_639_1\":null,\"file_path\":\"/semtexto.png\"},"
        "{\"iso_639_1\":\"en\",\"file_path\":\"/en.png\"},"
        "{\"iso_639_1\":\"de\",\"file_path\":\"/de.png\"},"
        "{\"iso_639_1\":\"en\",\"file_path\":\"/en2.png\"}]}";
    char u[200];
    extras_relacionado_logo_parse(imgs, "en", u, sizeof u);
    rc |= check("related logo: English png, svg skipped", !strcmp(u, "https://image.tmdb.org/t/p/w500/en.png"));
    extras_relacionado_logo_parse(imgs, "de", u, sizeof u);
    rc |= check("related logo: UI language first", !strcmp(u, "https://image.tmdb.org/t/p/w500/de.png"));
    extras_relacionado_logo_parse("{\"logos\":[{\"iso_639_1\":null,\"file_path\":\"/x.png\"}]}", "fr", u, sizeof u);
    rc |= check("related logo: textless when nothing else", !strcmp(u, "https://image.tmdb.org/t/p/w500/x.png"));
    extras_relacionado_logo_parse("{\"logos\":[]}", "en", u, sizeof u);
    rc |= check("related logo: none", !u[0]);
    extras_relacionado_logo_parse(NULL, "en", u, sizeof u);
    rc |= check("related logo: no body", !u[0]); }
  puts(rc ? "extras-hero: FALHOU" : "extras-hero: tudo ok");
  return rc ? 1 : 0;
}

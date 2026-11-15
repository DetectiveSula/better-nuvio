// Continue Watching keyed by a TMDB number gets its artwork from TMDB
// (owner, 15 Nov 2026: Street Fighter, "tmdb:1153576", showed "Artwork
// unavailable"). Fake network: only TMDB /movie/1153576 and /tv/42 answer, with a
// collection poster placed before the movie's own to check the top-level read.
//   bash tests/cw_tmdb_arte.sh
#define main dup_main_original
#define rede_baixar_com dup_baixar_com_original
#define rede_baixar dup_baixar_original
#include "trakt_cw_dup.c"
#undef main
#undef rede_baixar_com
#undef rede_baixar
int ajustes_cw_proximo(void) { return 0; }
int simkl_e_a_seguir(const char *id) { (void)id; return 0; }
const char *desc_chave_tmdb_reserva(void) { return "0123456789abcdef0123456789abcdef"; }
static int pedidos;
char *rede_baixar_com(const char *url, int s, const char *const *cab) { (void)url; (void)s; (void)cab; return NULL; }
char *rede_baixar(const char *url, int s) {
  (void)s;
  if (strstr(url, "api.themoviedb.org/3/movie/1153576?")) {
    pedidos++;
    return strdup("{\"backdrop_path\":\"/fundo.jpg\",\"belongs_to_collection\":{\"poster_path\":\"/colecao.jpg\",\"backdrop_path\":\"/colfundo.jpg\"},"
                  "\"id\":1153576,\"overview\":\"Ryu and Ken.\",\"poster_path\":\"/cartaz.jpg\",\"title\":\"Street Fighter\"}");
  }
  if (strstr(url, "api.themoviedb.org/3/tv/42?")) {
    pedidos++;
    return strdup("{\"backdrop_path\":null,\"name\":\"Serie\",\"poster_path\":\"/serie.jpg\"}");
  }
  return NULL;
}
int main(void) {
  CatItem v[2];
  memset(v, 0, sizeof v);
  snprintf(v[0].imdb, sizeof v[0].imdb, "tmdb:1153576"); snprintf(v[0].tipo, sizeof v[0].tipo, "movie"); v[0].progresso = 9;
  snprintf(v[1].imdb, sizeof v[1].imdb, "tmdb:42:1:3"); snprintf(v[1].tipo, sizeof v[1].tipo, "series"); v[1].progresso = 20;
  v[1].temporada = 1; v[1].episodio = 3;
  int n = trakt_enfeitar_lote(v, 2);
  printf("n=%d poster=%s fundo=%s titulo=%s | %s %s %s\n", n, v[0].poster, v[0].backdrop, v[0].titulo, v[1].poster, v[1].backdrop, v[1].titulo);
  assert(pedidos >= 2);
  assert(!strcmp(v[0].poster, "https://image.tmdb.org/t/p/w342/cartaz.jpg"));
  assert(!strcmp(v[0].backdrop, "https://image.tmdb.org/t/p/w1280/fundo.jpg"));
  assert(!strcmp(v[0].titulo, "Street Fighter") && !strcmp(v[0].imdb, "tmdb:1153576"));
  assert(!strcmp(v[1].poster, "https://image.tmdb.org/t/p/w342/serie.jpg") && !strcmp(v[1].titulo, "Serie"));
  assert(!strcmp(v[1].backdrop, v[1].poster));   /* no backdrop: the poster stands in, as for other ids */
  puts("PASS: Continue Watching entries with a TMDB number get TMDB poster, background and title");
  return 0;
}

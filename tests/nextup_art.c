// Offline regression: completed-card artwork and merged Next Up publication.
#include <assert.h>
#include "progresso.h"
#define NV_TESTE_INEDITO 1
#include "../src/home.c"

// Only persistence and rendering boundaries are stubbed; progress stays in memory.
char *dados_ler(const char *n) { (void)n; return NULL; }
int dados_gravar(const char *n, const char *c) { (void)n; (void)c; return 1; }
int dados_gravar_leve(const char *n, const char *c) { (void)n; (void)c; return 1; }
char *dados_caminho(char *d, unsigned t, const char *n) {
  (void)d; (void)t; (void)n; return NULL;
}
int perfis_ativo(void) { return 1; }
const char *i18n(const char *s) { return s; }
const char *idioma_mes_data(int m, const char *s) { (void)m; return s; }
int tex_falhou(const char *u) { (void)u; return 0; }
float tex_aspecto(const char *u) { (void)u; return 0; }
int tex_largura_fonte(const char *u) { (void)u; return 0; }   /* width unknown yet */
int ajustes_hero_arte_diferente(void) { return 0; }
int ajustes_hero_fonte(void) { return 0; }
int ajustes_cw_thumb_episodio(void) { return 1; }
int ajustes_cw_concluido(void) { return 90; }
// Stage E: whether the card is an unaired next-up (continuar.c); the rule
// itself is covered by tests/stage_e_episodios.c.
static int ineditoTeste;
int continuar_proximo_inedito(const CatItem *ci, long long agoraMs) {
  (void)ci; (void)agoraMs; return ineditoTeste;
}
int main(void) {
  CatItem a = {0};
  snprintf(a.imdb, sizeof a.imdb, "tt200");
  snprintf(a.tipo, sizeof a.tipo, "series");
  // Finish locally while an older tracker card and a catalog copy coexist.
  // Artwork must advance in this session, then survive a merged Next Up publication.
  {
    static CatItem items[2], next;
    static CatEp eps[2];
    CatFileira row = {0};
    char expected[512];
    items[0] = a; items[0].temporada = 1; items[0].episodio = 1;
    items[0].progresso = 30; items[0].retomadoMs = 1000;
    snprintf(items[0].imdb, sizeof items[0].imdb, "tt200:1:1");
    items[1] = items[0];
    snprintf(items[1].imdb, sizeof items[1].imdb, "tt200");
    snprintf(row.chave, sizeof row.chave, "continue_watching"); row.n = 1;
    cat_definir_tudo(items, 2, &row, 1);
    eps[0].temporada = eps[1].temporada = 1;
    eps[0].episodio = 1; eps[1].episodio = 2;
    cat_definir_episodios(1, eps, 2);
    assert(cat_salvar_progresso_ep(1, 3000, 3000, 1, 1, 1));
    next = *cat_item(0); next.episodio = 2; next.progresso = 0;
    snprintf(next.imdb, sizeof next.imdb, "tt200:1:2");
    snprintf(expected, sizeof expected, "%s", artehero_url_episodio(&next));
    assert(!strcmp(arte_card_retomada(cat_item(0)), expected));
    // Nuvio/Trakt/Simkl seeds carry the completion instant of episode 1.
    { ProgRegistro regs[PROG_MAX];
      assert(prog_ler(regs, PROG_MAX) > 0);
      next.retomadoMs = regs[0].lastWatchedMs;
    }
    cat_trocar_continuar(&next, 1);
    assert(cat_item(0)->episodio == 2 && cat_item(0)->progresso == 0);
    assert(!strcmp(arte_card_retomada(cat_item(0)), expected));
    // Genuine playback of the offered episode must still update normally.
    cat_aplicar_progresso(0, 600, 3000, 1, 2);
    assert(cat_item(0)->episodio == 2 && cat_item(0)->progresso == 20);
  }
  // Owner, 13/11/2026 (Abbott Elementary): rewatching an ALREADY-WATCHED
  // episode to the end writes a local record NEWER than the account's Next Up
  // seed. That finished record used to pull the seed back to the finished
  // episode at 100%, and cwInvalido then dropped the card from the row.
  {
    static CatItem items[2], next;
    static CatEp eps[2];
    CatFileira row = {0};
    items[0] = a; items[0].temporada = 1; items[0].episodio = 1;
    items[0].progresso = 30; items[0].retomadoMs = 1000;
    snprintf(items[0].imdb, sizeof items[0].imdb, "tt300:1:1");
    items[1] = items[0];
    snprintf(items[1].imdb, sizeof items[1].imdb, "tt300");
    snprintf(row.chave, sizeof row.chave, "continue_watching"); row.n = 1;
    cat_definir_tudo(items, 2, &row, 1);
    eps[0].temporada = eps[1].temporada = 1;
    eps[0].episodio = 1; eps[1].episodio = 2;
    cat_definir_episodios(1, eps, 2);
    assert(cat_salvar_progresso_ep(1, 3000, 3000, 1, 1, 1));   // E1 replayed to the end, now
    next = items[0]; next.episodio = 2; next.progresso = 0;
    next.retomadoMs = 1000;                                       // seed from the first watch
    snprintf(next.imdb, sizeof next.imdb, "tt300:1:2");
    cat_trocar_continuar(&next, 1);
    assert(!strncmp(cat_item(0)->imdb, "tt300:1:2", 9));
    assert(cat_item(0)->episodio == 2 && cat_item(0)->progresso == 0);
    // A newer PARTIAL replay of an earlier episode still wins, as before.
    assert(cat_salvar_progresso_ep(1, 600, 3000, 1, 1, 1));
    cat_trocar_continuar(&next, 1);
    assert(cat_item(0)->episodio == 1 && cat_item(0)->progresso == 20);
  }
  // "Itens por fileira: Sem limite" (14 Nov 2026): a page appended to a row
  // lands at the END of its window, skips titles the row already has, slides
  // the next row, and stops at the ceiling.
  {
    static CatItem v[5], mais[4];
    CatFileira rows[2];
    memset(rows, 0, sizeof rows);
    for (int i = 0; i < 5; i++) {
      v[i] = a; snprintf(v[i].tipo, sizeof v[i].tipo, "movie");
      snprintf(v[i].imdb, sizeof v[i].imdb, "tt90%d", i);
    }
    snprintf(rows[0].chave, sizeof rows[0].chave, "addon_movie_top"); rows[0].ini = 0; rows[0].n = 3;
    snprintf(rows[1].chave, sizeof rows[1].chave, "addon_movie_new"); rows[1].ini = 3; rows[1].n = 2;
    cat_definir_tudo(v, 5, rows, 2);
    for (int i = 0; i < 4; i++) { mais[i] = v[0]; snprintf(mais[i].imdb, sizeof mais[i].imdb, "tt95%d", i); }
    snprintf(mais[1].imdb, sizeof mais[1].imdb, "tt901");          // already in the row
    snprintf(mais[3].imdb, sizeof mais[3].imdb, "tt950");          // repeated in the page
    assert(cat_anexar_fileira("addon_movie_top", mais, 4, 200) == 2);
    assert(cat_fileira(0)->n == 5 && cat_fileira(1)->ini == 5 && cat_fileira(1)->n == 2);
    assert(!strcmp(cat_item(3)->imdb, "tt950") && !strcmp(cat_item(4)->imdb, "tt952"));
    assert(!strcmp(cat_item(5)->imdb, "tt903"));                   // next row moved, intact
    snprintf(mais[0].imdb, sizeof mais[0].imdb, "tt960"); snprintf(mais[2].imdb, sizeof mais[2].imdb, "tt962");
    assert(cat_anexar_fileira("addon_movie_top", mais, 4, 6) == 1 && cat_fileira(0)->n == 6);   // ceiling
    assert(cat_anexar_fileira("addon_movie_top", mais, 4, 200) == 1);                             // tt962 left
    assert(cat_anexar_fileira("no_such_row", mais, 4, 200) == 0);
  }
  // Stage E (continueWatchingImageModel, card style): an UNAIRED next-up shows
  // the title's landscape art first and its still only as the last resort.
  { CatItem up = a;
    const char *art, *still;
    snprintf(up.imdb, sizeof up.imdb, "tt201:2:1");
    up.temporada = 2; up.episodio = 1; up.progresso = 0;
    snprintf(up.backdrop, sizeof up.backdrop, "https://image.tmdb.org/t/p/w780/bd201.jpg");
    still = artehero_url_episodio(&up);
    ineditoTeste = 0;
    art = arte_card_retomada(&up);
    assert(art && still && !strcmp(art, still));          /* aired: still first */
    ineditoTeste = 1;
    art = arte_card_retomada(&up);
    assert(art && strcmp(art, still) && !strcmp(art, arte_por_formato(&up, 1)));
    /* Stage E: the HERO (Automatic source) of an unaired next-up shows the
     * show's art, never the episode "still" (the add-on's poster before air). */
    ineditoTeste = 1;
    { const char *h = arte_hero_do_item(&up);
      assert(h && strcmp(h, still)); }
    ineditoTeste = 0;
    { const char *h = arte_hero_do_item(&up);
      assert(h && !strcmp(h, still)); }                   /* aired: still first, as before */
    ineditoTeste = 1;
    up.backdrop[0] = 0; up.poster[0] = 0;                 /* no title art at all */
    art = arte_card_retomada(&up);
    assert(!arte_por_formato(&up, 1) ? (art && !strcmp(art, still)) : 1);
    ineditoTeste = 0;
  }
  puts("nextup_art: ok");
  return 0;
}

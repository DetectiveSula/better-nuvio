// DOWN from the hero always lands on the FIRST row (owner, 2026-11-26:
// "sometimes when opening a profile it goes from hero to streaming services
// collection instead of one down"). While a profile opens, the local
// collections row is first; the catalogue rows are then inserted above it and
// the focus follows that row by key, so foco.fileira points further down
// while the hero has focus. DOWN used that index and skipped the new rows.
#include "../src/homevazio.h"
#define homevazio_ativo(n) 0   /* no addons in the fixture: not the empty-home screen */
#include "../src/home.c"
#include <assert.h>
static void baixo(void) {
  SDL_Event e; memset(&e, 0, sizeof e);
  e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_DOWN;
  home_evento_(&e);
}
int main(void) {
  nFileiras = foco.nFileiras = 4;
  for (int r = 0; r < 4; r++) { foco.nColunas[r] = 6; fileiras[r].n = 6; }
  // Focus remapped by key to the collections row, now 4th, hero focused.
  focoHero = 1; foco.fileira = 3; foco.coluna = 2;
  baixo();
  assert(!focoHero && foco.fileira == 0 && foco.coluna == 0);
  // Coming back down to the row it went up from keeps the column.
  focoHero = 1; foco.fileira = 0; foco.coluna = 4;
  baixo();
  assert(!focoHero && foco.fileira == 0 && foco.coluna == 4);
  // An empty first row is skipped, as before.
  foco.nColunas[0] = 0; focoHero = 1; foco.fileira = 3;
  baixo();
  assert(!focoHero && foco.fileira == 1);
  puts("PASS: down from the hero lands one row down (first navigable row), never on a row remapped further down while rows load");
  return 0;
}

// Profile & Stats under the Dinamica layout's bar pill (src/perfil.c), no GL.
// M sweep, 04/10/2026: on the owner's Android TV the "‹ Profile & Stats" pill
// (y 44..104) sat on top of the avatar and the name ("Henrique Rocha" clipped
// at the top). With the pill on screen the whole page has to start 20 px below
// it; without it nothing moves.
#include "../src/perfil.c"
#include "dados.h"
#include <assert.h>

static void layout(int n) {
  char cam[700];
  FILE *f;
  snprintf(cam, sizeof cam, "%s/ajustes.txt", dados_dir());
  f = fopen(cam, "w");
  assert(f);
  fprintf(f, "homeLayoutLocal %d\n", n);
  fclose(f);
  ajustes_dir(dados_dir());
}

int main(void) {
  float px, py, pw, ph;
  dados_iniciar("deploy/app/art");

  layout(2);   // Dinamica: the bar is a pill at the top left
  assert(menu_pilula_rect(&px, &py, &pw, &ph));
  assert(pfDy() > 0.0f);
  assert(PF_TOPO >= py + ph + 20.0f - 0.5f);   // avatar/name start below the pill
  // The 30-day dashboard (perfil_painel.inc) starts below the pill too.
  assert(PN_TOPO >= py + ph + 20.0f - 0.5f);
  assert(rKpi(0).y == PN_KPI_Y);
  assert(PN_KPI_Y - PN_TOPO == PN_AV + 22.0f);
  // The middle row gives up height so the bottom row still fits above the notice.
  assert(rGrafico().y == PN_MEIO_Y && rGrafico().h >= 300.0f);
  assert(rTopo().h >= 200.0f && rTopo().y + rTopo().h <= PF_AVISO_Y);
  assert(rFato(3).y + rFato(3).h <= PF_AVISO_Y);

  layout(1);   // any other layout: the top left is free
  assert(!menu_pilula_rect(&px, &py, &pw, &ph));
  assert(pfDy() == 0.0f);
  assert(PF_TOPO == 56.0f && PN_TOPO == 48.0f && PN_KPI_Y == 154.0f);
  assert(rGrafico().h == 372.0f && rTopo().y + rTopo().h <= PF_AVISO_Y);

  puts("PASS: perfil_pilula");
  return 0;
}

// Exercise production menu routing, geometry and rotation cancellation without
// opening the app, a window, a network connection or an account session.
#define main legacy_menu_main
#include "menu_tvos.c"
#undef main

int main(void) {
  assert(legacy_menu_main() == 0);
  mobileTeste = 1;
  nv_layout_w = 1080; nv_layout_h = 2340;
  for (int layout = 0; layout < 3; layout++) {
    layoutTeste = layout;
    assert(phoneAtivo() && !tvAtivo() && !topo());
    assert(menu_pilula_titulo() && menu_barra_borda() == 0);
  }
  PhoneMenuGeometry full = phone_menu_geometry(1080, 2340, 9);
  assert(full.panel.x >= 0 && full.panel.x + full.panel.w <= 1080);
  assert(full.panel.y + full.panel.h < 2340 && full.maxScroll == 0);
  PhoneMenuGeometry compact = phone_menu_geometry(1080, 1200, 9);
  assert(compact.rowHeight >= 132 && compact.maxScroll > 0);
  assert(compact.rows.y + compact.rows.h <= compact.panel.y + compact.panel.h);

  menu_iniciar(); menu_definir_destino(MENU_INICIO); menu_abrir();
  phoneEscolher(MENU_BIBLIOTECA, 0);
  assert(!menu_aberto() && menu_destino() == MENU_BIBLIOTECA);
  assert(menu_escolheu() && menu_mudou_destino());
  menu_abrir(); phoneEscolher(MENU_RODAPE, 0);
  assert(menu_pediu_trocar() && menu_destino() == MENU_BIBLIOTECA);
  menu_abrir();
  phoneOffset = 0;
  toquerol_vincular(&phoneRolagem, compact.rows, 1, 0, compact.maxScroll, 1, &phoneOffset);
  PonteiroRolagem e = {PONT_ROL_INICIO, 1, 0, 0, compact.rows.x + 10, compact.rows.y + 10};
  assert(phoneRolar(&e));
  e.fase = PONT_ROL_MOVER; e.delta = -10000;
  assert(phoneRolar(&e) && phoneOffset == compact.maxScroll);
  nv_layout_w = 2340; nv_layout_h = 1080;
  assert(!phoneAtivo() && !phoneRolar(&e));
  phoneEscolher(MENU_BUSCAR, 0);  // stale portrait tap after rotation
  assert(menu_destino() == MENU_BIBLIOTECA);
  mobileTeste = 0;
  assert(!phoneAtivo() && tvAtivo()); // saved TV composition returns unchanged
  puts("phone_menu: PASS (TV parity, portrait routing, 44dp targets, short-window scroll, rotation)");
  return 0;
}

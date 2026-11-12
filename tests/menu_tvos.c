// Apple TV (Dinamica) sidebar: x1.2 at UI scale <= 90%, unchanged from 100%;
// Streaming opens past Settings with either D-pad focus or a mobile swipe.
#include "../src/menu.c"
#include <assert.h>
#include <stdio.h>

static int layoutTeste = HOME_LAYOUT_DINAMICA;
int gfx_vidro_camada;
float gfx_tex_aspect_atual;
float gfx_card_forcar_cover_atual;
static int hidePill, modernSidebar=1;
int txt_largura(TxtEstilo estilo,const char *s) { (void)estilo; return strlen(s)*15; }
int gfx_efeitos_minimos(void) { return 0; }
static GfxRect reservedBackdrop;
static int backdropReservations;
void gfx_vidro_fundo_reservar(GfxRect r) { reservedBackdrop=r; backdropReservations++; }
int ajustes_rail_moderna(void){return modernSidebar;}
int ajustes_animacoes_reduzidas(void){return 0;}
int ajustes_menu_ocultar_pilula(void){return hidePill;}
int ajustes_home_layout(void) { return layoutTeste; }
int ajustes_menu_explorar(void) { return 1; }
int ajustes_menu_guia(void) { return 1; }
int ajustes_menu_agenda(void) { return 1; }
int ajustes_menu_perfil(void) { return 1; }
int ajustes_rail_recolhida(void) { return 0; }
int ajustes_menu_topo(void) { return 0; }
static float escalaTeste = 1.0f;
float gfx_escala_ui(void) { return escalaTeste; }
float gfx_escala_txt(void) { return 1.0f; }
float gfx_escala_ui_base(void) { return escalaTeste; }
static int pastasTeste = 3;
int home_streaming_barra(const int **p) { static const int v[3] = {4, 5, 6}; if (p) *p = v; return pastasTeste; }
#ifdef NV_TOUCH_UI
static int mobileTeste;
float nv_layout_w = 1920.0f, nv_layout_h = 1080.0f;
int layout_modo_mobile(void) { return mobileTeste; }
#endif

static float painelReal(float ui) { escalaTeste = ui; return tvPainel().w * menuEscala(); }
static void tecla(SDL_Keycode k) {
  SDL_Event e = {0};
  e.type = SDL_KEYDOWN; e.key.keysym.sym = k; menu_evento(&e);
}
static int visivel(void) { float ys[NV_MENU_FOCOS_TV]; tvLayout(ys, NULL); return ys[MENU_ST0] >= 0.0f; }

int main(void) {
  tvAbre=0;tvPilAlfa=1;hidePill=0;assert(menu_pilula_alfa()>0);
  hidePill=1;assert(menu_pilula_alfa()==0);menu_abrir();assert(menu_aberto());
  menu_fechar();hidePill=0;

  float base = painelReal(1.0f);
  assert(fabsf(base - 355.0f * 0.9f) < 0.01f);
  assert(fabsf(painelReal(1.1f) - base) < 0.01f);
  assert(fabsf(painelReal(0.9f) - base * 0.9f * 1.2f) < 0.01f);
  assert(fabsf(painelReal(0.8f) - base * 0.8f * 1.2f) < 0.01f);
  layoutTeste = HOME_LAYOUT_PADRAO;
  assert(fabsf(menuEscala() - 0.9f * 0.8f) < 0.001f);
  layoutTeste = HOME_LAYOUT_DINAMICA;
  escalaTeste = 1.0f;

  // "Scroll down for streaming" ends the sidebar, right under Settings, only
  // while Streaming is collapsed and there is something to open.
  { float ys[NV_MENU_FOCOS_TV];
    aberto = 1; linha = MENU_AJUSTES; tvLayout(ys, NULL);
    assert(tvDicaY >= 0.0f && fabsf(tvDicaY - (ys[MENU_AJUSTES] + TV_LINHA_H)) < 0.01f);
    linha = MENU_ST0; tvLayout(ys, NULL);
    assert(tvDicaY < 0.0f);
    pastasTeste = 0; linha = MENU_AJUSTES; tvLayout(ys, NULL);
    assert(tvDicaY < 0.0f);
    pastasTeste = 3; }
  aberto = 1; linha = MENU_AJUSTES;
  assert(!visivel());
  tecla(SDLK_DOWN);
  assert(linha == MENU_ST0 && visivel());
  tecla(SDLK_DOWN);
  assert(linha == MENU_ST0 + 1 && visivel());
  tecla(SDLK_UP); tecla(SDLK_UP);
  assert(linha == MENU_AJUSTES && !visivel());
  tecla(SDLK_DOWN); menu_fechar();
  assert(!visivel());
#ifdef NV_TOUCH_UI
  // Swipe from an ordinary destination; touch scrolling must not need focus
  // on Settings. Cover both a fitting panel and one that already scrolls.
  for (int small = 0; small < 2; small++) {
    float ys[NV_MENU_FOCOS_TV];
    nv_layout_h = small ? 500.0f : 1080.0f;
    menu_abrir();
    GfxRect panel = tvPainel();
    tvToqueEscala = menuEscala();
    tvToqueRegiao = (GfxRect){panel.x * tvToqueEscala, panel.y * tvToqueEscala,
                             panel.w * tvToqueEscala, panel.h * tvToqueEscala};
    tvToqueMaximo = fmaxf(0.0f, tvLayout(ys, NULL) - panel.h);
    tvRolar = 0.0f;
    PonteiroRolagem e = {PONT_ROL_INICIO, 1, 0, 0,
                        tvToqueRegiao.x + 10, tvToqueRegiao.y + 10};
    mobileTeste = 0;
    assert(!tvToqueRolar(&e) && !visivel());
    mobileTeste = 1;
    e.eixoY = 0;
    assert(!tvToqueRolar(&e) && !visivel());
    e.eixoY = 1;
    e.x = tvToqueRegiao.x - 1;
    assert(!tvToqueRolar(&e) && !visivel());
    e.x = tvToqueRegiao.x + 10;
    assert(tvToqueRolar(&e));
    e.fase = PONT_ROL_MOVER;
    e.delta = 20;
    tvToqueRolar(&e);
    assert(!visivel());
    if (small) {
      e.delta = -20;
      assert(tvToqueRolar(&e) && !visivel());
    }
    e.delta = -1000;
    assert(tvToqueRolar(&e));
    assert(linha == MENU_ST0 && visivel());
    tvLayout(ys, NULL);
    assert(ys[MENU_ST0] - tvRolar < tvPainel().h);
    assert(ys[MENU_ST0] + TV_LINHA_H - tvRolar > 0);
    // Use the same focus callback and confirmation as a service tap.
    ponteiroLinha(MENU_ST0 + 1, 0);
    tecla(SDLK_RETURN);
    assert(menu_pediu_colecao() == 5 && !aberto && !visivel());
    menu_abrir();
    assert(!visivel());
    pastasTeste = 0;
    e.fase = PONT_ROL_INICIO;
    assert(tvToqueRolar(&e));
    e.fase = PONT_ROL_MOVER;
    tvToqueRolar(&e);
    assert(linha < MENU_ST0 && !visivel());
    pastasTeste = 3;
    menu_fechar();
  }
#endif
  // The Modern sidebar switch changes actual geometry, not just a label.
  layoutTeste=HOME_LAYOUT_MODERNA;modernSidebar=1;
  assert(!padrao() && geoEm(0,1).x>0);
  modernSidebar=0;assert(padrao() && geoEm(0,1).x==0);
  modernSidebar=1;
  // Over details there is no collapsed rail: blur must sample the same
  // footprint on every opening/closing frame, not morph across video holes.
  sobre=semRail=1;MenuGeo overlay=geoEm(expansaoVisual(),1);
  for(int frame=0;frame<=100;frame++){
    expande=frame/100.0f;MenuGeo at=geoEm(expansaoVisual(),1);
    assert(at.x==overlay.x&&at.y==overlay.y&&at.w==overlay.w&&at.h==overlay.h);
  }
  sobre=semRail=0;expande=.3f;assert(expansaoVisual()==expande);
  sobre=semRail=1;modernSidebar=0;assert(expansaoVisual()==expande);
  sobre=semRail=0;modernSidebar=1;
  puts("PASS: details overlay opening/closing keeps one blur footprint; Home morph retained");
  layoutTeste=HOME_LAYOUT_DINAMICA;
  // The Dynamic pill and panel must share one pre-content capture on every
  // route, including hidden pills, details, closing tails and small UI scales.
  for(int scale=0;scale<2;scale++) {
    escalaTeste=scale ? .7f : 1.0f;
    for(int route=0;route<MENU_N;route++) {
      destino=route;
      for(int overlayMode=0;overlayMode<2;overlayMode++) {
        sobre=semRail=overlayMode;
        aberto=0;tvAbre=tvAbreV=0;linha=route;
        backdropReservations=0;menu_preparar_fundo();
        assert(backdropReservations==1);
        GfxRect fixed=reservedBackdrop;
        assert(fixed.h==NV_LAYOUT_REAL_H);
        for(int frame=0;frame<240;frame++) {
          aberto=frame<120;
          menu_atualizar(frame%2 ? .05f : 1.0f/120,frame*17);
          menu_preparar_fundo();
          assert(memcmp(&fixed,&reservedBackdrop,sizeof fixed)==0);
          GfxRect p=tvPilula(),q=tvPainel();
          assert((p.x+p.w)*menuEscala()<=fixed.w);
          assert((q.x+q.w)*menuEscala()<=fixed.w);
        }
      }
    }
  }
  escalaTeste=1;destino=MENU_INICIO;sobre=semRail=0;
  layoutTeste=HOME_LAYOUT_MODERNA;
  puts("PASS: all Dynamic routes reserve one backdrop through repeated opening and closing");
  // Idle open frames must become exactly stable, including the spring velocity.
  aberto=1;expande=0;expandeV=0;desliza=0;
  for(int frame=0;frame<600;frame++)menu_atualizar(1.0f/60.0f,frame*17);
  assert(expande==1 && expandeV==0 && desliza==1);
  MenuGeo stable=geoEm(expande,1);
  for(int frame=0;frame<120;frame++)menu_atualizar(1.0f/60.0f,12000+frame*17);
  MenuGeo later=geoEm(expande,1);
  assert(stable.w==later.w && stable.h==later.h && stable.y==later.y);
  aberto=0;
  for(int frame=0;frame<600;frame++)menu_atualizar(1.0f/60.0f,15000+frame*17);
  assert(expande==0 && expandeV==0 && desliza==0);
  puts("PASS: Modern sidebar geometry switch and settled open/close animation");
  puts("PASS: Apple TV scale, D-pad collapse/expand, mobile swipe and service selection");
  return 0;
}

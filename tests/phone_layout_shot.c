// Isolated native renderer: temporary settings, local fixture art, no app/session.
#include "ajustes.h"
#include "addons.h"
#include "biblioteca.h"
#include "busca.h"
#include "catalogo.h"
#include "dados.h"
#include "gfx.h"
#include "home.h"
#include "layout.h"
#include "menu.h"
#include "ponteiro.h"
#include "tex_cache.h"
#include "text.h"
#include "rede.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *rede_baixar_st(const char *url, int timeout, const char *const *headers, int *status) {
  (void)url; (void)timeout; (void)headers; if (status) *status = 0; return NULL;
}
char *rede_postar_st(const char *url, int timeout, const char *const *headers, const char *body, int *status) {
  (void)url; (void)timeout; (void)headers; (void)body; if (status) *status = 0; return NULL;
}
char *rede_colocar_st(const char *url, int timeout, const char *const *headers, const char *body, int *status) {
  (void)url; (void)timeout; (void)headers; (void)body; if (status) *status = 0; return NULL;
}

static void openMenu(void) { menu_abrir(); }
static void capture(const char *folder, const char *name, int width, int height) {
  unsigned char *pixels = malloc(width * height * 4);
  SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
  assert(pixels && s);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  for (int y = 0; y < height; y++)
    memcpy((char *)s->pixels + y * s->pitch, pixels + (height - y - 1) * width * 4, width * 4);
  char path[1024]; snprintf(path, sizeof path, "%s/%s.png", folder, name);
  assert(IMG_SavePNG(s, path) == 0);
  SDL_FreeSurface(s); free(pixels);
}
int main(int argc, char **argv) {
  assert(argc == 3 && getenv("NUVIO_DADOS"));
  const int sidebar = getenv("NUVIO_SHOT_DYNAMIC_SIDEBAR") != NULL;
  const int width = sidebar ? 960 : 360, height = sidebar ? 540 : 780;
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_Window *window = SDL_CreateWindow("Phone layout renderer", 0, 0, width, height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(window && SDL_GL_CreateContext(window));
  layout_tela_definir(width, height); layout_modo_definir(1);
  glViewport(0, 0, width, height); gfx_tamanho_alvo(width, height);
  assert(gfx_iniciar() && txt_iniciar("deploy/app", 1.0f / 3));
  gfx_icones_dir("deploy/app/art"); tex_iniciar(32); ajustes_iniciar();
  dados_iniciar(getenv("NUVIO_DADOS"));
  char path[1024]; snprintf(path, sizeof path, "%s/ajustes.txt", getenv("NUVIO_DADOS"));
  FILE *f = fopen(path, "w"); assert(f);
  fprintf(f, "interfaceModoLocal 1\nidioma 1\nhomeLayoutLocal %s\nmodernSidebar 0\n", argv[2]);
  if (sidebar) fprintf(f,"modernSidebarBlur 0\nvidroLocal 0\nvidroFoscoLocal 1\nvidroQualidadeLocal 2\n");
  fclose(f);
  ajustes_dir(getenv("NUVIO_DADOS"));
  assert(layout_modo_mobile() && (sidebar || ajustes_rail_largura_fixa() == 0));
  float savedText = ajustes_escala_texto();
  for (int i = 0; !sidebar && i < 2; i++) {
    float preference = i ? 1.3f : .85f;
    gfx_escala_txt_definir(preference);
    assert(fabsf(gfx_escala_txt() - preference * 2) < .001f);
    float previous = gfx_escala_txt_entrar();
    assert(fabsf(gfx_escala_tela_txt() - gfx_escala_txt()) < .001f);
    assert(fabsf(gfx_poster_k() * gfx_escala_txt() - 1) < .001f);
    gfx_escala_sair(previous);
    layout_modo_definir(0);
    assert(fabsf(gfx_escala_txt() - preference) < .001f);
    layout_modo_definir(1);
    layout_tela_definir(height, width);
    assert(fabsf(gfx_escala_txt() - preference) < .001f);
    layout_tela_definir(width, height);
  }
  gfx_escala_txt_definir(savedText);
  assert(home_iniciar("deploy/app/art"));
  AddonRemoto addon = {.nome = "Local render fixture", .url = "http://127.0.0.1:1/manifest.json", .ativo = 1};
  assert(addons_definir_lista(&addon, 1));
  CatItem items[12] = {0}; CatFileira rows[3] = {0};
  const char *names[] = {"Continue Watching", "Popular Movies", "Trending Shows"};
  for (int r = 0; r < 3; r++) {
    snprintf(rows[r].chave, sizeof rows[r].chave, "%s", r == 0 ? "continue_watching" : r == 1 ? "popular_movies" : "trending_series");
    snprintf(rows[r].titulo, sizeof rows[r].titulo, "%s", names[r]);
    rows[r].ini = r * 4; rows[r].n = 4;
    snprintf(rows[r].tipo, sizeof rows[r].tipo, "%s", r == 2 ? "series" : "movie");
    for (int j = 0; j < 4; j++) {
      int i = r * 4 + j; CatItem *item = &items[i];
      snprintf(item->imdb, sizeof item->imdb, "tt900%04d", i);
      snprintf(item->titulo, sizeof item->titulo, "Native phone title %d", i + 1);
      snprintf(item->tipo, sizeof item->tipo, "%s", r == 2 ? "series" : "movie");
      snprintf(item->meta, sizeof item->meta, "2024 · 1h 45m");
      snprintf(item->genero, sizeof item->genero, "Drama · Adventure");
      snprintf(item->sinopse, sizeof item->sinopse, "Your films and shows, with the same Nuvio layout and touch navigation.");
      snprintf(item->backdrop, sizeof item->backdrop, "deploy/app/art/%02d.jpg", i);
      snprintf(item->poster, sizeof item->poster, "deploy/app/art/poster/%02d.jpg", i);
      item->nota = 80; if (!r) item->progresso = 35;
    }
  }
  cat_definir_tudo(items, 12, rows, 3); menu_iniciar(); ponteiro_iniciar();
  ponteiro_teste_toque(1);
  for (int panel = 0; panel < 6; panel++) {
    if (sidebar) menu_iniciar();
    if (panel == 1) menu_abrir();
    if (panel == 2) { menu_fechar(); addons_esquecer(); }
    if (panel == 3) { busca_iniciar(); menu_definir_destino(MENU_BUSCAR); }
    if (panel == 4) { biblioteca_iniciar(); menu_definir_destino(MENU_BIBLIOTECA); }
    if (panel == 5) { ajustes_iniciar(); menu_definir_destino(MENU_AJUSTES); }
    for (int i = 0; i < (sidebar ? 180 : 80); i++) {
      Uint32 now = SDL_GetTicks(); tex_bombear(8); txt_novo_quadro(); tex_novo_quadro();
      gfx_novo_quadro(); ponteiro_quadro(now);
      if (sidebar) {
        if(i==2 || i==100) menu_abrir();
        if(i==65 || i==145) menu_fechar();
        menu_atualizar(.016f,now);
        menu_preparar_fundo();
      }
      glClearColor(.05f, .05f, .06f, 1); glClear(GL_COLOR_BUFFER_BIT);
      if (panel == 3) { busca_atualizar(.016f, now); busca_desenhar(now); }
      else if (panel == 4) { biblioteca_atualizar(.016f, now); biblioteca_desenhar(now); }
      else if (panel == 5) { ajustes_atualizar(.016f, now); ajustes_desenhar(now); }
      else { home_atualizar(.016f, now); home_desenhar(now); }
      ponteiro_borda_esquerda(120, openMenu);
      if (!sidebar) menu_atualizar(.016f, now);
      int sourcesBefore = gfx_vidro_com_fonte;
      menu_desenhar(now);
      if(sidebar && menu_aberto() && i>=4) assert(gfx_vidro_com_fonte > sourcesBefore);
      const char *names[] = {"phone-home", "phone-drawer", "phone-empty", "phone-search", "phone-library", "phone-settings"};
      ponteiro_desenhar(); if (i == 79) capture(argv[1], names[panel], width, height);
      if(sidebar && i==40) { char name[64];snprintf(name,sizeof name,"dynamic-sidebar-page-%d",panel);capture(argv[1],name,width,height); }
      SDL_GL_SwapWindow(window);
    }
  }
  if (sidebar) {
    // Worst-case fallback: frost off, moving focus, and high-contrast page
    // content changing under the panel every frame. The panel's empty margin
    // must not inherit any of those foreground changes.
    setenv("NUVIO_SHOT_VIDRO_FOSCO","0",1);ajustes_teste_vidro_env();
    assert(!ajustes_vidro_fosco());
    menu_iniciar();menu_abrir();
    for(int i=0;i<240;i++)menu_atualizar(.016f,i*16);
    for(int i=0;i<240;i++) {
      Uint32 now=SDL_GetTicks();
      if(i%4==0) {
        SDL_Event key={0};key.type=SDL_KEYDOWN;
        key.key.keysym.sym=(i/32)%2 ? SDLK_UP : SDLK_DOWN;menu_evento(&key);
      }
      menu_atualizar(.016f,now);
      unsigned char reference[4]={0};
      for(int pass=0;pass<2;pass++) {
        txt_novo_quadro();tex_novo_quadro();gfx_novo_quadro();ponteiro_quadro(now);menu_preparar_fundo();
        glClearColor(.06f,.06f,.06f,1);glClear(GL_COLOR_BUFFER_BIT);
        gfx_cor((GfxRect){0,0,300,NV_TELA_H},0,pass,0,1-pass,1);
        menu_desenhar(now);
        unsigned char pixel[4];glReadPixels(20,height-200,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
        if(!pass)memcpy(reference,pixel,4);
        for(int channel=0;channel<3;channel++)if(abs((int)pixel[channel]-reference[channel])>1) {
          fprintf(stderr,"scroll probe frame=%d rgb=%d,%d,%d reference=%d,%d,%d\n",i,pixel[0],pixel[1],pixel[2],reference[0],reference[1],reference[2]);
          capture(argv[1],"scroll-probe",width,height);assert(0);
        }
        SDL_GL_SwapWindow(window);
      }
    }
    puts("dynamic_sidebar: PASS (240 focus-navigation fallback frames block alternating foreground colours)");
  }
  puts(sidebar ? "dynamic_sidebar: PASS (1080 rendered frames, blur source retained through repeated open/close on Home, empty Home, Search, Library, Settings)" : "phone_layout_shot: PASS (portrait typography, saved scale/TV/landscape, Home, drawer, empty setup)");
  return 0;
}

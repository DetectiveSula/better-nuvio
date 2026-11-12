/* Stage H trailer polish (Android): the selected zoom is applied before the
 * first visible frame, the artwork is held until the native layout confirms
 * it, there is no full-frame step or periodic re-apply, and the reveal is
 * sticky. Real src/trailer.c with the video layer stubbed. */
#include "trailer.h"
#include "video.h"
#include "idiomacod.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static Uint32 relogio = 1000;
Uint32 SDL_GetTicks(void) { return relogio; }

static int pronto, tocando, larg, alt, janelaOk, falhou, fim;
static int nJanela, nFonte, nReaplicar;
static int ultSx, ultSy, ultSw, ultSh;
static float zoom = 1.34f;

int  video_iniciar_auto(void) { return 1; }
int  video_tocar(const char *u) { (void)u; return 1; }
void video_parar(void) {}
int  video_pronto(void) { return pronto; }
int  video_tocando(void) { return tocando; }
int  video_ativo(void) { return pronto; }
int  video_largura(void) { return larg; }
int  video_altura(void) { return alt; }
int  video_falhou(void) { return falhou; }
int  video_terminou(void) { return fim; }
int  video_conflito_recurso(void) { return 0; }
void video_bombear(void) {}
void video_volume(int v) { (void)v; }
void video_pausar(int p) { (void)p; }
int  video_janela_pronta(void) { return janelaOk; }
void video_janela(int x, int y, int w, int h) { (void)x; (void)y; (void)w; (void)h; nJanela++; }
void video_janela_fonte(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) {
  (void)dx; (void)dy; (void)dw; (void)dh;
  nFonte++; ultSx = sx; ultSy = sy; ultSw = sw; ultSh = sh;
}
int  video_recorte_fonte(void) { return 1; }
void video_recorte_reaplicar(void) { nReaplicar++; }
float ajustes_trailer_zoom(void) { return zoom; }
float gfx_escala_tela_txt(void) { return 1.0f; }
static int idiomaApp = IDIOMA_EN;
int  ajustes_idioma(void) { return idiomaApp; }
int  trailerfonte_tizen(void) { return 0; }
int  trailerfonte_com_som(int tizen) { (void)tizen; return 1; }

#undef __APPLE__
#include "../src/trailer.c"

static void quadro(Uint32 ms) { relogio += ms; trailer_atualizar(relogio); }

int main(void) {
  GfxRect hero = { 0, 0, 1920, 1080 };

  /* 1. The pure reveal rule. */
  assert(!trailer_portao_android(1, 1, 0, -1, 0));        /* crop not sent yet */
  assert(!trailer_portao_android(0, 0, 0, 100, 100));     /* layout not confirmed */
  assert(trailer_portao_android(0, 1, 0, -1, 10));        /* crop + layout: show */
  assert(trailer_portao_android(1, 0, 1, -1, 0));         /* sticky once shown */
  assert(trailer_portao_android(1, 0, 0, 2500, 0));       /* never confirms: 2.5 s after playing */
  assert(!trailer_portao_android(1, 0, 0, 2499, 5999));
  assert(trailer_portao_android(1, 0, 0, -1, 6000));      /* never plays: 6 s after open */

  /* 2. Cinema zoom: opening shows nothing until the frame size is known. */
  trailer_abrir("https://trailer/a.mp4", hero, 0, 0);
  assert(trailer_aberto() && !trailer_mostra_video());
  assert(nFonte == 0);
  quadro(16);
  assert(!trailer_mostra_video());
  /* Frame size known BEFORE playing: the zoom crop goes out at once, exactly
   * once, and never a full-frame step first (the old jump). */
  pronto = 1; larg = 1920; alt = 800;
  quadro(16);
  assert(nFonte == 1);
  { int sx, sy, sw, sh;
    trailer_recorte(1920, 800, 1.34f, 1920, 1080, &sx, &sy, &sw, &sh);
    assert(ultSx == sx && ultSy == sy && ultSw == sw && ultSh == sh);
    assert(ultSw < 1920 || ultSh < 800); }              /* a real crop, not the full frame */
  /* Still hidden until the native layout confirms it (surface recreation, if
   * any, happens here — behind the artwork). */
  assert(!trailer_mostra_video());
  tocando = 1; quadro(16);
  assert(!trailer_mostra_video());
  janelaOk = 1;
  assert(trailer_mostra_video());
  /* Sticky: a later layout request does not hide the video again. */
  janelaOk = 0;
  assert(trailer_mostra_video());
  /* No periodic re-apply and no second crop while it keeps playing. */
  quadro(1600); quadro(1600); quadro(1600); quadro(1600);
  assert(nFonte == 1 && nReaplicar == 0);

  /* 3. A new source resets the gate; Ultra zoom; layout never confirms ->
   * shown 2.5 s after playing (never a stuck trailer). */
  zoom = 1.55f; pronto = 0; tocando = 0; janelaOk = 0; nFonte = 0;
  trailer_abrir("https://trailer/b.mp4", hero, 0, 0);
  assert(!trailer_mostra_video());
  pronto = 1; quadro(16);
  assert(nFonte == 1 && !trailer_mostra_video());
  tocando = 1; quadro(16);
  quadro(2400); assert(!trailer_mostra_video());
  quadro(200);  assert(trailer_mostra_video());

  /* 4. Original (zoom 1.0): same order, no jump. */
  zoom = 1.0f; pronto = 0; tocando = 0; janelaOk = 0; nFonte = 0;
  trailer_abrir("https://trailer/c.mp4", hero, 0, 0);
  pronto = 1; quadro(16);
  assert(nFonte == 1 && !trailer_mostra_video());
  janelaOk = 1;
  assert(trailer_mostra_video());

  /* 5. Moving the trailer (detail/hero resize) re-crops once, still shown. */
  { int antes = nFonte;
    GfxRect menor = { 100, 100, 960, 540 };
    trailer_rect(menor);
    assert(nFonte == antes + 1 && trailer_mostra_video()); }

  trailer_fechar();

  /* 6. Burned-in subtitles (best effort): the "likely subtitled" signal. */
  assert(trailer_legenda_provavel("ko", "en"));     /* foreign title, English app */
  assert(trailer_legenda_provavel("es", "pt"));
  assert(!trailer_legenda_provavel("en", "en"));    /* English titles: centred */
  assert(!trailer_legenda_provavel("en", "pt"));
  assert(!trailer_legenda_provavel("ja", "ja"));    /* same as the app language */
  assert(!trailer_legenda_provavel("", "en") && !trailer_legenda_provavel(NULL, "en"));

  /* 7. The crop, every zoom mode, 16:9 (IMDb, bars inside) and matted (Apple
   * 2.39) frames, full-screen hero and the wide Default banner. Same zoom and
   * size; only the vertical trim moves up; never a black strip. */
  { static const float zs[] = { 1.15f, 1.34f, 1.55f };
    static const int fr[][2] = { { 1920, 1080 }, { 1920, 804 } };
    static const float ds[][2] = { { 1920, 1080 }, { 1920, 528 } };
    int moved = 0;
    for (int zi = 0; zi < 3; zi++) for (int fi = 0; fi < 2; fi++) for (int di = 0; di < 2; di++) {
      int vw = fr[fi][0], vh = fr[fi][1], a[4], b[4];
      int matted = (float)vw / (float)vh > 1.85f;
      int barra = matted ? 0 : (int)((float)vh * (1.0f - (16.0f / 9.0f) / 2.39f) * 0.5f);
      trailer_recorte_legenda(vw, vh, zs[zi], ds[di][0], ds[di][1], 0, &a[0], &a[1], &a[2], &a[3]);
      trailer_recorte_legenda(vw, vh, zs[zi], ds[di][0], ds[di][1], 1, &b[0], &b[1], &b[2], &b[3]);
      assert(a[0] == b[0] && a[2] == b[2] && a[3] == b[3]);      /* same zoom/size/sides */
      assert(b[1] >= a[1] && b[1] + b[3] <= vh);                 /* moved up only, in frame */
      { int baixo = vh - b[1] - b[3];
        assert(baixo <= (vh - a[1] - a[3]));                     /* never trims more bottom */
        assert(baixo <= barra + 1);                              /* no black strip (16:9) */
        if (matted) assert(baixo <= 1); }                        /* matted: bottom kept */
      if (b[1] != a[1]) moved++;
      { int c[4]; trailer_recorte(vw, vh, zs[zi], ds[di][0], ds[di][1], &c[0], &c[1], &c[2], &c[3]);
        assert(!memcmp(a, c, sizeof c)); }                      /* legenda 0 == the old crop */
    }
    assert(moved > 0);
    /* Ultra on a 16:9 frame: the picture bottom (where subtitles sit) is kept. */
    { int a[4], b[4];
      trailer_recorte_legenda(1920, 1080, 1.55f, 1920, 1080, 0, &a[0], &a[1], &a[2], &a[3]);
      trailer_recorte_legenda(1920, 1080, 1.55f, 1920, 1080, 1, &b[0], &b[1], &b[2], &b[3]);
      assert(b[1] > a[1] && 1080 - b[1] - b[3] <= 138); }
    /* Matted frame in the wide banner: the whole bottom stays. */
    { int b[4];
      trailer_recorte_legenda(1920, 804, 1.34f, 1920, 528, 1, &b[0], &b[1], &b[2], &b[3]);
      assert(b[1] + b[3] >= 802); } }

  /* 8. A foreign-language trailer end to end: one crop, already shifted,
   * still behind the art until the layout confirms (flicker fix intact). */
  zoom = 1.55f; pronto = 0; tocando = 0; janelaOk = 0; nFonte = 0; idiomaApp = IDIOMA_EN;
  trailer_idioma_original("ko");
  trailer_abrir("https://trailer/ko.mp4", hero, 0, 0);
  pronto = 1; larg = 1920; alt = 1080; quadro(16);
  { int b[4];
    trailer_recorte_legenda(1920, 1080, 1.55f, 1920, 1080, 1, &b[0], &b[1], &b[2], &b[3]);
    assert(nFonte == 1 && ultSy == b[1] && ultSh == b[3] && ultSx == b[0] && ultSw == b[2]); }
  assert(!trailer_mostra_video());
  janelaOk = 1; assert(trailer_mostra_video());
  /* the language arriving later never re-crops (no geometry change) */
  trailer_idioma_original("en"); quadro(1600); quadro(1600);
  assert(nFonte == 1 && nReaplicar == 0);
  trailer_fechar();
  /* the same trailer in the app's own language: centred */
  pronto = 0; nFonte = 0; janelaOk = 0;
  trailer_idioma_original("en");
  trailer_abrir("https://trailer/en.mp4", hero, 0, 0);
  pronto = 1; quadro(16);
  { int a[4];
    trailer_recorte(1920, 1080, 1.55f, 1920, 1080, &a[0], &a[1], &a[2], &a[3]);
    assert(nFonte == 1 && ultSy == a[1]); }
  trailer_fechar();
  puts("ok Android trailer: zoom before first frame, art held until layout confirms, no re-apply, sticky; subtitle-safe crop in every zoom mode");
  return 0;
}

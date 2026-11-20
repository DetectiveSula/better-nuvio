// Ver acentocustom.h.
#include "acentocustom.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// --- pure -----------------------------------------------------------------------
void acentocustom_hsv(unsigned rgb, float hsv[3]) {
  float r = ((rgb >> 16) & 255) / 255.0f, g = ((rgb >> 8) & 255) / 255.0f, b = (rgb & 255) / 255.0f;
  float mx = fmaxf(r, fmaxf(g, b)), mn = fminf(r, fminf(g, b)), d = mx - mn, h = 0;
  if (d > 1e-6f) {
    if (mx == r) h = 60.0f * fmodf((g - b) / d, 6.0f);
    else if (mx == g) h = 60.0f * ((b - r) / d + 2.0f);
    else h = 60.0f * ((r - g) / d + 4.0f);
  }
  if (h < 0) h += 360.0f;
  hsv[0] = h; hsv[1] = mx > 1e-6f ? d / mx : 0; hsv[2] = mx;
}
unsigned acentocustom_rgb(const float hsv[3]) {
  float h = fmodf(hsv[0], 360.0f), s = hsv[1], v = hsv[2], c, x, m, r = 0, g = 0, b = 0;
  if (h < 0) h += 360.0f;
  s = s < 0 ? 0 : s > 1 ? 1 : s; v = v < 0 ? 0 : v > 1 ? 1 : v;
  c = v * s; x = c * (1 - fabsf(fmodf(h / 60.0f, 2.0f) - 1)); m = v - c;
  if (h < 60) { r = c; g = x; } else if (h < 120) { r = x; g = c; } else if (h < 180) { g = c; b = x; }
  else if (h < 240) { g = x; b = c; } else if (h < 300) { r = x; b = c; } else { r = c; b = x; }
  return ((unsigned)lroundf((r + m) * 255) << 16) | ((unsigned)lroundf((g + m) * 255) << 8) | (unsigned)lroundf((b + m) * 255);
}
int acentocustom_hex(const char *s, unsigned *rgb) {
  unsigned v = 0;
  int i;
  if (!s) return 0;
  if (*s == '#') s++;
  for (i = 0; i < 6; i++) {
    char c = s[i];
    int d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
    if (d < 0) return 0;
    v = v * 16 + (unsigned)d;
  }
  if (s[6]) return 0;
  *rgb = v;
  return 1;
}

#ifndef NV_ACENTOCUSTOM_SO_REGRAS
#include "ajustes.h"
#include "anim.h"
#include "corviva.h"
#include "escala.h"
#include "gfx.h"
#include "idioma.h"
#include "text.h"

// Zones, top to bottom. SLOTS only in gradient mode.
enum { Z_MODO, Z_SLOTS, Z_AMOSTRAS, Z_MATIZ, Z_SAT, Z_BRILHO, Z_HEX, Z_BOTOES, Z_N };
// The quick swatches of official Nuvio TV (ThemeColorPicker.kt).
static const unsigned AMOSTRAS[9] = { 0xFF6B6B, 0xFFB37A, 0xFFD45C, 0x22D37C, 0x4DE3FF,
                                      0x3185F5, 0xB75AFF, 0xEC70A9, 0xFFFFFF };
// The hex pad: 0-9, A-F, then Clear, Delete, Use color.
#define PAD_TECLAS 16
enum { PAD_LIMPAR = 16, PAD_APAGAR, PAD_USAR, PAD_N };

static int aberto, pronto, gradiente, slot, zona, amostra, botao, hexAberto, hexFoco, hexN;
static unsigned cores[3];
static float hsv[3];            // of the slot being edited (kept so hue survives grey)
static char hex[8];
static float entrada;

static unsigned corAtual(void) { return cores[gradiente ? slot : 1]; }
static void definir(unsigned c) {
  if (gradiente) cores[slot] = c;
  else cores[0] = cores[1] = cores[2] = c;
}
static void lerHsv(void) { acentocustom_hsv(corAtual(), hsv); }

void acentocustom_abrir(const unsigned c[3], int grad) {
  memcpy(cores, c, sizeof cores);
  gradiente = grad ? 1 : 0;
  if (!gradiente) cores[0] = cores[2] = cores[1];
  slot = 0; zona = Z_MODO; amostra = 0; botao = 1; hexAberto = 0; pronto = 0;
  entrada = 0;
  aberto = 1;
  lerHsv();
}
int acentocustom_aberto(void) { return aberto; }
void acentocustom_fechar(void) { aberto = 0; hexAberto = 0; }
int acentocustom_resultado(unsigned c[3], int *grad) {
  if (!pronto) return 0;
  pronto = 0;
  memcpy(c, cores, sizeof cores);
  if (grad) *grad = gradiente;
  return 1;
}
#ifdef AJUSTES_TESTE
void acentocustom_teste(int z, int s, int h) {
  zona = z; slot = s; lerHsv();
  if (h) { hexAberto = 1; hexFoco = 10; snprintf(hex, sizeof hex, "%06X", corAtual()); hexN = 6; }
}
#endif

static int zonaVale(int z) { return z != Z_SLOTS || gradiente; }
static void mover(int d) {
  int z = zona + d;
  while (z >= 0 && z < Z_N && !zonaVale(z)) z += d;
  if (z >= 0 && z < Z_N) zona = z;
}
static void ajustar(int canal, int dir, int rapido) {
  float passo = canal == 0 ? 3.0f : 0.01f;
  if (rapido) passo *= 3.0f;
  hsv[canal] += passo * (float)dir;
  if (canal == 0) { if (hsv[0] < 0) hsv[0] = 0; if (hsv[0] > 359) hsv[0] = 359; }
  else { if (hsv[canal] < 0) hsv[canal] = 0; if (hsv[canal] > 1) hsv[canal] = 1; }
  definir(acentocustom_rgb(hsv));
}

static void eventoHex(SDL_Keycode k, int ok) {
  int col = hexFoco < PAD_TECLAS ? hexFoco % 8 : hexFoco - PAD_TECLAS, lin = hexFoco < PAD_TECLAS ? hexFoco / 8 : 2;
  if (k == SDLK_ESCAPE || k == SDLK_AC_BACK) { hexAberto = 0; return; }
  // The remote's number keys type straight in.
  if (k >= SDLK_0 && k <= SDLK_9 && hexN < 6) { hex[hexN++] = (char)('0' + (k - SDLK_0)); hex[hexN] = 0; return; }
  if (k == SDLK_LEFT) { if (col > 0) col--; }
  else if (k == SDLK_RIGHT) { if (col < (lin < 2 ? 7 : 2)) col++; }
  else if (k == SDLK_UP) { if (lin > 0) lin--; }
  else if (k == SDLK_DOWN) { if (lin < 2) lin++; }
  else if (ok) {
    if (hexFoco < PAD_TECLAS) {
      // The first key replaces the code shown (official: "Your first key
      // replaces the current code").
      if (hexN == 6 && !hex[7]) { hexN = 0; hex[0] = 0; }
      hex[7] = 1;
      if (hexN < 6) { hex[hexN++] = "0123456789ABCDEF"[hexFoco]; hex[hexN] = 0; }
    } else if (hexFoco == PAD_LIMPAR) { hexN = 0; hex[0] = 0; hex[7] = 1; }
    else if (hexFoco == PAD_APAGAR) { if (hexN) hex[--hexN] = 0; hex[7] = 1; }
    else {
      unsigned c;
      if (hexN == 6 && acentocustom_hex(hex, &c)) { definir(c); lerHsv(); hexAberto = 0; }
    }
    return;
  }
  if (lin < 2) { if (col > 7) col = 7; hexFoco = lin * 8 + col; }
  else { if (col > 2) col = 2; hexFoco = PAD_TECLAS + col; }
}

void acentocustom_evento(const SDL_Event *e) {
  SDL_Keycode k;
  int ok, dir;
  if (!aberto || !e || e->type != SDL_KEYDOWN) return;
  k = e->key.keysym.sym;
  ok = k == SDLK_RETURN || k == SDLK_KP_ENTER;
  if (hexAberto) { eventoHex(k, ok); return; }
  if (k == SDLK_ESCAPE || k == SDLK_AC_BACK || k == SDLK_BACKSPACE) { aberto = 0; return; }
  if (k == SDLK_UP) { mover(-1); return; }
  if (k == SDLK_DOWN) { mover(1); return; }
  dir = k == SDLK_LEFT ? -1 : k == SDLK_RIGHT ? 1 : 0;
  switch (zona) {
    case Z_MODO:
      if (dir || ok) {
        int g = dir ? dir < 0 : !gradiente;
        if (g != gradiente) {
          gradiente = g;
          if (!gradiente) cores[0] = cores[2] = cores[1];
          slot = 0; lerHsv();
        }
      }
      break;
    case Z_SLOTS:
      if (dir) { slot += dir; if (slot < 0) slot = 0; if (slot > 2) slot = 2; lerHsv(); }
      break;
    case Z_AMOSTRAS:
      if (dir) { amostra += dir; if (amostra < 0) amostra = 0; if (amostra > 8) amostra = 8; }
      else if (ok) { definir(AMOSTRAS[amostra]); lerHsv(); }
      break;
    case Z_MATIZ: case Z_SAT: case Z_BRILHO:
      if (dir) ajustar(zona - Z_MATIZ, dir, e->key.repeat);
      break;
    case Z_HEX:
      if (ok) { hexAberto = 1; hexFoco = 0; snprintf(hex, sizeof hex, "%06X", corAtual()); hexN = 6; hex[7] = 0; }
      break;
    case Z_BOTOES:
      if (dir) botao = dir > 0;
      else if (ok) { if (botao) pronto = 1; aberto = 0; }
      break;
  }
}

void acentocustom_atualizar(float dt) {
  if (!aberto) return;
  entrada = anim_reduzida(anim_mola(entrada, 1.0f, dt, NV_MOLA_TELA), 1.0f, ajustes_animacoes_reduzidas());
}

// --- drawing ----------------------------------------------------------------------------
#define AC_TX 243, 242, 239, 255
static void rgbf(unsigned c, float o[3]) { o[0] = ((c >> 16) & 255) / 255.0f; o[1] = ((c >> 8) & 255) / 255.0f; o[2] = (c & 255) / 255.0f; }
static float lum(unsigned c) {
  float f[3], l[3];
  rgbf(c, f);
  for (int i = 0; i < 3; i++) l[i] = f[i] <= 0.04045f ? f[i] / 12.92f : powf((f[i] + 0.055f) / 1.055f, 2.4f);
  return 0.2126f * l[0] + 0.7152f * l[1] + 0.0722f * l[2];
}
// Ink on a fill: the app's dark ink (#121316) on light colours, white on deep ones.
static int tinta(unsigned c) {
  float y = lum(c), escura = (y + 0.05f) / (0.0065f + 0.05f), branca = 1.05f / (y + 0.05f);
  return branca >= escura ? 255 : 18;
}
static TxtLinha tx(TxtEstilo e, const char *s, int foco) {
  int t = ajustes_tinta_foco();
  return foco ? txt_linha(e, s, t, t, t, 255) : txt_linha(e, s, AC_TX);
}
static void chip(GfxRect r, int foco, float a) {
  if (foco) ajustes_ui_pilula_acento(r, r.h * 0.5f > 32 ? 22 : r.h * 0.5f, a);
  else ajustes_ui_chip(r, r.h * 0.5f > 32 ? 22 : r.h * 0.5f, 0.07f, a);
}
static void disco(float cx, float cy, float d, unsigned c, float a) {
  float f[3];
  rgbf(c, f);
  gfx_cor((GfxRect){ cx - d * 0.5f, cy - d * 0.5f, d, d }, 0.5f, f[0], f[1], f[2], a);
}
// The theme's colours as the app paints a focused surface: flat, or the
// gradient program with the three stops (as novidades148.c previews it).
static void superficie(GfxRect r, float raio, float a) {
  if (!gradiente) { float f[3]; rgbf(cores[1], f); gfx_cor(r, raio / r.h, f[0], f[1], f[2], a); return; }
  { float guarda[3][3];
    memcpy(guarda, nv_grad_viva, sizeof guarda);
    for (int i = 0; i < 3; i++) rgbf(cores[i], nv_grad_viva[i]);
    gfx_rect(r, 0, GFX_COR_GRAD, 0, 0, 0, raio / r.h, 1, 1, 1, a);
    memcpy(nv_grad_viva, guarda, sizeof guarda); }
}
// A horizontal colour bar made of thin slices.
static void barra(GfxRect r, int canal, float a) {
  const int N = 48;
  float w = r.w / (float)N, h[3] = { hsv[0], hsv[1], hsv[2] };
  for (int i = 0; i < N; i++) {
    float t = ((float)i + 0.5f) / (float)N, f[3];
    if (canal == 0) { h[0] = t * 360.0f; h[1] = 1; h[2] = 1; }
    else if (canal == 1) { h[1] = t; h[2] = 1; }
    else { h[2] = t; }
    rgbf(acentocustom_rgb(h), f);
    gfx_cor((GfxRect){ r.x + (float)i * w, r.y, w + 0.6f, r.h }, 0, f[0], f[1], f[2], a);
  }
}
static void slider(GfxRect r, int canal, const char *rot, const char *val, int foco, float a) {
  TxtLinha t = tx(TXT_ILHA_ITEM, rot, foco), v = tx(TXT_V2_24, val, foco);
  GfxRect b = { r.x + 26, r.y + r.h - 30, r.w - 52, 12 };
  float pos;
  chip(r, foco, a);
  txt_desenhar_alpha(t, r.x + 26, r.y + 12, a);
  txt_desenhar_alpha(v, r.x + r.w - 26 - v.w, r.y + 14, (foco ? 0.85f : 0.6f) * a);
  barra(b, canal, a);
  pos = canal == 0 ? hsv[0] / 359.0f : hsv[canal];
  { float kx = b.x + pos * b.w, ky = b.y + b.h * 0.5f;
    gfx_cor((GfxRect){ kx - 13, ky - 13, 26, 26 }, 0.5f, 0, 0, 0, 0.6f * a);
    gfx_cor((GfxRect){ kx - 11, ky - 11, 22, 22 }, 0.5f, 1, 1, 1, a); }
}

static void desenharHex(float a) {
  GfxRect m = { (NV_VTELA_W - 820) * 0.5f, (NV_VTELA_H - 476) * 0.5f, 820, 476 };
  float x = m.x + 44, y = m.y + 44, kw = (m.w - 88 - 7 * 10) / 8.0f;
  char mostra[16];
  ajustes_ui_veu();
  ajustes_ui_ilha_a(m, 36, 1, a);
  ajustes_ui_kicker("Código hex", x, y, a);
  y += 34;
  { TxtLinha l = txt_linha(TXT_V2_24, "Escolha seis caracteres (0–9, A–F). A primeira tecla troca o código atual.", AC_TX);
    txt_desenhar_alpha(l, x, y, 0.62f * a); y += 46; }
  snprintf(mostra, sizeof mostra, "#%-6s", hex);
  { unsigned c;
    TxtLinha l = txt_linha(TXT_ILHA_PERGUNTA, mostra, AC_TX);
    if (hexN == 6 && acentocustom_hex(hex, &c)) disco(x + 24, y + 26, 44, c, a);
    txt_desenhar_alpha(l, x + 64, y + (52 - l.h) * 0.5f, a); y += 76; }
  for (int i = 0; i < PAD_TECLAS; i++) {
    char s[2] = { "0123456789ABCDEF"[i], 0 };
    GfxRect r = { x + (float)(i % 8) * (kw + 10), y + (float)(i / 8) * 72, kw, 62 };
    TxtLinha t = tx(TXT_ILHA_ITEM, s, hexFoco == i);
    chip(r, hexFoco == i, a);
    txt_desenhar_alpha(t, r.x + (r.w - t.w) * 0.5f, r.y + (r.h - t.h) * 0.5f, a);
  }
  y += 2 * 72 + 8;
  { static const char *const rot[3] = { "Limpar", "Apagar", "Usar cor" };
    static const char *const ic[3] = { "aj_x", "aj_delete", "aj_check" };
    float bx = x;
    for (int i = 0; i < 3; i++) bx += ajustes_ui_botao(rot[i], ic[i], bx, y, hexFoco == PAD_TECLAS + i) + 12; }
}

void acentocustom_desenhar(void) {
  GfxRect m = { (NV_VTELA_W - 1440) * 0.5f, (NV_VTELA_H - 900) * 0.5f, 1440, 900 };
  float a = entrada, x, y, lw, rx, rw;
  char v[32];
  if (!aberto) return;
  // A step darker than Settings' veil: the option list behind is all text, and
  // through the glass it read as a second layer of words.
  gfx_cor((GfxRect){ 0, 0, NV_VTELA_W, NV_VTELA_H }, 0, 0, 0, 0, 0.66f * a);
  ajustes_ui_ilha_a(m, 36, 1, a);
  x = m.x + 44; y = m.y + 44;
  ajustes_ui_kicker("Cor de destaque · Personalizada", x, y, a);
  y += 30;
  { TxtLinha t = txt_linha(TXT_ILHA_PERGUNTA, gradiente ? "Seu degradê" : "Sua cor", AC_TX); txt_desenhar_alpha(t, x, y, a); y += 54; }
  { TxtLinha t = txt_linha(TXT_AJ_TEXTO, gradiente ? "Escolha três cores para o tema. Selecione uma cor para editá-la."
                                                   : "Escolha uma cor para o tema.", AC_TX);
    txt_desenhar_alpha(t, x, y, 0.62f * a); y += 52; }
  // Gradient | Single color.
  { static const char *const rot[2] = { "Degradê", "Uma cor" };
    float bx = x;
    int foco = zona == Z_MODO;
    for (int i = 0; i < 2; i++) {
      int sel = (i == 0) == gradiente;
      TxtLinha t = tx(TXT_V2_SEG, rot[i], foco && sel);
      GfxRect r = { bx, y, 44 + (float)t.w + (sel ? 34 : 0), 56 };
      if (foco && sel) ajustes_ui_pilula_acento(r, 28, a);
      else ajustes_ui_chip(r, 28, sel ? 0.16f : 0.06f, a);
      if (sel) { float ti = foco ? ajustes_tinta_foco() / 255.0f : 0.953f; gfx_icone((GfxRect){ r.x + 18, r.y + 17, 22, 22 }, "aj_check", ti, ti, ti, a); }
      txt_desenhar_alpha(t, r.x + 22 + (sel ? 34 : 0), r.y + (56 - t.h) * 0.5f, (sel ? 1.0f : 0.6f) * a);
      bx += r.w + 10;
    }
    y += 56 + 22; }
  lw = 820; rx = x + lw + 40; rw = m.x + m.w - 44 - rx;
  { float top = y;
    // Color 1 / 2 / 3.
    if (gradiente) {
      float sw = (lw - 2 * 10) / 3.0f;
      for (int i = 0; i < 3; i++) {
        GfxRect r = { x + (float)i * (sw + 10), y, sw, 62 };
        int foco = zona == Z_SLOTS && slot == i;
        char s[24];
        snprintf(s, sizeof s, i18n("Cor %d"), i + 1);
        chip(r, foco, a);
        if (!foco && slot == i) gfx_anel_fora(r, 0.5f, 2, 2, 1, 1, 1, 0.5f * a);
        disco(r.x + 26 + 13, r.y + 31, 26, cores[i], a);
        { TxtLinha t = tx(TXT_ILHA_ITEM, s, foco); txt_desenhar_alpha(t, r.x + 26 + 26 + 14, r.y + (62 - t.h) * 0.5f, a); }
      }
      y += 62 + 18;
    }
    // Nine quick swatches.
    { float d = 56, gap = (lw - 9 * d) / 8.0f;
      for (int i = 0; i < 9; i++) {
        float cx = x + d * 0.5f + (float)i * (d + gap), cy = y + d * 0.5f, s = zona == Z_AMOSTRAS && amostra == i ? 1.12f : 1.0f;
        float ar, ag, ab;
        ajustes_acento(&ar, &ag, &ab);
        disco(cx, cy, d * s, AMOSTRAS[i], a);
        if (AMOSTRAS[i] == corAtual()) gfx_icone((GfxRect){ cx - 13, cy - 13, 26, 26 }, "aj_check",
                                                 tinta(AMOSTRAS[i]) / 255.0f, tinta(AMOSTRAS[i]) / 255.0f, tinta(AMOSTRAS[i]) / 255.0f, a);
        if (zona == Z_AMOSTRAS && amostra == i)
          gfx_anel_fora((GfxRect){ cx - d * s * 0.5f, cy - d * s * 0.5f, d * s, d * s }, 0.5f, 4, 3, ar, ag, ab, a);
      }
      y += d + 22; }
    snprintf(v, sizeof v, "%d°", (int)lroundf(hsv[0]));
    slider((GfxRect){ x, y, lw, 84 }, 0, "Matiz", v, zona == Z_MATIZ, a); y += 84 + 10;
    snprintf(v, sizeof v, "%d%%", (int)lroundf(hsv[1] * 100));
    slider((GfxRect){ x, y, lw, 84 }, 1, "Saturação", v, zona == Z_SAT, a); y += 84 + 10;
    snprintf(v, sizeof v, "%d%%", (int)lroundf(hsv[2] * 100));
    slider((GfxRect){ x, y, lw, 84 }, 2, "Brilho", v, zona == Z_BRILHO, a); y += 84 + 10;
    { GfxRect r = { x, y, lw, 60 };
      int foco = zona == Z_HEX;
      TxtLinha t = tx(TXT_ILHA_ITEM, "Digitar código hex", foco);
      char h[12];
      chip(r, foco, a);
      { float ti = foco ? ajustes_tinta_foco() / 255.0f : 0.953f; gfx_icone((GfxRect){ r.x + 24, r.y + 19, 22, 22 }, "aj_pencil", ti, ti, ti, a); }
      txt_desenhar_alpha(t, r.x + 24 + 22 + 14, r.y + (60 - t.h) * 0.5f, a);
      snprintf(h, sizeof h, "#%06X", corAtual());
      { TxtLinha c = tx(TXT_ILHA_ITEM, h, foco);
        txt_desenhar_alpha(c, r.x + r.w - 24 - c.w, r.y + (60 - c.h) * 0.5f, a); } }
    // Preview: what a focused item looks like with this theme.
    { GfxRect p = { rx, top, rw, y + 60 - top };
      GfxRect amostra = { p.x + 34, p.y + 96, p.w - 68, 64 };
      unsigned meio = gradiente ? cores[1] : cores[1];
      int ti = tinta(meio);
      ajustes_ui_chip(p, 24, 0.05f, a);
      ajustes_ui_kicker("PRÉVIA", p.x + 34, p.y + 34, a);
      superficie(amostra, 32, a);
      { TxtLinha t = txt_linha(TXT_ILHA_ITEM, "Item em foco", ti, ti, ti, 255);
        txt_desenhar_alpha(t, amostra.x + (amostra.w - t.w) * 0.5f, amostra.y + (64 - t.h) * 0.5f, a); }
      // A card with the focus ring, and a chip in the theme's colour.
      { GfxRect card = { p.x + 34, amostra.y + 64 + 40, 150, 225 };
        float f[3];
        gfx_cor(card, 12.0f / card.h, 0.16f, 0.17f, 0.2f, a);
        rgbf(cores[gradiente ? 0 : 1], f);
        gfx_anel_fora(card, 12.0f / card.h, 5, 4, f[0], f[1], f[2], a);
        { TxtLinha t = txt_linha(TXT_V2_24, "Cartão em foco", AC_TX); txt_desenhar_alpha(t, card.x + card.w + 28, card.y + 20, 0.8f * a); }
        { GfxRect c = { card.x + card.w + 28, card.y + 64, 190, 48 };
          float cf[3];
          rgbf(cores[gradiente ? 2 : 1], cf);
          gfx_cor(c, 0.5f, cf[0], cf[1], cf[2], 0.22f * a);
          { TxtLinha t = txt_linha(TXT_V2_24, "Ligado", (int)(cf[0] * 255), (int)(cf[1] * 255), (int)(cf[2] * 255), 255);
            txt_desenhar_alpha(t, c.x + (c.w - t.w) * 0.5f, c.y + (48 - t.h) * 0.5f, a); } } } }
    y += 60 + 26;
  }
  // Footer: the remote hint, Cancel, Save theme.
  { const char *k[3] = { "← →", "↑ ↓", "Voltar" }, *l[3] = { "Ajustar", "Mover", "Cancelar" };
    float by = m.y + m.h - 44 - 60, bs = 28 + 32 + (float)txt_largura(TXT_ILHA_ITEM, "Salvar tema") + 28,
          bc = 28 + 32 + (float)txt_largura(TXT_ILHA_ITEM, "Cancelar") + 28;
    ajustes_ui_dicas_modal(k, l, 3, x, by + 30, a);
    ajustes_ui_botao("Cancelar", "aj_x", m.x + m.w - 44 - bs - 14 - bc, by, zona == Z_BOTOES && !botao);
    ajustes_ui_botao("Salvar tema", "aj_check", m.x + m.w - 44 - bs, by, zona == Z_BOTOES && botao); }
  if (hexAberto) desenharHex(a);
}
#endif

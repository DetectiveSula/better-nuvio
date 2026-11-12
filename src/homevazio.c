#include <stdlib.h>
#include <string.h>
#include "homevazio.h"
#include "addons.h"
#include "sessao.h"
#include "sync.h"
#include "gfx.h"
#include "text.h"
#include "idioma.h"
#include "layout.h"
#include "qrtex.h"
#include "descoberta.h"
#include "ponteiro.h"
#include "telefoneui.h"

#define HV_W 1240.0f
#define HV_H 520.0f
#define HV_QR 340.0f
int homevazio_ativo(int fileiras) {
  if (sessao_logada() && sync_estado() != SYNC_PRONTO && !sync_ultimo_ok()) return 0;
  if (!addons_n()) return 1;
  if (fileiras || desc_montando()) return 0;
  for (int i = 0; i < addons_n(); i++)
    if (addons_tem_catalogo(i)) return 0;
  return 1;
}

static GLuint texQr;
static GLuint qrTextura(void) {
  if (!texQr) qr_textura(&texQr, HOMEVAZIO_URL);
  return texQr;
}

static void ok(int a, int b) { (void)a; (void)b; sync_manual_addons(); }
static void phoneGuide(int a, int b) { (void)a; (void)b; SDL_OpenURL(HOMEVAZIO_URL); }

static void phoneEmpty(float alfa) {
  float w = NV_TELA_W - 120, x = 60;
  float height = 780, y = fmaxf(240, (NV_TELA_H - height) * .42f);
  GfxRect panel = {x, y, w, height};
  gfx_cor(panel, 48 / height, .12f, .12f, .14f, alfa);
  TxtLinha title = txt_linha_corta(TXT_TITULO3, "Sua Home está vazia", 245, 245, 247, 255, w - 96);
  txt_desenhar_alpha(title, x + 48, y + 60, alfa);
  txt_bloco(TXT_HEADLINE, "Set up add-ons in your Nuvio account to see movies and shows.",
    190, 193, 203, x + 48, y + 172, w - 96, 56, alfa, 4);
  GfxRect guide = {x + 48, y + height - 344, w - 96, 132};
  GfxRect sync = {x + 48, y + height - 180, w - 96, 132};
  gfx_vidro_pilula_cheia(guide, .5f, 1, alfa);
  gfx_cor(sync, .5f, .21f, .21f, .24f, alfa);
  TxtLinha g = txt_linha_corta(TXT_HEADLINE, "Setup guide", 24, 24, 28, 255, guide.w - 64);
  TxtLinha s = txt_linha_corta(TXT_HEADLINE, "Sincronizar addons", 245, 245, 247, 255, sync.w - 64);
  txt_desenhar_alpha(g, guide.x + (guide.w - g.w) / 2, guide.y + (guide.h - g.h) / 2, alfa);
  txt_desenhar_alpha(s, sync.x + (sync.w - s.w) / 2, sync.y + (sync.h - s.h) / 2, alfa);
  ponteiro_alvo(guide.x, guide.y, guide.w, guide.h, NULL, phoneGuide, 0, 0);
  ponteiro_alvo(sync.x, sync.y, sync.w, sync.h, NULL, ok, 0, 0);
}

void homevazio_desenhar(float alfa) {
  if (telefoneui_ativo() && NV_TELA_H > NV_TELA_W) {
    float scale = gfx_escala(); gfx_escala_sair(1);
    phoneEmpty(alfa); gfx_escala_sair(scale); return;
  }
  GfxRect p = { (NV_TELA_W - HV_W) * 0.5f, (NV_TELA_H - HV_H) * 0.5f + 40.0f, HV_W, HV_H };
  GfxRect qr = { p.x + p.w - HV_QR - 56.0f, p.y + (p.h - HV_QR) * 0.5f, HV_QR, HV_QR };
  float tx = p.x + 64.0f, tw = qr.x - tx - 56.0f, y = p.y + 72.0f;
  const char *rot = i18n("Sincronizar addons");
  GfxRect b;
  GLuint t;
  TxtLinha l;
  if (alfa <= 0.003f) return;
  gfx_vidro_material(p, 0.06f, GFX_VIDRO_DENS, alfa);
  l = txt_linha_corta(TXT_HEADLINE, "Sua Home está vazia", 255, 255, 255, 255, tw);
  txt_desenhar_alpha(l, tx, y, alfa);
  y += l.h + 22.0f;
  y += txt_bloco(TXT_BODY, "Configure seus add-ons no celular, na sua conta Nuvio. A TV acompanha sozinha.",
                200, 203, 210, tx, y, tw, 40, alfa, 3) + 14.0f;
  txt_bloco(TXT_CAPTION, "Aponte a câmera do celular para o QR code e siga o guia.",
            160, 163, 172, tx, y, tw, 34, alfa, 2);
  l = txt_linha(TXT_BODY, rot, 24, 24, 26, 255);
  b = (GfxRect){ tx, p.y + p.h - 64.0f - 76.0f, (float)l.w + 88.0f, 76.0f };
  gfx_vidro_pilula_cheia(b, 0.5f, 1.0f, alfa);   // sempre focado: e o unico alvo
  txt_desenhar_alpha(l, b.x + (b.w - l.w) * 0.5f, b.y + (b.h - l.h) * 0.5f, alfa);
  ponteiro_alvo(b.x, b.y, b.w, b.h, NULL, ok, 0, 0);
  gfx_cor((GfxRect){ qr.x - 14.0f, qr.y - 14.0f, qr.w + 28.0f, qr.h + 28.0f }, 0.05f, 1, 1, 1, alfa);
  t = qrTextura();
  if (t) {
    gfx_tex_aspect_atual = 0.0f;
    gfx_rect(qr, t, GFX_SNAP, 0, 0, 0, 0, 0, 0, 0, alfa);
  }
}

int homevazio_evento(const SDL_Event *e) {
  SDL_Keycode k;
  if (e->type != SDL_KEYDOWN && e->type != SDL_KEYUP) return 0;
  k = e->key.keysym.sym;
  if (k != SDLK_RETURN && k != SDLK_KP_ENTER && k != SDLK_SPACE) return 0;
  if (e->type == SDL_KEYDOWN && !e->key.repeat) ok(0, 0);
  return 1;
}

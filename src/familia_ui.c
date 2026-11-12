// Stage H Family Watching — Option D: avatar and name (top left), status pill
// (top right), title, "S2 E5 · 22m left" line, progress bar and a play/check
// icon over the bottom veil. Everything reads the FamCartao kept by
// familia_app.c, never the CatItem progress (the item stays neutral, so the
// details page under the current profile never sees another profile's state).
#include "familia_ui.h"
#include "familia.h"
#include "ajustes.h"
#include "badges.h"
#include "continuar.h"
#include "layout.h"
#include "text.h"
#include "tex_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int familia_perfil_do_item(const CatItem *ci) {
  if (!ci || strncmp(ci->socialSlug, "familia:", 8)) return 0;
  return atoi(ci->socialSlug + 8);
}

static void corDoPerfil(const char *hex, float *r, float *g, float *b) {
  unsigned v = 0x3B4252;
  if (hex && hex[0] == '#' && strlen(hex) >= 7) v = (unsigned)strtoul(hex + 1, NULL, 16) & 0xFFFFFF;
  *r = ((v >> 16) & 255) / 255.0f; *g = ((v >> 8) & 255) / 255.0f; *b = (v & 255) / 255.0f;
}

void familia_desenhar_cartao(const CatItem *ci, GfxRect r, float raio, float foco) {
  FamCartao c;
  char pilula[64], linha[96];
  int perfil = familia_perfil_do_item(ci);
  int estilo = ajustes_amigos_estilo();
  // Portrait/presence layouts keep TV-readable type rather than shrinking
  // every label with their narrower artwork.
  float esc = r.w / familia_estilo_largura(estilo);
  float pad = NV_CW_PAD * esc, largura = r.w - pad * 2;
  (void)foco;
  if (!ci) return;
  if (!perfil || !familia_cartao_do_perfil(perfil, &c)) memset(&c, 0, sizeof c);
  gfx_veu_base(r, raio, NV_CW_VEU_F, NV_CW_VEU_A);

  // Top left: who. A dark chip keeps the name legible over any artwork.
  if (c.perfil) {
    float d = 30.0f * esc, chipPad = 6.0f * esc;
    TxtLinha nome = txt_linha_corta(TXT_CAPTION2, c.nome, 245, 246, 248, 255, largura * 0.55f);
    GfxRect chip = { r.x + pad, r.y + pad, d + chipPad * 3 + nome.w, d + chipPad * 2 };
    GfxRect disco = { chip.x + chipPad, chip.y + chipPad, d, d };
    GLuint foto = c.avatar[0] ? tex_obter_larg(c.avatar, d * 2) : 0;
    float cr, cg, cb;
    gfx_cor(chip, 0.5f, 0.04f, 0.045f, 0.055f, 0.72f);
    corDoPerfil(c.cor, &cr, &cg, &cb);
    gfx_rect(disco, 0, GFX_DISCO, 0, 0, 0, 0, cr, cg, cb, 1);
    if (foto) {
      gfx_tex_aspect_atual = tex_aspecto(c.avatar);
      gfx_rect(disco, foto, GFX_AVATAR, 0, 0, 0, 0, 1, 1, 1, 1);
      gfx_tex_aspect_atual = 0;
    } else if (c.nome[0]) {
      char ini[8]; size_t z = 1;
      while (z < 4 && (c.nome[z] & 0xc0) == 0x80) z++;
      memcpy(ini, c.nome, z); ini[z] = 0;
      TxtLinha l = txt_linha(TXT_CAPTION2, ini, 250, 250, 252, 255);
      txt_desenhar(l, disco.x + (d - l.w) * 0.5f, disco.y + (d - l.h) * 0.5f);
    }
    txt_desenhar(nome, disco.x + d + chipPad, chip.y + (chip.h - nome.h) * 0.5f);
  }

  familia_rotulos(c.perfil ? &c : NULL, (long long)time(NULL) * 1000LL,
                  pilula, sizeof pilula, linha, sizeof linha);
  // Top right: what kind of activity.
  if (pilula[0] && estilo != 3 && estilo != 5) {
    float bw = badge_largura(pilula);
    if (bw <= largura * 0.42f)
      badge_desenhar(r.x + r.w - pad - bw, r.y + pad + 1.0f * esc, pilula,
                     c.estado == FAM_EM_CURSO ? BADGE_REALCE : BADGE_NEUTRO, 1.0f);
  }

  // Bottom: icon, title, detail line, progress.
  float ic = 34.0f * esc;
  float livre = largura - ic - 14.0f * esc;
  float base = r.y + r.h - (c.estado == FAM_EM_CURSO ? 26.0f : 18.0f) * esc;
  if (linha[0] && estilo != 5) {
    TxtLinha l = txt_linha_corta(TXT_CW_META, linha, 222, 226, 234, 255, livre);
    base -= l.h;
    txt_desenhar_alpha(l, r.x + pad, base, 1);
    base -= 4.0f * esc;
  }
  TxtLinha titulo = txt_linha_corta(TXT_CW_TITULO, ci->titulo, 247, 248, 250, 255, livre);
  base -= titulo.h;
  txt_desenhar_alpha(titulo, r.x + pad, base, 1);
  if (c.estado) {
    GfxRect ri = { r.x + r.w - pad - ic, r.y + r.h - (c.estado == FAM_EM_CURSO ? 26.0f : 18.0f) * esc - ic,
                   ic, ic };
    gfx_icone(ri, c.estado == FAM_EM_CURSO ? "aj_circle-play" : "aj_circle-check",
              0.97f, 0.97f, 0.98f, 0.95f);
  }
  if (c.estado == FAM_EM_CURSO && estilo != 5) {
    float h = NV_CW_BAR_H * esc, cheio = largura * (c.pct > 100 ? 1.0f : c.pct / 100.0f);
    GfxRect trilho = { r.x + pad, r.y + r.h - NV_CW_BAR_BOTTOM * esc - h, largura, h };
    if (cheio < h) cheio = h;
    gfx_cor(trilho, 0.5f, 1, 1, 1, 0.22f);
    gfx_cor((GfxRect){ trilho.x, trilho.y, cheio, h }, 0.5f, 0.96f, 0.965f, 0.98f, 0.98f);
  }
}

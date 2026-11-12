// Stage H Family Watching: the Option D card overlay (see familia.h). The
// artwork itself is drawn by home.c through the ordinary landscape-card path,
// so the selected artwork provider applies exactly as on any other row.
#ifndef NV_FAMILIA_UI_H
#define NV_FAMILIA_UI_H
#include "gfx.h"
#include "catalogo.h"

#define NV_FAM_W 520.0f   // 16:9 cinematic card
#define NV_FAM_H 292.0f

// Shared measurement policy: drawing, scrolling and focus use the same size.
// Keep the existing friendsRowStyleLocal ID/value range for compatibility.
static inline float familia_estilo_largura(int estilo) {
  static const float w[] = {520, 640, 300, 600, 360, 280};
  return w[estilo >= 0 && estilo < 6 ? estilo : 0];
}
static inline float familia_estilo_altura(int estilo) {
  static const float h[] = {292, 360, 450, 160, 480, 280};
  return h[estilo >= 0 && estilo < 6 ? estilo : 0];
}

// "familia:<profile_index>" in CatItem.socialSlug marks a Family card.
int  familia_perfil_do_item(const CatItem *ci);
void familia_desenhar_cartao(const CatItem *ci, GfxRect r, float raio, float foco);
#endif

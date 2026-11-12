#ifndef NV_PHONE_MENU_GEOMETRY_H
#define NV_PHONE_MENU_GEOMETRY_H
#include "gfx.h"
#include <math.h>

typedef struct { GfxRect panel, rows; float step, rowHeight, maxScroll; } PhoneMenuGeometry;

/* Base canvas has a 1080-unit short edge. 132 units gives a 44dp target on
 * a 360dp phone. Short portrait windows scroll rather than shrinking text. */
static inline PhoneMenuGeometry phone_menu_geometry(float width, float height, int rows) {
  PhoneMenuGeometry g;
  g.step = 144; g.rowHeight = 132;
  g.panel = (GfxRect){40, 180, fmaxf(1, width - 80),
    fmaxf(1, fminf(height - 228, 168 + rows * g.step))};
  g.rows = (GfxRect){g.panel.x + 24, g.panel.y + 144,
    fmaxf(1, g.panel.w - 48), fmaxf(1, g.panel.h - 168)};
  g.maxScroll = fmaxf(0, rows * g.step - 12 - g.rows.h);
  return g;
}
#endif

// CUSTOM ACCENT COLOUR (owner, 18 Nov 2026: "an option for custom like regular
// Nuvio has in settings"). Official Nuvio TV's Appearance › Color Theme › Custom
// (CustomThemeEditor.kt / ThemeColorPicker.kt): Gradient or Single color, three
// colour slots, nine quick swatches, Hue / Saturation / Brightness sliders, a
// hex code pad and a live preview, then Cancel / Save theme. Drawn with
// Settings' own modal pieces (ajustes_ui_*); hosted by ajustes.c.
#ifndef NV_ACENTOCUSTOM_H
#define NV_ACENTOCUSTOM_H
#include <SDL2/SDL.h>

// Colours are 0xRRGGBB. A solid theme has the three equal.
void acentocustom_abrir(const unsigned cores[3], int gradiente);
int  acentocustom_aberto(void);
void acentocustom_fechar(void);
void acentocustom_evento(const SDL_Event *e);
void acentocustom_atualizar(float dt);
void acentocustom_desenhar(void);
// 1 once after Save theme, with the chosen colours (solid: the three equal).
int  acentocustom_resultado(unsigned cores[3], int *gradiente);

// --- pure (tests) ---
void acentocustom_hsv(unsigned rgb, float hsv[3]);    // h 0..360, s and v 0..1
unsigned acentocustom_rgb(const float hsv[3]);
int  acentocustom_hex(const char *s, unsigned *rgb);  // "#RRGGBB" or "RRGGBB"

#ifdef AJUSTES_TESTE
// Screenshots: open on a zone (see acentocustom.c), slot and pad.
void acentocustom_teste(int zona, int slot, int hex);
#endif
#endif

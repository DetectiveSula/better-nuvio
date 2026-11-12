#ifndef NV_POSTPLAY_PREVIEW_H
#define NV_POSTPLAY_PREVIEW_H
#include "gfx.h"
#include <SDL2/SDL.h>
int postplay_preview_supported(void);
void postplay_preview_close(void);
/* Called only for the focused post-play image, after its poster is drawn. */
void postplay_preview_draw(const char *id,int enabled,unsigned delay,Uint32 now,GfxRect image);
#endif

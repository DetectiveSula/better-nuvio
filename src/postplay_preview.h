#ifndef NV_POSTPLAY_PREVIEW_H
#define NV_POSTPLAY_PREVIEW_H
#include "gfx.h"
#include <SDL2/SDL.h>
/* The focused card's title: the Apple lookup needs its name and year
 * (trailerapple_pedir), the YouTube one its TMDB id when known. */
typedef struct {
  const char *imdb;    /* "tt..." */
  const char *titulo;  /* "" when unknown: no Apple lookup */
  const char *meta;    /* catalog line or year ("2022 · 3 seasons", "2022") */
  int serie;
  long tmdb;           /* 0 when unknown */
} PostplayPreviewItem;
int postplay_preview_supported(void);
void postplay_preview_close(void);
/* Called only for the focused post-play image, after its poster is drawn.
 * Sources follow "Trailer source" (trailerfonte.h): Automatic is Apple TV,
 * then YouTube; a source that fails in the player gives way to the next. */
void postplay_preview_draw(const PostplayPreviewItem *it,int enabled,unsigned delay,Uint32 now,GfxRect image);
#endif

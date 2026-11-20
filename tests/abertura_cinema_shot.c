// The cinematic opening (abertura.c, style "Padrão"), frame by frame, for review.
//   bash tests/abertura_cinema_shot.sh <dir>     (frames f000.bmp ... every 50 ms)
#include "abertura.h"
#include "ajustes.h"
#include "gfx.h"
#include "logoapp.h"
#include "tex_cache.h"
#include "text.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int LW = 1920, LH = 1080;

static void salvar(const char *nome) {
  SDL_Surface *s = SDL_CreateRGBSurface(0, LW, LH, 24, 0xff, 0xff00, 0xff0000, 0);
  unsigned char *p, *t;
  assert(s);
  glFinish(); glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, LW, LH, GL_RGB, GL_UNSIGNED_BYTE, s->pixels);
  p = s->pixels; t = malloc((size_t)s->pitch);
  for (int y = 0; y < LH / 2; y++) {
    memcpy(t, p + y * s->pitch, (size_t)s->pitch);
    memcpy(p + y * s->pitch, p + (LH - 1 - y) * s->pitch, (size_t)s->pitch);
    memcpy(p + (LH - 1 - y) * s->pitch, t, (size_t)s->pitch);
  }
  free(t);
  assert(SDL_SaveBMP(s, nome) == 0);
  SDL_FreeSurface(s);
}

int main(int argc, char **argv) {
  const char *dir = argc > 1 ? argv[1] : ".";
  char nome[700];
  SDL_Window *w;
  SDL_SetHint("SDL_MAC_BACKGROUND_APP", "1");
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  w = SDL_CreateWindow("abertura", 0, 0, LW, LH, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(w && SDL_GL_CreateContext(w));
  glViewport(0, 0, LW, LH); gfx_tamanho_alvo(LW, LH); assert(gfx_iniciar());
  assert(txt_iniciar("deploy/app", 1)); tex_iniciar(64);
  ajustes_dir(getenv("NUVIO_DADOS")); ajustes_iniciar();
  abertura_iniciar("deploy/app/art");
  for (int i = 0; i <= 50; i++) {
    gfx_novo_quadro();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); glViewport(0, 0, LW, LH);
    glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
    abertura_cinema_quadro((float)i * 0.05f);
    snprintf(nome, sizeof nome, "%s/f%03d.bmp", dir, i);
    salvar(nome);
  }
  printf("abertura_cinema_shot: 53 frames in %s\n", dir);
  return 0;
}

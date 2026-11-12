// Render the real post-play cards and inspect the actual text draw bounds.
// Official Nuvio layout (12/11): the compact next-episode / post-credits /
// still-watching card at the bottom-right. Every text line must stay inside
// the card in all 31 languages and 3 UI scales, and the card on screen.
#include "plrui.h"
#include "ajustes.h"
#include "idiomacod.h"
#include "tex_cache.h"
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures, texts, language, kind;
static float scale;
static GfxRect card;
static void recordText(TxtLinha l, float x, float y, float alpha);
#define txt_desenhar_alpha recordText
#include "../src/posplay.c"
#undef txt_desenhar_alpha

static void recordText(TxtLinha l, float x, float y, float alpha) {
  txt_desenhar_alpha(l, x, y, alpha);
  if (alpha <= 0 || l.w <= 0 || card.w <= 0) return;
  texts++;
  if (x < card.x + 19 || x + l.w > card.x + card.w - 19 + .01f ||
      y < card.y || y + l.h > card.y + card.h + .01f) {
    fprintf(stderr, "FAIL lang=%d kind=%d scale=%.2f text x=%.1f..%.1f y=%.1f card=%.1f,%.1f %.1fx%.1f\n",
            language, kind, scale, x, x + l.w, y, card.x, card.y, card.w, card.h);
    failures++;
  }
}
static void savePng(const char *dir, const char *phase) {
  char path[1024];
  unsigned char *pixels = malloc(1920 * 1080 * 4);
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
  assert(pixels && surface);
  glFinish();
  glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  for (int y = 0; y < 1080; y++)
    memcpy((char *)surface->pixels + y * surface->pitch, pixels + (1079 - y) * 1920 * 4, 1920 * 4);
  snprintf(path, sizeof path, "%s/%s-%s-%s%s.png", dir, phase,
           kind == 0 ? "movie" : kind == 1 ? "series" : kind == 3 ? "credits-next" : kind == 4 ? "still" : "credits", language == IDIOMA_EN ? "en" : "de",
           scale > 1 ? "-ui150" : "");
  assert(IMG_SavePNG(surface, path) == 0);
  printf("PNG %s\n", path);
  SDL_FreeSurface(surface);
  free(pixels);
}
int main(int argc, char **argv) {
  assert(argc == 3);
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_Window *window = SDL_CreateWindow("Post-play hints proof", 0, 0, 1920, 1080,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(window);
  SDL_GLContext context = SDL_GL_CreateContext(window);
  assert(context && gfx_iniciar() && txt_iniciar("deploy/app", 1));
  gfx_tamanho_alvo(1920, 1080);
  glViewport(0, 0, 1920, 1080);
  gfx_icones_dir("deploy/app/art");
  tex_iniciar(16);
  assert(ajustes_shot_valor("vidroLocal", 1));
  CatItem item = {0};
  CatEp episode = {0};
  snprintf(item.imdb, sizeof item.imdb, "tt0000001");
  snprintf(item.titulo, sizeof item.titulo, "Offline post-play fixture");
  cat_definir(&item, 1);
  episode.temporada = 1; episode.episodio = 2;
  snprintf(episode.duracao, sizeof episode.duracao, "56");
  snprintf(episode.nome, sizeof episode.nome, "The road ahead");
  snprintf(episode.sinopse, sizeof episode.sinopse, "A new journey begins as the story continues.");
  cat_definir_episodios(0, &episode, 1);
  const float scales[] = {1, 1.3f, 1.5f};
  for (unsigned s = 0; s < sizeof scales / sizeof *scales; s++) {
    scale = scales[s]; gfx_escala_ui_definir(scale);
    for (language = 0; language < IDIOMA_N; language++) {
      assert(ajustes_shot_valor("idioma", language + 1));
      assert(ajustes_idioma() == language);
      // 0 movie credits, 1 next episode, 2 series credits without another
      // episode, 3 series credits with Next (two pills), 4 still watching.
      for (kind = 0; kind < 5; kind++) {
        posplay_shot(0, kind != 0, kind == 2 ? -1 : 1, kind == 2 ? 0 : 2, kind == 1 ? 9000 : 0);
        aguardaCena = kind == 0 || kind == 2 || kind == 3;
        stillWatchingPrompt = kind == 4;
        foco = 0;
        txt_novo_quadro(); tex_novo_quadro(); gfx_novo_quadro();
        glClearColor(.14f, .18f, .22f, 1); glClear(GL_COLOR_BUFFER_BIT);
        // First pass records the card, second checks the text against it.
        card = (GfxRect){0}; texts = 0;
        posplay_desenhar(1000, 0);
        card = posplay_shot_cartao();
        posplay_desenhar(1000, 0);
        assert(texts >= 2);
        { float s = gfx_escala_ui(), vw = 1920.0f / s, vh = 1080.0f / s;
          if (card.x < 40 || card.x + card.w > vw - 40 + .01f || card.y + card.h > vh - 40 + .01f) {
            fprintf(stderr, "FAIL lang=%d kind=%d scale=%.2f card off screen\n", language, kind, scale);
            failures++;
          } }
        if (s != 1 && (language == IDIOMA_EN || language == IDIOMA_DE))
          savePng(argv[1], argv[2]);
      }
    }
  }
  printf("%s: 31 languages, 5 card states, 3 UI scales; %d violations\n",
         failures ? "FAIL" : "PASS", failures);
  tex_encerrar(); txt_encerrar(); gfx_encerrar();
  SDL_GL_DeleteContext(context); SDL_DestroyWindow(window); SDL_Quit();
  return failures ? 1 : 0;
}

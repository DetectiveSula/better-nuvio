// O RELOGIO DE 24 H do Perfil (painel A + C) desenhado com gfx_setor. Prende:
//  1) o corte do quad (setor.h) nao muda um pixel: o mesmo relogio por
//     gfx_arco no quadrado inteiro tem de dar a mesma imagem;
//  2) a fatia do pico (22 h) sai no acento claro, a de 3 h (vazia) sai trilho,
//     o miolo e o vao entre fatias ficam no fundo.
// Saida: <prefixo>-relogio.png para olho humano. Ver tests/relogio_shot.sh.
#include "gfx.h"
#include "setor.h"
#include "nvtmp.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W 1920
#define H 1080
static GLuint fbo, fboTex;
static const int PCT[24] = { 3,1,0,0,0,0,0,1,1,0,0,1,2,2,1,1,2,3,5,9,14,19,20,15 };
static const float CX = 960, CY = 540, R0 = 108, VAO = 0.12f / 24.0f;
static float raioFora(int h) { return 116 + PCT[h] * 8.8f; }

static void relogio(int cortado) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, W, H);
  glClearColor(0.027f, 0.067f, 0.075f, 1);   // #071113
  glClear(GL_COLOR_BUFFER_BIT);
  gfx_novo_quadro();
  for (int h = 0; h < 24; h++) {
    float ini = h / 24.0f + VAO, frac = 1.0f / 24.0f - 2 * VAO, r1 = raioFora(h);
    float c[3];
    if (h == 22)          { c[0] = 0.60f; c[1] = 0.95f; c[2] = 0.92f; }   // pico
    else if (PCT[h] >= 9) { c[0] = 0.18f; c[1] = 0.77f; c[2] = 0.75f; }   // acento
    else if (PCT[h] > 0)  { c[0] = 0.12f; c[1] = 0.50f; c[2] = 0.49f; }
    else                  { c[0] = 0.09f; c[1] = 0.16f; c[2] = 0.18f; }   // trilho
    if (cortado) gfx_setor(CX, CY, R0, r1, ini, frac, c[0], c[1], c[2], 1);
    else gfx_arco((GfxRect){ CX - r1, CY - r1, 2 * r1, 2 * r1 }, r1 - R0, ini, frac, c[0], c[1], c[2], 1);
  }
  glFinish();
}

static unsigned char *ler(void) {
  unsigned char *p = malloc(W * H * 4);
  assert(p);
  glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, p);
  return p;
}
// Pixel em coordenadas de TELA (y para baixo).
static const unsigned char *px(const unsigned char *p, float x, float y) {
  return p + ((H - 1 - (int)y) * W + (int)x) * 4;
}
static void noSetor(int h, float frac, float *x, float *y) {
  setor_ponto(CX, CY, (R0 + raioFora(h)) / 2, (h + 0.5f) / 24.0f, x, y);
  (void)frac;
}

int main(int argc, char **argv) {
  const char *saida = argc > 1 ? argv[1] : nvt("/nuvio-relogio");
  char nome[600];
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  IMG_Init(IMG_INIT_PNG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_Window *w = SDL_CreateWindow("relogio", 0, 0, 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(w && SDL_GL_CreateContext(w));
  glGenTextures(1, &fboTex);
  glBindTexture(GL_TEXTURE_2D, fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  gfx_tamanho_alvo(W, H);
  assert(gfx_iniciar());

  relogio(0);
  unsigned char *cheio = ler();
  relogio(1);
  unsigned char *cort = ler();
  int dif = 0, maxd = 0;
  for (int i = 0; i < W * H * 4; i++) {
    int d = abs(cheio[i] - cort[i]);
    if (d > 2) dif++;
    if (d > maxd) maxd = d;
  }
  SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, SDL_PIXELFORMAT_RGBA32);
  for (int r = 0; r < H; r++) memcpy((char *)s->pixels + r * s->pitch, cort + (H - 1 - r) * W * 4, W * 4);
  snprintf(nome, sizeof nome, "%s-relogio.png", saida);
  assert(IMG_SavePNG(s, nome) == 0);
  printf("corte vs quadrado inteiro: %d canais diferentes (max %d)\n", dif, maxd);
  assert(dif == 0);

  float x, y;
  const unsigned char *q;
  noSetor(22, 0, &x, &y); q = px(cort, x, y);
  printf("22 h: %d %d %d\n", q[0], q[1], q[2]);
  assert(q[1] > 220 && q[0] > 130);
  noSetor(3, 0, &x, &y); q = px(cort, x, y);
  printf(" 3 h: %d %d %d\n", q[0], q[1], q[2]);
  assert(abs(q[0] - 23) < 6 && abs(q[1] - 41) < 6);
  q = px(cort, CX, CY);
  assert(q[0] < 12 && q[1] < 22);                      // miolo = fundo
  setor_ponto(CX, CY, (R0 + raioFora(21)) / 2, 22.0f / 24.0f, &x, &y);   // vao 21|22 h
  q = px(cort, x, y);
  printf("vao 21|22: %d %d %d\n", q[0], q[1], q[2]);
  assert(q[1] < 140);
  setor_ponto(CX, CY, raioFora(2) + 20, 2.5f / 24.0f, &x, &y);          // fora da fatia baixa
  q = px(cort, x, y);
  assert(q[1] < 22);

  // PONTAS NITIDAS: 3 px para dentro da ponta inicial da fatia de 22 h ja e a
  // cor cheia, e 3 px para fora ja e o fundo (a rampa e de 1 px, em pixels;
  // com a rampa antiga de 0,01 volta, a 175 px do centro, ~11 px de borrao).
  {
    float rm = (R0 + raioFora(22)) / 2, a = 22.0f / 24.0f + VAO, dv = 3.0f / (6.2831853f * rm);
    setor_ponto(CX, CY, rm, a + dv, &x, &y); q = px(cort, x, y);
    printf("ponta +3 px: %d %d %d\n", q[0], q[1], q[2]);
    assert(q[1] > 235);
    setor_ponto(CX, CY, rm, a - dv, &x, &y); q = px(cort, x, y);
    printf("ponta -3 px: %d %d %d\n", q[0], q[1], q[2]);
    assert(q[1] < 30);
  }
  // O ARCO DO PLAYER usa o mesmo shader (player.c: anel de 3 px de progresso
  // e o giro de 1/4 de volta). Captura em 4x para olho humano: pontas nitidas,
  // sem degrau, e a ponta inicial do giro (que cruza o topo) sem corte duro.
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, W, H);
    glClear(GL_COLOR_BUFFER_BIT);
    gfx_novo_quadro();
    gfx_arco((GfxRect){ 900, 500, 56, 56 }, 3.0f, 0.0f, 0.37f, 1, 1, 1, 0.9f);
    gfx_arco((GfxRect){ 980, 500, 56, 56 }, 3.0f, 0.8f, 0.25f, 1, 1, 1, 0.9f);
    glFinish();
    unsigned char *p = ler();
    SDL_Surface *z = SDL_CreateRGBSurfaceWithFormat(0, 160, 80, 32, SDL_PIXELFORMAT_RGBA32);
    for (int r = 0; r < 80; r++)
      memcpy((char *)z->pixels + r * z->pitch, p + ((H - 1 - (490 + r)) * W + 890) * 4, 160 * 4);
    SDL_Surface *g = SDL_CreateRGBSurfaceWithFormat(0, 640, 320, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitScaled(z, NULL, g, NULL);
    snprintf(nome, sizeof nome, "%s-arco-player.png", saida);
    assert(IMG_SavePNG(g, nome) == 0);
    printf("captura: %s\n", nome);
    // O topo do giro (0.8 + 0.25 cruza 1.0) esta aceso: o arco nao se partiu.
    q = px(p, 980 + 28, 500 + 1.5f);
    assert(q[0] > 150);
    free(p);
  }
  puts("PASS: relogio_shot");
  return 0;
}

// A caixa de um setor de anel (src/setor.h), sem GL: todo ponto do setor cabe
// nela, e ela nao passa do circulo nem cresce alem do necessario.
#include "setor.h"
#include <stdio.h>
#include <stdlib.h>

static int falhas, n;
#define CHECA(c, ...) do { n++; if (!(c)) { falhas++; printf("FALHA %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)
static int perto(float a, float b) { return fabsf(a - b) < 0.01f; }

int main(void) {
  float c[4];
  setor_caixa(0, 0, 50, 100, 0.0f, 0.25f, 0, c);
  CHECA(perto(c[0], 0) && perto(c[1], -100) && perto(c[2], 100) && perto(c[3], 100),
        "quarto de cima-direita %.1f %.1f %.1f %.1f", c[0], c[1], c[2], c[3]);
  setor_caixa(0, 0, 50, 100, 0.95f, 0.10f, 0, c);       // atravessa o topo
  CHECA(perto(c[1], -100), "topo dentro: y0 %.2f", c[1]);
  setor_caixa(960, 540, 54, 140, 22.0f / 24.0f, 0.9f / 24.0f, 2, c);   // 22 h
  CHECA(c[0] + c[2] <= 960 + 2.01f && c[1] + c[3] <= 540 + 2.01f && c[2] < 80 && c[3] < 150,
        "22 h no quadrante de cima-esquerda, pequeno: %.1f %.1f %.1f %.1f", c[0], c[1], c[2], c[3]);
  setor_caixa(10, 20, 0, 30, 0, 1, 1, c);
  CHECA(perto(c[0], -21) && perto(c[1], -11) && perto(c[2], 62) && perto(c[3], 62), "volta inteira");

  srand(7);
  for (int t = 0; t < 2000; t++) {
    float ini = (float)rand() / (float)RAND_MAX, frac = 0.005f + 0.99f * (float)rand() / (float)RAND_MAX;
    float r0 = 30.0f * (float)rand() / (float)RAND_MAX, r1 = r0 + 1 + 120.0f * (float)rand() / (float)RAND_MAX;
    setor_caixa(500, 400, r0, r1, ini, frac, 0, c);
    CHECA(c[0] >= 500 - r1 - 0.01f && c[1] >= 400 - r1 - 0.01f &&
          c[0] + c[2] <= 500 + r1 + 0.01f && c[1] + c[3] <= 400 + r1 + 0.01f, "dentro do circulo");
    for (int i = 0; i <= 40; i++) for (int k = 0; k <= 8; k++) {
      float x, y;
      setor_ponto(500, 400, r0 + (r1 - r0) * k / 8.0f, ini + frac * i / 40.0f, &x, &y);
      if (x < c[0] - 0.01f || x > c[0] + c[2] + 0.01f || y < c[1] - 0.01f || y > c[1] + c[3] + 0.01f) {
        CHECA(0, "ponto fora: ini %.3f frac %.3f", ini, frac);
        i = 41; break;
      }
    }
  }
  printf("setor: %d checagens, %d falhas\n", n, falhas);
  return falhas != 0;
}

// Geometria de um SETOR DE ANEL (a fatia do relogio de 24 h do Perfil): a
// caixa que o contem, para gfx_setor desenhar so ela e nao o quadrado inteiro
// do circulo. Sem GL: tests/setor.sh prende a conta.
//
// Angulos em FRACAO DE VOLTA, 0 = topo, sentido horario — a mesma convencao de
// gfx_arco. Coordenadas de tela (y para baixo).
#ifndef NV_SETOR_H
#define NV_SETOR_H
#include <math.h>

static inline void setor_ponto(float cx, float cy, float r, float volta, float *x, float *y) {
  float a = volta * 6.28318531f;
  *x = cx + r * sinf(a);
  *y = cy - r * cosf(a);
}

// out = { x, y, w, h }, com `margem` px de folga (a rampa de 1 px do shader).
static inline void setor_caixa(float cx, float cy, float r0, float r1, float ini, float frac,
                               float margem, float out[4]) {
  float x0, y0, x1, y1, px, py;
  int k, k0, k1;
  if (frac >= 1.0f) {
    out[0] = cx - r1 - margem; out[1] = cy - r1 - margem;
    out[2] = out[3] = 2.0f * (r1 + margem);
    return;
  }
  setor_ponto(cx, cy, r1, ini, &px, &py);
  x0 = x1 = px; y0 = y1 = py;
#define SETOR_INCLUI(X, Y) do { if ((X) < x0) x0 = (X); if ((X) > x1) x1 = (X); \
                                if ((Y) < y0) y0 = (Y); if ((Y) > y1) y1 = (Y); } while (0)
  setor_ponto(cx, cy, r1, ini + frac, &px, &py); SETOR_INCLUI(px, py);
  setor_ponto(cx, cy, r0, ini, &px, &py);        SETOR_INCLUI(px, py);
  setor_ponto(cx, cy, r0, ini + frac, &px, &py); SETOR_INCLUI(px, py);
  // Os pontos cardeais (topo, direita, base, esquerda) dentro do arco esticam
  // a caixa ate a borda de fora.
  k0 = (int)ceilf(ini * 4.0f);
  k1 = (int)floorf((ini + frac) * 4.0f);
  for (k = k0; k <= k1; k++) {
    setor_ponto(cx, cy, r1, (float)k * 0.25f, &px, &py);
    SETOR_INCLUI(px, py);
  }
#undef SETOR_INCLUI
  out[0] = x0 - margem; out[1] = y0 - margem;
  out[2] = x1 - x0 + 2.0f * margem; out[3] = y1 - y0 + 2.0f * margem;
}

#endif

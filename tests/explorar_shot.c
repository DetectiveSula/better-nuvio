// Captura a tela Explorar 2.0 SEM janela visivel: a janela GL nasce escondida,
// o desenho vai para um FBO e o quadro sai por glReadPixels, em PNG.
//
// So o caminho LOCAL (sem chave do TMDB, sem rede): e o que a tela tem de
// mostrar sozinha. Catalogo sintetico com cartazes do pacote
// (deploy/app/art/poster); nomes de pessoas sao de exemplo.
//
// Quadros:
//   1-climas     a grade de climas (portal)
//   2-clima      o primeiro clima aberto
//   3-vizinhanca a toca no primeiro titulo do clima
//   4-trilha     depois de descer dois degraus (trilha com tres passos)
//   5-subiu      Voltar uma vez: um degrau acima
//   6-detalhe    entrada pelo Detalhe (explorar_abrir_titulo)
// Alem das capturas, confere pelo retrato publicado (mapa_vizinhos_copiar)
// que cada OK desceu de fato, que Voltar subiu e que Voltar no primeiro degrau
// da entrada pelo Detalhe pede a pagina do titulo de volta.
#include "explorar.h"
#include "nvtmp.h"
#include "mapa.h"
#include "catalogo.h"
#include "gfx.h"
#include "text.h"
#include "tex_cache.h"
#include "ajustes.h"
#include "rail_shot.h"
#include "dados.h"
#include "vidro_telas.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint fbo, fboTex;
static const char *saida;

static void tecla(SDL_Keycode k) {
  SDL_Event e;
  memset(&e, 0, sizeof e);
  e.type = SDL_KEYDOWN;
  e.key.keysym.sym = k;
  explorar_evento(&e);
}

static void quadros(int n, const char *nome) {
  int i;
  rail_shot_aplicar();
  for (i = 0; i < n; i++) {
    SDL_PumpEvents();
    txt_novo_quadro();
    tex_novo_quadro();
    tex_bombear(10);
    gfx_novo_quadro();
    vt_antes();
    explorar_atualizar(1.0f / 60.0f, SDL_GetTicks());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, 1920, 1080);
    glClearColor(0.051f, 0.051f, 0.051f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    explorar_desenhar(SDL_GetTicks());
    rail_shot_desenhar(MENU_EXPLORAR);
    vt_depois();
    if (vt_ligado()) continue;
    glFinish();
    if (nome && i == n - 1) {
      unsigned char *pix = malloc(1920 * 1080 * 4);
      SDL_Surface *s;
      char cam[700];
      int y;
      assert(pix);
      glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, pix);
      s = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
      assert(s);
      for (y = 0; y < 1080; y++)
        memcpy((char *)s->pixels + y * s->pitch, pix + (1079 - y) * 1920 * 4, 1920 * 4);
      snprintf(cam, sizeof cam, "%s-%s.png", saida, nome);
      assert(IMG_SavePNG(s, cam) == 0);
      SDL_FreeSurface(s);
      free(pix);
      printf("%s  desenhos=%d\n", cam, gfx_n_rect);
    }
    SDL_Delay(2);
  }
}

// --- catalogo de teste ----------------------------------------------------------
// O catalogo EMBARCADO (deploy/app/art/catalogo.txt): titulo, cartaz, backdrop,
// generos e sinopse de verdade e coerentes entre si. So direcao, elenco, nota e
// progresso sao de exemplo (o arquivo nao os tem).

static int indiceDe(const char *titulo) {
  int i;
  for (i = 0; i < cat_n(); i++) if (!strcmp(cat_item(i)->titulo, titulo)) return i;
  return -1;
}

static void semearCatalogo(void) {
  static CatItem itens[64];
  static const char *const direcao[] = { "Lena Hart", "Iris Moon", "Rui Sato", "" };
  static const char *const ator[] = { "Tomás Weber", "Paulo Vidal", "Ana Kowalski" };
  static const struct { const char *t; int p; } prog[] = {
    { "The Prestige", 96 }, { "Frequency", 96 }, { "Prisoners", 45 },
    { "The Martian", 96 }, { "3 Body Problem", 45 } };
  char linha[2048];
  FILE *f = fopen("deploy/app/art/catalogo.txt", "r");
  int n = 0, k;
  assert(f);
  memset(itens, 0, sizeof itens);
  while (n < 64 && fgets(linha, sizeof linha, f)) {
    char *campo[8] = { 0 }, *p = linha;
    CatItem *c = &itens[n];
    int nc = 0;
    linha[strcspn(linha, "\r\n")] = 0;
    while (nc < 8) { campo[nc++] = p; p = strchr(p, '|'); if (!p) break; *p++ = 0; }
    if (nc < 8) continue;
    snprintf(c->imdb, sizeof c->imdb, "tt-exp-%02d", n);
    snprintf(c->tipo, sizeof c->tipo, "%s", strncmp(campo[4], "Programa", 8) ? "movie" : "series");
    snprintf(c->titulo, sizeof c->titulo, "%s", campo[3]);
    snprintf(c->genero, sizeof c->genero, "%s", campo[4]);
    snprintf(c->meta, sizeof c->meta, "%s", campo[5]);
    snprintf(c->sinopse, sizeof c->sinopse, "%s", campo[7]);
    snprintf(c->poster, sizeof c->poster, "deploy/app/art/%s", campo[1]);
    snprintf(c->backdrop, sizeof c->backdrop, "deploy/app/art/%s", campo[0]);
    if (!strcmp(c->tipo, "movie"))
      snprintf(c->direcao, sizeof c->direcao, "%s", direcao[n % 4]);
    snprintf(c->elenco[0].nome, sizeof c->elenco[0].nome, "%s", ator[n % 3]);
    c->nElenco = 1;
    c->nota = 61 + (n * 7) % 27;
    for (k = 0; k < (int)(sizeof prog / sizeof prog[0]); k++)
      if (!strcmp(prog[k].t, c->titulo)) c->progresso = prog[k].p;
    n++;
  }
  fclose(f);
  assert(n >= 30);
  cat_definir_tudo(itens, n, NULL, 0);
}

static void focoPublicado(char *dst, size_t n, int *grupos) {
  static MapaVizinhos v;
  unsigned rev = 0;
  int g;
  assert(mapa_vizinhos_copiar(&v, &rev));
  snprintf(dst, n, "%s", v.foco.titulo);
  *grupos = 0;
  for (g = 0; g < MAPA_VIZ_GRUPOS; g++) if (v.g[g].n > 0) (*grupos)++;
}

int main(int argc, char **argv) {
  SDL_Window *win;
  SDL_GLContext gl;
  char raiz[128], passo1[128], passo2[128], volta[128];
  int grupos;
  saida = argc > 1 ? argv[1] : nvt("/nuvio-explorar");
  // Idioma, acento e animacoes pelo caminho de verdade (ajustes.txt na pasta
  // de dados temporaria): NUVIO_SHOT_EN=1 para ingles, NUVIO_SHOT_THEME=<n>
  // para o acento, NUVIO_SHOT_REDUZ=1 para animacoes reduzidas.
  ajustes_iniciar();
  dados_iniciar("deploy/app/art");
  { char caminho[700]; FILE *f;
    const char *en = getenv("NUVIO_SHOT_EN"), *tema = getenv("NUVIO_SHOT_THEME");
    const char *reduz = getenv("NUVIO_SHOT_REDUZ");
    snprintf(caminho, sizeof caminho, "%s/ajustes.txt", dados_dir());
    f = fopen(caminho, "w");
    assert(f);
    fprintf(f, "idioma %d\nselected_theme %d\nanimacoes %d\n",
            en && *en == '1', tema && *tema ? atoi(tema) : 2, reduz && *reduz == '1');
    fclose(f);
    ajustes_dir(dados_dir()); }
  SDL_SetHint("SDL_MAC_BACKGROUND_APP", "1");
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  win = SDL_CreateWindow("explorar-shot", 0, 0, 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(win);
  gl = SDL_GL_CreateContext(win);
  assert(gl);
  glGenTextures(1, &fboTex);
  glBindTexture(GL_TEXTURE_2D, fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glViewport(0, 0, 1920, 1080);
  gfx_tamanho_alvo(1920, 1080);
  assert(gfx_iniciar());
  assert(txt_iniciar("deploy/app", 1));
  tex_iniciar(64);
  gfx_icones_dir("deploy/app/art");
  gfx_tex_esquecer(0);
  semearCatalogo();

  // Os climas saem do catalogo, sem rede: o primeiro (maior afinidade) tem de
  // ter titulos.
  int primeiroClima;
  { static MapaClimas cl;
    unsigned rev = 0;
    int i, f = 0, se = 0, soma = 0;
    mapa_climas_pedir();
    assert(mapa_climas_copiar(&cl, &rev));
    assert(cl.n == MAPA_CLIMA_N && cl.c[0].n > 0 && cl.c[0].afinidade >= cl.c[1].afinidade);
    primeiroClima = cl.c[0].id;
    // Contagem por tipo (filtro Filmes/Series da TV): sem /discover, soma o total.
    for (i = 0; i < cl.n; i++) {
      assert(cl.c[i].filmes + cl.c[i].series == cl.c[i].total);
      f += cl.c[i].filmes > 0; se += cl.c[i].series > 0; soma += cl.c[i].total;
    }
    assert(f > 0 && se > 0 && soma > 0); }

  // NUVIO_SHOT_TEAL=1: o acento mais proximo do teal do mock-up, pela regua
  // de cores do menu rapido (o selected_theme do arquivo nao vale aqui).
  if (getenv("NUVIO_SHOT_TEAL") && *getenv("NUVIO_SHOT_TEAL") == '1') {
    int i, melhor = -1; float nota = -9.0f;
    for (i = 0; i < 18; i++) {
      float r, g, b, n;
      ajustes_rapido_cor(i, &r, &g, &b);
      n = g + b - 2.0f * r - fabsf(g - b);
      if (n > nota) { nota = n; melhor = i; }
    }
    if (melhor >= 0) ajustes_rapido_cor_definir(melhor);
  }
  { float ar, ag, ab; ajustes_acento(&ar, &ag, &ab);
    printf("explorar_shot: acento %.2f %.2f %.2f\n", ar, ag, ab); }
  explorar_iniciar();
  // NUVIO_VIDRO_TELAS=<dir> (tests/vidro_telas.sh): glass budget, climates.
  if (vt_ligado()) {
    int i;
    vt_vidro();
    for (i = 0; i < 150; i++) { quadros(1, NULL); SDL_Delay(2); }
    vt_zerar(); quadros(30, NULL); vt_png("explorar"); vt_relatar("explore", 2);
    for (i = 0; i < 6; i++) { tecla(i & 1 ? SDLK_LEFT : SDLK_RIGHT); quadros(4, NULL); }
    vt_relatar("explore", 0);
    return vtFalhas ? 1 : 0;
  }
  quadros(90, "1-climas");
  // TV: o foco nasce no botao do destaque, que abre o clima de maior afinidade.
  { int sec = -1, idx = -1;
    explorar_tv_foco(&sec, &idx);
    assert(sec == 0 && idx == 0); }
  tecla(SDLK_RETURN);
  assert(explorar_clima_aberto() == primeiroClima);
  quadros(70, "2-clima");
  // A segunda fileira (ja vistos): mesmo tamanho, com selo; volta para a primeira.
  tecla(SDLK_DOWN);
  quadros(50, "2b-clima-vistos");
  tecla(SDLK_UP);
  quadros(20, NULL);

  tecla(SDLK_RETURN);
  quadros(70, "3-vizinhanca");
  focoPublicado(raiz, sizeof raiz, &grupos);
  assert(raiz[0] && grupos >= 2);

  // Dois degraus: o primeiro cartaz do primeiro grupo, depois o segundo cartaz.
  tecla(SDLK_RETURN);
  quadros(10, NULL);
  focoPublicado(passo1, sizeof passo1, &grupos);
  assert(strcmp(passo1, raiz) && grupos >= 1);
  tecla(SDLK_RIGHT);
  tecla(SDLK_RETURN);
  quadros(70, "4-trilha");
  focoPublicado(passo2, sizeof passo2, &grupos);
  assert(strcmp(passo2, passo1) && strcmp(passo2, raiz));

  tecla(SDLK_ESCAPE);
  quadros(50, "5-subiu");
  focoPublicado(volta, sizeof volta, &grupos);
  assert(!strcmp(volta, passo1));

  // Entrada pelo Detalhe: a toca comeca no titulo da pagina.
  explorar_iniciar();
  { MapaObra o;
    assert(mapa_obra_do_catalogo(indiceDe("Prisoners"), &o));
    explorar_abrir_titulo(&o); }
  quadros(70, "6-detalhe");
  focoPublicado(volta, sizeof volta, &grupos);
  assert(!strcmp(volta, "Prisoners"));
  // Voltar no primeiro degrau devolve a pagina do titulo.
  tecla(SDLK_ESCAPE);
  { int idx = -1;
    assert(explorar_pediu_abrir(&idx) && idx == indiceDe("Prisoners")); }

  // TV: a pagina de climas do mock-up, secao por secao, pelo D-pad.
  explorar_iniciar();
  quadros(40, NULL);
  { int sec, idx, aberto;
    tecla(SDLK_RIGHT);                      // segundo destaque (pontos)
    quadros(40, "7-destaque2");
    tecla(SDLK_DOWN);                       // colecoes
    explorar_tv_foco(&sec, &idx); assert(sec == 1 && idx == 0);
    tecla(SDLK_RIGHT);
    quadros(40, "8-colecoes");
    explorar_tv_foco(&sec, &idx); assert(sec == 1 && idx == 1);
    tecla(SDLK_DOWN);                       // continuar explorando (ha progresso)
    explorar_tv_foco(&sec, &idx); assert(sec == 2 && idx == 0);
    quadros(40, "9-continuar");
    tecla(SDLK_DOWN);                       // filtros
    explorar_tv_foco(&sec, &idx); assert(sec == 3 && idx == 0);
    tecla(SDLK_RIGHT); tecla(SDLK_RETURN);  // Filmes
    quadros(40, "10-filtro-filmes");
    tecla(SDLK_RIGHT); tecla(SDLK_RETURN);  // Series
    tecla(SDLK_LEFT); tecla(SDLK_LEFT); tecla(SDLK_RETURN);   // Todos de novo
    tecla(SDLK_DOWN);                       // grade
    explorar_tv_foco(&sec, &idx); assert(sec == 4 && idx == 0);
    tecla(SDLK_DOWN); tecla(SDLK_RIGHT);
    explorar_tv_foco(&sec, &idx); assert(sec == 4 && idx == 5);
    quadros(50, "11-grade");
    // Ordem editorial na grade: a posicao 5 e o clima 5 ("Família complicada").
    tecla(SDLK_RETURN);
    aberto = explorar_clima_aberto();
    assert(aberto == 5);
    tecla(SDLK_ESCAPE);                     // volta com o foco onde estava
    assert(explorar_clima_aberto() == -1);
    explorar_tv_foco(&sec, &idx); assert(sec == 4 && idx == 5);
    // Para cima ate o banner, e Esquerda na borda devolve a barra lateral.
    { int k; for (k = 0; k < 8; k++) tecla(SDLK_UP); }
    explorar_tv_foco(&sec, &idx); assert(sec == 0);
    (void)explorar_quer_sair();
    tecla(SDLK_LEFT); tecla(SDLK_LEFT);
    assert(explorar_quer_sair()); }

  printf("explorar_shot: raiz=%s -> %s -> %s; capturas gravadas\n", raiz, passo1, passo2);
  explorar_encerrar();
  tex_encerrar();
  txt_encerrar();
  gfx_encerrar();
  SDL_GL_DeleteContext(gl);
  SDL_DestroyWindow(win);
  IMG_Quit();
  SDL_Quit();
  return 0;
}

// CAPTURAS DA TELA DE PERFIL E STATS, sem rede e sem conta do Trakt.
//
// Existe pelo mesmo motivo de tests/perfilsel_visual.c: interface de TV nao se
// revisa lendo codigo. A tela e olhada a 3 m, e colisao de texto, cinza sobre
// cinza e foco que nao se acha sao invisiveis numa leitura do fonte — foi assim
// que o aviso do topo ficou desenhado DUAS vezes e que o numeral do streak
// passou por cima da legenda dele.
//
// O snapshot e PerfilDados de mentira, com a forma que o trakt.c produz de
// verdade: `parcial` ligado, `aviso` preenchido (os dois juntos eram o caso em
// que o aviso aparecia em duplicata), nome + usuario + periodo, um mes cheio de
// atividade irregular e quatro destaques com arte do PACOTE — nenhuma URL de
// rede, nenhum dado de pessoa.
//
// ONDE ESCREVE. Este teste NAO grava dado de app, mas chama dados_iniciar()
// como o app chama, e por isso se recusa a rodar se dados_dir() nao for a pasta
// descartavel que tests/perfil_shot.sh exporta. A regra do repo: nunca ~/.nuvio.
//
//   bash tests/perfil_shot.sh [prefixo]
#include "perfil.h"
#include "nvtmp.h"
#include "ajustes.h"
#include "rail_shot.h"
#include "dados.h"
#include "gfx.h"
#include "text.h"
#include "tex_cache.h"
#include "socialvis.h"
#include "perfilnuvio.h"
#include "layout.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static SDL_Window *win;
// RETRATO (NUVIO_SHOT_RETRATO=1, build com -DNV_TOUCH_UI): celular em pe,
// 1080 x 1920, o painel em coluna rolando (pfRetrato em perfil.c).
static int SW = 1920, SH = 1080, retrato;

static void tecla(SDL_Keycode k) {
  SDL_Event e = { 0 };
  e.type = SDL_KEYDOWN; e.key.keysym.sym = k;
  perfil_evento(&e);
}

// 90 quadros: sobra para a mola de entrada assentar (NV_MOLA_TELA), para a do
// foco fechar os 120 ms medidos e para o decode das artes subir para a GPU.
static void captura(const char *nome) {
  int i;
  rail_shot_aplicar();
  for (i = 0; i < 90; i++) {
    SDL_PumpEvents();
    txt_novo_quadro(); tex_novo_quadro(); tex_bombear(6); gfx_novo_quadro();
    perfil_atualizar(1.0f / 60.0f, SDL_GetTicks());
    glClearColor(0.051f, 0.051f, 0.051f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
    perfil_desenhar(SDL_GetTicks());
    rail_shot_desenhar(MENU_PERFIL);
    if (i == 89) {
      unsigned char *pix = malloc((size_t)SW * SH * 4);
      SDL_Surface *s;
      int y;
      assert(pix);
      glReadPixels(0, 0, SW, SH, GL_RGBA, GL_UNSIGNED_BYTE, pix);
      s = SDL_CreateRGBSurfaceWithFormat(0, SW, SH, 32, SDL_PIXELFORMAT_RGBA32);
      assert(s);
      for (y = 0; y < SH; y++)
        memcpy((char *)s->pixels + y * s->pitch, pix + (size_t)(SH - 1 - y) * SW * 4, (size_t)SW * 4);
      assert(SDL_SaveBMP(s, nome) == 0);
      SDL_FreeSurface(s); free(pix);
    }
    SDL_GL_SwapWindow(win); SDL_Delay(2);
  }
  printf("  %s  (preenchimento %.2f telas, %d desenhos)\n",
         nome, gfx_fill, gfx_n_rect);
}

// O PAINEL DE 30 DIAS montado pelo CAMINHO REAL (perfilnuvio_montar) sobre
// vistos e progresso de mentira: os mesmos dias do mockup "Option A + C"
// (42,5 h, maratona de From em 29/10, filmes em seis dias). Cartazes do
// pacote de design (design/universo-2026), nenhuma URL de rede.
static ContaVisto SV[400];
static ProgRegistro SR[400];
static int nSV, nSR;
static void ev(const char *id, const char *tipo, int t, int e, long long fim, int durMin) {
  ContaVisto *v = &SV[nSV++];
  ProgRegistro *r = &SR[nSR++];
  snprintf(v->id, sizeof v->id, "%s", id); snprintf(v->tipo, sizeof v->tipo, "%s", tipo);
  v->temporada = t; v->episodio = e; v->vistoMs = fim;
  r->perfil = 1;
  snprintf(r->contentId, sizeof r->contentId, "%s", id); snprintf(r->tipo, sizeof r->tipo, "%s", tipo);
  r->temporada = t; r->episodio = e; r->posSeg = r->durSeg = durMin * 60.0; r->lastWatchedMs = fim;
}
static long long quando(int mes, int dia, int h, int m) {
  struct tm tm = {0};
  tm.tm_year = 126; tm.tm_mon = mes - 1; tm.tm_mday = dia; tm.tm_hour = h; tm.tm_min = m; tm.tm_isdst = -1;
  return (long long)mktime(&tm) * 1000;
}
static void montar(PerfilDados *d) {
  // Minutos de serie e de filme por dia, 09/10 a 07/11.
  static const int SERIE[30] = { 0,72,18,0,48,30,54, 0,0,90,132,0,36,60, 30,150,60,114,0,0,288,84,0,0,66,120,96,54,84,72 };
  static const int FILME[30] = { 0,0,132,0,0,156,66, 0,0,0,0,0,210,0, 0,0,108,0,0,0,0,0,0,0,0,0,0,0,120,0 };
  static const char *SHOW[4] = { "tt9813792", "tt12637874", "tt0000101", "tt0000102" };
  static const char *MOVIE[6] = { "tt0482571", "tt1392214", "tt11097384", "tt0000201", "tt0000202", "tt0000203" };
  static PerfilMeta M[10];
  static const struct { const char *id, *tit, *poster, *gen; int dur; } MD[10] = {
    { "tt9813792", "From", "design/universo-2026/assets/poster-04.jpg", "Terror, Mistério, Drama", 50 },
    { "tt12637874", "Fallout", "design/universo-2026/assets/poster-00.jpg", "Ficção científica, Drama", 60 },
    { "tt0000101", "Widow's Bay", "design/universo-2026/assets/poster-01.jpg", "Mistério, Terror", 38 },
    { "tt0000102", "Uma Série de Eventos Muito Longos com Nome Enorme", "design/universo-2026/assets/poster-02.jpg", "Drama", 30 },
    { "tt0482571", "O Grande Truque", "design/universo-2026/assets/poster-02.jpg", "Drama, Mistério", 130 },
    { "tt1392214", "Os Suspeitos", "design/universo-2026/assets/poster-09.jpg", "Crime, Drama", 153 },
    { "tt11097384", "Spaceman", "design/universo-2026/assets/poster-10.jpg", "Ficção científica", 107 },
    { "tt0000201", "Filme Quatro", "design/universo-2026/assets/poster-12.jpg", "Drama", 0 },
    { "tt0000202", "Filme Cinco", "design/universo-2026/assets/poster-13.jpg", "Comédia", 0 },
    { "tt0000203", "Filme Seis", "design/universo-2026/assets/poster-15.jpg", "Ação", 0 },
  };
  int ep[4] = { 1, 1, 1, 1 }, filme = 0;
  nSV = nSR = 0;
  for (int i = 0; i < 30; i++) {
    int mes = i < 23 ? 10 : 11, dd = i < 23 ? 9 + i : i - 22;
    long long fim = quando(mes, dd, i == 29 ? 20 : 23, i == 29 ? 30 : 10);
    if (FILME[i]) {
      int dur = FILME[i] > 180 ? FILME[i] / 2 : FILME[i];
      ev(MOVIE[filme % 6], "movie", 0, 0, quando(mes, dd, 19, 50), dur); filme++;
      if (FILME[i] > 180) { ev(MOVIE[filme % 6], "movie", 0, 0, quando(mes, dd, 17, 30), dur); filme++; }
    }
    if (i == 20) {   // 29/10: a maratona de From (5 x 50 min) e um Widow's Bay
      long long t = quando(10, 29, 18, 30);
      for (int k = 0; k < 5; k++) { t += 55 * 60000LL; ev(SHOW[0], "series", 2, 3 + k, t, 50); }
      ev(SHOW[2], "series", 1, 3, quando(10, 29, 17, 40), 38);
      continue;
    }
    for (int m = SERIE[i], k = i % 4; m > 0; k = (k + 1) % 4) {
      int dur = MD[k].dur;
      ev(SHOW[k], "series", 1, ep[k]++, fim, dur);
      fim -= (dur + 8) * 60000LL; m -= dur;
    }
  }
  for (int i = 0; i < 10; i++) {
    snprintf(M[i].id, sizeof M[i].id, "%s", MD[i].id);
    snprintf(M[i].titulo, sizeof M[i].titulo, "%s", MD[i].tit);
    snprintf(M[i].poster, sizeof M[i].poster, "%s", MD[i].poster);
    snprintf(M[i].genero, sizeof M[i].genero, "%s", MD[i].gen);
    M[i].duracaoMin = MD[i].dur;
  }
  // "Neste dia": Os Suspeitos, um ano antes.
  { ContaVisto *v = &SV[nSV++];
    snprintf(v->id, sizeof v->id, "tt1392214"); snprintf(v->tipo, sizeof v->tipo, "movie");
    v->vistoMs = quando(11, 7, 21, 40) - 365LL * 86400000LL; }
  perfilnuvio_montar(SV, nSV, SR, nSR, 1, M, 10, quando(11, 7, 21, 0), d);
  snprintf(d->nome, sizeof d->nome, "Henrique Rocha");
  printf("  painel: %d min, %d eps, %d filmes, %d titulos, maratona %d eps, streak %d\n",
         d->minutos, d->episodios, d->filmes, d->titulos, d->maratona.episodios, d->streakAtual);
}

int main(int argc, char **argv) {
  const char *prefixo = argc > 1 ? argv[1] : nvt("/nuvio-perfil");
  char nome[600];
  PerfilDados d;
  SDL_GLContext gl;
  int i;

#ifdef NV_TOUCH_UI
  if (getenv("NUVIO_SHOT_RETRATO")) {
    retrato = 1; SW = 1080; SH = 1920;
    layout_tela_definir(SW, SH); layout_modo_definir(1);
  }
#endif
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  win = SDL_CreateWindow("Nuvio: revisão do Perfil e Stats",
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         SW, SH, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  assert(win);
  gl = SDL_GL_CreateContext(win); assert(gl);
  SDL_GL_SetSwapInterval(0);
  glViewport(0, 0, SW, SH);
  gfx_tamanho_alvo(SW, SH);
  assert(gfx_iniciar());
  assert(txt_iniciar("deploy/app", 1));
  tex_iniciar(64);
  gfx_icones_dir("deploy/app/art");

  // GUARDA DE PASTA. O argumento de dados_iniciar e a pasta de ARTE, nao um
  // desvio dos dados: o desvio e a variavel NUVIO_DADOS. Sem esta conferencia
  // um teste grafico escreve em ~/.nuvio, os dados REAIS de quem roda.
  dados_iniciar(NULL);
  { const char *dir = dados_dir();
    const char *tmp = getenv("NUVIO_TESTE_DIR");
    if (!tmp || !*tmp || !dir || strcmp(dir, tmp)) {
      fprintf(stderr,
              "perfil_shot: recusando rodar fora de uma pasta descartável.\n"
              "  dados_dir()=%s   NUVIO_TESTE_DIR=%s\n"
              "  Rode por tests/perfil_shot.sh, que exporta NUVIO_DADOS.\n",
              dir && *dir ? dir : "(nenhuma)", tmp ? tmp : "(vazia)");
      return 1;
    } }

  // IDIOMA E TEMA VINDOS DO DISCO, pelo caminho real (ajustes_dir le
  // ajustes.txt da pasta de dados). Duas razoes:
  //   - "idioma 0" poe a interface em portugues; o padrao de fabrica e o INGLES
  //     (ajustes.c:620), e quem revisa esta tela le portugues;
  //   - "selected_theme 2" e o acento OCEANO. O anel de foco tem de sair AZUL:
  //     ele estava cravado em branco nesta tela (o antigo anel() usava
  //     0.96/0.96/0.98), entao o tema nao fazia nada aqui. Azul ao lado das
  //     celulas violeta tambem mostra que o acento do TEMA e o violeta do DADO
  //     sao dois papeis, e nao a mesma cor.
  { char caminho[700]; FILE *f;
    snprintf(caminho, sizeof caminho, "%s/ajustes.txt", dados_dir());
    f = fopen(caminho, "w");
    assert(f);
    // NUVIO_SHOT_IDIOMA=<n>: o indice de idiomacod.h (1 = ingles, 26 = japones...).
    fprintf(f, "idioma %d\nselected_theme 2\n", getenv("NUVIO_SHOT_IDIOMA") ? atoi(getenv("NUVIO_SHOT_IDIOMA")) : 0);
    // NUVIO_SHOT_VIDRO=1: Interface de vidro ligada (V_LIGA: 0 = Ligado).
    // NUVIO_SHOT_VIDRO=0: o material solido (vidroLocal 1).
    if (getenv("NUVIO_SHOT_VIDRO"))
      fprintf(f, "vidroLocal %d\n", *getenv("NUVIO_SHOT_VIDRO") == '0' ? 1 : 0);
    fclose(f);
    // A MIGRACAO UNICA DA 2.0 (ajustes.c) desliga o vidro na primeira leitura;
    // com NUVIO_SHOT_VIDRO pedido, ela ja conta como feita.
    if (getenv("NUVIO_SHOT_VIDRO")) {
      snprintf(caminho, sizeof caminho, "%s/aparencia-20.txt", dados_dir());
      f = fopen(caminho, "w");
      assert(f);
      fputs("1\n", f);
      fclose(f);
    }
    ajustes_dir(dados_dir()); }

  montar(&d);
  // O CARTAO DE AMIGOS le socialvis. Com -DNV_SOCIALVIS_DEMO (perfil_shot.sh)
  // entram os amigos de exemplo de socialvis.c — tres, um vendo agora —, que
  // e o caso do mockup; sem a flag o cartao sai no estado sem amigos.
#ifdef NV_SOCIALVIS_DEMO
  socialvis_demo(4);
#endif

  // 1. CARREGANDO: o esqueleto tem de cair nas mesmas caixas do conteudo real.
  //    Compare esta captura com a seguinte — nada pode saltar de lugar.
  perfil_iniciar(); perfil_abrir(); perfil_definir_carregando(1);
  snprintf(nome, sizeof nome, "%s-carregando.bmp", prefixo);
  captura(nome);

  // 2. PRONTO: o foco abre no grafico, no dia de hoje.
  perfil_definir_dados(&d);
#ifdef NV_TOUCH_UI
  if (retrato) {
    extern float nv_layout_w, nv_layout_h;
    // ajustes_dir aplicou o layout gravado (TV); o celular em pe vem por cima.
    layout_tela_definir(SW, SH); layout_modo_definir(1);
    printf("  retrato: layout %.0f x %.0f\n", nv_layout_w, nv_layout_h);
    snprintf(nome, sizeof nome, "%s-retrato-topo.bmp", prefixo);
    captura(nome);
    tecla(SDLK_DOWN);                     // cartazes: a coluna rola ate eles
    snprintf(nome, sizeof nome, "%s-retrato-cartazes.bmp", prefixo);
    captura(nome);
    tex_encerrar(); txt_encerrar(); gfx_encerrar();
    SDL_GL_DeleteContext(gl); SDL_DestroyWindow(win); SDL_Quit();
    puts("PASS: capturas do Perfil e Stats (retrato) gravadas.");
    return 0;
  }
#endif
  snprintf(nome, sizeof nome, "%s-painel.bmp", prefixo);
  captura(nome);

  // 3. Nove dias para tras: 29/10, o dia da maratona (a etiqueta aparece).
  for (i = 0; i < 9; i++) tecla(SDLK_LEFT);
  snprintf(nome, sizeof nome, "%s-dia.bmp", prefixo);
  captura(nome);

  // 4. OK abre a folha do dia; Voltar fecha so a folha.
  tecla(SDLK_RETURN); tecla(SDLK_DOWN);
  snprintf(nome, sizeof nome, "%s-folha.bmp", prefixo);
  captura(nome);
  { SDL_Event e = { 0 };
    e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_ESCAPE;
    perfil_evento(&e);
    assert(perfil_aberto()); }

  // 5. Baixo vai aos cartazes; direita anda um.
  tecla(SDLK_DOWN); tecla(SDLK_RIGHT);
  snprintf(nome, sizeof nome, "%s-cartazes.bmp", prefixo);
  captura(nome);

  // 6. Cima volta ao grafico; cima de novo vai a pilula Amigos.
  tecla(SDLK_UP); tecla(SDLK_UP);
  snprintf(nome, sizeof nome, "%s-amigos.bmp", prefixo);
  captura(nome);

#ifdef NV_SOCIALVIS_DEMO
  // 4c. O DUELO com o Pedro (o amigo de exemplo com perfil do servidor). A
  //     comparacao de exemplo SO existe aqui, na captura: a do app vem de
  //     recomenda_amigo e nunca de numero fixo.
  { SvPerfil pp;
    int k = socialvis_amigo_indice("nuvio:pedro");
    assert(k >= 0 && socialvis_perfil("nuvio:pedro", &pp));
    pp.estado = SV_PERFIL_OK; pp.compartilha = 1;
    pp.cmp[SV_CMP_MATCH].estado = SV_CMPE_OK; pp.cmp[SV_CMP_MATCH].pct = 82;
    pp.cmp[SV_CMP_MATCH].iguais = 14; pp.cmp[SV_CMP_MATCH].total = 17;
    pp.cmp[SV_CMP_COMUM].estado = SV_CMPE_OK;
    pp.cmp[SV_CMP_COMUM].filmes = 9; pp.cmp[SV_CMP_COMUM].series = 5;
    socialvis_definir_perfil_extra("nuvio:pedro", &pp);
    tecla(SDLK_RETURN);
    for (i = 0; i < k; i++) tecla(SDLK_RIGHT); }
  snprintf(nome, sizeof nome, "%s-duelo.bmp", prefixo);
  captura(nome);
  tecla(SDLK_DOWN);
  snprintf(nome, sizeof nome, "%s-duelo-cartazes.bmp", prefixo);
  captura(nome);
  // Um amigo SEM perfil do servidor: os numeros dele sao "—" com o motivo.
  tecla(SDLK_UP); tecla(SDLK_LEFT);
  snprintf(nome, sizeof nome, "%s-duelo-sem-dado.bmp", prefixo);
  captura(nome);
  // Voltar sai do duelo para o resumo, e nao da tela.
  { SDL_Event e = { 0 };
    e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_ESCAPE;
    perfil_evento(&e);
    assert(perfil_aberto()); }
#endif

  // 5. SEM IDENTIDADE E SEM GENERO: o caso do Trakt que so devolve historico.
  //    A banda de numeros nao pode desabar nem deslocar as secoes de baixo.
  { PerfilDados m = d;
    m.nome[0] = m.usuario[0] = m.avatar[0] = 0;
    m.nGeneros = 0; m.cobertura |= PF_COB_ESTIMADO; m.marcados = 12;
    perfil_definir_dados(&m); }
  snprintf(nome, sizeof nome, "%s-sem-identidade.bmp", prefixo);
  captura(nome);

  // 6. ESTADO VAZIO: sem snapshot, OK pede nova tentativa.
  perfil_definir_dados(NULL);
  perfil_definir_estado(PERFIL_ESTADO_DESCONECTADO, NULL);
  snprintf(nome, sizeof nome, "%s-vazio.bmp", prefixo);
  captura(nome);

  tex_encerrar(); txt_encerrar(); gfx_encerrar();
  SDL_GL_DeleteContext(gl); SDL_DestroyWindow(win); SDL_Quit();
  puts("PASS: capturas do Perfil e Stats gravadas.");
  return 0;
}

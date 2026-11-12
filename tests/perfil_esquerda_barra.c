// #371 (Samsung S95C .tpk, 2.0.2): on "Profile & Stats", Left at the left edge
// opens the sidebar OVER the page (app.c: saiuPorEsquerda -> menu_abrir). The
// page used to call perfil_fechar() there, which clears `aberto`, and
// perfil_desenhar / perfil_evento both return early when !aberto: the sidebar
// opened over an empty screen and, once closed, the page stayed blank and dead.
// Left at the edge must ask for the bar WITHOUT closing the page; Back still closes.
#include "../src/perfil.c"
#include "dados.h"
#include "socialvis.h"
#include <assert.h>

static void tecla(SDL_Keycode k) {
  SDL_Event e;
  memset(&e, 0, sizeof e);
  e.type = SDL_KEYDOWN;
  e.key.keysym.sym = k;
  perfil_evento(&e);
}

// The 30-day dashboard (perfil_painel.inc): focus opens on TODAY in the chart,
// so the left edge is day 0, reached by walking back through the days.
static void abrir_com_dados(int nTopo) {
  PerfilDados d;
  memset(&d, 0, sizeof d);
  snprintf(d.nome, sizeof d.nome, "Test");
  d.minutos = 90; d.nDias = 30; d.cobertura = PF_COB_NUVIO;
  d.nObras = nTopo; d.nTopo = nTopo;
  for (int i = 0; i < nTopo; i++) { snprintf(d.obras[i].titulo, sizeof d.obras[i].titulo, "T%d", i); d.topo[i] = i; }
  perfil_iniciar();
  perfil_abrir();                         // opens first, as the router does; data arrives after
  perfil_definir_dados(&d);
}
static void ate_dia0(void) { for (int i = 0; i < PERFIL_DIAS - 1; i++) tecla(SDLK_LEFT); }

// Friends come from socialvis (the same feed the Friends card reads).
static void com_amigos(int n) {
  SvEvento v[3];
  memset(v, 0, sizeof v);
  for (int i = 0; i < n; i++) {
    snprintf(v[i].pessoaId, sizeof v[i].pessoaId, "nuvio:a%d", i);
    snprintf(v[i].pessoaNome, sizeof v[i].pessoaNome, "Amigo%d", i);
    v[i].acao = SV_FIM; v[i].reacao = SV_REAC_NADA; v[i].pct = -1; v[i].restanteMin = -1;
    snprintf(v[i].imdb, sizeof v[i].imdb, "tt%d", i);
    snprintf(v[i].titulo, sizeof v[i].titulo, "Titulo%d", i);
    v[i].quando = (long long)time(NULL) - 3600;
  }
  socialvis_definir_feed(v, n);
}

int main(void) {
  dados_iniciar("deploy/app/art");
  ajustes_dir(dados_dir());

  abrir_com_dados(3);                     // chart, focus on today
  assert(secao == 0 && dia == PERFIL_DIAS - 1);
  ate_dia0();                             // walking back stays inside the page
  assert(!perfil_quer_sair() && dia == 0);
  tecla(SDLK_LEFT);                       // day 0: the edge
  assert(perfil_quer_sair());             // the router opens the bar
  assert(perfil_aberto());                // ...and the page must stay alive under it

  abrir_com_dados(3);                     // posters row: the first poster is the edge
  tecla(SDLK_DOWN);
  assert(secao == 1 && item == 0);
  tecla(SDLK_RIGHT); tecla(SDLK_LEFT);
  assert(!perfil_quer_sair() && item == 0);
  tecla(SDLK_LEFT);
  assert(perfil_quer_sair());
  assert(perfil_aberto());

  com_amigos(2);                          // the Friends pill (UP from the chart)
  abrir_com_dados(0);
  assert(rostosCabem() == 2);
  tecla(SDLK_UP);
  assert(secao == 2);
  tecla(SDLK_LEFT);
  assert(perfil_quer_sair());
  assert(perfil_aberto());

  com_amigos(0);                          // nothing watched, no friends
  abrir_com_dados(0);
  ate_dia0(); tecla(SDLK_LEFT);
  assert(perfil_quer_sair());
  assert(perfil_aberto());

  abrir_com_dados(3);                     // Back is still the way out to Home
  tecla(SDLK_AC_BACK);
  assert(perfil_quer_sair());
  assert(!perfil_aberto());

  puts("PASS: perfil_esquerda_barra");
  return 0;
}

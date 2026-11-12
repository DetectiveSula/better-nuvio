// PR #455: Trakt primeiro, Simkl so como fallback; resposta de outro perfil
// ou conta nunca e publicada. O worker e o de app.c; somente os trackers e
// a identidade da sessao sao dubles, sem rede nem janela.
#define trakt_ativo teste_trakt_ativo
#define trakt_perfil teste_trakt_perfil
#define simkl_ativo teste_simkl_ativo
#define simkl_perfil teste_simkl_perfil
#define sessao_logada teste_sessao_logada
#define sessao_usuario teste_sessao_usuario
#define perfis_ativo teste_perfis_ativo
#define metaprov_meta teste_metaprov_meta
#include "../src/app.c"
#include "stub_m3u.h"
#include <assert.h>

static int tkLigado, skLigado, tkOk, skOk, tkChamadas, skChamadas;
static int perfilAtual = 1;
static const char *contaAtual = "conta-teste";
unsigned long long simklauth_credencial_copiar(char *dst, size_t tam) {
  (void)dst; (void)tam; return 1;
}
int teste_trakt_ativo(void) { return tkLigado; }
int teste_simkl_ativo(void) { return skLigado; }
int teste_sessao_logada(void) { return 1; }
const char *teste_sessao_usuario(void) { return contaAtual; }
int teste_perfis_ativo(void) { return perfilAtual; }
int teste_trakt_perfil(PerfilDados *d) {
  tkChamadas++;
  snprintf(d->nome, sizeof d->nome, "Trakt");
  return tkOk;
}
static int metaChamadas;
char *teste_metaprov_meta(const char *tipo, const char *id, int seg, int *prov) {
  (void)tipo; (void)seg; if (prov) *prov = 0;
  metaChamadas++;
  char *c = malloc(256);
  snprintf(c, 256, "{\"meta\":{\"name\":\"Titulo %s\",\"genres\":[\"Drama\"],\"runtime\":\"50 min\"}}", id);
  return c;
}
int teste_simkl_perfil(PerfilDados *d) {
  skChamadas++;
  snprintf(d->nome, sizeof d->nome, "Simkl");
  return skOk;
}

static void consultar(int tk, int sk, int okTk, int okSk, int antigo) {
  PerfilPedido *p = calloc(1, sizeof *p);
  assert(p);
  tkLigado=tk; skLigado=sk; tkOk=okTk; skOk=okSk;
  tkChamadas=skChamadas=0;
  p->geracao=atomic_load(&perfilGeracao) - (unsigned)antigo;
  p->perfil=1;
  snprintf(p->conta, sizeof p->conta, "conta-teste");
  memset(&perfilPendente, 0, sizeof perfilPendente);
  perfilSucesso=0;
  atomic_store(&perfilCarga, 1);
  carregarPerfil(p);
}

// Pedido de Nuvio sync: o que pedirPerfil copia do fio principal, sem rede.
static void consultarNuvio(int antigo) {
  PerfilPedido *p = calloc(1, sizeof *p);
  assert(p);
  tkLigado = skLigado = 1; tkOk = skOk = 1;
  tkChamadas = skChamadas = 0;
  p->geracao = atomic_load(&perfilGeracao) - (unsigned)antigo;
  p->perfil = 1;
  snprintf(p->conta, sizeof p->conta, "conta-teste");
  p->nuvio = 1;
  p->agoraMs = 1794000000000LL;
  snprintf(p->nome, sizeof p->nome, "Sam");
  p->vistos = calloc(2, sizeof *p->vistos);
  p->prog = calloc(1, sizeof *p->prog);
  snprintf(p->vistos[0].id, sizeof p->vistos[0].id, "tt7000");
  snprintf(p->vistos[0].tipo, sizeof p->vistos[0].tipo, "series");
  p->vistos[0].temporada = 1; p->vistos[0].episodio = 1;
  p->vistos[0].vistoMs = p->agoraMs - 3600000LL;
  p->nVistos = 1;
  p->nMetas = 1;
  snprintf(p->metas[0].id, sizeof p->metas[0].id, "tt7000");
  snprintf(p->tipos[0], sizeof p->tipos[0], "series");
  memset(&perfilPendente, 0, sizeof perfilPendente);
  perfilSucesso = 0;
  atomic_store(&perfilCarga, 1);
  carregarPerfil(p);
}

int main(void) {
  consultar(1, 1, 1, 1, 0);
  assert(tkChamadas == 1 && skChamadas == 0);
  assert(perfilSucesso && !strcmp(perfilPendente.nome, "Trakt"));
  consultar(1, 1, 0, 1, 0);
  assert(tkChamadas == 1 && skChamadas == 1);
  assert(perfilSucesso && !strcmp(perfilPendente.nome, "Simkl"));
  consultar(0, 1, 0, 1, 0);
  assert(!tkChamadas && skChamadas == 1 && perfilSucesso);
  consultar(0, 0, 0, 0, 0);
  assert(!tkChamadas && !skChamadas && !perfilSucesso);
  assert(atomic_load(&perfilCarga) == 2);
  consultar(1, 1, 0, 0, 0);
  assert(!perfilSucesso && atomic_load(&perfilCarga) == 2);
  consultar(0, 1, 0, 1, 1);
  assert(!perfilSucesso && !perfilPendente.nome[0] && atomic_load(&perfilCarga) == 1);
  perfilAtual=2;
  consultar(0, 1, 0, 1, 0);
  assert(!perfilSucesso && !perfilPendente.nome[0]);
  perfilAtual=1; contaAtual="outra-conta";
  consultar(0, 1, 0, 1, 0);
  assert(!perfilSucesso && !perfilPendente.nome[0]);
  perfilAtual=1; contaAtual="conta-teste";
  consultarNuvio(0);
  assert(!tkChamadas && !skChamadas && metaChamadas == 1);
  assert(perfilSucesso && !strcmp(perfilPendente.nome, "Sam"));
  assert(perfilPendente.cobertura & PF_COB_NUVIO);
  assert(perfilPendente.episodios == 1 && perfilPendente.minutos == 50);
  assert(perfilPendente.nTopo == 1 && !strcmp(perfilPendente.obras[0].titulo, "Titulo tt7000"));
  consultarNuvio(0);
  assert(metaChamadas == 1);          // segunda carga sai do cache
  assert(perfilSucesso && perfilPendente.minutos == 50);
  consultarNuvio(1);
  assert(!perfilSucesso && !perfilPendente.nome[0]);
  puts("PASS: perfil_tracker (Nuvio sync primeiro; meta em cache; resposta obsoleta)");
  puts("PASS: perfil_tracker (prioridade, fallback, falhas e resposta obsoleta)");
  return 0;
}

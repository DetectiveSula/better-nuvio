/* Stage H Family Watching — the live transport with a FAKE account server:
 * only the two existing read RPCs are called, with the other profile's id;
 * other accounts, release builds, failures and mid-read account changes give
 * nothing. Compiled with FAM_CONTA_TESTE set to the fixture account. */
#include "familia.h"
#include "perfis.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *i18n(const char *s) { return s; }
static Uint32 relogio = 1000;
Uint32 SDL_GetTicks(void) { return relogio; }
static ContaPerfil pf[2];
int ajustes_familia(void) { return 1; }
int ajustes_familia_partilha_perfil(int p) { return p == 2; }
int perfis_n(void) { return 2; }
const ContaPerfil *perfis_item(int i) { return i >= 0 && i < 2 ? &pf[i] : NULL; }
int perfis_ativo(void) { return 1; }
int perfil_infantil(int p) { (void)p; return 0; }
int perfil_infantil_ativo(void) { return 0; }
void desc_repetir(void) {}

/* fake session + server */
static const char *usuario = "fixture-user";
static int logado = 1, trocarNoMeio, status = 200, chamadas;
static char nomes[16][48], corpos[16][128];
int sessao_logada(void) { return logado; }
int sessao_identidade_local_copiar(char *u, size_t nu, char *j, size_t nj) {
  snprintf(u, nu, "%s", usuario); snprintf(j, nj, "fixture-token"); return logado; }
int sessao_token_copiar(char *d, size_t n) { snprintf(d, n, "fixture-token"); return 1; }
char *nuvem_rpc_com(const char *f, const char *c, const char *b, int *st) {
  (void)b;
  snprintf(nomes[chamadas], sizeof nomes[0], "%s", f);
  snprintf(corpos[chamadas], sizeof corpos[0], "%s", c);
  chamadas++;
  if (trocarNoMeio && chamadas == 2) usuario = "other-user";
  *st = status;
  if (status != 200) return strdup("{\"message\":\"x\"}");
  if (!strcmp(f, "sync_pull_watch_progress"))
    return strdup("[{\"content_id\":\"tt0903747\",\"content_type\":\"series\",\"season\":2,\"episode\":5,"
                  "\"position_ms\":1500000,\"duration_ms\":2820000,\"last_watched\":\"2026-11-06T18:00:00Z\"}]");
  return strdup("[{\"content_id\":\"tt0111161\",\"content_type\":\"movie\",\"season\":null,\"episode\":null,"
                "\"watched_at\":\"2026-11-05T10:00:00Z\"}]");
}

static int ler(void) {
  FamCartao c[FAM_MAX];
  relogio += FAM_REFAZ_MS + 6000; familia_home_aberta(); familia_passo();
  return familia_atualizar(c, FAM_MAX);
}

int main(void) {
  for (int i = 0; i < 2; i++) { pf[i].indice = i + 1; snprintf(pf[i].nome, sizeof pf[i].nome, "P%d", i + 1); }
  assert(familia_rede_conta_permitida("5e32e901") && familia_rede_conta_permitida("e0af81c2"));
  assert(!familia_rede_conta_permitida("-") && !familia_rede_conta_permitida("") && !familia_rede_conta_permitida(NULL));
  assert(!familia_tem_busca());
  familia_rede_registrar();
  assert(familia_tem_busca());

  /* Test account: exactly the two read RPCs, for profile 2 only. */
  assert(ler() == 1 && chamadas == 2);
  assert(!strcmp(nomes[0], "sync_pull_watch_progress") && strstr(corpos[0], "\"p_profile_id\":2"));
  assert(!strcmp(nomes[1], "sync_pull_watched_items") && strstr(corpos[1], "\"p_profile_id\":2"));
  { FamCartao c; assert(familia_cartao_do_perfil(2, &c) && c.estado == FAM_EM_CURSO && c.episodio == 5); }
  for (int i = 0; i < chamadas; i++) assert(!strstr(nomes[i], "push") && !strstr(nomes[i], "delete"));

  /* Another account reads the same way (any signed-in account). */
  chamadas = 0; usuario = "other-user";
  assert(ler() == 1 && chamadas == 2);
  for (int i = 0; i < chamadas; i++) assert(!strstr(nomes[i], "push") && !strstr(nomes[i], "delete"));
  /* Signed out: nothing. */
  usuario = "fixture-user"; logado = 0; chamadas = 0;
  assert(ler() == 0 && chamadas == 0);
  /* Server errors: no card. */
  logado = 1; status = 500;
  assert(ler() == 0 && chamadas == 2);
  /* Account switched during the read: result dropped. */
  status = 200; chamadas = 0; trocarNoMeio = 1;
  assert(ler() == 0);
  puts("ok Family transport: any signed-in account, two read RPCs with the other profile's id, nothing when signed out/errors/account change");
  return 0;
}

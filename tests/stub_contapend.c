// Jornal da conta (contapend.c) desligado, para os testes que linkam sync.c
// sem conta de verdade. O jornal tem teste proprio: tests/contapend.sh.
long long contapend_agora_ms(void) { return 0; }
int  contapend_aplicar_local(void) { return 0; }
int  contapend_enviar(void) { return 0; }
void visto_iniciar(void) {}
int  contapend_reenviar(void) { return 0; }
void contapend_esquecer(void) {}
int  contapend_pendentes(void) { return 0; }
void contapend_podar(long long desdeMs) { (void)desdeMs; }
int  contapend_lista(const char *i, const char *t, const char *n, const char *p, int s) {
  (void)i; (void)t; (void)n; (void)p; (void)s; return 0;
}
void contapend_chutar(void) {}
// Salvos sem migracao pendente (o modulo tem teste proprio: tests/salvos.sh).
// So para quem NAO linka src/salvos.c.
#ifndef STUB_CONTAPEND_COM_SALVOS
struct SalvoItem;
int  salvos_n(void) { return 0; }
const void *salvos_item(int i) { (void)i; return 0; }
int  salvos_migracao_conta(int *p) { (void)p; return 0; }
void salvos_migracao_conta_feita(void) {}
int  salvos_perfil_atual(void) { return 1; }
#endif

// #360: synchronization harnesses stub the addon list; counts are verified
// against real addons.c in addons_diferenca.sh.
#include "addons.h"
__attribute__((weak)) void addons_diferenca(const AddonRemoto *a, int na,
    const AddonRemoto *d, int nd, int *e, int *s, int *m) {
  (void)a; (void)na; (void)d; (void)nd;
  if (e) *e = 0;
  if (s) *s = 0;
  if (m) *m = 0;
}

// Account-session collaborators the PR's sync.c calls. Fixtures that link
// sync.c without the real modules get inert weak defaults; a linked real
// module (strong definition) overrides them.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "sessao.h"
#include "debrid.h"
#define WEAK __attribute__((weak))
WEAK void recomenda_esquecer_conta(void) {}
int perfis_puxar(void);
WEAK int perfis_puxar_sessao(uint64_t s, const char *u, const char *j) { (void)s; (void)u; (void)j; return perfis_puxar(); }
WEAK void perfis_invalidar_sessao(void) {}
// The cycle now pins identity and token up front and calls nuvem_*; the
// fixtures still intercept at sessao_tabela / sessao_rpc, so forward there.
WEAK char *nuvem_tabela(const char *t, const char *c, const char *b, int *st) {
  (void)b; return sessao_tabela(t, c, st); }
WEAK char *nuvem_rpc_com(const char *f, const char *c, const char *b, int *st) {
  (void)b; return sessao_rpc(f, c, st); }
WEAK int sessao_identidade_local_copiar(char *u, size_t nu, char *j, size_t nj) {
  if (!sessao_logada()) return 0;
  snprintf(u, nu, "%s", sessao_usuario()); snprintf(j, nj, "token-de-teste"); return 1; }
WEAK int sessao_token_copiar(char *t, size_t n) { snprintf(t, n, "token-de-teste"); return 1; }
WEAK void sessao_token_recusado(const char *t) { (void)t; }
WEAK void extras_abrir_sessao(void) {}
WEAK void desc_sessao_preparada(void) {}
WEAK void desc_sessao_passo(void) {}
WEAK void desc_abrir_sessao(void) {}
WEAK void debrid_conta_iniciar(DebridConta *c) { (void)c; }
WEAK void debrid_conta_definir(DebridConta *c, const char *s, const char *k) { (void)c; (void)s; (void)k; }
WEAK int debrid_aplicar_chaves_conta(const DebridConta *c, uint64_t s) { (void)c; (void)s; return 0; }
WEAK void ajustes_perfil_padrao(void) {}
WEAK unsigned long long ajustes_sync_hash(void) { return 0; }
int syncprog_puxar(void);
int syncprog_empurrar(void);
WEAK int syncprog_puxar_sessao(int p, uint64_t s) { (void)p; (void)s; return syncprog_puxar(); }
WEAK int syncprog_empurrar_sessao(int p, uint64_t s) { (void)p; (void)s; return syncprog_empurrar(); }
// Mach-O cannot link an undefined weak_import into a static fixture, so the
// no-profile session (generation 0) and the sync.c logout hooks are weak defaults.
WEAK uint64_t perfis_sessao_geracao(void) { return 0; }
WEAK int perfis_sessao_atual(uint64_t g) { return g == 0; }
WEAK int perfis_sessao_travar(uint64_t g) { return g == 0; }
WEAK void perfis_sessao_destravar(void) {}
WEAK void fontevolta_esquecer_tudo(void) {}
WEAK void guia_fonte_esquecer(const char *id) { (void)id; }
#include "contalib.h"
WEAK int ajustes_fonte_biblioteca(void) { return 0; }
WEAK int contalib_n(void) { return 0; }
WEAK const ContaLibItem *contalib_item(int i) { (void)i; return NULL; }
WEAK void iptv_contas_esquecer_todas(void) {}
WEAK void m3u_fontes_invalidar(void) {}
WEAK void stalker_fontes_invalidar(void) {}
WEAK void xtream_fontes_invalidar(void) {}
WEAK int traktauth_desconectado(void) { return 0; }
WEAK char *nuvem_addon_alternar(const char *query, int enabled, const char *bearer,
                                unsigned deadline, int *status) {
  (void)query; (void)enabled; (void)bearer; (void)deadline;
  if (status) *status = 503;
  return NULL;
}

// Stage I/J settings bridge and later account hooks, for harnesses that link
// sync.c without ajustes.c. Weak: a test that links the real module wins.
WEAK void ajustes_better_contexto(void) {}
WEAK void ajustes_better_observar(void) {}
WEAK int  ajustes_better_passo(void) { return 0; }
WEAK void ajustes_better_ciclo(uint64_t session) { (void)session; }
WEAK void ajustes_better_esquecer(void) {}
WEAK int  ajustes_compartilhados_resposta(const char *r, char **out) { (void)r; if (out) *out = NULL; return 0; }
WEAK const char *ajustes_compartilhados_motivo(void) { return ""; }
WEAK void ajustes_compartilhados_contexto(void) {}
WEAK void ajustes_compartilhados_esquecer(void) {}
WEAK void ajustes_compartilhados_observar(void) {}
WEAK int  ajustes_compartilhados_pendentes(void) { return 0; }
WEAK int  ajustes_compartilhados_aplicar(const char *json) { (void)json; return 0; }
WEAK int  ajustes_compartilhados_mesclar(const char *base, char **out) { (void)base; if (out) *out = NULL; return 0; }
WEAK void ajustes_compartilhados_confirmar(const char *sent) { (void)sent; }
WEAK int  ajustes_cw_fonte(void) { return 0; }
WEAK int  ajustes_edicao_perfil(void) { return 0; }
WEAK void ajustes_edicao_encerrar(void) {}
WEAK int  ajustes_migrar_escopo(void) { return 0; }
WEAK void perfil_infantil_esquecer(void) {}
int perfis_ativo(void);
// Real rule (perfis.c): the active profile, or 1 when it borrows the primary's add-ons.
WEAK int  perfis_home_config(void) { return perfis_ativo(); }
WEAK int  syncprog_reenviar_deletes(void) { return 0; }

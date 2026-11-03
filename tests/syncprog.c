// Contrato do progresso com a conta: o corpo do push tem o formato do web, o
// pull aceita os formatos que o servidor devolve, e o cenario de rollback
// (pull antigo aplicado por cima do local recem-assistido) NAO regride.
// Sem rede: sessao_rpc e um duble que grava o pedido e devolve o combinado.
#include "syncprog.h"
#include "progresso.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvtmp.h"

// ---- dublês ----------------------------------------------------------------
static char *arquivo;
static int   perfil = 1;
static long long agora = 1757000000000LL;

static char *deleteFile, *dismissFile;
static int failDeleteWrite;
static char **fileFor(const char *n) {
  if (!strcmp(n, "progresso-deletes.txt")) return &deleteFile;
  if (!strcmp(n, "cwoculto.txt")) return &dismissFile;
  return &arquivo;
}
char *dados_ler(const char *n) { char *s = *fileFor(n); return s ? strdup(s) : NULL; }
int dados_gravar(const char *n, const char *c) { if (failDeleteWrite && !strcmp(n, "progresso-deletes.txt")) return 0; char **f = fileFor(n); free(*f); *f = strdup(c); return !!*f; }
int dados_apagar(const char *n) { char **f = fileFor(n); free(*f); *f = NULL; return 1; }
const char *dados_cliente_id(void) { return "cliente-teste-0001"; }
int   perfis_ativo(void) { return perfil; }
uint64_t perfis_sessao_geracao(void) { return 0; }
int perfis_sessao_atual(uint64_t g) { return !g; }
int perfis_sessao_travar(uint64_t g) { return !g; }
void perfis_sessao_destravar(void) {}
static long long relogio(void) { return agora; }

static char ultimaFuncao[64], ultimoCorpo[4096];
static int  chamadas, proximoStatus = 200;
static const char *proximaResposta = "[]";
static void (*duranteRpc)(void);
char *sessao_rpc(const char *funcao, const char *corpo, int *status) {
  snprintf(ultimaFuncao, sizeof ultimaFuncao, "%s", funcao);
  snprintf(ultimoCorpo, sizeof ultimoCorpo, "%s", corpo);
  chamadas++;
  if (duranteRpc) duranteRpc();
  if (status) *status = proximoStatus;
  return proximaResposta ? strdup(proximaResposta) : NULL;
}
char *nuvem_rpc_com(const char *fn, const char *corpo, const char *jwt, int *st) {
  (void)jwt;
  return sessao_rpc(fn, corpo, st);
}
int sessao_identidade_local_copiar(char *u, size_t nu, char *j, size_t nj) {
  snprintf(u, nu, "usuario-teste");
  snprintf(j, nj, "token-teste");
  return 1;
}
int sessao_token_copiar(char *j, size_t nj) { snprintf(j, nj, "token-teste"); return 1; }
void sessao_token_recusado(const char *t) { (void)t; }
const char *sessao_usuario(void) { return "usuario-teste"; }

// Catalogo: so tt1234567 e tt7654321 existem.
static int aplicadosNoCatalogo;
static double ultimaPosAplicada;
int cat_indice_por_imdb(const char *imdb) {
  if (!strcmp(imdb, "tt1234567")) return 0;
  if (!strcmp(imdb, "tt7654321")) return 1;
  return -1;
}
void cat_aplicar_progresso(int i, double pos, double dur, int t, int e) {
  (void)i; (void)dur; (void)t; (void)e;
  aplicadosNoCatalogo++;
  ultimaPosAplicada = pos;
}

static void zerar(void) {
  failDeleteWrite = 0;
  free(arquivo); arquivo = NULL;
  free(deleteFile); deleteFile = NULL; free(dismissFile); dismissFile = NULL;
  prog_invalidar();
  syncprog_esquecer();
  chamadas = 0; proximoStatus = 200; proximaResposta = "[]";
  duranteRpc = NULL;
  aplicadosNoCatalogo = 0; ultimaPosAplicada = -1;
  ultimoCorpo[0] = 0; ultimaFuncao[0] = 0;
}

static int tem(const char *trecho) { return strstr(ultimoCorpo, trecho) != NULL; }

static ProgRegistro porChave(const char *chave) {
  ProgRegistro c; memset(&c, 0, sizeof c);
  prog_por_chave(chave, &c);
  return c;
}

// ---- casos -----------------------------------------------------------------

static void pushNoFormatoDoWeb(void) {
  ProgRegistro r[8];
  zerar();
  agora = 1757000100000LL;
  assert(prog_gravar_local("tt1234567", 4, 9, 1432, 2640, -1));      // serie de catalogo
  assert(prog_gravar_local("tt7654321", 0, 0, 5400, 7200, -1));      // filme
  assert(prog_gravar_local("tt5550000", 0, 0, 10, 30, -1));          // dur < 60s: nao vai
  assert(prog_pendentes(r, 8) == 3);

  assert(syncprog_empurrar() == 3);
  assert(chamadas == 1 && !strcmp(ultimaFuncao, "sync_push_watch_progress"));
  assert(tem("\"p_profile_id\":1"));
  assert(tem("\"p_origin_client_id\":\"cliente-teste-0001\""));
  // Serie: chave do web, tipo certo, episodio separado, video_id sintetico, ms.
  assert(tem("\"content_id\":\"tt1234567\""));
  assert(tem("\"content_type\":\"series\""));
  assert(tem("\"video_id\":\"__nuvio_episode__:4:9\""));
  assert(tem("\"season\":4") && tem("\"episode\":9"));
  assert(tem("\"position\":1432000") && tem("\"duration\":2640000"));
  assert(tem("\"last_watched\":1757000100000"));
  assert(tem("\"progress_key\":\"tt1234567_s4e9\""));
  // Filme: season/episode nulos, video_id = content_id, chave = content_id.
  assert(tem("\"content_type\":\"movie\""));
  assert(tem("\"video_id\":\"tt7654321\""));
  assert(tem("\"season\":null") && tem("\"episode\":null"));
  assert(tem("\"progress_key\":\"tt7654321\""));
  // O antigo formato NAO aparece.
  assert(!tem("tt1234567:4:9"));
  assert(tem("tt5550000"));
  // 2xx: as duas enviadas deixam de ser pendentes; a curta continua.
  assert(prog_pendentes(r, 8) == 0);
  // Sem pendentes que valham, nao ha pedido.
  chamadas = 0;
  assert(syncprog_empurrar() == 0 && chamadas == 0);
  puts("ok  push: formato do web, so pendentes, marca empurrados em 2xx");
}

static void pushFalhoMantemPendente(void) {
  ProgRegistro r[8];
  zerar();
  assert(prog_gravar_local("tt1234567", 4, 9, 1432, 2640, -1));
  proximoStatus = 500;
  assert(syncprog_empurrar() == -1);
  assert(prog_pendentes(r, 8) == 1 && r[0].pendente);
  puts("ok  push com erro: continua pendente");
}

static void pullAceitaOsFormatosDoServidor(void) {
  ProgRegistro s;
  zerar();
  agora = 1757000200000LL;
  proximaResposta =
    "[{\"content_id\":\"tt1234567\",\"content_type\":\"series\",\"season\":4,\"episode\":9,"
    "  \"position_ms\":900000,\"duration_ms\":2640000,\"position\":900000,\"duration\":2640000,"
    "  \"updated_at\":\"2026-09-04T18:00:00.250+00:00\",\"progress_key\":\"tt1234567_s4e9\"},"
    " {\"content_id\":\"tt7654321\",\"content_type\":\"movie\",\"season\":null,\"episode\":null,"
    "  \"position\":5400000,\"duration\":7200000,\"last_watched\":1757000150000},"
    " {\"content_id\":\"tt5550000:2:3\",\"position\":1000,\"duration\":100000,\"last_watched\":1757000150},"
    " {\"content_id\":\"tt9990000\",\"position\":10,\"duration\":500}]";
  assert(syncprog_puxar() == 3);            // a de dur <= 1s fica de fora
  assert(!strcmp(ultimaFuncao, "sync_pull_watch_progress") && tem("\"p_profile_id\":1"));
  assert(syncprog_puxadas() == 3);
  { int casaram = -1;
    assert(syncprog_aplicar(&casaram) == 3);
    assert(casaram == 2 && aplicadosNoCatalogo == 2);   // tt5550000 nao esta no catalogo
    assert(syncprog_puxadas() == 0); }
  s = porChave("tt1234567_s4e9");
  assert(s.chave[0] && s.posSeg == 900 && s.durSeg == 2640 && !s.pendente);
  assert(s.lastWatchedMs == 1788544800250LL);        // 2026-09-04T18:00:00.250Z
  s = porChave("tt7654321");
  assert(s.chave[0] && s.posSeg == 5400 && s.lastWatchedMs == 1757000150000LL && !strcmp(s.tipo, "movie"));
  // Cliente antigo mandou id composto: chave recalculada, segundos virados em ms.
  s = porChave("tt5550000_s2e3");
  assert(s.chave[0] && s.temporada == 2 && s.episodio == 3 && !strcmp(s.contentId, "tt5550000"));
  assert(s.lastWatchedMs == 1757000150000LL);
  assert(!porChave("tt9990000").chave[0]);
  puts("ok  pull: ISO e numero, ms e s, id composto, retem sem catalogo");
}

static void rollbackNaoAcontece(void) {
  ProgRegistro s;
  zerar();
  agora = 1757000300000LL;
  // 1. Assistiu aqui ate 1500s. Pendente.
  assert(prog_gravar_local("tt1234567", 4, 9, 1500, 2640, -1));
  // 2. Ciclo: pull traz o servidor ANTIGO (900s, 5 min atras) ...
  proximaResposta =
    "[{\"content_id\":\"tt1234567\",\"season\":4,\"episode\":9,"
    "  \"position\":900000,\"duration\":2640000,\"last_watched\":1757000000000}]";
  assert(syncprog_puxar() == 1);
  // 3. ... e empurra o novo.
  assert(syncprog_empurrar() == 1);
  assert(tem("\"position\":1500000"));
  // 4. No fio principal, o puxado e aplicado. Antes do conserto, aqui a posicao
  //    voltava para 900 e o arquivo era regravado com ela.
  assert(syncprog_aplicar(NULL) == 0);
  assert(aplicadosNoCatalogo == 0);
  s = porChave("tt1234567_s4e9");
  assert(s.posSeg == 1500 && !s.pendente);           // ja subiu, e continua 1500
  // 5. Proximo ciclo: o servidor agora tem o nosso (ou algo mais novo). Entra.
  proximaResposta =
    "[{\"content_id\":\"tt1234567\",\"season\":4,\"episode\":9,"
    "  \"position\":1600000,\"duration\":2640000,\"last_watched\":1757000400000}]";
  assert(syncprog_puxar() == 1 && syncprog_aplicar(NULL) == 1);
  assert(aplicadosNoCatalogo == 1 && ultimaPosAplicada == 1600);
  assert(porChave("tt1234567_s4e9").posSeg == 1600);
  // 6. Um eco velho depois disso nao regride.
  proximaResposta =
    "[{\"content_id\":\"tt1234567\",\"season\":4,\"episode\":9,"
    "  \"position\":900000,\"duration\":2640000,\"last_watched\":1757000000000}]";
  assert(syncprog_puxar() == 1 && syncprog_aplicar(NULL) == 0);
  assert(porChave("tt1234567_s4e9").posSeg == 1600);
  puts("ok  rollback: pull antigo nao volta a posicao, nem antes nem depois do push");
}

static void migradoSobeComChaveCerta(void) {
  zerar();
  // Arquivo do formato antigo: serie de catalogo (5 colunas) e item do Trakt.
  arquivo = strdup("tt1234567\t1432\t2640\t4\t9\ntt7654321:1:2\t100\t3000\n");
  prog_invalidar();
  agora = 1757000500000LL;
  assert(syncprog_empurrar() == 2);
  assert(tem("\"progress_key\":\"tt1234567_s4e9\"") && tem("\"content_type\":\"series\""));
  assert(tem("\"progress_key\":\"tt7654321_s1e2\"") && tem("\"content_id\":\"tt7654321\""));
  assert(!tem("tt7654321:1:2"));
  // Sem hora conhecida, vai a de agora — nunca 0.
  assert(tem("\"last_watched\":1757000500000"));
  puts("ok  linhas antigas sobem como serie, com a chave do web");
}

static void avancarDurantePush(void) {
  // Mesmo milissegundo: comparar somente a hora ainda perderia este avanco.
  assert(prog_gravar_local("tt1234567", 4, 9, 1500, 2640, -1));
}

static void trocarPerfilDurantePush(void) { perfil = 2; }

static void confirmacaoPreservaEscritaNovaEPerfil(void) {
  ProgRegistro r;
  zerar(); perfil = 1; agora = 1757000600000LL;
  assert(prog_gravar_local("tt1234567", 4, 9, 1432, 2640, -1));
  duranteRpc = avancarDurantePush;
  assert(syncprog_empurrar() == 1 && tem("\"position\":1432000"));
  r = porChave("tt1234567_s4e9");
  assert(r.posSeg == 1500 && r.pendente);
  duranteRpc = NULL;
  assert(syncprog_empurrar() == 1 && tem("\"position\":1500000"));
  assert(!porChave("tt1234567_s4e9").pendente);

  zerar(); perfil = 1;
  assert(prog_gravar_local("tt1234567", 4, 9, 100, 2640, -1));
  perfil = 2;
  assert(prog_gravar_local("tt1234567", 4, 9, 200, 2640, -1));
  perfil = 1; duranteRpc = trocarPerfilDurantePush;
  assert(syncprog_empurrar() == 1 && tem("\"p_profile_id\":1"));
  assert(perfil == 2 && porChave("tt1234567_s4e9").pendente);
  perfil = 1;
  assert(!porChave("tt1234567_s4e9").pendente);
  duranteRpc = NULL;
  puts("ok  confirmacao do push preserva escrita mais nova e perfil diferente");
}

static void pullRecusaNumerosForaDaFaixa(void) {
  zerar(); perfil = 1;
  proximaResposta =
    "[{\"content_id\":\"tt1\",\"season\":1e100,\"episode\":1,\"position\":10000,\"duration\":600000},"
    "{\"content_id\":\"tt2\",\"season\":1,\"episode\":1e100,\"position\":10000,\"duration\":600000},"
    "{\"content_id\":\"tt3\",\"season\":1,\"episode\":1.5,\"position\":10000,\"duration\":600000},"
    "{\"content_id\":\"tt4\",\"position\":10000,\"duration\":1e100},"
    "{\"content_id\":\"tt5\",\"position\":1e100,\"duration\":600000},"
    "{\"content_id\":\"tt6\",\"season\":1,\"episode\":1e309,\"position\":10000,\"duration\":600000},"
    "{\"content_id\":\"tt7\",\"position_ms\":1e309,\"position\":10000,\"duration\":600000},"
    "{\"content_id\":\"tt8\",\"duration_ms\":1e309,\"position\":10000,\"duration\":600000},"
    "{\"content_id\":\"tt9\",\"position_ms\":\"bad\",\"position\":10000,\"duration\":600000},"
    "{\"content_id\":\"tt7654321\",\"season\":null,\"episode\":null,\"position_ms\":null,\"duration_ms\":null,\"position\":20000,\"duration\":600000},"
    "{\"content_id\":\"tt1234567\",\"position\":10000,\"duration\":600000,\"updated_at\":1e100,\"last_watched\":1757000150000}]";
  assert(syncprog_puxar() == 2);
  assert(syncprog_aplicar(NULL) == 2);
  assert(porChave("tt1234567").lastWatchedMs == 1757000150000LL);
  assert(porChave("tt7654321").posSeg == 20);
  assert(!porChave("tt1_s0e1").chave[0]);
  puts("ok  pull rejeita coordenadas e tempos fora da faixa antes dos casts");
}

static void trocarPerfilDurantePull(void) { perfil = 2; }

static void pullDoPerfilAnteriorNaoSeMistura(void) {
  zerar(); perfil = 1;
  proximaResposta = "[{\"content_id\":\"tt1234567\",\"position\":10000,\"duration\":600000}]";
  duranteRpc = trocarPerfilDurantePull;
  assert(syncprog_puxar() == 1 && tem("\"p_profile_id\":1"));
  assert(perfil == 2 && syncprog_aplicar(NULL) == 0);
  assert(!porChave("tt1234567").chave[0]);
  perfil = 1; duranteRpc = NULL;
  assert(syncprog_puxar() == 1);
  perfil = 2; // A troca tambem pode ocorrer depois da resposta e antes da aplicacao.
  assert(syncprog_aplicar(NULL) == 0 && !porChave("tt1234567").chave[0]);
  perfil = 1;
  assert(syncprog_puxar() == 1 && syncprog_aplicar(NULL) == 1);
  assert(porChave("tt1234567").posSeg == 10);
  puts("ok  pull do perfil anterior e descartado depois da troca de perfil");
}

static void deleteSurvivesRestartAndSync(void) {
  zerar(); perfil = 1; agora++;
  assert(prog_gravar_local("tt1234567", 1, 1, 20, 600, 1));
  assert(prog_gravar_local("tt1234567", 1, 2, 30, 600, 1));
  perfil = 2;
  assert(prog_gravar_local("tt1234567", 1, 1, 40, 600, 1));
  perfil = 1;
  char *preDeleteDisk = strdup(arquivo);
  assert(prog_excluir_titulo("tt1234567:1:2"));
  free(arquivo); arquivo = preDeleteDisk; // crash after journal, before progress file replacement
  ProgDelete ds[8]; assert(prog_deletes(ds, 8, 1) == 3);
  prog_invalidar(); // process restart, same persisted files
  assert(prog_deletes(ds, 8, 1) == 3);
  assert(!porChave("tt1234567_s1e1").chave[0]);
  proximaResposta = "[{\"content_id\":\"tt1234567\",\"season\":1,\"episode\":1,\"position\":20000,\"duration\":600000,\"last_watched\":1999999999999}]";
  assert(syncprog_puxar() == 1 && syncprog_aplicar(NULL) == 0);
  proximoStatus = 500;
  assert(syncprog_empurrar() == -1 && prog_deletes(ds, 8, 1) == 3);
  proximoStatus = 200;
  assert(syncprog_empurrar() == 0 && prog_deletes(ds, 8, 1) == 0);
  prog_invalidar(); assert(!porChave("tt1234567_s1e1").chave[0]);
  assert(!strcmp(ultimaFuncao, "sync_delete_watch_progress"));
  assert(tem("p_origin_client_id"));
  perfil = 2; assert(porChave("tt1234567_s1e1").posSeg == 40);
  perfil = 1;
  puts("ok  title deletion covers all episodes, persists, blocks echoes, retries, isolates profiles");
}
static void intentionalPlaybackCancelsOnlyItsDelete(void) {
  zerar(); perfil = 1; agora++;
  assert(prog_gravar_local("tt1234567", 1, 1, 20, 600, 1));
  assert(prog_excluir_titulo("tt1234567"));
  ProgDelete old[4]; int n = prog_deletes(old, 4, 1); assert(n == 2);
  agora++;
  assert(prog_gravar_local("tt1234567", 1, 1, 50, 600, 1));
  for (int i = 0; i < n; i++) if (!strcmp(old[i].chave, "tt1234567_s1e1")) assert(!prog_delete_atual(&old[i]));
  assert(syncprog_empurrar() == 1);
  assert(!strcmp(ultimaFuncao, "sync_push_watch_progress") && tem("50000"));
  puts("ok  intentional playback cancels its delete, remaining deletes precede push");
}
static void failedCancellationCannotDeleteNewPlayback(void) {
  zerar(); perfil = 1; agora++;
  assert(prog_gravar_local("tt1234567", 1, 1, 20, 600, 1));
  assert(prog_excluir_titulo("tt1234567"));
  failDeleteWrite = 1;
  assert(prog_gravar_local("tt1234567", 1, 1, 20, 600, 1)); // same wall clock, same position
  prog_invalidar();
  ProgDelete ds[4]; int n = prog_deletes(ds, 4, 1);
  assert(n == 1 && !strcmp(ds[0].chave, "tt1234567")); // episode upsert cancelled its old delete durably
  failDeleteWrite = 0;
  assert(syncprog_empurrar() == 1 && !strcmp(ultimaFuncao, "sync_push_watch_progress"));
  prog_invalidar(); assert(prog_deletes(ds, 4, 1) == 0);
  puts("ok  local mutation generation protects playback across failed delete-journal cancellation and restart");
}
static void newPlaybackDuringPull(void) {
  agora++;
  assert(prog_gravar_local("tt1234567", 0, 0, 80, 600, 1));
  // Also exercise the race where the new upsert was acknowledged before apply.
  const char *keys[] = {"tt1234567"}; prog_marcar_empurrados(keys, 1);
}
static void snapshotsAreAuthoritativeButSafe(void) {
  zerar(); perfil = 1; agora++;
  assert(prog_gravar_local("tt1234567", 0, 0, 20, 600, 1));
  const char *keys[] = {"tt1234567"}; prog_marcar_empurrados(keys, 1);
  proximaResposta = "{}"; assert(syncprog_puxar() == -1);
  assert(porChave(keys[0]).chave[0]);
  proximaResposta = "["; assert(syncprog_puxar() == -1);
  proximaResposta = "[{\"content_id\":\"ttOTHER\" \"position\":20000,\"duration\":600000}]";
  assert(syncprog_puxar() == -1); // malformed object must never prune other titles
  proximaResposta = "[{\"content_id\":\"bad\"}]";
  assert(syncprog_puxar() == 0 && syncprog_aplicar(NULL) == 0);
  assert(porChave(keys[0]).chave[0]); // invalid rows cannot make an authoritative empty snapshot
  proximaResposta = "[]"; duranteRpc = newPlaybackDuringPull;
  assert(syncprog_puxar() == 0 && syncprog_aplicar(NULL) == 0);
  assert(porChave(keys[0]).posSeg == 80);
  duranteRpc = NULL;
  assert(syncprog_puxar() == 0 && syncprog_aplicar(NULL) == 1);
  assert(!porChave(keys[0]).chave[0]);
  assert(prog_gravar_local("tt1234567", 0, 0, 40, 600, 1));
  assert(syncprog_puxar() == 0 && syncprog_aplicar(NULL) == 0);
  assert(porChave(keys[0]).pendente);
  puts("ok  valid empty snapshots delete missing synced rows; malformed and concurrent snapshots cannot");
}
static void dismissalIsNotATimestampLease(void) {
  zerar(); perfil = 1; agora++;
  prog_ocultar_continuar("tt1234567:1:2");
  prog_invalidar();
  assert(prog_oculto_vence("tt1234567:2:1", agora + 100000));
  ProgRegistro r = {0}; strcpy(r.contentId, "tt1234567");
  r.temporada = 1; r.episodio = 2; r.posSeg = 30; r.durSeg = 600; r.lastWatchedMs = agora + 10000;
  assert(prog_aplicar_remoto(&r));
  assert(prog_oculto_vence("tt1234567", agora + 100000));
  perfil = 2; assert(!prog_oculto_vence("tt1234567", 0)); perfil = 1;
  agora += 20000;
  assert(prog_gravar_local("tt1234567", 1, 2, 40, 600, 1));
  assert(!prog_oculto_vence("tt1234567", 0));
  for (int i = 0; i < 80; i++) { char id[24]; snprintf(id, sizeof id, "ttDISMISS%d", i); prog_ocultar_continuar(id); }
  prog_invalidar(); assert(prog_oculto_vence("ttDISMISS0", 0) && prog_oculto_vence("ttDISMISS79", 0));
  puts("ok  next-up dismissal persists across sync and seed changes; local save clears it; no 64-item eviction");
}
static void acknowledgedDeleteInvalidatesInflightPull(void) {
  zerar(); perfil = 1; agora++;
  assert(prog_gravar_local("tt1234567", 1, 1, 20, 600, 1));
  assert(prog_excluir_titulo("tt1234567"));
  proximaResposta = "[{\"content_id\":\"tt1234567\",\"season\":1,\"episode\":1,\"position\":20000,\"duration\":600000}]";
  assert(syncprog_puxar() == 1);
  assert(syncprog_reenviar_deletes() == 2); // ACK races the main-thread mailbox consumption
  assert(syncprog_aplicar(NULL) == 0 && !porChave("tt1234567_s1e1").chave[0]);
  puts("ok  a delete ACK cannot release stale progress already in the pull mailbox");
}
static void emptySuccessBodiesAreAcknowledged(void) {
  zerar(); perfil = 1; agora++;
  assert(prog_gravar_local("tt1234567", 0, 0, 20, 600, 1));
  proximoStatus = 204; proximaResposta = NULL;
  assert(syncprog_empurrar() == 1 && !porChave("tt1234567").pendente);
  assert(prog_excluir_titulo("tt1234567"));
  assert(syncprog_reenviar_deletes() == 1);
  ProgDelete ds[2]; assert(prog_deletes(ds, 2, 1) == 0);
  puts("ok  successful empty 204 responses acknowledge both upserts and deletes");
}
static void playbackTimeBeatsServerWriteTime(void) {
  zerar(); perfil = 1;
  proximaResposta = "[{\"content_id\":\"tt1234567\",\"position\":20000,\"duration\":600000,\"last_watched\":1757000150000,\"updated_at\":1999999999999}]";
  assert(syncprog_puxar() == 1 && syncprog_aplicar(NULL) == 1);
  assert(porChave("tt1234567").lastWatchedMs == 1757000150000LL);
  assert(prog_concluir_local("ttUNKNOWN", 0, 0));
  prog_invalidar();
  assert(porChave("ttUNKNOWN").posSeg == .001 && porChave("ttUNKNOWN").durSeg == .001);
  assert(syncprog_empurrar() == 1 && tem("\"position\":1") && tem("\"duration\":1"));
  const char *longId = "provider:abcdefghijklmnopqrstuvwxyz";
  assert(prog_gravar_local(longId, 2, 3, 10, 600, 1));
  prog_invalidar();
  assert(!strcmp(porChave("provider:abcdefghijklmnopqrstuvwxyz_s2e3").contentId, longId));
  puts("ok  playback timestamp, unknown-duration completion and provider ID persistence match Android");
}

int main(void) {
  prog_definir_relogio(relogio);
  pushNoFormatoDoWeb();
  pushFalhoMantemPendente();
  pullAceitaOsFormatosDoServidor();
  rollbackNaoAcontece();
  migradoSobeComChaveCerta();
  confirmacaoPreservaEscritaNovaEPerfil();
  pullRecusaNumerosForaDaFaixa();
  pullDoPerfilAnteriorNaoSeMistura();
  deleteSurvivesRestartAndSync();
  intentionalPlaybackCancelsOnlyItsDelete();
  failedCancellationCannotDeleteNewPlayback();
  snapshotsAreAuthoritativeButSafe();
  dismissalIsNotATimestampLease();
  playbackTimeBeatsServerWriteTime();
  emptySuccessBodiesAreAcknowledged();
  acknowledgedDeleteInvalidatesInflightPull();
  puts("syncprog: tudo ok");
  return 0;
}

// Stage H Family Watching — the live read transport (owner-approved 7 Nov 2026
// for the test account; widened the same day to every signed-in account, at
// the owner's request: "doesn't show on other accounts").
//
// READ-ONLY, EXISTING ENDPOINTS: the same two per-profile RPCs Better already
// calls for the active profile (syncprog.c, sync.c), with the OTHER profile's
// p_profile_id and the same account session. Nothing is written; there is no
// push, delete or upsert anywhere in this file.
//
// GATES (all must hold, checked on every read):
//   - signed in (any account), and still the same account when the reads end;
//   - familia_app.c already asked only for eligible profiles (row on, viewer not
//     a child, target opted in, no PIN, not child).
// Logs: profile number, row counts, HTTP status and the 8-char fingerprint only.
#include "familia.h"
#include "nuvem.h"
#include "sessao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAM_PROGRESSO_LIMITE 481   // = syncprog SP_MAX (PROG_MAX + 1)
#define FAM_VISTOS_PAGINA    900   // = CONTALIB_VISTO_PAGINA, first page only
// Any signed-in account (a real fingerprint; "-" means no account).
int familia_rede_conta_permitida(const char *impressao) {
  return impressao && impressao[0] && strcmp(impressao, "-");
}

// One RPC body -> its single best row (newest usable activity).
static int melhorDe(const char *corpo, int vistos, int cap, FamAtividade *out) {
  FamAtividade *tmp = calloc((size_t)cap, sizeof *tmp);
  int n, ok = 0;
  if (!tmp) return 0;
  n = vistos ? familia_ler_vistos(corpo, tmp, cap) : familia_ler_progresso(corpo, tmp, cap);
  if (n > 0) ok = familia_escolher(tmp, n, out);
  free(tmp);
  return ok;
}

static int buscarConta(int perfil, FamAtividade *out, int max, void *u) {
  char usuario[80], jwt[3000], fp[9], corpo[128];
  char *r;
  int st = 0, k = 0;
  (void)u;
  if (perfil <= 0 || max < 2 || !sessao_logada()) return 0;
  if (!sessao_identidade_local_copiar(usuario, sizeof usuario, jwt, sizeof jwt)) return 0;
  sessao_impressao(usuario, fp);
  memset(jwt, 0, sizeof jwt);
  if (!familia_rede_conta_permitida(fp)) return 0;
  // Worker thread: a refresh-capable copy of the current token.
  if (!sessao_token_copiar(jwt, sizeof jwt)) return 0;

  snprintf(corpo, sizeof corpo, "{\"p_profile_id\":%d,\"p_limit\":%d}", perfil, FAM_PROGRESSO_LIMITE);
  r = nuvem_rpc_com("sync_pull_watch_progress", corpo, jwt, &st);
  if (r && st >= 200 && st < 300 && melhorDe(r, 0, FAM_PROGRESSO_LIMITE, &out[k])) k++;
  printf("[familia] conta %s perfil %d: progresso HTTP %d\n", fp, perfil, st);
  free(r);

  st = 0;
  snprintf(corpo, sizeof corpo, "{\"p_profile_id\":%d,\"p_page\":1,\"p_page_size\":%d}",
           perfil, FAM_VISTOS_PAGINA);
  r = nuvem_rpc_com("sync_pull_watched_items", corpo, jwt, &st);
  if (r && st >= 200 && st < 300 && melhorDe(r, 1, FAM_VISTOS_PAGINA, &out[k])) k++;
  printf("[familia] conta %s perfil %d: vistos HTTP %d, %d linha(s) uteis\n", fp, perfil, st, k);
  free(r);
  memset(jwt, 0, sizeof jwt);

  // The account changed while we were reading: drop the result.
  {
    char agora[80], j2[3000];
    int mesma = sessao_identidade_local_copiar(agora, sizeof agora, j2, sizeof j2) &&
                !strcmp(agora, usuario);
    memset(j2, 0, sizeof j2);
    if (!mesma) return 0;
  }
  return k;
}

void familia_rede_registrar(void) {
  familia_definir_busca(buscarConta, NULL);
}

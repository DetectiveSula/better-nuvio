// A ordem das fileiras da home, como a pessoa deixou no app web.
//
// O app ja chamava `sync_pull_home_catalog_settings` e so CONTAVA a resposta:
// quem organizou a home no celular via a TV ignorar tudo. Aqui a resposta vira
// uma lista de chaves ordenada e um conjunto de chaves desligadas, que a
// descoberta consome no mesmo ponto onde ja aplicava art/fileiras.txt.
//
// Stage G: regular Nuvio's synced order/visibility is authoritative (fileiras.c
// contaOficial); Native-exclusive rows are anchored around it. Empty/failed pulls preserve the last valid TV snapshot.
// SO LEITURA, de proposito. Nao existe tela de reordenar no nativo e, sem ela,
// empurrar de volta mandaria a lista que o aparelho conseguiu montar — que e
// menor que a real sempre que um addon demora a responder — e apagaria a
// configuracao da pessoa nos outros aparelhos. A trava esta em PLANO-CONTA-SYNC.md.
//
// A REGRA QUE NAO PODE SER ESQUECIDA: uniao, nao substituicao. O remoto so
// conhece os catalogos que existiam quando foi gravado; aplicar a ordem crua
// APAGA os que apareceram depois. Medido na OLED65C9 como
// `{"localItems":54,"remoteItems":43}` em todo arranque — 54 nunca igualava 43,
// a home era reescrita e reinvalidada a cada boot e nunca convergia. Por isso
// quem aplica (descoberta.c) poe a ordem remota primeiro, ignorando o que nao
// existe local, e mantem os locais desconhecidos no FIM, na ordem que ja tinham.
#ifndef NV_CATORDEM_H
#define NV_CATORDEM_H

// Itens da ordem da conta, ligados e desligados. A ordem do web tem UM item
// por catalogo de TODO addon instalado mais as colecoes: o Xperience sozinho
// declara 606, e a conta do #255 passou de 768 — o teto antigo. O que ficava
// alem dele perdia a posicao E o "desligado": catalogo de pasta desligado no
// web voltava como fileira solta na TV. Passar do teto agora e dito no log.
#define CATORD_MAX   2048
#define CATORD_CHAVE 192

#include <ctype.h>
#include <stddef.h>
#include <string.h>
// Regular Nuvio's catalogTypeKey (domain/model/CatalogReference.kt): movie,
// series, channel and tv match case-insensitively and are written lowercase;
// every other type keeps its trimmed manifest spelling. Row identities on both
// sides of the sync use this, never a blanket lowercase.
static inline void catordem_tipo_chave(const char *tipo, char *dst, size_t tam) {
  static const char *const CONHECIDOS[] = { "movie", "series", "channel", "tv" };
  size_t n, k;
  if (!dst || !tam) return;
  if (!tipo) { dst[0] = 0; return; }
  // dst may alias tipo (callers normalise in place): read before writing.
  while (*tipo && isspace((unsigned char)*tipo)) tipo++;
  n = strlen(tipo);
  while (n && isspace((unsigned char)tipo[n - 1])) n--;
  if (n >= tam) n = tam - 1;
  memmove(dst, tipo, n); dst[n] = 0;
  for (k = 0; k < sizeof CONHECIDOS / sizeof *CONHECIDOS; k++) {
    size_t i, m = strlen(CONHECIDOS[k]);
    if (m != n) continue;
    for (i = 0; i < n && tolower((unsigned char)dst[i]) == CONHECIDOS[k][i]; i++);
    if (i == n) { memcpy(dst, CONHECIDOS[k], n + 1); return; }
  }
}

// Le a resposta crua da RPC. Aceita `[{"settings_json":{...}}]`, o objeto
// solto, e as duas formas do blob (moderna com `items[]`, legada com arrays de
// chaves). Devolve 1 quando o que foi lido MUDA o que ja estava aplicado — e o
// unico caso em que vale remontar a home; uma resposta igual a da volta
// anterior nao pode custar uma remontagem a cada ciclo de sync.
//
// Missing configuration preserves the current state. Empty snapshots, like the pinned TV client, preserve the last valid
// configuration; explicit logout/profile reset clears it.
int catordem_ler(const char *respostaJson);
unsigned catordem_revisao(void);
// Read-only profile projection: -1 unknown, 0 disabled, 1 enabled.
int catordem_estado(const char *chave);

int         catordem_tem_ordem(void);
int         catordem_n(void);
const char *catordem_chave(int i);        // "" fora da faixa

// 1 quando a conta desligou esta fileira. Confere as DUAS chaves como o web:
// a curta (<addonId>_<tipo>_<catalogoId>) e a de desativar, que carrega a URL
// base e o nome do catalogo.
int catordem_oculta(const char *chave, const char *chaveDesativar);

// A REGRA, num lugar so. Recebe as chaves locais na ordem em que ja estavam e
// escreve em `saida` os INDICES delas na ordem final: primeiro as que o remoto
// conhece, na ordem do remoto; depois todas as outras, no fim, na ordem
// original. Nada e descartado — a contagem devolvida e sempre `nLocais`
// (limitada por `max`), e chave remota que nao existe local nao entra.
//
// Devolve quantos indices escreveu. Sem ordem remota, e a identidade.
int catordem_unir(const char *const *locais, int nLocais, int *saida, int max);

// Os dois booleanos do blob. `tem_*` responde se a chave EXISTIA: ausente nao e
// `false`, e "mantem o que esta na TV".
int catordem_tem_ocultar_nao_lancados(void);
int catordem_ocultar_nao_lancados(void);
int catordem_tem_ocultar_sublinhado(void);
int catordem_ocultar_sublinhado(void);

// Esquece a configuracao lida. Chamar no logout junto com o resto: a ordem da
// home e da conta de quem saiu.
void catordem_esquecer(void);

#endif

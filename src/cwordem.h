// Teto de "Continuar assistindo" (issue #205). descoberta.c (CONT_MAX) e
// home.c (CW_TETO) repetem este numero; a tabela de futuros tambem.
#define CWO_FILEIRA_MAX 40

// ORDENACAO DE "CONTINUAR ASSISTINDO" (issue #127).
//
// Ajustes -> Continuar assistindo -> Ordenacao sempre existiu (AJ_CW_ORDEM,
// chave `continueWatchingSortMode` da conta), mas NINGUEM lia
// ajustes_cw_ordem(): as tres escolhas davam a mesma fileira. "Separar
// futuros" prometia uma fileira de proximos episodios que nunca aparecia.
//
// Reference: supplied Nuvio Android HomeViewModelContinueWatching.kt,
// sortContinueWatchingItems and splitUpcomingItems. Input is already sorted by
// lastWatched (resume) or sortTimestamp (next-up, including release alerts).
// default: preserve that order across both groups.
// streaming_style: released items in that order, then upcoming by release date.
// split_upcoming: main row preserves recency; upcoming gets its own row sorted
// by release date. Unknown release dates count as aired, as in Android.
// The native 40-card memory budget remains separate from ordering policy.
//
// O QUE O NATIVO CHAMA DE "A SEGUIR" e o item de progresso 0 que o Trakt (pelo
// historico) ou o Simkl (next_to_watch) mandou como proximo episodio —
// trakt_e_a_seguir/simkl_e_a_seguir. A DATA DE ESTREIA vem do `released` do
// episodio no meta do Cinemeta, lido em trakt.c no mesmo GET que ja confirmava
// que o episodio existe; ela mora numa tabela lateral aqui, e nao no CatItem,
// porque sizeof(CatItem) e o cabecalho do cache em disco.
//
// Modulo sem dependencia de rede, SDL ou catalogo: a regra roda num teste de
// host (tests/cwordem.sh) do jeito que roda na TV.
#ifndef NV_CWORDEM_H
#define NV_CWORDEM_H
#include <limits.h>
#include <stddef.h>

// Os valores de AJ_CW_ORDEM, na ordem de V_CW_ORDEM/W_CW_ORDEM em ajustes.c.
enum { CWO_PADRAO = 0, CWO_STREAMING = 1, CWO_SEPARAR = 2 };

// "Nao se sabe a data". LLONG_MIN pelo mesmo motivo de PROX_SEM_DATA: 0 e
// uma data legitima (1/1/1970).
#define CWO_SEM_DATA LLONG_MIN

// Um candidato da fileira, ja na ordem por instante (o mais recente primeiro)
// que montarContinuar deu. So o que a regra precisa.
typedef struct {
  int aSeguir;          // 1 = "a seguir" (progresso 0, proximo episodio)
  long long estreiaMs;  // estreia do episodio, CWO_SEM_DATA se nao se sabe
} CwoItem;

// 1 quando o item e futuro: "a seguir" com estreia conhecida e DEPOIS de agora.
// Sem data conta como exibido, como o `hasAired !== false` do web.
int cwo_futuro(const CwoItem *it, long long agoraMs);

// Reorder already-recency-sorted input. Released progress and next-up retain
// their shared order; future next-up follows by release date in nondefault
// modes. Return the number of cards in the main (non-upcoming) partition.
int cwo_ordenar(const CwoItem *v, int n, int modo, long long agoraMs, int *perm);

// Keep the prefix of the ordered main/upcoming partitions within max cards.
// No quota may displace a more highly ranked card with an upcoming episode.
void cwo_corte(int principal, int nFut, int max, int *nPrincipal, int *nFuturos);

// --- Datas de estreia (escritas por trakt.c, lidas por descoberta.c) --------
// `id` e o composto "tt123:2:5". Chamada de varios fios do enfeite ao mesmo
// tempo; protegida por trava.
void      cwo_marcar_estreia(const char *id, long long ms);
long long cwo_estreia(const char *id);   // CWO_SEM_DATA se nao registrada
// Stage E (official release rules, epdata.h): the raw add-on `released`, the
// seed's last-watched instant and season, and the release alert computed
// from them at call time. Seed season -1 = unknown.
void cwo_marcar_estreia_cru(const char *id, const char *cru);
int  cwo_estreia_cru(const char *id, char *dst, size_t cap);
void cwo_marcar_semente(const char *id, long long vistoMs, int temporadaSemente);
int  cwo_alerta(const char *id, int temporada, long long agoraMs, int *novaTemporada);

// --- Quem esta na fileira de futuros (escrito por descoberta.c, lido pela home)
// montarContinuar decide UMA vez quem e futuro e publica os ids aqui antes de
// publicar a fileira; a home so pergunta. Duas decisoes (uma na montagem, outra
// no desenho) poderiam discordar na virada da estreia.
void cwo_publicar_futuros(const char *const *ids, int n);
int  cwo_e_futuro(const char *id);
// Sobe quando o CONJUNTO muda. A home a poe no hash das fileiras: a lista
// publicada pode ser identica (mesmos ids, mesma ordem) e so a divisao mudar —
// uma estreia que passou, por exemplo —, e sem este termo o guarda de
// sincronizarFileiras achava que nada tinha mudado.
unsigned cwo_revisao(void);

// --- O rotulo do card futuro ------------------------------------------------
// O card de "Proximos episodios" dizia "A seguir", igual ao episodio que ja
// foi ao ar e pode tocar agora — e o hero, "A SEGUIR" (C9 do dono, Brothers
// T1E6, 24/09). O futuro diz QUANDO: "Estreia 21 out" / "Airs Oct 21".
// Esta e so a data: "21 out" (PT) / "Oct 21" (EN), com o ano quando nao e o
// de `agoraMs` ("21 out 2027" / "Oct 21, 2027"); `maiusc` para o hero
// ("21 OUT"). Dia pelo calendario UTC. Devolve 0, com dst vazio, sem data.
//
// `idioma` e um IDIOMA_* (idiomacod.h). 0 = pt e 1 = en, como o `ingles` que
// este parametro era: quem passa 0/1 continua certo. Romeno, ucraniano e russo
// escrevem "21 окт" (dia primeiro, como o portugues).
int cwo_data_curta(long long estreiaMs, long long agoraMs, int idioma, int maiusc,
                   char *dst, size_t cap);

// --- "A seguir" DA CONTA NUVIO (issue #199) ----------------------------------
// Sem Trakt e sem Simkl, o "a seguir" sai dos vistos da conta (watched_items),
// como no web com a fonte Nuvio Sync (getContinueWatchingNextUpSeedOptions,
// homeScreen.js:2202 — includeWatchedItemSeeds). Antes a fonte "conta" so dava
// os pausados: 804 episodios vistos na conta e nenhum "a seguir" na fileira
// (log da C9 do issue, 01/10). Os ids ("tt:S:E") moram aqui, ao lado das
// estreias, porque trakt.c (que confere no Cinemeta que o episodio existe) e a
// home (que escreve "A seguir"/"Estreia") os leem, e os dois ja linkam este
// arquivo. Trocado inteiro de uma vez, sob trava: nada de janela vazia.
void cwo_conta_definir(const char *const *ids, int n);
// Tambem diz sim para os ajustados pela desmarcacao (cwo_ajustados_definir).
int  cwo_conta_a_seguir(const char *id);
void cwo_ajustados_definir(const char *const *ids, int n);
// A virada de temporada do enfeite (S3E10 nao existe, vira S4E1) muda o id.
void cwo_conta_trocar(const char *velho, const char *novo);

// O "a seguir" que VIROU DE TEMPORADA so entra quando a estreia ja passou ou
// esta a ate 7 dias; sem data nao entra. shouldShowNextUpEpisodeForContinue
// Watching (homeScreen.js:1845, CW_NEXT_UP_NEW_SEASON_UNAIRED_WINDOW_DAYS):
// sem isto, toda serie terminada com a temporada seguinte anunciada no
// Cinemeta (sem data, ou para daqui a um ano) viraria um card "a seguir".
// Dias contados pelo calendario UTC, como cwo_data_curta.
int cwo_virada_aceita(long long estreiaMs, long long agoraMs);

#endif

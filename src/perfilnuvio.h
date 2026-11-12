// PAINEL DE 30 DIAS DO PERFIL, MONTADO SO COM NUVIO SYNC.
//
// Entrada: os VISTOS da conta (sync_pull_watched_items -> ContaVisto: um
// watched_at por episodio/filme) e as linhas de PROGRESSO (sync_pull_watch_
// progress -> ProgRegistro: posicao, duracao, ultimo instante), mais METAS
// opcionais (titulo, cartaz, generos, duracao) que o chamador resolve na
// biblioteca ou no add-on de meta. Saida: os campos do painel em PerfilDados
// (perfildados.h, bloco "PAINEL DE 30 DIAS") e, por compatibilidade com a tela
// de hoje, os campos antigos (minutos, filmes, episodios, atividade, streak,
// destaques, generos).
//
// SEM REDE, SEM SDL, SEM ESTADO GLOBAL: tests/perfilnuvio.sh prende a regra.
// O relogio vem em `agoraMs`; o fuso e o do processo (localtime_r).
//
// O QUE O DADO NAO DIZ, E COMO A CONTA LIDA (docs/profile-stats-dashboard):
//   - um watched_at por item: reassistir SUBSTITUI a data, nao soma. Contamos
//     itens terminados, nunca "reproducoes";
//   - watched_at e o FIM: inicio = fim - duracao;
//   - marcar em lote (3+ episodios da mesma serie em ate 120 s) nao e assistir:
//     fica fora de horas, grafico, relogio e maratona, e conta em `marcados`;
//   - duracao: progresso (durSeg) -> meta -> 42 min episodio / 110 min filme
//     (estimada; PF_COB_ESTIMADO quando passa de 20% dos minutos);
//   - progresso inacabado conta a POSICAO uma vez (so a ultima e sincronizada).
#ifndef NV_PERFILNUVIO_H
#define NV_PERFILNUVIO_H

#include "perfildados.h"
#include "contalib.h"
#include "progresso.h"

#define PN_LOTE_JANELA_MS   (120LL * 1000)   // marcacao em lote
#define PN_LOTE_MIN         3
#define PN_MARATONA_VAO_MS  (30LL * 60 * 1000)
#define PN_EP_PADRAO_MIN    42
#define PN_FILME_PADRAO_MIN 110
#define PN_CONCLUIDO        0.90

typedef struct {
  char id[24];                 // content_id puro
  char titulo[128];
  char poster[NV_ARTE_URL_MAX];
  char genero[160];            // "Drama, Mistério" ou "Filme · Drama · Crime"
  int  duracaoMin;             // 0 = desconhecida
} PerfilMeta;

// Monta o painel. `perfil` filtra as linhas de progresso (os vistos ja vem do
// perfil ativo). Zera `d` inteiro antes. Devolve o numero de itens (terminados
// + parciais) que caem na janela.
int perfilnuvio_montar(const ContaVisto *v, int nv,
                       const ProgRegistro *r, int nr, int perfil,
                       const PerfilMeta *m, int nm,
                       long long agoraMs, PerfilDados *d);

// Quais obras precisam de meta (titulo/cartaz/generos/duracao): as da janela
// por minutos provisorios, mais a candidata de "neste dia". Ate `max` ids,
// sem repeticao; `tipos` recebe "movie"/"series". Devolve quantos.
int perfilnuvio_ids_meta(const ContaVisto *v, int nv,
                         const ProgRegistro *r, int nr, int perfil,
                         long long agoraMs,
                         char ids[][24], char tipos[][8], int max);

// Le um /meta no formato Stremio ({"meta":{name, poster, genres[], runtime}})
// e preenche SO os campos ainda vazios de `m` (a biblioteca, quando sabe, vence).
// "runtime" vem como "45 min", "1h 50min" ou numero. Devolve 1 se achou o meta.
int perfilnuvio_meta_ler(const char *corpo, PerfilMeta *m);

#endif

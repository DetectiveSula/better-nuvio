// Snapshot independente de UI para Perfil e Stats. Produtores (Trakt, Simkl)
// preenchem esta estrutura em workers; perfil.c apenas guarda e desenha.
#ifndef NV_PERFILDADOS_H
#define NV_PERFILDADOS_H

#include "arteurl.h"
#include <stdint.h>

// QUATRO, e nao oito e seis. A tela nao rola mais: cabem quatro cards de
// destaque e quatro pastilhas de genero, e buscar mais do que se desenha so
// produz campo morto no snapshot.
#define PERFIL_MAX_GENEROS    4
#define PERFIL_MAX_DESTAQUES  4
#define PERFIL_MAX_DIAS       42

typedef struct {
  char nome[40];
  int  quantidade;
  uint32_t cor;              // 0xRRGGBB; zero usa a paleta da tela
} PerfilGenero;

typedef struct {
  char id[64];               // imdb/trakt/slug, devolvido sem interpretacao
  char titulo[128];
  char detalhe[96];          // ex.: "T2E3 · Solo" ou "Filme"
  char poster[NV_ARTE_URL_MAX];
  char backdrop[768];
  int  plays;
  int  minutos;
} PerfilDestaque;

#define PERFIL_DIAS        30
#define PERFIL_MAX_OBRAS   24
#define PERFIL_MAX_TOPO     6
#define PERFIL_ITENS_DIA    3

enum {
  PF_COB_NUVIO    = 1,   // snapshot montado de Nuvio sync (perfilnuvio.c)
  PF_COB_ESTIMADO = 2,   // mais de 20% dos minutos sao duracao estimada
  PF_COB_GENEROS  = 4    // ao menos um genero resolvido
};

typedef struct {
  char id[24];               // content_id puro ("tt...", "kitsu:...")
  char tipo[8];              // "movie" | "series"
  char titulo[128];          // vazio quando nem biblioteca nem meta sabem
  char poster[NV_ARTE_URL_MAX];
  int  minutos;
  int  episodios;            // episodios terminados na janela (0 em filme)
} PerfilObra;

typedef struct {
  int  obra;                 // indice em obras, -1 quando ficou fora do teto
  int  temporada, epIni, epFim;   // 0 em filme
  int  minutos;
  long long inicioMs;
} PerfilDiaItem;

typedef struct PerfilDados {
  char nome[96];
  char usuario[64];
  char avatar[768];
  char periodo[48];          // ex.: "SETEMBRO 2026"

  int minutos;
  int plays;
  int filmes;
  int episodios;

  int streakAtual;
  // streakAnterior FOI REMOVIDO: era declarado, zerado por memset e nunca lido
  // nem escrito por ninguem — nem pelo produtor (trakt.c), nem pela tela.
  int diasAtivosMes;
  int diasAtivosAno;
  int primeiroDiaSemana;     // 0=domingo..6=sabado
  int nDias;
  unsigned short atividade[PERFIL_MAX_DIAS];

  int nGeneros;
  PerfilGenero generos[PERFIL_MAX_GENEROS];
  int nDestaques;
  PerfilDestaque destaques[PERFIL_MAX_DESTAQUES];
  // Zero e o padrao seguro para produtores antigos. O snapshot mensal nao
  // comprova cobertura anual nem uma sequencia que cruza a virada do mes.
  int parcial;
  int anoCompleto;
  int streakCompleto;
  char aviso[160];

  // ---- PAINEL DE 30 DIAS (Nuvio sync; perfilnuvio.c) ----------------------
  // Preenchido so por perfilnuvio_montar. Janela movel: indice 0 = 29 dias
  // locais atras, PERFIL_DIAS-1 = hoje. Produtores antigos (trakt/simkl)
  // deixam tudo zerado e `cobertura` = 0.
  long long janelaInicioMs;
  int titulos;                 // obras distintas na janela
  int minutosFilmes, minutosSeries;
  int marcados;                // episodios marcados em lote (fora das horas)
  unsigned short minutosDia[PERFIL_DIAS], minutosFilmeDia[PERFIL_DIAS];
  int minutosHora[24];         // hora local
  int minutosSemana[7];        // 0=domingo
  int diaMaisAtivo;            // 0..6, -1 sem dado
  int nObras;
  PerfilObra obras[PERFIL_MAX_OBRAS];
  int nTopo;
  int topo[PERFIL_MAX_TOPO];   // indices em obras, por minutos
  unsigned char nDiaItens[PERFIL_DIAS];
  PerfilDiaItem diaItens[PERFIL_DIAS][PERFIL_ITENS_DIA];
  struct {
    int obra;                  // -1 = nenhuma
    int episodios, minutos, temporada, epIni, epFim;
    long long inicioMs, fimMs;
  } maratona;
  int concluidas, iniciadas;
  struct { int obra; long long ms; } hojeAnos;   // obra -1 = nada
  unsigned cobertura;          // PF_COB_*
} PerfilDados;

#endif

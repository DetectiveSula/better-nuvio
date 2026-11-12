#ifndef NV_FONTEFILTRO_H
#define NV_FONTEFILTRO_H

#include <stdint.h>

#define FONTEFILTRO_MAX_GB 1000

enum {
  FONTEFILTRO_RESOLUCAO, FONTEFILTRO_QUALIDADE, FONTEFILTRO_VISUAL,
  FONTEFILTRO_AUDIO, FONTEFILTRO_CANAIS, FONTEFILTRO_ENCODE,
  FONTEFILTRO_LINGUAS, FONTEFILTRO_N
};

typedef struct {
  uint64_t excluir[FONTEFILTRO_N];
  unsigned maxGB;                 /* GB decimais; zero = sem limite. */
  uint64_t preferir[FONTEFILTRO_N], exigir[FONTEFILTRO_N];
  unsigned minGB, maxResultados, porResolucao, porQualidade, ordenar;
  char gruposObrigatorios[512];
  char grupos[512];               /* Grupos excluidos, separados por virgula. */
} FonteFiltroConfig;

typedef struct {
  uint64_t valores[FONTEFILTRO_N]; /* Zero = desconhecido; bucket proprio nos limites. */
  uint64_t tamanhoBytes;          /* Zero = desconhecido. */
  char grupo[80];
  int naoVod;                     /* Request scope; travels with this source. */
} FonteFiltroDados;

typedef struct {
  const char *resolucao, *qualidade, *visual, *audio, *canais, *encode;
  const char *linguas, *grupo, *filename, *texto;
  uint64_t tamanhoBytes;
} FonteFiltroEntrada;

/* IDs de escolha sao persistentes. A ultima escolha de cada categoria e
   Desconhecido. Rotulos e nomes tecnicos nao dependem dos badges de tela. */
int fontefiltro_opcoes_n(int categoria);
const char *fontefiltro_opcao(int categoria, int opcao);
const char *fontefiltro_rotulo(int categoria);
uint64_t fontefiltro_bit(int categoria, int opcao);
uint64_t fontefiltro_desconhecido(int categoria);
uint64_t fontefiltro_valores(int categoria, const char *texto);

int fontefiltro_config_validar(const FonteFiltroConfig *cfg);
/* Valida e normaliza CSV, preservando saida original em caso de erro. */
int fontefiltro_grupos_normalizar(const char *csv, char *saida, unsigned n);
void fontefiltro_classificar(const FonteFiltroEntrada *entrada, FonteFiltroDados *saida);
/* Linguas: exclui apenas se TODAS as linguas declaradas forem excluidas.
   Demais tags: uma correspondencia basta. Desconhecido e opcao explicita. */
/* Sorting preserves ties; source indices are never rewritten. Limits apply to
   known/unknown groups after hard filters. Zero limits preserve the original behaviour. */
int fontefiltro_pontuacao(const FonteFiltroConfig *,const FonteFiltroDados *);
int fontefiltro_selecionar(const FonteFiltroConfig *,const FonteFiltroDados *,int,
                          const unsigned char *elegivel,int *indices);
int fontefiltro_permite(const FonteFiltroConfig *cfg, const FonteFiltroDados *dados);

#endif

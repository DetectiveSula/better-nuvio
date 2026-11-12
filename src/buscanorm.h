// Normalizacao do texto da BUSCA, sem dependencia de tela.
//
// Vive fora de busca.c para poder ser testada sozinha (tests/buscanorm.sh) e
// porque ela decide o que a busca LOCAL considera igual — nao o que vai aos
// addons (esses recebem o termo como a pessoa digitou; ver refiltrar).
#ifndef NV_BUSCANORM_H
#define NV_BUSCANORM_H
#include <stddef.h>

// Copia `s` em `destino` sem caixa e sem acento, como UTF-8:
//   - ASCII: minusculo;
//   - latino com acento (Latin-1, Latin Extended-A e as virgulas do romeno
//     ș/ț): a letra base, ASCII;
//   - cirilico (russo, ucraniano, bielorrusso, bulgaro, servio): minusculo, com
//     "ё" dobrado em "е";
//   - qualquer outra escrita passa como veio (antes virava espaco, e um termo
//     em cirilico ou grego virava uma sequencia de espacos que casava com
//     qualquer titulo).
// Byte invalido vira espaco. Sempre termina em NUL, sem cortar no meio de uma
// sequencia UTF-8.
void busca_normalizar(const char *s, char *destino, size_t tam);

// Quantos caracteres (pontos de codigo) tem `s`.
int busca_codepoints(const char *s);

// Tira o ultimo CARACTER de `s` (nao o ultimo byte): a tecla "apagar" nao pode
// deixar meia sequencia UTF-8 no campo. Devolve o novo tamanho em bytes.
size_t busca_apagar_ultimo(char *s, size_t n);

// TITLE RELEVANCE for ordering results (both already busca_normalizar'd):
//   0 exact title ("rocky"; also "the matrix" for "matrix")
//   1 title starts with the query as whole word(s) ("rocky ii", "rocky balboa")
//   2 the same after a leading "the/a/an" ("the rocky horror picture show")
//   3 the query ends inside a longer first word ("rockyfeller")
//   4 a later word starts with the query ("meet rocky")
//   5 the query appears elsewhere in the title
//   6 not in the title at all (the add-on matched other fields)
// Lower is better.
int busca_relevancia(const char *tituloNorm, const char *alvoNorm);

#endif

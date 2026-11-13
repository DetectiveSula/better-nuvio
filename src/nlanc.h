// "OCULTAR NAO LANCADOS" (hideUnreleasedContent) PARA O QUE VEM DO CATALOGO.
//
// O ajuste existia e so a Biblioteca o lia (biblioteca.c). As fileiras da Home
// e a grade de uma colecao / "Ver tudo" mostravam um filme de 2099 mesmo com ele
// ligado (issue #369, item 1).
//
// A regra e a unica que o catalogo prova: o ANO que abre `CatItem.meta`
// ("2099", "2099  ·  120 min", "2099-") e MAIOR que o ano de agora. Sem ano no
// meta (muito addon nao manda releaseInfo) nao se esconde: nao ha como saber, e
// esconder faria sumir titulo bom. Titulo que estreia ainda neste ano tambem
// fica: o meta so tem o ano, nao a data.
#ifndef NV_NLANC_H
#define NV_NLANC_H
// 1 = o meta traz um ano de 4 digitos no inicio e ele e > anoAtual.
// So cabecalho: descoberta.c entra em muitos testes com lista fixa de fontes.
static inline int nlanc_meta_futuro(const char *meta, int anoAtual) {
  int ano = 0;
  if (!meta) return 0;
  for (int i = 0; i < 4; i++) {
    if (meta[i] < '0' || meta[i] > '9') return 0;
    ano = ano * 10 + (meta[i] - '0');
  }
  // "20245" nao e ano: o quinto digito invalida a leitura.
  if (meta[4] >= '0' && meta[4] <= '9') return 0;
  return ano > anoAtual;
}

// THE FULL DATE FIRST (owner, 13 Nov 2026: "hide unreleased on but still
// showing some unreleased titles"). The year rule above let through anything
// due later THIS year. Regular Nuvio (ReleaseInfoUtils.isUnreleased) reads the
// meta's `released` date first and only then the year. `valor` starts with
// YYYY-MM-DD (a plain date or an ISO instant; the calendar day is compared).
// 1 = after today, 0 = today or earlier, -1 = no readable date.
static inline int nlanc_data_futura(const char *valor, int ano, int mes, int dia) {
  int v[3] = { 0, 0, 0 }, pos[3] = { 0, 5, 8 }, tam[3] = { 4, 2, 2 };
  if (!valor) return -1;
  for (int k = 0; k < 3; k++)
    for (int i = 0; i < tam[k]; i++) {
      char c = valor[pos[k] + i];
      if (c < '0' || c > '9') return -1;
      v[k] = v[k] * 10 + (c - '0');
    }
  if (valor[4] != '-' || valor[7] != '-' || (valor[10] >= '0' && valor[10] <= '9')) return -1;
  if (v[1] < 1 || v[1] > 12 || v[2] < 1 || v[2] > 31) return -1;
  if (v[0] != ano) return v[0] > ano;
  if (v[1] != mes) return v[1] > mes;
  return v[2] > dia;
}
#endif

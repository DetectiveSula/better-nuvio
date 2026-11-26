// Nuvio+ 1.0.13 (26 Nov 2026): smoother Samsung scrolling, trailers, sources
// that play, subtitle sync. Compact card (no previews): the list says it all.
// Settings › About › What's new reopens it.
static const NovItem ITENS_B113[] = {
  { .grupo = "Destaques", .icone = "aj_gauge", .nome = "Rolagem mais suave",
    .frase = "Subir e descer na Home e passar pelos episódios ficou muito mais fluido, com o vidro fosco mantido.", .linhas = 4,
    .plataformas = NOV_TPK | NOV_OUTRAS },
  { .icone = "aj_film", .nome = "Trailers melhores",
    .frase = "Trailers no seu idioma, em qualidade maior, só os oficiais e com som desde o início. Os da Apple TV também tocam.", .linhas = 5 },
  { .icone = "aj_list-video", .nome = "Fontes que tocam",
    .frase = "A lista esconde as fontes que esta TV não toca, e a reprodução automática nunca escolhe uma delas.", .linhas = 4,
    .plataformas = NOV_TPK | NOV_OUTRAS },
  { .icone = "aj_captions", .nome = "Legendas sincronizadas",
    .frase = "As legendas acertam o tempo pelo áudio e continuam certas depois de avançar.", .linhas = 4 },
  { .grupo = "Mais novidades", .icone = "aj_keyboard", .nome = "Teclado maior",
    .frase = "O teclado da Busca ficou bem maior.", .linhas = 3 },
  { .icone = "aj_key-round", .nome = "Chave do TMDB na conta",
    .frase = "A chave digitada na TV vale para todos os perfis e volta depois de reinstalar.", .linhas = 4 },
  { .icone = "aj_images", .nome = "Pôsteres mais rápidos",
    .frase = "Os pôsteres aparecem mais depressa e os já vistos não carregam de novo.", .linhas = 4,
    .plataformas = NOV_TPK | NOV_OUTRAS },
  { .grupo = "Correções", .icone = "aj_layout-list", .nome = "Coleções na Home",
    .frase = "A arte da coleção não aparece mais por trás das fileiras.", .linhas = 3 },
  { .icone = "aj_arrow-down", .nome = "Descer do destaque",
    .frase = "Descer do destaque sempre vai para a primeira fileira, mesmo enquanto a Home carrega.", .linhas = 4 },
  { .icone = "aj_eye-off", .nome = "Dolby Vision",
    .frase = "Fontes Dolby Vision que ficavam roxas nesta TV não aparecem mais.", .linhas = 4,
    .plataformas = NOV_TPK | NOV_OUTRAS },
};
static const int ARTES_B113[] = { 4, 9, 2, 0 };
static const NovConteudo NOV_ATUAL = {
  .versao = "Nuvio+ 1.0.13", .arquivo = "novidades-better-113.txt",
  .subtitulo = NULL,
  .itens = ITENS_B113, .nItens = (int)(sizeof ITENS_B113 / sizeof *ITENS_B113),
  .cenas = NULL, .nCenas = 0,
  .artes = ARTES_B113, .nArtes = 4,
};

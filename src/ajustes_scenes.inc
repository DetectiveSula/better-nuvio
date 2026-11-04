// Visual identities are not category indices. Never use these for navigation.
enum { AJS_REPRODUCAO, AJS_IDIOMAS, AJS_FONTES, AJS_TVAOVIVO, AJS_HOME, AJS_APARENCIA, AJS_TITULO, AJS_CARTAZES, AJS_CONTAS, AJS_DESEMPENHO, AJS_SOBRE, AJS_N, AJS_TRAILERS = AJS_N };
static const int ajCategoryScene[] = { AJS_CONTAS, AJS_CONTAS, AJS_APARENCIA, AJS_HOME, AJS_FONTES, AJS_REPRODUCAO, AJS_TITULO, AJS_CONTAS, AJS_CONTAS, AJS_DESEMPENHO, AJS_SOBRE };
static int ajSceneForCategory(int category) { return category >= 0 && category < (int)(sizeof ajCategoryScene / sizeof *ajCategoryScene) ? ajCategoryScene[category] : AJS_CONTAS; }
static int ajSceneForOption(int op) { switch (op) {
case AJ_QUALIDADE: return AJS_REPRODUCAO;
case AJ_FONTE_HDR: return AJS_REPRODUCAO;
case AJ_DV: return AJS_REPRODUCAO;
case AJ_ATMOS: return AJS_REPRODUCAO;
#if (defined(NV_TPK))
case AJ_AUDIO_OFFSET: return AJS_REPRODUCAO;
#endif
case AJ_FONTE_MANUAL: return AJS_REPRODUCAO;
case AJ_FONTE_PRIORIDADE: return AJS_REPRODUCAO;
case AJ_FONTE_IDIOMA_AUDIO: return AJS_REPRODUCAO;
case AJ_FONTE_PRAZO: return AJS_REPRODUCAO;
case AJ_FONTE_REPOR: return AJS_REPRODUCAO;
case AJ_TAM_MAX: return AJS_REPRODUCAO;
case AJ_FONTE_GRUPO: return AJS_REPRODUCAO;
case AJ_FONTE_GRUPO_SESSOES: return AJS_REPRODUCAO;
case AJ_LINK_REUSAR: return AJS_REPRODUCAO;
case AJ_LINK_PRAZO: return AJS_REPRODUCAO;
case AJ_PAUSA_OVERLAY: return AJS_REPRODUCAO;
case AJ_PAUSA_COMENTARIOS: return AJS_REPRODUCAO;
case AJ_PAUSA_NOTAS: return AJS_REPRODUCAO;
case AJ_PAUSA_ELENCO: return AJS_REPRODUCAO;
case AJ_TELA_ABERTURA: return AJS_REPRODUCAO;
case AJ_ABERTURA_ADDON: return AJS_REPRODUCAO;
case AJ_ABERTURA_FUNDO: return AJS_REPRODUCAO;
case AJ_ABERTURA_ESTILO: return AJS_REPRODUCAO;
case AJ_ESTAT_TAMANHO: return AJS_REPRODUCAO;
case AJ_ESTAT_LADO: return AJS_REPRODUCAO;
case AJ_PLR_CLASSIF: return AJS_REPRODUCAO;
case AJ_PULAR_ABERTURA: return AJS_REPRODUCAO;
case AJ_PULAR_RESUMO: return AJS_REPRODUCAO;
case AJ_PULAR_CREDITOS: return AJS_REPRODUCAO;
case AJ_PROXIMO_AUTO: return AJS_REPRODUCAO;
case AJ_PROXIMO_CONTAGEM: return AJS_REPRODUCAO;
case AJ_REACAO_CREDITOS: return AJS_REPRODUCAO;
case AJ_CW_CONCLUIDO: return AJS_REPRODUCAO;
case AJ_CW_MINIMO: return AJS_REPRODUCAO;
case AJ_PLR_FORMATO: return AJS_REPRODUCAO;
case AJ_PLR_FORMATO_POS: return AJS_REPRODUCAO;
case AJ_PLR_CODEC: return AJS_REPRODUCAO;
case AJ_SAIDA_PLAYER: return AJS_REPRODUCAO;
case AJ_PROPORCAO_PADRAO: return AJS_REPRODUCAO;
case AJ_BRILHO_PLAYER: return AJS_REPRODUCAO;
case AJ_HDR_UI: return AJS_REPRODUCAO;
case AJ_SEEKR_LIGADO: return AJS_REPRODUCAO;
case AJ_SEEKR_CHAVE: return AJS_REPRODUCAO;
case AJ_SEEKR_TESTAR: return AJS_REPRODUCAO;
case AJ_SEEKR_FITA: return AJS_REPRODUCAO;
case AJ_FONTE_REGEX: return AJS_REPRODUCAO;
case AJ_FONTE_REGEX_PADRAO: return AJS_REPRODUCAO;
case AJ_FONTE_REGEX_MODELO: return AJS_REPRODUCAO;
case AJ_FONTE_ESCOPO: return AJS_REPRODUCAO;
case AJ_FONTE_ADDONS_PERM: return AJS_REPRODUCAO;
case AJ_FONTE_PLUGINS_PERM: return AJS_REPRODUCAO;
case AJ_FONTE_ORDEM: return AJS_REPRODUCAO;
case AJ_FONTE_ORDEM_USO: return AJS_REPRODUCAO;
case AJ_FONTE_OUTROS: return AJS_REPRODUCAO;
case AJ_FONTE_AUTO: return AJS_REPRODUCAO;
case AJ_FONTE_TOCAR_CONFERINDO: return AJS_REPRODUCAO;
case AJ_FONTE_AQUECER: return AJS_REPRODUCAO;
case AJ_FONTE_CONFERIR_VARIAS: return AJS_REPRODUCAO;
case AJ_FONTE_PREPARAR: return AJS_REPRODUCAO;
#if (defined(NV_ANDROID))
case AJ_FONTE_PRECARREGAR: return AJS_REPRODUCAO;
#endif
case AJ_TAM_MIN: return AJS_REPRODUCAO;
case AJ_FONTE_TEXTO: return AJS_REPRODUCAO;
case AJ_FONTE_ORDEM_ADDON: return AJS_REPRODUCAO;
case AJ_SELOS_CORES: return AJS_REPRODUCAO;
case AJ_SELOS_PACOTE: return AJS_REPRODUCAO;
case AJ_SELOS_PACOTE_ADD: return AJS_REPRODUCAO;
case AJ_SELOS_PACOTE_REM: return AJS_REPRODUCAO;
case AJ_DTS_AC3: return AJS_REPRODUCAO;
#if (defined(NV_ANDROID))
case AJ_TUNNELING: return AJS_REPRODUCAO;
#endif
#if (!defined(NV_TPK) && !defined(NV_ANDROID))
case AJ_DV_MKV: return AJS_REPRODUCAO;
#endif
#if (!defined(NV_TPK) && !defined(NV_ANDROID)) && (defined(NV_DV7_CONVERSION))
case AJ_DV7: return AJS_REPRODUCAO;
#endif
case AJ_CACHE_SEEK: return AJS_REPRODUCAO;
case AJ_MANTER_VIDEO: return AJS_REPRODUCAO;
case AJ_RETOMADA_MORNA: return AJS_REPRODUCAO;
case AJ_MANTER_PRAZO: return AJS_REPRODUCAO;
case AJ_SEEKR_AJUSTE: return AJS_REPRODUCAO;
#if (defined(NV_ANDROID) || (!defined(NV_TPK) && !defined(__EMSCRIPTEN__)))
case AJ_BUFFER: return AJS_REPRODUCAO;
#endif
#if (defined(NV_ANDROID))
case AJ_PLAYER_EXTERNO: return AJS_REPRODUCAO;
#endif
#if (defined(NV_ANDROID))
case AJ_PLAYER_ANIME: return AJS_REPRODUCAO;
#endif
#if (!defined(__EMSCRIPTEN__))
case AJ_P2P_LIGADO: return AJS_REPRODUCAO;
#endif
#if (!defined(__EMSCRIPTEN__))
case AJ_P2P_URL: return AJS_REPRODUCAO;
#endif
#if (!defined(__EMSCRIPTEN__))
case AJ_P2P_TESTAR: return AJS_REPRODUCAO;
#endif
#if (!defined(__EMSCRIPTEN__)) && (!defined(NV_ANDROID) || defined(NV_P2P_MOTOR))
case AJ_P2P_LIMITE: return AJS_REPRODUCAO;
#endif
#if (!defined(__EMSCRIPTEN__)) && (!defined(NV_ANDROID) || defined(NV_P2P_MOTOR))
case AJ_P2P_DESTINO: return AJS_REPRODUCAO;
#endif
case AJ_IDIOMA: return AJS_IDIOMAS;
case AJ_AUD_LINGUA: return AJS_IDIOMAS;
case AJ_AUD_LINGUA2: return AJS_IDIOMAS;
case AJ_LEG_LINGUA: return AJS_IDIOMAS;
case AJ_LEG_FORCADA: return AJS_IDIOMAS;
case AJ_LEG_MOSTRAR: return AJS_IDIOMAS;
case AJ_LEG_SYNC_AUTO: return AJS_IDIOMAS;
#if (defined(NV_ANDROID))
case AJ_LEG_SYNC_AUDIO: return AJS_IDIOMAS;
#endif
case AJ_LEG1_TAMANHO: return AJS_IDIOMAS;
case AJ_LEG1_FONTE: return AJS_IDIOMAS;
case AJ_LEG1_COR: return AJS_IDIOMAS;
case AJ_LEG1_OPACIDADE: return AJS_IDIOMAS;
case AJ_LEG1_FUNDO: return AJS_IDIOMAS;
case AJ_LEG1_POSICAO: return AJS_IDIOMAS;
case AJ_LEG1_BORDA: return AJS_IDIOMAS;
case AJ_LEG1_COR_BORDA: return AJS_IDIOMAS;
case AJ_LEG1_NEGRITO: return AJS_IDIOMAS;
case AJ_LEG1_ATRASO: return AJS_IDIOMAS;
case AJ_LEG1_RESTAURAR: return AJS_IDIOMAS;
case AJ_LEG_LINGUA2: return AJS_IDIOMAS;
case AJ_LEG2_TAMANHO: return AJS_IDIOMAS;
case AJ_LEG2_FONTE: return AJS_IDIOMAS;
case AJ_LEG2_COR: return AJS_IDIOMAS;
case AJ_LEG2_OPACIDADE: return AJS_IDIOMAS;
case AJ_LEG2_FUNDO: return AJS_IDIOMAS;
case AJ_LEG2_POSICAO: return AJS_IDIOMAS;
case AJ_LEG2_BORDA: return AJS_IDIOMAS;
case AJ_LEG2_COR_BORDA: return AJS_IDIOMAS;
case AJ_LEG2_NEGRITO: return AJS_IDIOMAS;
case AJ_LEG2_ATRASO: return AJS_IDIOMAS;
case AJ_LEG2_RESTAURAR: return AJS_IDIOMAS;
case AJ_LEG2_POS: return AJS_IDIOMAS;
case AJ_LEG_CONTORNO_COR: return AJS_IDIOMAS;
case AJ_TMDB_IDIOMA: return AJS_IDIOMAS;
case AJ_ADDONS: return AJS_FONTES;
case AJ_PLUGINS: return AJS_FONTES;
case AJ_ADDONS_PRINCIPAL: return AJS_FONTES;
case AJ_BUSCA_CINEMETA: return AJS_FONTES;
case AJ_BUSCA_NUVIO: return AJS_FONTES;
case AJ_BUSCA_ORIGEM: return AJS_FONTES;
case AJ_FILTROS_FONTES: return AJS_FONTES;
case AJ_FALHA_PROVEDORES: return AJS_FONTES;
case AJ_FALHA_PROVEDOR_ORDEM: return AJS_FONTES;
case AJ_FALHA_ADDONS: return AJS_FONTES;
case AJ_FALHA_ADDON_ORDEM: return AJS_FONTES;
case AJ_FALHA_ADDON_LIMITE: return AJS_FONTES;
case AJ_PROVEDORES_ESCOPO: return AJS_FONTES;
case AJ_DEBRID_RD: return AJS_FONTES;
case AJ_DEBRID_TB: return AJS_FONTES;
case AJ_DEBRID_PM: return AJS_FONTES;
case AJ_DEBRID_AD: return AJS_FONTES;
case AJ_DEBRID_AD_TESTAR: return AJS_FONTES;
case AJ_DEBRID_RD_TESTAR: return AJS_FONTES;
case AJ_DEBRID_TB_TESTAR: return AJS_FONTES;
case AJ_DEBRID_PM_TESTAR: return AJS_FONTES;
case AJ_JF_LIGADO: return AJS_FONTES;
case AJ_JF_SERVIDOR: return AJS_FONTES;
case AJ_JF_ENTRAR: return AJS_FONTES;
case AJ_JF_SAIR: return AJS_FONTES;
case AJ_EM_SERVIDOR: return AJS_FONTES;
case AJ_EM_ENTRAR: return AJS_FONTES;
case AJ_EM_SAIR: return AJS_FONTES;
case AJ_PX_ENTRAR: return AJS_FONTES;
case AJ_PX_SERVIDOR: return AJS_FONTES;
case AJ_PX_SAIR: return AJS_FONTES;
case AJ_IPTV_ORDEM: return AJS_TVAOVIVO;
case AJ_IPTV_CONTA: return AJS_TVAOVIVO;
case AJ_IPTV_NOME: return AJS_TVAOVIVO;
case AJ_IPTV_ATIVA: return AJS_TVAOVIVO;
case AJ_IPTV_LISTA_STALKER: return AJS_TVAOVIVO;
case AJ_IPTV_ADD_STALKER: return AJS_TVAOVIVO;
case AJ_IPTV_LISTA_XTREAM: return AJS_TVAOVIVO;
case AJ_IPTV_ADD_XTREAM: return AJS_TVAOVIVO;
case AJ_IPTV_LISTA_M3U: return AJS_TVAOVIVO;
case AJ_IPTV_ADD_M3U: return AJS_TVAOVIVO;
case AJ_IPTV_REMOVER: return AJS_TVAOVIVO;
case AJ_IPTV_ACOES: return AJS_TVAOVIVO;
case AJ_IPTV_EXCLUIR: return AJS_TVAOVIVO;
case AJ_XTREAM_SERVIDOR: return AJS_TVAOVIVO;
case AJ_XTREAM_USUARIO: return AJS_TVAOVIVO;
case AJ_XTREAM_SENHA: return AJS_TVAOVIVO;
case AJ_XTREAM_CONTA: return AJS_TVAOVIVO;
case AJ_XTREAM_CONEXOES: return AJS_TVAOVIVO;
case AJ_XTREAM_LIMPAR: return AJS_TVAOVIVO;
case AJ_STALKER_PORTAL: return AJS_TVAOVIVO;
case AJ_STALKER_MAC: return AJS_TVAOVIVO;
case AJ_STALKER_LIMPAR: return AJS_TVAOVIVO;
case AJ_M3U_URL: return AJS_TVAOVIVO;
case AJ_M3U_EPG: return AJS_TVAOVIVO;
case AJ_M3U_LIMPAR: return AJS_TVAOVIVO;
case AJ_IPTV_SALVAR: return AJS_TVAOVIVO;
case AJ_IPTV_CANCELAR: return AJS_TVAOVIVO;
case AJ_GUIA_ORDEM: return AJS_TVAOVIVO;
case AJ_GUIA_FONTE: return AJS_TVAOVIVO;
case AJ_GUIA_RECARREGAR: return AJS_TVAOVIVO;
case AJ_EPG_PAIS: return AJS_TVAOVIVO;
case AJ_EPG_MEMORIA: return AJS_TVAOVIVO;
case AJ_GUIA_CATEGORIAS: return AJS_TVAOVIVO;
case AJ_URL_LISTA: return AJS_TVAOVIVO;
case AJ_URL_M3U_ADD: return AJS_TVAOVIVO;
case AJ_URL_XMLTV_ADD: return AJS_TVAOVIVO;
case AJ_URL_REM: return AJS_TVAOVIVO;
case AJ_LIVETV_SUGERIR: return AJS_TVAOVIVO;
case AJ_LIVETV_DIAG: return AJS_TVAOVIVO;
case AJ_LIVETV_RES: return AJS_TVAOVIVO;
case AJ_LIVETV_ESPERA: return AJS_TVAOVIVO;
case AJ_LIVETV_FORMATO: return AJS_TVAOVIVO;
case AJ_LIVETV_MODO: return AJS_TVAOVIVO;
case AJ_LIVETV_PROXY: return AJS_TVAOVIVO;
case AJ_HOME_LAYOUT: return AJS_HOME;
case AJ_LANDSCAPE: return AJS_HOME;
case AJ_HERO: return AJS_HOME;
case AJ_HERO_CHEIO: return AJS_HOME;
case AJ_FUNDO_DIN: return AJS_HOME;
case AJ_FUNDO_DIN_COR: return AJS_HOME;
case AJ_HERO_CATALOGOS: return AJS_HOME;
case AJ_HERO_GENERO: return AJS_HOME;
case AJ_HERO_SINOPSE: return AJS_HOME;
case AJ_HERO_NOTAS: return AJS_HOME;
case AJ_HERO_ANO: return AJS_HOME;
case AJ_HERO_DURACAO: return AJS_HOME;
case AJ_HERO_PASTA_DETALHES: return AJS_HOME;
case AJ_HERO_TAMANHO: return AJS_HOME;
case AJ_HERO_SINOPSE_TAM: return AJS_HOME;
case AJ_FIL_ORDEM: return AJS_HOME;
case AJ_FIL_LIMITE: return AJS_HOME;
case AJ_ITENS_FILEIRA: return AJS_HOME;
case AJ_ESPACO_FILEIRAS: return AJS_HOME;
case AJ_ESPACO_TITULOS: return AJS_HOME;
case AJ_OCULTAR_NLANC: return AJS_HOME;
case AJ_NOTAS_HOME: return AJS_HOME;
case AJ_SELO_VISTO: return AJS_HOME;
case AJ_ABAS_ESTILO: return AJS_HOME;
case AJ_OCULTAR_FILMES_VISTOS: return AJS_HOME;
case AJ_BIB_HOME: return AJS_HOME;
case AJ_AMIGOS_ESTILO: return AJS_HOME;
case AJ_CURADORIA_URL: return AJS_HOME;
case AJ_COLECAO_EXIBICAO: return AJS_HOME;
case AJ_CW_LIGADO: return AJS_HOME;
case AJ_CW_OK: return AJS_HOME;
case AJ_CW_ESTILO: return AJS_HOME;
case AJ_CW_THUMB: return AJS_HOME;
case AJ_CW_BLUR_PROX: return AJS_HOME;
case AJ_CW_ORDEM: return AJS_HOME;
case AJ_CW_NAO_EXIBIDOS: return AJS_HOME;
case AJ_CW_RETIDO_TAMBEM: return AJS_HOME;
case AJ_CW_DIAS: return AJS_HOME;
case AJ_MENU_TOPO: return AJS_HOME;
case AJ_RAIL_MODERNA: return AJS_HOME;
case AJ_RAIL: return AJS_HOME;
case AJ_MENU_EXPLORAR: return AJS_HOME;
case AJ_MENU_GUIA: return AJS_HOME;
case AJ_MENU_AGENDA: return AJS_HOME;
case AJ_MENU_PERFIL: return AJS_HOME;
case AJ_PS_FUNDO: return AJS_HOME;
case AJ_PS_CONTINUAR: return AJS_HOME;
case AJ_HERO_TRANSICAO: return AJS_HOME;
case AJ_ROTULOS: return AJS_HOME;
case AJ_NOME_ADDON: return AJS_HOME;
case AJ_SUFIXO_TIPO: return AJS_HOME;
case AJ_CW_FURTHEST: return AJS_HOME;
case AJ_RAIL_BLUR: return AJS_HOME;
#if ((defined(NV_TPK) || defined(__EMSCRIPTEN__)) && !defined(NV_TPK40))
case AJ_MIC_BUSCA: return AJS_HOME;
#endif
#if (defined(NV_TOUCH_UI))
case AJ_INTERFACE_MODO: return AJS_APARENCIA;
#endif
#if (defined(NV_TECLADO_SIS))
case AJ_TECLADO_ANDROID: return AJS_APARENCIA;
#endif
case AJ_TEMA: return AJS_APARENCIA;
case AJ_ARTE_TOM: return AJS_APARENCIA;
case AJ_FUNDO: return AJS_APARENCIA;
case AJ_COR_LOGO: return AJS_APARENCIA;
case AJ_VIDRO: return AJS_APARENCIA;
case AJ_VIDRO_CONTORNO: return AJS_APARENCIA;
case AJ_FOCO_ACENTO: return AJS_APARENCIA;
case AJ_SUAVIZAR_DEGRADES: return AJS_APARENCIA;
case AJ_TITULO_FIL: return AJS_APARENCIA;
case AJ_PALCO_FUNDO: return AJS_APARENCIA;
case AJ_LOGO_APP: return AJS_APARENCIA;
case AJ_ABERTURA: return AJS_APARENCIA;
case AJ_ICONE_APP: return AJS_APARENCIA;
case AJ_FONTE_UI: return AJS_APARENCIA;
case AJ_TAMANHO_UI: return AJS_APARENCIA;
case AJ_ESCALA_TEXTO: return AJS_APARENCIA;
case AJ_DET_ESPACO: return AJS_APARENCIA;
case AJ_TAMANHO_AJUSTES: return AJS_APARENCIA;
case AJ_LAYOUT_AJUSTES: return AJS_APARENCIA;
case AJ_ANIM: return AJS_APARENCIA;
case AJ_RELOGIO: return AJS_APARENCIA;
case AJ_RELOGIO_12H: return AJS_APARENCIA;
case AJ_ESMAECER: return AJS_APARENCIA;
case AJ_DESCANSO_ESTILO: return AJS_APARENCIA;
case AJ_DESCANSO_FONTE: return AJS_APARENCIA;
case AJ_VIDRO_OPAC: return AJS_APARENCIA;
case AJ_VIDRO_FOSCO: return AJS_APARENCIA;
case AJ_VIDRO_QUALIDADE: return AJS_APARENCIA;
case AJ_DET_VEU: return AJS_TITULO;
case AJ_DET_DATA_CHEIA: return AJS_TITULO;
case AJ_DET_BLUR_NAO_VISTOS: return AJS_TITULO;
case AJ_EP_TAMANHO: return AJS_TITULO;
case AJ_NT_IMDB: return AJS_TITULO;
case AJ_NT_TOMATES: return AJS_TITULO;
case AJ_NT_AUDIENCIA: return AJS_TITULO;
case AJ_NT_META: return AJS_TITULO;
case AJ_NT_METAUSER: return AJS_TITULO;
case AJ_NT_TRAKT: return AJS_TITULO;
case AJ_NT_TMDB: return AJS_TITULO;
case AJ_NT_LETTER: return AJS_TITULO;
case AJ_NT_MAL: return AJS_TITULO;
case AJ_NT_EBERT: return AJS_TITULO;
case AJ_NT_SCORE: return AJS_TITULO;
case AJ_NT_ORDEM: return AJS_TITULO;
case AJ_NT_EP: return AJS_TITULO;
case AJ_MDB_CHAVE: return AJS_TITULO;
case AJ_TMDB_LIGADO: return AJS_TITULO;
case AJ_MDB_LIGADO: return AJS_TITULO;
case AJ_TMDB_ARTE: return AJS_TITULO;
case AJ_TMDB_BASICO: return AJS_TITULO;
case AJ_TMDB_FICHA: return AJS_TITULO;
case AJ_TMDB_DATAS: return AJS_TITULO;
case AJ_TMDB_ELENCO: return AJS_TITULO;
case AJ_TMDB_PROD: return AJS_TITULO;
case AJ_TMDB_REDES: return AJS_TITULO;
case AJ_TMDB_EPS: return AJS_TITULO;
case AJ_TMDB_TRAILERS: return AJS_TITULO;
case AJ_TMDB_MAIS: return AJS_TITULO;
case AJ_TMDB_COL: return AJS_TITULO;
case AJ_TMDB_CW: return AJS_TITULO;
case AJ_DET_META_EXT: return AJS_TITULO;
case AJ_DET_SO_CINEMETA: return AJS_TITULO;
case AJ_EXPANDIR: return AJS_CARTAZES;
case AJ_EXPANDIR_ATRASO: return AJS_CARTAZES;
case AJ_BORDA_FOCO: return AJS_CARTAZES;
case AJ_MENU_CARTAZ: return AJS_CARTAZES;
case AJ_LARGURA_DP: return AJS_CARTAZES;
case AJ_RAIO_DP: return AJS_CARTAZES;
case AJ_PROF: return AJS_CARTAZES;
case AJ_HERO_FUNDO: return AJS_CARTAZES;
case AJ_HERO_ARTE_DIF: return AJS_CARTAZES;
case AJ_POSTER_PROV: return AJS_CARTAZES;
case AJ_FANART_CHAVE: return AJS_CARTAZES;
case AJ_HERO_TRAILER: return AJS_CARTAZES;
case AJ_FOCO_TRAILER: return AJS_CARTAZES;
case AJ_DET_TRAILER_AUTO: return AJS_CARTAZES;
case AJ_DET_TRAILER: return AJS_CARTAZES;
case AJ_HERO_TRAILER_SOM: return AJS_CARTAZES;
case AJ_TRAILER_FONTE: return AJS_CARTAZES;
#if (defined(__EMSCRIPTEN__))
case AJ_TRAILER_AUDIO: return AJS_CARTAZES;
#endif
#if (defined(NV_TPK))
case AJ_TRAILER_ZOOM_TPK: return AJS_CARTAZES;
#endif
case AJ_LOGO_TRAILER: return AJS_CARTAZES;
case AJ_GRAD_CLASSICO: return AJS_CARTAZES;
case AJ_MURAL_TITULOS: return AJS_CARTAZES;
case AJ_PROF_BRILHO: return AJS_CARTAZES;
case AJ_ADDON_POSTER: return AJS_CARTAZES;
case AJ_ADDON_FUNDO: return AJS_CARTAZES;
case AJ_ADDON_LOGO: return AJS_CARTAZES;
case AJ_COL_ARTE_CONTA: return AJS_CARTAZES;
case AJ_POSTER_INST: return AJS_CARTAZES;
case AJ_POSTER_TOKEN: return AJS_CARTAZES;
case AJ_POSTER_EXTRA: return AJS_CARTAZES;
case AJ_POSTER_CHAVE: return AJS_CARTAZES;
case AJ_POSTER_MODELO: return AJS_CARTAZES;
case AJ_POSTER_TESTAR: return AJS_CARTAZES;
case AJ_HERO_TRAILER_ESPERA: return AJS_CARTAZES;
case AJ_TRAILER_QUAL: return AJS_CARTAZES;
case AJ_TRAILER_ASPECTO: return AJS_CARTAZES;
case AJ_PERFIL_ATIVO: return AJS_CONTAS;
case AJ_SYNC: return AJS_CONTAS;
case AJ_PERFIL_PESQ: return AJS_CONTAS;
case AJ_PERFIL_EDITAR: return AJS_CONTAS;
case AJ_PERFIL_INFANTIL: return AJS_CONTAS;
case AJ_SAIR: return AJS_CONTAS;
case AJ_TRAKT: return AJS_CONTAS;
case AJ_SIMKL: return AJS_CONTAS;
case AJ_SALVOS_DEST: return AJS_CONTAS;
case AJ_CW_FONTE: return AJS_CONTAS;
case AJ_CW_PROXIMO: return AJS_CONTAS;
case AJ_HIST_CONTA: return AJS_CONTAS;
case AJ_SOCIAL: return AJS_CONTAS;
case AJ_DISCORD: return AJS_CONTAS;
case AJ_ENQUETES: return AJS_CONTAS;
case AJ_RESOLUCAO: return AJS_DESEMPENHO;
case AJ_GPU_EFEITOS: return AJS_DESEMPENHO;
case AJ_AMPLIAR_UI: return AJS_DESEMPENHO;
case AJ_AMPLIACAO_ALGO: return AJS_DESEMPENHO;
case AJ_AMPLIACAO_NITIDEZ: return AJS_DESEMPENHO;
case AJ_HDR_INTERFACE: return AJS_DESEMPENHO;
case AJ_QUALIDADE_IMG: return AJS_DESEMPENHO;
case AJ_NAV_RAPIDA: return AJS_DESEMPENHO;
case AJ_MEDIDOR: return AJS_DESEMPENHO;
case AJ_DIAGNOSTICO: return AJS_DESEMPENHO;
case AJ_VELOCIDADE: return AJS_DESEMPENHO;
case AJ_ESPACO: return AJS_DESEMPENHO;
#if (defined(NV_ANDROID))
case AJ_LIMPAR_CACHE: return AJS_DESEMPENHO;
#endif
case AJ_TEX_MB: return AJS_DESEMPENHO;
case AJ_GUIA: return AJS_SOBRE;
case AJ_NOVIDADES20: return AJS_SOBRE;
case AJ_DICASCONTROLE: return AJS_SOBRE;
case AJ_APOIAR: return AJS_SOBRE;
case AJ_AVISO_LEGAL: return AJS_SOBRE;
case AJ_VERSAO_I: return AJS_SOBRE;
case AJ_ATUALIZAR: return AJS_SOBRE;
case AJ_VER_REGISTRO: return AJS_SOBRE;
case AJ_ENVIAR_LOG: return AJS_SOBRE;
case AJ_ENVIO_AUTO: return AJS_SOBRE;
case AJ_LOG_REDE: return AJS_SOBRE;
#if (defined(NV_ANDROID))
case AJ_BAIXAR_REGISTRO: return AJS_SOBRE;
#endif
default: return AJS_CONTAS; } }
static int ajArtBlockForOption(int op) { switch (op) {
case AJ_QUALIDADE: return 0;
case AJ_FONTE_HDR: return 0;
case AJ_DV: return 0;
case AJ_ATMOS: return 0;
#if (defined(NV_TPK))
case AJ_AUDIO_OFFSET: return 0;
#endif
case AJ_FONTE_MANUAL: return 1;
case AJ_FONTE_PRIORIDADE: return 1;
case AJ_FONTE_IDIOMA_AUDIO: return 1;
case AJ_FONTE_PRAZO: return 1;
case AJ_FONTE_REPOR: return 1;
case AJ_TAM_MAX: return 1;
case AJ_FONTE_GRUPO: return 1;
case AJ_FONTE_GRUPO_SESSOES: return 1;
case AJ_LINK_REUSAR: return 1;
case AJ_LINK_PRAZO: return 1;
case AJ_PAUSA_OVERLAY: return 2;
case AJ_PAUSA_COMENTARIOS: return 2;
case AJ_PAUSA_NOTAS: return 2;
case AJ_PAUSA_ELENCO: return 2;
case AJ_TELA_ABERTURA: return 2;
case AJ_ABERTURA_ADDON: return 2;
case AJ_ABERTURA_FUNDO: return 2;
case AJ_ABERTURA_ESTILO: return 2;
case AJ_ESTAT_TAMANHO: return 2;
case AJ_ESTAT_LADO: return 2;
case AJ_PLR_CLASSIF: return 2;
case AJ_PULAR_ABERTURA: return 2;
case AJ_PULAR_RESUMO: return 2;
case AJ_PULAR_CREDITOS: return 2;
case AJ_PROXIMO_AUTO: return 2;
case AJ_PROXIMO_CONTAGEM: return 2;
case AJ_REACAO_CREDITOS: return 2;
case AJ_CW_CONCLUIDO: return 2;
case AJ_CW_MINIMO: return 2;
case AJ_PLR_FORMATO: return 2;
case AJ_PLR_FORMATO_POS: return 2;
case AJ_PLR_CODEC: return 2;
case AJ_SAIDA_PLAYER: return 2;
case AJ_PROPORCAO_PADRAO: return 2;
case AJ_BRILHO_PLAYER: return 2;
case AJ_HDR_UI: return 2;
case AJ_SEEKR_LIGADO: return 3;
case AJ_SEEKR_CHAVE: return 3;
case AJ_SEEKR_TESTAR: return 3;
case AJ_SEEKR_FITA: return 3;
case AJ_FONTE_REGEX: return 4;
case AJ_FONTE_REGEX_PADRAO: return 4;
case AJ_FONTE_REGEX_MODELO: return 4;
case AJ_FONTE_ESCOPO: return 4;
case AJ_FONTE_ADDONS_PERM: return 4;
case AJ_FONTE_PLUGINS_PERM: return 4;
case AJ_FONTE_ORDEM: return 4;
case AJ_FONTE_ORDEM_USO: return 4;
case AJ_FONTE_OUTROS: return 4;
case AJ_FONTE_AUTO: return 4;
case AJ_FONTE_TOCAR_CONFERINDO: return 4;
case AJ_FONTE_AQUECER: return 4;
case AJ_FONTE_CONFERIR_VARIAS: return 4;
case AJ_FONTE_PREPARAR: return 4;
#if (defined(NV_ANDROID))
case AJ_FONTE_PRECARREGAR: return 4;
#endif
case AJ_TAM_MIN: return 4;
case AJ_FONTE_TEXTO: return 4;
case AJ_FONTE_ORDEM_ADDON: return 4;
case AJ_SELOS_CORES: return 4;
case AJ_SELOS_PACOTE: return 4;
case AJ_SELOS_PACOTE_ADD: return 4;
case AJ_SELOS_PACOTE_REM: return 4;
case AJ_DTS_AC3: return 5;
#if (defined(NV_ANDROID))
case AJ_TUNNELING: return 5;
#endif
#if (!defined(NV_TPK) && !defined(NV_ANDROID))
case AJ_DV_MKV: return 5;
#endif
#if (!defined(NV_TPK) && !defined(NV_ANDROID)) && (defined(NV_DV7_CONVERSION))
case AJ_DV7: return 5;
#endif
case AJ_CACHE_SEEK: return 5;
case AJ_MANTER_VIDEO: return 5;
case AJ_RETOMADA_MORNA: return 5;
case AJ_MANTER_PRAZO: return 5;
case AJ_SEEKR_AJUSTE: return 5;
#if (defined(NV_ANDROID) || (!defined(NV_TPK) && !defined(__EMSCRIPTEN__)))
case AJ_BUFFER: return 5;
#endif
#if (defined(NV_ANDROID))
case AJ_PLAYER_EXTERNO: return 5;
#endif
#if (defined(NV_ANDROID))
case AJ_PLAYER_ANIME: return 5;
#endif
#if (!defined(__EMSCRIPTEN__))
case AJ_P2P_LIGADO: return 6;
#endif
#if (!defined(__EMSCRIPTEN__))
case AJ_P2P_URL: return 6;
#endif
#if (!defined(__EMSCRIPTEN__))
case AJ_P2P_TESTAR: return 6;
#endif
#if (!defined(__EMSCRIPTEN__)) && (!defined(NV_ANDROID) || defined(NV_P2P_MOTOR))
case AJ_P2P_LIMITE: return 6;
#endif
#if (!defined(__EMSCRIPTEN__)) && (!defined(NV_ANDROID) || defined(NV_P2P_MOTOR))
case AJ_P2P_DESTINO: return 6;
#endif
case AJ_IDIOMA: return 0;
case AJ_AUD_LINGUA: return 0;
case AJ_AUD_LINGUA2: return 0;
case AJ_LEG_LINGUA: return 0;
case AJ_LEG_FORCADA: return 0;
case AJ_LEG_MOSTRAR: return 0;
case AJ_LEG_SYNC_AUTO: return 0;
#if (defined(NV_ANDROID))
case AJ_LEG_SYNC_AUDIO: return 0;
#endif
case AJ_LEG1_TAMANHO: return 1;
case AJ_LEG1_FONTE: return 1;
case AJ_LEG1_COR: return 1;
case AJ_LEG1_OPACIDADE: return 1;
case AJ_LEG1_FUNDO: return 1;
case AJ_LEG1_POSICAO: return 1;
case AJ_LEG1_BORDA: return 1;
case AJ_LEG1_COR_BORDA: return 1;
case AJ_LEG1_NEGRITO: return 1;
case AJ_LEG1_ATRASO: return 1;
case AJ_LEG1_RESTAURAR: return 1;
case AJ_LEG_LINGUA2: return 2;
case AJ_LEG2_TAMANHO: return 2;
case AJ_LEG2_FONTE: return 2;
case AJ_LEG2_COR: return 2;
case AJ_LEG2_OPACIDADE: return 2;
case AJ_LEG2_FUNDO: return 2;
case AJ_LEG2_POSICAO: return 2;
case AJ_LEG2_BORDA: return 2;
case AJ_LEG2_COR_BORDA: return 2;
case AJ_LEG2_NEGRITO: return 2;
case AJ_LEG2_ATRASO: return 2;
case AJ_LEG2_RESTAURAR: return 2;
case AJ_LEG2_POS: return 2;
case AJ_LEG_CONTORNO_COR: return 2;
case AJ_TMDB_IDIOMA: return 3;
case AJ_ADDONS: return 0;
case AJ_PLUGINS: return 0;
case AJ_ADDONS_PRINCIPAL: return 0;
case AJ_BUSCA_CINEMETA: return 0;
case AJ_BUSCA_NUVIO: return 0;
case AJ_BUSCA_ORIGEM: return 0;
case AJ_FILTROS_FONTES: return 1;
case AJ_FALHA_PROVEDORES: return 1;
case AJ_FALHA_PROVEDOR_ORDEM: return 1;
case AJ_FALHA_ADDONS: return 1;
case AJ_FALHA_ADDON_ORDEM: return 1;
case AJ_FALHA_ADDON_LIMITE: return 1;
case AJ_PROVEDORES_ESCOPO: return 2;
case AJ_DEBRID_RD: return 2;
case AJ_DEBRID_TB: return 2;
case AJ_DEBRID_PM: return 2;
case AJ_DEBRID_AD: return 2;
case AJ_DEBRID_AD_TESTAR: return 2;
case AJ_DEBRID_RD_TESTAR: return 2;
case AJ_DEBRID_TB_TESTAR: return 2;
case AJ_DEBRID_PM_TESTAR: return 2;
case AJ_JF_LIGADO: return 3;
case AJ_JF_SERVIDOR: return 3;
case AJ_JF_ENTRAR: return 3;
case AJ_JF_SAIR: return 3;
case AJ_EM_SERVIDOR: return 3;
case AJ_EM_ENTRAR: return 3;
case AJ_EM_SAIR: return 3;
case AJ_PX_ENTRAR: return 3;
case AJ_PX_SERVIDOR: return 3;
case AJ_PX_SAIR: return 3;
case AJ_IPTV_ORDEM: return 0;
case AJ_IPTV_CONTA: return 0;
case AJ_IPTV_NOME: return 0;
case AJ_IPTV_ATIVA: return 0;
case AJ_IPTV_LISTA_STALKER: return 0;
case AJ_IPTV_ADD_STALKER: return 0;
case AJ_IPTV_LISTA_XTREAM: return 0;
case AJ_IPTV_ADD_XTREAM: return 0;
case AJ_IPTV_LISTA_M3U: return 0;
case AJ_IPTV_ADD_M3U: return 0;
case AJ_IPTV_REMOVER: return 0;
case AJ_IPTV_ACOES: return 0;
case AJ_IPTV_EXCLUIR: return 0;
case AJ_XTREAM_SERVIDOR: return 0;
case AJ_XTREAM_USUARIO: return 0;
case AJ_XTREAM_SENHA: return 0;
case AJ_XTREAM_CONTA: return 0;
case AJ_XTREAM_CONEXOES: return 0;
case AJ_XTREAM_LIMPAR: return 0;
case AJ_STALKER_PORTAL: return 0;
case AJ_STALKER_MAC: return 0;
case AJ_STALKER_LIMPAR: return 0;
case AJ_M3U_URL: return 0;
case AJ_M3U_EPG: return 0;
case AJ_M3U_LIMPAR: return 0;
case AJ_IPTV_SALVAR: return 0;
case AJ_IPTV_CANCELAR: return 0;
case AJ_GUIA_ORDEM: return 1;
case AJ_GUIA_FONTE: return 1;
case AJ_GUIA_RECARREGAR: return 1;
case AJ_EPG_PAIS: return 1;
case AJ_EPG_MEMORIA: return 1;
case AJ_GUIA_CATEGORIAS: return 1;
case AJ_URL_LISTA: return 1;
case AJ_URL_M3U_ADD: return 1;
case AJ_URL_XMLTV_ADD: return 1;
case AJ_URL_REM: return 1;
case AJ_LIVETV_SUGERIR: return 1;
case AJ_LIVETV_DIAG: return 2;
case AJ_LIVETV_RES: return 2;
case AJ_LIVETV_ESPERA: return 2;
case AJ_LIVETV_FORMATO: return 3;
case AJ_LIVETV_MODO: return 3;
case AJ_LIVETV_PROXY: return 3;
case AJ_HOME_LAYOUT: return 0;
case AJ_LANDSCAPE: return 0;
case AJ_HERO: return 0;
case AJ_HERO_CHEIO: return 0;
case AJ_FUNDO_DIN: return 0;
case AJ_FUNDO_DIN_COR: return 0;
case AJ_HERO_CATALOGOS: return 0;
case AJ_HERO_GENERO: return 0;
case AJ_HERO_SINOPSE: return 0;
case AJ_HERO_NOTAS: return 0;
case AJ_HERO_ANO: return 0;
case AJ_HERO_DURACAO: return 0;
case AJ_HERO_PASTA_DETALHES: return 0;
case AJ_HERO_TAMANHO: return 0;
case AJ_HERO_SINOPSE_TAM: return 0;
case AJ_FIL_ORDEM: return 1;
case AJ_FIL_LIMITE: return 1;
case AJ_ITENS_FILEIRA: return 1;
case AJ_ESPACO_FILEIRAS: return 1;
case AJ_ESPACO_TITULOS: return 1;
case AJ_OCULTAR_NLANC: return 1;
case AJ_NOTAS_HOME: return 1;
case AJ_SELO_VISTO: return 1;
case AJ_ABAS_ESTILO: return 1;
case AJ_OCULTAR_FILMES_VISTOS: return 1;
case AJ_BIB_HOME: return 1;
case AJ_AMIGOS_ESTILO: return 1;
case AJ_CURADORIA_URL: return 1;
case AJ_COLECAO_EXIBICAO: return 2;
case AJ_CW_LIGADO: return 3;
case AJ_CW_OK: return 3;
case AJ_CW_ESTILO: return 3;
case AJ_CW_THUMB: return 3;
case AJ_CW_BLUR_PROX: return 3;
case AJ_CW_ORDEM: return 3;
case AJ_CW_NAO_EXIBIDOS: return 3;
case AJ_CW_RETIDO_TAMBEM: return 3;
case AJ_CW_DIAS: return 3;
case AJ_MENU_TOPO: return 4;
case AJ_RAIL_MODERNA: return 4;
case AJ_RAIL: return 4;
case AJ_MENU_EXPLORAR: return 4;
case AJ_MENU_GUIA: return 4;
case AJ_MENU_AGENDA: return 4;
case AJ_MENU_PERFIL: return 4;
case AJ_PS_FUNDO: return 5;
case AJ_PS_CONTINUAR: return 5;
case AJ_HERO_TRANSICAO: return 6;
case AJ_ROTULOS: return 7;
case AJ_NOME_ADDON: return 7;
case AJ_SUFIXO_TIPO: return 7;
case AJ_CW_FURTHEST: return 8;
case AJ_RAIL_BLUR: return 9;
#if ((defined(NV_TPK) || defined(__EMSCRIPTEN__)) && !defined(NV_TPK40))
case AJ_MIC_BUSCA: return 9;
#endif
#if (defined(NV_TOUCH_UI))
case AJ_INTERFACE_MODO: return 0;
#endif
#if (defined(NV_TECLADO_SIS))
case AJ_TECLADO_ANDROID: return 0;
#endif
case AJ_TEMA: return 0;
case AJ_ARTE_TOM: return 0;
case AJ_FUNDO: return 0;
case AJ_COR_LOGO: return 0;
case AJ_VIDRO: return 0;
case AJ_VIDRO_CONTORNO: return 0;
case AJ_FOCO_ACENTO: return 0;
case AJ_SUAVIZAR_DEGRADES: return 0;
case AJ_TITULO_FIL: return 1;
case AJ_PALCO_FUNDO: return 1;
case AJ_LOGO_APP: return 2;
case AJ_ABERTURA: return 2;
case AJ_ICONE_APP: return 2;
case AJ_FONTE_UI: return 3;
case AJ_TAMANHO_UI: return 3;
case AJ_ESCALA_TEXTO: return 3;
case AJ_DET_ESPACO: return 3;
case AJ_TAMANHO_AJUSTES: return 3;
case AJ_LAYOUT_AJUSTES: return 3;
case AJ_ANIM: return 4;
case AJ_RELOGIO: return 4;
case AJ_RELOGIO_12H: return 4;
case AJ_ESMAECER: return 5;
case AJ_DESCANSO_ESTILO: return 5;
case AJ_DESCANSO_FONTE: return 5;
case AJ_VIDRO_OPAC: return 6;
case AJ_VIDRO_FOSCO: return 6;
case AJ_VIDRO_QUALIDADE: return 6;
case AJ_DET_VEU: return 0;
case AJ_DET_DATA_CHEIA: return 0;
case AJ_DET_BLUR_NAO_VISTOS: return 0;
case AJ_EP_TAMANHO: return 0;
case AJ_NT_IMDB: return 1;
case AJ_NT_TOMATES: return 1;
case AJ_NT_AUDIENCIA: return 1;
case AJ_NT_META: return 1;
case AJ_NT_METAUSER: return 1;
case AJ_NT_TRAKT: return 1;
case AJ_NT_TMDB: return 1;
case AJ_NT_LETTER: return 1;
case AJ_NT_MAL: return 1;
case AJ_NT_EBERT: return 1;
case AJ_NT_SCORE: return 1;
case AJ_NT_ORDEM: return 1;
case AJ_NT_EP: return 1;
case AJ_MDB_CHAVE: return 1;
case AJ_TMDB_LIGADO: return 2;
case AJ_MDB_LIGADO: return 3;
case AJ_TMDB_ARTE: return 4;
case AJ_TMDB_BASICO: return 4;
case AJ_TMDB_FICHA: return 4;
case AJ_TMDB_DATAS: return 4;
case AJ_TMDB_ELENCO: return 4;
case AJ_TMDB_PROD: return 4;
case AJ_TMDB_REDES: return 4;
case AJ_TMDB_EPS: return 4;
case AJ_TMDB_TRAILERS: return 4;
case AJ_TMDB_MAIS: return 4;
case AJ_TMDB_COL: return 4;
case AJ_TMDB_CW: return 4;
case AJ_DET_META_EXT: return 4;
case AJ_DET_SO_CINEMETA: return 4;
case AJ_EXPANDIR: return 0;
case AJ_EXPANDIR_ATRASO: return 0;
case AJ_BORDA_FOCO: return 0;
case AJ_MENU_CARTAZ: return 0;
case AJ_LARGURA_DP: return 1;
case AJ_RAIO_DP: return 1;
case AJ_PROF: return 1;
case AJ_HERO_FUNDO: return 2;
case AJ_HERO_ARTE_DIF: return 2;
case AJ_POSTER_PROV: return 2;
case AJ_FANART_CHAVE: return 2;
case AJ_HERO_TRAILER: return 3;
case AJ_FOCO_TRAILER: return 3;
case AJ_DET_TRAILER_AUTO: return 3;
case AJ_DET_TRAILER: return 3;
case AJ_HERO_TRAILER_SOM: return 4;
case AJ_TRAILER_FONTE: return 4;
#if (defined(__EMSCRIPTEN__))
case AJ_TRAILER_AUDIO: return 4;
#endif
#if (defined(NV_TPK))
case AJ_TRAILER_ZOOM_TPK: return 4;
#endif
case AJ_LOGO_TRAILER: return 4;
case AJ_GRAD_CLASSICO: return 5;
case AJ_MURAL_TITULOS: return 5;
case AJ_PROF_BRILHO: return 6;
case AJ_ADDON_POSTER: return 7;
case AJ_ADDON_FUNDO: return 7;
case AJ_ADDON_LOGO: return 7;
case AJ_COL_ARTE_CONTA: return 7;
case AJ_POSTER_INST: return 7;
case AJ_POSTER_TOKEN: return 7;
case AJ_POSTER_EXTRA: return 7;
case AJ_POSTER_CHAVE: return 7;
case AJ_POSTER_MODELO: return 7;
case AJ_POSTER_TESTAR: return 7;
case AJ_HERO_TRAILER_ESPERA: return 8;
case AJ_TRAILER_QUAL: return 8;
case AJ_TRAILER_ASPECTO: return 8;
case AJ_PERFIL_ATIVO: return 0;
case AJ_SYNC: return 0;
case AJ_PERFIL_PESQ: return 0;
case AJ_PERFIL_EDITAR: return 0;
case AJ_PERFIL_INFANTIL: return 0;
case AJ_SAIR: return 0;
case AJ_TRAKT: return 1;
case AJ_SIMKL: return 1;
case AJ_SALVOS_DEST: return 1;
case AJ_CW_FONTE: return 1;
case AJ_CW_PROXIMO: return 1;
case AJ_HIST_CONTA: return 1;
case AJ_SOCIAL: return 1;
case AJ_DISCORD: return 1;
case AJ_ENQUETES: return 2;
case AJ_RESOLUCAO: return 0;
case AJ_GPU_EFEITOS: return 0;
case AJ_AMPLIAR_UI: return 0;
case AJ_AMPLIACAO_ALGO: return 0;
case AJ_AMPLIACAO_NITIDEZ: return 0;
case AJ_HDR_INTERFACE: return 0;
case AJ_QUALIDADE_IMG: return 0;
case AJ_NAV_RAPIDA: return 0;
case AJ_MEDIDOR: return 0;
case AJ_DIAGNOSTICO: return 0;
case AJ_VELOCIDADE: return 0;
case AJ_ESPACO: return 0;
#if (defined(NV_ANDROID))
case AJ_LIMPAR_CACHE: return 0;
#endif
case AJ_TEX_MB: return 0;
case AJ_GUIA: return 0;
case AJ_NOVIDADES20: return 0;
case AJ_DICASCONTROLE: return 0;
case AJ_APOIAR: return 0;
case AJ_AVISO_LEGAL: return 0;
case AJ_VERSAO_I: return 0;
case AJ_ATUALIZAR: return 0;
case AJ_VER_REGISTRO: return 1;
case AJ_ENVIAR_LOG: return 1;
case AJ_ENVIO_AUTO: return 1;
case AJ_LOG_REDE: return 1;
#if (defined(NV_ANDROID))
case AJ_BAIXAR_REGISTRO: return 1;
#endif
default: return 0; } }

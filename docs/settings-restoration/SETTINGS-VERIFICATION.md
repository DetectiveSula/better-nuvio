# Per-setting verification ledger

**This is not an all-settings functional certification.** Every binding is listed below; value-range checks are separate from consumer tests. See `SETTINGS-EXPANSION.md` for fixes, suites and device limitations.

Android bindings: 434. Host: 433. Original bindings preserved: 413. Host value contracts checked: 341. Focused functional bindings: 30. Filter subchoices: 228.

| Setting | Key | Value contract | Functional verification |
|---|---|---|---|
| AJ_QUALIDADE — Maximum quality | qualidade | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DV — Dolby Vision | dolbyVision | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ATMOS — Prefer Dolby Atmos sound | dolbyAtmos | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG_LINGUA — Subtitle language | legendaIdioma | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AUD_LINGUA — Audio language | audioIdioma | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PAUSA_OVERLAY — Pause panel | pauseOverlayEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_MANUAL — Pick the source yourself | escolherFonteManual | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_AUTO — Automatic source | fonteAutoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_REPOR — Another source on failure | fonteReporLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_TEXTO — Source text | fonteTextoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LANDSCAPE — Landscape posters | modernLandscapePostersEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_CHEIO — Full-screen backdrop | modernHeroFullScreenBackdropEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_FUNDO — Featured background image | heroFundoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_ARTE_DIF — Hero with different art | heroDifferentFromCard | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_TRAILER — Trailer on the hero | trailerHero | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FIL_LIMITE — Row limit | -limiteFileiras | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FIL_ORDEM — Reorder and enable rows | -ordenarFileiras | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_RAIL — Sidebar | collapseSidebar | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RAIL_MODERNA — Modern sidebar | modernSidebar | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RAIL_BLUR — Modern sidebar blur | modernSidebarBlur | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO — Show hero | heroSectionEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_CATALOGOS — Hero catalogs | -heroCatalogKeys | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_PS_FUNDO — Profile picker background | perfilFundoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DESCOBRIR — Discover location | discoverLocation | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ROTULOS — Poster labels | posterLabelsEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NOME_ADDON — Add-on name in the row | catalogAddonNameEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SUFIXO_TIPO — Movie or series in the row name | catalogTypeSuffixEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_OCULTAR_NLANC — Hide unreleased | hideUnreleasedContent | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NOTAS_HOME — IMDb rating on posters | homeImdbRatingsVisibility | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_GRAD_CLASSICO — Old highlight on the focused poster | classicFocusGradientEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_LIGADO — Show "Continue Watching" | continueWatchingEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_OK — OK on the card | cwOkLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_FONTE — Source for "Continue Watching" | cwFonteLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_ESTILO — "Continue Watching" style | continueWatchingCardStyle | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_THUMB — Episode thumbnail | useEpisodeThumbnailsInCw | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_BLUR_PROX — Blur next episode | blurContinueWatchingNextUp | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_FURTHEST — Next up from furthest episode | nextUpFromFurthestEpisode | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_NAO_EXIBIDOS — Show unaired episodes | showUnairedNextUp | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_ORDEM — Sort order | continueWatchingSortMode | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_BLUR_NAO_VISTOS — Blur unwatched | blurUnwatchedEpisodes | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_TRAILER — Trailer button | detailPageTrailerButtonEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_META_EXT — Prefer external metadata | preferExternalMetaAddonDetail | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_DATA_CHEIA — Full release date | showFullReleaseDate | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_VEU — Backdrop dimming | detalheVeu | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_TRAILER_AUTO — Auto-play trailer | trailerAuto | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TRAILER_QUAL — Trailer quality | trailerQualidade | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TRAILER_ASPECTO — Trailer aspect | trailerAspecto | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TRAILER_FONTE — Trailer source | trailerFonteLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_EXPANDIR — Expand poster on focus | focusedPosterBackdropExpandEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_EXPANDIR_ATRASO — Expansion delay | focusedPosterBackdropExpandDelaySeconds | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NAV_RAPIDA — Fast horizontal navigation | fastHorizontalNavigationEnabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BORDA_FOCO — Border on the focused card | bordaFocoCartaz | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PROF — Depth effect | cardDepthEnabled | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_BORDA — Edge glow | cardDepthEdgeStrength | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_BRILHO — Top sheen strength | cardDepthSheenStrength | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_COBERTURA — Edge coverage | cardDepthEdgeCoverage | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_POSTERS — Depth on posters | cardDepthPostersEnabled | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_CW — Depth on "Continue" | cardDepthContinueWatchingEnabled | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_EPS — Depth on episodes | cardDepthEpisodeCardsEnabled | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_ELENCO — Depth on cast | cardDepthCastEnabled | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_PROF_TRAILERS — Depth on trailers | cardDepthTrailersEnabled | PASS: every legal choice and numeric boundary | Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending. |
| AJ_LARGURA_DP — Poster card width | posterCardWidthDp | PASS: every legal choice and numeric boundary | Six widths and five corner presets change actual Home geometry/radius. Existing custom numeric values remain compatible; real TV sizing pending. |
| AJ_RAIO_DP — Poster corners | posterCardCornerRadiusDp | PASS: every legal choice and numeric boundary | Six widths and five corner presets change actual Home geometry/radius. Existing custom numeric values remain compatible; real TV sizing pending. |
| AJ_QUALIDADE_IMG — Image quality | qualidadeImagem | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IDIOMA — App language | idioma | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ANIM — Animations | animacoes | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RESOLUCAO — Interface resolution | resolucaoUi | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TEMA — Accent color | selected_theme | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_COR_LOGO — Logo color | corDaLogoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PERFIL_ATIVO — Profile | -perfil | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_SYNC — Sync | -sync | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_ADDONS — Add-ons | -addons | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_STALKER_PORTAL — MAC portal (Stalker) | -stalkerPortal | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_STALKER_MAC — Portal MAC | -stalkerMac | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_STALKER_LIMPAR — Remove the Stalker portal | -stalkerLimpar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_XTREAM_SERVIDOR — Xtream server | -xtreamServidor | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_XTREAM_USUARIO — Xtream username | -xtreamUsuario | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_XTREAM_SENHA — Xtream password | -xtreamSenha | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_XTREAM_LIMPAR — Remove Xtream | -xtreamLimpar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_XTREAM_CONTA — Xtream account | -xtreamConta | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_EPG_PAIS — TV guide source | epgPaisLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SALVOS_DEST — Library source | salvosDestino | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TRAKT — Trakt | -trakt | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_SIMKL — Simkl | -simkl | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_SAIR — Sign out | -sair | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_VERSAO_I — Version | -versao | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_ATUALIZAR — Check for updates | -atualizar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_ENVIAR_LOG — Send log | -registro | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_ENVIO_AUTO — Send logs automatically | envioAuto | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ESPACO — Images kept in memory | -espaco | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_TEX_MB — Memory for images | texturasMB | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_LIGADO — TMDB data | tmdb_enabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_IDIOMA — Language of title data | tmdb_language | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_ARTE — Localized artwork | tmdb_use_artwork | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_BASICO — Title and overview | tmdb_use_basic_info | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_FICHA — Details | tmdb_use_details | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_DATAS — Release dates | tmdb_use_release_dates | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_ELENCO — Cast and crew | tmdb_use_credits | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_PROD — Production companies | tmdb_use_productions | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_REDES — Networks and studios | tmdb_use_networks | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_EPS — Episodes | tmdb_use_episodes | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_TRAILERS — Trailers | tmdb_use_trailers | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_MAIS — "More like this" | tmdb_use_more_like_this | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_COL — Collections and sagas | tmdb_use_collections | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TMDB_CW — Enrich "Continue Watching" | tmdb_enrich_continue_watching | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_LIGADO — MDBList ratings | mdblist_enabled | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_CHAVE — MDBList key | -mdblistChave | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_MDB_TRAKT — Trakt ratings | mdblist_show_trakt | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_IMDB — IMDb ratings | mdblist_show_imdb | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_TMDB — TMDB ratings | mdblist_show_tmdb | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_LETTER — Letterboxd ratings | mdblist_show_letterboxd | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_TOMATES — Rotten Tomatoes ratings | mdblist_show_tomatoes | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_AUDIENCIA — Audience score | mdblist_show_audience | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_META — Metacritic ratings | mdblist_show_metacritic | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MDB_MAL — MyAnimeList ratings | mdblist_show_mal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FANART_CHAVE — fanart.tv key | -fanartChave | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DIAGNOSTICO — Diagnostics and optimization | -diagnostico | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_VELOCIDADE — Speed test | -velocidade | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_MENU_EXPLORAR — Explore in the sidebar | menuExplorarLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MENU_GUIA — TV Guide in the sidebar | menuGuiaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MENU_AGENDA — Schedule in the sidebar | menuAgendaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MENU_PERFIL — Profile & Stats in the sidebar | menuPerfilLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FOCO_TRAILER — Trailer on the focused poster | trailerCartazModo | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ITENS_FILEIRA — Items per row | itensFileiraLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_UI — Interface font | fonteInterface | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_GPU_EFEITOS — Visual effects | gpuEfeitosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_VIDRO — Glass UI | vidroLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_P2P_LIGADO — P2P server (experimental) | p2pLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_P2P_URL — P2P server address | -p2pEndereco | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_P2P_TESTAR — Test P2P server | -p2pTestar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_POSTER_PROV — Custom posters | posterProvLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_POSTER_INST — SpatialPosters address | -posterInst | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_POSTER_TOKEN — SpatialPosters token | -posterToken | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_POSTER_EXTRA — SpatialPosters parameters | -posterExtra | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_POSTER_CHAVE — RPDB key | -posterChave | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_POSTER_MODELO — Poster URL template | -posterModelo | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_POSTER_TESTAR — Test posters | -posterTestar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_HOME_LAYOUT — Home layout | homeLayoutLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DEBRID_AD — AllDebrid key | -debridAD | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DEBRID_AD_TESTAR — Test AllDebrid key | -debridADTestar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DEBRID_RD — Real-Debrid key | -debridRD | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DEBRID_TB — TorBox key | -debridTB | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DEBRID_PM — Premiumize key | -debridPM | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DEBRID_RD_TESTAR — Test Real-Debrid key | -debridRDTestar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DEBRID_TB_TESTAR — Test TorBox key | -debridTBTestar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DEBRID_PM_TESTAR — Test Premiumize key | -debridPMTestar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_DET_SO_CINEMETA — Always use the Nuvio catalog | soCinemetaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_IMDB — IMDb | notaTituloImdb | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_TOMATES — Rotten Tomatoes (critics) | notaTituloTomates | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_AUDIENCIA — Popcornmeter (audience) | notaTituloAudiencia | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_META — Metacritic (critics) | notaTituloMeta | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_METAUSER — Metacritic (users) | notaTituloMetaUser | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_TRAKT — Trakt | notaTituloTrakt | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_TMDB — TMDB | notaTituloTmdb | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_LETTER — Letterboxd | notaTituloLetter | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_MAL — MyAnimeList | notaTituloMal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_EBERT — Roger Ebert (critic) | notaTituloEbert | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_SCORE — MDBList score | notaTituloScore | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PERFIL_PESQ — Searchable profile | -perfilPesquisavel | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PERFIL_EDITAR — My public profile | -perfilEditar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_VIDRO_CONTORNO — Glass outline | vidroContornoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_TRAILER_SOM — Trailer sound | trailerDestaqueSomLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_TRAILER_ESPERA — Hero trailer delay | trailerDestaqueEsperaLocal | PASS: every legal choice and numeric boundary | Effective milliseconds, every legal new detail-delay value and disk reload; production Home/detail timing consumers traced. Live trailer timing on TV pending. |
| AJ_CW_CONCLUIDO — Watched percentage | cwConcluidoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ADDONS_PRINCIPAL — Use the primary profile's add-ons | addonsPrincipalLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_TRANSICAO — Hero transition | heroTransicaoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ADDON_POSTER — Addon posters | posterAddonLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ADDON_FUNDO — Addon hero background | fundoAddonLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ADDON_LOGO — Addon logo | logoAddonLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_COL_ARTE_CONTA — Account folder artwork | colArteContaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SELOS_CORES — Colored badges | selosColoridosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_TRAILER_SOM — Title page trailer sound | trailerDetalheSomLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LIVETV_RES — Main resolution | liveTvResolucaoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LIVETV_FORMATO — Xtream format | xtreamFormatoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LIVETV_ESPERA — Wait for a channel to open | liveTvEsperaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LIVETV_DIAG — Live TV diagnostics | -liveTvDiag | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_LIVETV_MODO — Channel player mode | liveTvModoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LIVETV_PROXY — Relay the channel through the app (proxy) | liveTvProxyLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RELOGIO — Clock on screen | relogioTelaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RELOGIO_POS — Clock position | relogioPosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SELO_VISTO — Watched badge on posters | seloVistoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SEEKR_LIGADO — Seek bar thumbnails | seekrLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SEEKR_CHAVE — Seekr key | -seekrChave | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_SEEKR_TESTAR — Test Seekr key | -seekrTestar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_SEEKR_FITA — Thumbnail strip | seekrFitaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SEEKR_AJUSTE — Thumbnail sync | seekrAjusteLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SAIDA_PLAYER — When leaving mid-movie | saidaPlayerLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_REACAO_CREDITOS — Ask what you thought at the credits | reacaoCreditosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_PRAZO — How long to wait for add-ons | fonteEsperaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_VER_REGISTRO — View the log on screen | -verRegistro | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_MEDIDOR — Performance meter | medidorFormaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_GUIA — User guide | -guiaUso | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_SELOS_PACOTE — Badge pack | -selosPacote | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SELOS_PACOTE_ADD — Add badge pack | -selosPacoteAdd | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_SELOS_PACOTE_REM — Remove pack | -selosPacoteRem | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_TAMANHO_UI — Interface size | tamanhoUiLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FUNDO — Background | fundoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_VIDRO_OPAC — Glass opacity | vidroOpacLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_VIDRO_FOSCO — Frosted glass | vidroFoscoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ICONE_APP — App Icon & Banner | iconeAppLocal | PASS: every legal choice and numeric boundary | All 10 icon/banner/TV-launcher resource pairs and entitlement policy. Existing linked selection is retained. Actual launcher alias switching/cache refresh NOT device-validated; locked without entitlement. |
| AJ_DISCORD — Discord | -discord | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_TAMANHO_AJUSTES — Settings size | tamanhoAjustesLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LOGO_TRAILER — Hide logo during trailer | logoTrailerLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG_LINGUA2 — Second subtitle language | legendaSecundariaIdioma | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG_SYNC_AUDIO — Sync by audio | legendaSyncAudioLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CACHE_SEEK — Keep the downloaded part on disk | cacheSeekLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TRAILER_ZOOM_TPK — Trailer and player zoom (experimental) | trailerZoomTpkLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PLUGINS — Plugins | -plugins | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_JF_LIGADO — Personal servers (experimental) | jellyfinLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_JF_SERVIDOR — Jellyfin address | -jellyfinServidor | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_JF_ENTRAR — Sign in to Jellyfin | -jellyfinEntrar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_JF_SAIR — Sign out of Jellyfin | -jellyfinSair | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_AVANCADAS — Show advanced options | avancadasLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_POS — Second subtitle position | legenda2PosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_TAMANHO — Size | legenda2EscalaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_COR — Color | legenda2CorLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_FUNDO — Background | legenda2FundoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_BORDA — Edge | legenda2BordaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_EM_SERVIDOR — Emby address | -embyServidor | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_EM_ENTRAR — Sign in to Emby | -embyEntrar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_EM_SAIR — Sign out of Emby | -embySair | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_PX_ENTRAR — Sign in to Plex | -plexEntrar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_PX_SERVIDOR — Plex server | -plexServidor | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_PX_SAIR — Sign out of Plex | -plexSair | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FONTE_PRIORIDADE — What Nuvio prioritizes | fontePrioridadeLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_HDR — Dolby Vision and HDR | fonteHdrLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LOGO_APP — App logo | logoAppLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ABERTURA — App opening | aberturaAppLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ENQUETES — Receive polls | enquetesLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BUSCA_CINEMETA — Search Cinemeta | buscaCinemetaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ESMAECER — Screensaver | esmaecerLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BRILHO_PLAYER — Player interface brightness | brilhoPlayerLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NOVIDADES20 — What's new in 2.0 | -novidades20 | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_MANTER_VIDEO — Keep the video ready on exit | manterVideoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DESCANSO_ESTILO — Screensaver style | descansoEstiloLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DESCANSO_FONTE — Showcase titles | descansoFonteLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RELOGIO_12H — Clock format | relogio12hLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TAM_MAX — Maximum file size | tamanhoMaxLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TAM_MIN — Minimum file size | tamanhoMinLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_APOIAR — Support the project | -apoiar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_LEG_FORCADA — Only signs when the audio is already in your language | legendaForcadaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PLR_CLASSIF — Age rating in the player | classifPlayerLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_ESCOPO — Auto-play sources | streamAutoPlaySource | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_ADDONS_PERM — Allowed add-ons | -fonteAddonsPermitidos | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FONTE_PLUGINS_PERM — Allowed plugins | -fontePluginsPermitidos | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FONTE_OUTROS — Use the others if there are none | fonteOutrosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_REGEX — Filter sources by name (regex) | fonteRegexLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_REGEX_PADRAO — Filter pattern | -fonteRegexPadrao | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FONTE_REGEX_MODELO — Ready-made filter | -fonteRegexModelo | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ESPACO_FILEIRAS — Space between rows | espacoFileirasLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ESPACO_TITULOS — Space between titles | espacoTitulosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PROPORCAO_PADRAO — Default aspect ratio | proporcaoPadraoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_RETIDO_TAMBEM — Also in Continue watching | cwRetidoTambemLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PS_CONTINUAR — Continue on profile picker | psContinuarLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DTS_AC3 — Convert DTS so the TV can play it | dtsSaidaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DV_MKV — Dolby Vision in MKV (experimental) | dvMkvLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HIST_CONTA — Send history to the Nuvio account | histContaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SOCIAL — Social features | socialLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_ORDEM — Add-on order | -fonteOrdem | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FONTE_ORDEM_USO — Use the order | fonteOrdemUsoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_TOCAR_CONFERINDO — Play while checking the source | fonteTocarConferindoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_AQUECER — Warm up connections when a title opens | fonteAquecerLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_CONFERIR_VARIAS — Check several sources at once | fonteConferirVariasLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_PREPARAR — Prepare the source when a title opens | fontePrepararLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_PROXIMO — Next episode in Continue | cwProximoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BUSCA_NUVIO — Nuvio catalog results in search | buscaNuvioLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BUSCA_ORIGEM — Show the source on search results | buscaOrigemLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LAYOUT_AJUSTES — Settings layout | ajustesLayoutLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_P2P_LIMITE — P2P space limit | p2pLimiteLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_P2P_DESTINO — P2P cache location | p2pDestinoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_ORDEM_ADDON — Source order | fonteOrdemLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG_SYNC_AUTO — Automatic subtitle sync | legendaSyncAutoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG_MOSTRAR — Show in player | legendaMostrarLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AVISO_LEGAL — Legal notice | -avisoLegal | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_HDR_UI — Interface brightness in HDR | hdrUiLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TELA_ABERTURA — Video opening screen | telaAberturaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_MINIMO — Minimum to enter Continue | cwMinimoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BUFFER — Buffer size | bufferPlayerLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PULAR_ABERTURA — Skip the intro automatically | pularAberturaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PULAR_RESUMO — Skip the recap automatically | pularResumoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PROXIMO_AUTO — Play the next episode automatically | proximoAutoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PULAR_CREDITOS — Skip the credits automatically | pularCreditosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PROXIMO_CONTAGEM — Next episode countdown | proximoContagemLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_INTERFACE_MODO — Interface mode | interfaceModoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MIC_BUSCA — Mic button in search | micBuscaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MENU_TOPO — Menu position | menuTopoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ABAS_ESTILO — Tab style | abasEstiloLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TECLADO_ANDROID — Keyboard | tecladoAndroidLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG_CONTORNO_COR — Subtitle outline color | legendaContornoCorLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FALHA_PROVEDORES — Switch provider on failure | -falhaProvedores | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FALHA_PROVEDOR_ORDEM — Provider order and limits | -falhaProvedorOrdem | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FALHA_RD_LIMITE — Links on Real-Debrid | -falhaRDLinks | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FALHA_TB_LIMITE — Links on TorBox | -falhaTBLinks | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FALHA_PM_LIMITE — Links on Premiumize | -falhaPMLinks | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FALHA_AD_LIMITE — Links on AllDebrid | -falhaADLinks | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FALHA_ADDONS — Switch add-on on failure | -falhaAddons | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FALHA_ADDON_ORDEM — Add-on order and limits | -falhaAddonOrdem | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FALHA_ADDON_LIMITE — Links in this add-on | -falhaAddonLinks | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RETOMADA_MORNA — Quick resume on return | retomadaMornaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MANTER_PRAZO — Time to keep video ready | manterVideoPrazoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ESTAT_TAMANHO — Live statistics size | estatTamanhoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_OCULTAR_FILMES_VISTOS — Hide watched movies | hideWatchedMoviesLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ESTAT_LADO — Live statistics side | estatLadoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DICASCONTROLE — Remote tips | -dicasControle | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_HERO_GENERO — Show genre on hero | heroGeneroLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ABERTURA_ADDON — Show add-on on opening | aberturaAddonLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_SINOPSE — Show synopsis on hero | heroSinopseLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_NOTAS — Show ratings on hero | heroNotasLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_ANO — Show year on hero | heroAnoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_DURACAO — Show runtime on hero | heroDuracaoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CURADORIA_URL — Curation address | -curadoria | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_ABERTURA_FUNDO — Opening screen backdrop | aberturaFundoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_SINOPSE_TAM — Synopsis size | heroSynopsisSizeLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_EP_TAMANHO — Episode card size | epTamanhoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_PRECARREGAR — Preload video | fontePrecarregarLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PROVEDORES_ESCOPO — Provider setup | -provedoresEscopo | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MURAL_TITULOS — Titles in Mural | muralTitulosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AMIGOS_ESTILO — Friends row style | friendsRowStyleLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_IDIOMA_AUDIO — Prefer audio language | fonteIdiomaAudioLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AUD_LINGUA2 — Second audio language | audioSecundarioIdioma | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BAIXAR_REGISTRO — Download log | -baixarRegistro | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_FILTROS_FONTES — Filters & Sorting | -filtrosFontes | Not applicable: action/readout (not executed by boundary loop) | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_FILTRO_TAM_MAX — Maximum source size | -filtroTamanhoMax | PASS: every legal choice and numeric boundary | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_FILTRO_GRUPOS — Excluded release groups | -filtroGrupos | Not applicable: action/readout (not executed by boundary loop) | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_LEG1_TAMANHO — Size | -legenda1Tamanho | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_FONTE — Subtitle font | -legenda1Fonte | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_COR — Color | -legenda1Cor | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_OPACIDADE — Opacity | -legenda1Opacidade | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_FUNDO — Background | -legenda1Fundo | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_POSICAO — Position | -legenda1Posicao | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_BORDA — Edge | -legenda1Borda | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_ATRASO — Delay | -legenda1Atraso | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_NEGRITO — Bold | -legenda1Negrito | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_COR_BORDA — Outline color | -legenda1CorBorda | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG1_RESTAURAR — Restore default | -legenda1Restaurar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_LEG2_FONTE — Subtitle font | legenda2FonteLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_OPACIDADE — Opacity | legenda2OpacidadeLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_POSICAO — Position | legenda2PosicaoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_ATRASO — Delay | legenda2AtrasoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_NEGRITO — Bold | legenda2NegritoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_COR_BORDA — Outline color | legenda2CorBordaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LEG2_RESTAURAR — Restore default | -legenda2Restaurar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_COLECAO_EXIBICAO — Default collection view | colecaoExibicaoPadraoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PLAYER_EXTERNO — Player | playerExternoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TRAILER_AUDIO — Trailer audio | trailerAudioLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PLAYER_ANIME — Anime player | playerExternoAnimeLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ABERTURA_ESTILO — Opening screen style | aberturaEstiloLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MENU_CARTAZ — Enriched context menu | menuCartazLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TITULO_FIL — Row titles | rowTitleStyleLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_VIDRO_QUALIDADE — Glass quality | vidroQualidadeLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FUNDO_DIN — Dynamic layout background | fundoDinamicaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FUNDO_DIN_COR — Background color | fundoDinamicaCorLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_ESCALA_TEXTO — Text and component scale | escalaTextoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_GRUPO — Prefer binge group | fonteGrupoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FONTE_GRUPO_SESSOES — Reuse binge group across sessions | fonteGrupoSessoesLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LINK_REUSAR — Reuse last link | linkReusarLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LINK_PRAZO — Last link cache duration | linkPrazoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DET_ESPACO — Row spacing | detEspacoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FOCO_ACENTO — Accent-colour focus | focoAcentoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_NT_ORDEM — Title line rating order | notaTituloOrdem | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_ARTE_TOM — Art tone | artToneLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PALCO_FUNDO — Stage background | palcoFundoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_URL_LISTA — Lists and guides by URL | -urlLista | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_URL_M3U_ADD — Add M3U list | -urlM3uAdd | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_URL_XMLTV_ADD — Add XMLTV guide | -urlXmltvAdd | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_URL_REM — Remove the chosen one | -urlRem | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_NT_EP — Episode rating | notaEpisodio | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LIVETV_SUGERIR — Suggest channel categories | liveTvSugerirLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AUDIO_OFFSET — Audio offset (ms) | audioOffsetMsLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_XTREAM_CONEXOES — Limited connections | -xtreamConexoes | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AMPLIAR_UI — Scale interface | ampliarUiLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_GUIA_FONTE — TV guide sources | guiaFonteLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_GUIA_RECARREGAR — Refresh channels and guide | -guiaRecarregar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_M3U_URL — M3U playlist URL | -m3uUrl | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_M3U_EPG — XMLTV guide URL | -m3uEpg | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_M3U_LIMPAR — Remove M3U playlist | -m3uLimpar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_IPTV_CONTA — Live TV account | -iptvConta | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_ADD_XTREAM — Add Xtream account | -iptvAddXtream | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_IPTV_ADD_M3U — Add M3U list | -iptvAddM3u | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_IPTV_ADD_STALKER — Add Stalker portal | -iptvAddStalker | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_IPTV_NOME — Account name | -iptvNome | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_IPTV_ATIVA — Account enabled | -iptvAtiva | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_REMOVER — Remove Live TV account | -iptvRemover | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_IPTV_LISTA_XTREAM — Xtream | -iptvListaXtream | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_LISTA_M3U — M3U | -iptvListaM3u | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_LISTA_STALKER — Stalker | -iptvListaStalker | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_ACOES —  | -iptvAcoes | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_EXCLUIR — Remove Live TV account | -iptvExcluir | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_SALVAR — Save | -iptvSalvar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_IPTV_CANCELAR — Back | -iptvCancelar | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_GUIA_ORDEM — Sort order | guiaOrdemLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_IPTV_ORDEM — Order | -iptvOrdem | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_EPG_MEMORIA — Guide memory | epgMemoriaLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_GUIA_CATEGORIAS — Hide categories | -guiaCategorias | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_BIB_HOME — Library row | libraryRowLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AMPLIACAO_ALGO — Upscaling algorithm | ampliarAlgoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_AMPLIACAO_NITIDEZ — Sharpness | ampliarNitidezLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HDR_INTERFACE — HDR UI (test) | hdrInterfaceLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LIMPAR_CACHE — Clear cache | -limparCache | Not applicable: action/readout (not executed by boundary loop) | NOT individually functionally certified by this run |
| AJ_TUNNELING — Tunneled Playback | tunneledPlaybackLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_DV7 — Dolby Vision P7 to P8.1 (experimental) | dv7ConversionLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_LOG_REDE — Log on the local network | logRedeLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PLR_FORMATO — Format in the player | plrFormatoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PERFIL_INFANTIL — Kids profile | -perfilInfantil | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PAUSA_COMENTARIOS — Trakt comments when paused | pausaComentariosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PAUSA_NOTAS — Ratings when paused | pausaNotasLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PAUSA_ELENCO — Cast when paused | pausaElencoLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PLR_FORMATO_POS — Format position in the player | plrFormatoPosLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_PLR_CODEC — Show the audio codec | plrCodecLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_SUAVIZAR_DEGRADES — Smooth gradients | suavizarDegradesLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_TAMANHO — Hero size | heroSizeLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_HERO_PASTA_DETALHES — Show folder details on hero | heroFolderDetailsLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_CW_DIAS — Days in Trakt Continue Watching (0 = all) | traktContinueWatchingDaysCapLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RESOLVE_LINKS — Resolve playable links | resolvePlayableLinksLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RESOLVE_PROVIDER — Resolve with | preferredResolverProviderLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RESOLVE_PREPARE — Prepare links | -preparePlayableLinks | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_RESOLVE_LIMIT — Links to prepare | playableLinkPreparationLimitLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_TB_CACHED_ADD — Allow cached TorBox additions | torboxCachedAddLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FAMILIA — Family Watching | familyWatchingLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_FAMILIA_PARTILHAR — Share my watching activity | familyShareActivityLocal | PASS: every legal choice and numeric boundary | NOT individually functionally certified by this run |
| AJ_MENU_OCULTAR_PILULA — Hide modern sidebar pill | hideModernSidebarPillLocal | PASS: every legal choice and numeric boundary | Persistence, Dynamic pill alpha, opening navigation and existing menu geometry. Hidden Modern rail uses the existing pointer-edge path; real remote/pointer validation pending. |
| AJ_DET_TRAILER_ESPERA — Detail trailer delay | detailTrailerDelayTenthsLocal | PASS: every legal choice and numeric boundary | Effective milliseconds, every legal new detail-delay value and disk reload; production Home/detail timing consumers traced. Live trailer timing on TV pending. |
| AJ_POSPLAY_RECOM — Post-play recommendations | postPlayRecommendationsLocal | PASS: every legal choice and numeric boundary | Actual recommendation policy at all 100 percentages; actual next-episode timer at all 20 prompt intervals, OK/Off/autoplay-Off and disk reload. Real playback/UI pending. |
| AJ_POSPLAY_PERCENT — Show recommendations at | postPlayThresholdPercentLocal | PASS: every legal choice and numeric boundary | Actual recommendation policy at all 100 percentages; actual next-episode timer at all 20 prompt intervals, OK/Off/autoplay-Off and disk reload. Real playback/UI pending. |
| AJ_AINDA_EPISODIOS — Are you still watching? | stillWatchingEpisodeCountLocal | PASS: every legal choice and numeric boundary | Actual recommendation policy at all 100 percentages; actual next-episode timer at all 20 prompt intervals, OK/Off/autoplay-Off and disk reload. Real playback/UI pending. |
| AJ_LEG_AUTO_COMPLETA — Automatically load full subtitles | automaticFullSubtitlesLocal | PASS: every legal choice and numeric boundary | Actual automatic selector with mocked embedded/addon/late tracks; saved language and per-title None honoured. Live provider/player device validation pending. |
| AJ_FILTRO_MIN_GB — Minimum source size | -filterMinGB | PASS: every legal choice and numeric boundary | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_FILTRO_MAX_RESULT — Maximum results | -filterMaxResults | PASS: every legal choice and numeric boundary | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_FILTRO_POR_RES — Per-resolution limit | -filterResolutionLimit | PASS: every legal choice and numeric boundary | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_FILTRO_POR_QUAL — Per-quality limit | -filterQualityLimit | PASS: every legal choice and numeric boundary | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_FILTRO_SORT — Sort results | -filterSort | PASS: every legal choice and numeric boundary | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_FILTRO_REQUIRED_GROUPS — Required release groups | -filterRequiredGroups | Not applicable: action/readout (not executed by boundary loop) | Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending. |
| AJ_POSPLAY_TRAILER — Post-play trailer preview | postPlayTrailerLocal | PASS: every legal choice and numeric boundary | All 301 delay values in mock preview lifecycle; Android software-decoder/URL/low-memory policies and both native ABIs compile. Texture composition, decoding and CPU load NOT device-validated. Unsupported on LG/Tizen/desktop. |
| AJ_POSPLAY_TRAILER_ESPERA — Post-play trailer delay | postPlayTrailerDelayTenthsLocal | PASS: every legal choice and numeric boundary | All 301 delay values in mock preview lifecycle; Android software-decoder/URL/low-memory policies and both native ABIs compile. Texture composition, decoding and CPU load NOT device-validated. Unsupported on LG/Tizen/desktop. |

## Filter subchoices

Each listed option was exercised through Preferred, Required and Excluded UI callbacks and disk reloads, as well as the production filter policy. These are additional sheet controls, not replacement binding IDs.

| Dimension | Option | Mode | Result |
|---|---|---|---|
| 0 | 2160p | Preferred | PASS (local mocks) |
| 0 | 2160p | Required | PASS (local mocks) |
| 0 | 2160p | Excluded | PASS (local mocks) |
| 0 | 1440p | Preferred | PASS (local mocks) |
| 0 | 1440p | Required | PASS (local mocks) |
| 0 | 1440p | Excluded | PASS (local mocks) |
| 0 | 1080p | Preferred | PASS (local mocks) |
| 0 | 1080p | Required | PASS (local mocks) |
| 0 | 1080p | Excluded | PASS (local mocks) |
| 0 | 720p | Preferred | PASS (local mocks) |
| 0 | 720p | Required | PASS (local mocks) |
| 0 | 720p | Excluded | PASS (local mocks) |
| 0 | 576p | Preferred | PASS (local mocks) |
| 0 | 576p | Required | PASS (local mocks) |
| 0 | 576p | Excluded | PASS (local mocks) |
| 0 | 480p | Preferred | PASS (local mocks) |
| 0 | 480p | Required | PASS (local mocks) |
| 0 | 480p | Excluded | PASS (local mocks) |
| 0 | 360p | Preferred | PASS (local mocks) |
| 0 | 360p | Required | PASS (local mocks) |
| 0 | 360p | Excluded | PASS (local mocks) |
| 0 | Unknown | Preferred | PASS (local mocks) |
| 0 | Unknown | Required | PASS (local mocks) |
| 0 | Unknown | Excluded | PASS (local mocks) |
| 1 | BluRay REMUX | Preferred | PASS (local mocks) |
| 1 | BluRay REMUX | Required | PASS (local mocks) |
| 1 | BluRay REMUX | Excluded | PASS (local mocks) |
| 1 | BluRay | Preferred | PASS (local mocks) |
| 1 | BluRay | Required | PASS (local mocks) |
| 1 | BluRay | Excluded | PASS (local mocks) |
| 1 | WEB-DL | Preferred | PASS (local mocks) |
| 1 | WEB-DL | Required | PASS (local mocks) |
| 1 | WEB-DL | Excluded | PASS (local mocks) |
| 1 | WEBRip | Preferred | PASS (local mocks) |
| 1 | WEBRip | Required | PASS (local mocks) |
| 1 | WEBRip | Excluded | PASS (local mocks) |
| 1 | HDRip | Preferred | PASS (local mocks) |
| 1 | HDRip | Required | PASS (local mocks) |
| 1 | HDRip | Excluded | PASS (local mocks) |
| 1 | HC HD-Rip | Preferred | PASS (local mocks) |
| 1 | HC HD-Rip | Required | PASS (local mocks) |
| 1 | HC HD-Rip | Excluded | PASS (local mocks) |
| 1 | DVDRip | Preferred | PASS (local mocks) |
| 1 | DVDRip | Required | PASS (local mocks) |
| 1 | DVDRip | Excluded | PASS (local mocks) |
| 1 | HDTV | Preferred | PASS (local mocks) |
| 1 | HDTV | Required | PASS (local mocks) |
| 1 | HDTV | Excluded | PASS (local mocks) |
| 1 | CAM | Preferred | PASS (local mocks) |
| 1 | CAM | Required | PASS (local mocks) |
| 1 | CAM | Excluded | PASS (local mocks) |
| 1 | TS | Preferred | PASS (local mocks) |
| 1 | TS | Required | PASS (local mocks) |
| 1 | TS | Excluded | PASS (local mocks) |
| 1 | TC | Preferred | PASS (local mocks) |
| 1 | TC | Required | PASS (local mocks) |
| 1 | TC | Excluded | PASS (local mocks) |
| 1 | SCR | Preferred | PASS (local mocks) |
| 1 | SCR | Required | PASS (local mocks) |
| 1 | SCR | Excluded | PASS (local mocks) |
| 1 | Unknown | Preferred | PASS (local mocks) |
| 1 | Unknown | Required | PASS (local mocks) |
| 1 | Unknown | Excluded | PASS (local mocks) |
| 2 | HDR+DV | Preferred | PASS (local mocks) |
| 2 | HDR+DV | Required | PASS (local mocks) |
| 2 | HDR+DV | Excluded | PASS (local mocks) |
| 2 | Somente DV | Preferred | PASS (local mocks) |
| 2 | Somente DV | Required | PASS (local mocks) |
| 2 | Somente DV | Excluded | PASS (local mocks) |
| 2 | Somente HDR | Preferred | PASS (local mocks) |
| 2 | Somente HDR | Required | PASS (local mocks) |
| 2 | Somente HDR | Excluded | PASS (local mocks) |
| 2 | HDR10+ | Preferred | PASS (local mocks) |
| 2 | HDR10+ | Required | PASS (local mocks) |
| 2 | HDR10+ | Excluded | PASS (local mocks) |
| 2 | HDR10 | Preferred | PASS (local mocks) |
| 2 | HDR10 | Required | PASS (local mocks) |
| 2 | HDR10 | Excluded | PASS (local mocks) |
| 2 | DV | Preferred | PASS (local mocks) |
| 2 | DV | Required | PASS (local mocks) |
| 2 | DV | Excluded | PASS (local mocks) |
| 2 | HDR | Preferred | PASS (local mocks) |
| 2 | HDR | Required | PASS (local mocks) |
| 2 | HDR | Excluded | PASS (local mocks) |
| 2 | HLG | Preferred | PASS (local mocks) |
| 2 | HLG | Required | PASS (local mocks) |
| 2 | HLG | Excluded | PASS (local mocks) |
| 2 | 10bit | Preferred | PASS (local mocks) |
| 2 | 10bit | Required | PASS (local mocks) |
| 2 | 10bit | Excluded | PASS (local mocks) |
| 2 | 3D | Preferred | PASS (local mocks) |
| 2 | 3D | Required | PASS (local mocks) |
| 2 | 3D | Excluded | PASS (local mocks) |
| 2 | IMAX | Preferred | PASS (local mocks) |
| 2 | IMAX | Required | PASS (local mocks) |
| 2 | IMAX | Excluded | PASS (local mocks) |
| 2 | AI | Preferred | PASS (local mocks) |
| 2 | AI | Required | PASS (local mocks) |
| 2 | AI | Excluded | PASS (local mocks) |
| 2 | SDR | Preferred | PASS (local mocks) |
| 2 | SDR | Required | PASS (local mocks) |
| 2 | SDR | Excluded | PASS (local mocks) |
| 2 | H-OU | Preferred | PASS (local mocks) |
| 2 | H-OU | Required | PASS (local mocks) |
| 2 | H-OU | Excluded | PASS (local mocks) |
| 2 | H-SBS | Preferred | PASS (local mocks) |
| 2 | H-SBS | Required | PASS (local mocks) |
| 2 | H-SBS | Excluded | PASS (local mocks) |
| 2 | Unknown | Preferred | PASS (local mocks) |
| 2 | Unknown | Required | PASS (local mocks) |
| 2 | Unknown | Excluded | PASS (local mocks) |
| 3 | Atmos | Preferred | PASS (local mocks) |
| 3 | Atmos | Required | PASS (local mocks) |
| 3 | Atmos | Excluded | PASS (local mocks) |
| 3 | DD+ | Preferred | PASS (local mocks) |
| 3 | DD+ | Required | PASS (local mocks) |
| 3 | DD+ | Excluded | PASS (local mocks) |
| 3 | DD | Preferred | PASS (local mocks) |
| 3 | DD | Required | PASS (local mocks) |
| 3 | DD | Excluded | PASS (local mocks) |
| 3 | DTS:X | Preferred | PASS (local mocks) |
| 3 | DTS:X | Required | PASS (local mocks) |
| 3 | DTS:X | Excluded | PASS (local mocks) |
| 3 | DTS-HD MA | Preferred | PASS (local mocks) |
| 3 | DTS-HD MA | Required | PASS (local mocks) |
| 3 | DTS-HD MA | Excluded | PASS (local mocks) |
| 3 | DTS-HD | Preferred | PASS (local mocks) |
| 3 | DTS-HD | Required | PASS (local mocks) |
| 3 | DTS-HD | Excluded | PASS (local mocks) |
| 3 | DTS-ES | Preferred | PASS (local mocks) |
| 3 | DTS-ES | Required | PASS (local mocks) |
| 3 | DTS-ES | Excluded | PASS (local mocks) |
| 3 | DTS | Preferred | PASS (local mocks) |
| 3 | DTS | Required | PASS (local mocks) |
| 3 | DTS | Excluded | PASS (local mocks) |
| 3 | TrueHD | Preferred | PASS (local mocks) |
| 3 | TrueHD | Required | PASS (local mocks) |
| 3 | TrueHD | Excluded | PASS (local mocks) |
| 3 | OPUS | Preferred | PASS (local mocks) |
| 3 | OPUS | Required | PASS (local mocks) |
| 3 | OPUS | Excluded | PASS (local mocks) |
| 3 | FLAC | Preferred | PASS (local mocks) |
| 3 | FLAC | Required | PASS (local mocks) |
| 3 | FLAC | Excluded | PASS (local mocks) |
| 3 | AAC | Preferred | PASS (local mocks) |
| 3 | AAC | Required | PASS (local mocks) |
| 3 | AAC | Excluded | PASS (local mocks) |
| 3 | Unknown | Preferred | PASS (local mocks) |
| 3 | Unknown | Required | PASS (local mocks) |
| 3 | Unknown | Excluded | PASS (local mocks) |
| 4 | 7.1 | Preferred | PASS (local mocks) |
| 4 | 7.1 | Required | PASS (local mocks) |
| 4 | 7.1 | Excluded | PASS (local mocks) |
| 4 | 6.1 | Preferred | PASS (local mocks) |
| 4 | 6.1 | Required | PASS (local mocks) |
| 4 | 6.1 | Excluded | PASS (local mocks) |
| 4 | 5.1 | Preferred | PASS (local mocks) |
| 4 | 5.1 | Required | PASS (local mocks) |
| 4 | 5.1 | Excluded | PASS (local mocks) |
| 4 | 2.0 | Preferred | PASS (local mocks) |
| 4 | 2.0 | Required | PASS (local mocks) |
| 4 | 2.0 | Excluded | PASS (local mocks) |
| 4 | Unknown | Preferred | PASS (local mocks) |
| 4 | Unknown | Required | PASS (local mocks) |
| 4 | Unknown | Excluded | PASS (local mocks) |
| 5 | AV1 | Preferred | PASS (local mocks) |
| 5 | AV1 | Required | PASS (local mocks) |
| 5 | AV1 | Excluded | PASS (local mocks) |
| 5 | HEVC | Preferred | PASS (local mocks) |
| 5 | HEVC | Required | PASS (local mocks) |
| 5 | HEVC | Excluded | PASS (local mocks) |
| 5 | AVC | Preferred | PASS (local mocks) |
| 5 | AVC | Required | PASS (local mocks) |
| 5 | AVC | Excluded | PASS (local mocks) |
| 5 | XviD | Preferred | PASS (local mocks) |
| 5 | XviD | Required | PASS (local mocks) |
| 5 | XviD | Excluded | PASS (local mocks) |
| 5 | DivX | Preferred | PASS (local mocks) |
| 5 | DivX | Required | PASS (local mocks) |
| 5 | DivX | Excluded | PASS (local mocks) |
| 5 | Unknown | Preferred | PASS (local mocks) |
| 5 | Unknown | Required | PASS (local mocks) |
| 5 | Unknown | Excluded | PASS (local mocks) |
| 6 | Inglês | Preferred | PASS (local mocks) |
| 6 | Inglês | Required | PASS (local mocks) |
| 6 | Inglês | Excluded | PASS (local mocks) |
| 6 | Hindi | Preferred | PASS (local mocks) |
| 6 | Hindi | Required | PASS (local mocks) |
| 6 | Hindi | Excluded | PASS (local mocks) |
| 6 | Italiano | Preferred | PASS (local mocks) |
| 6 | Italiano | Required | PASS (local mocks) |
| 6 | Italiano | Excluded | PASS (local mocks) |
| 6 | Espanhol | Preferred | PASS (local mocks) |
| 6 | Espanhol | Required | PASS (local mocks) |
| 6 | Espanhol | Excluded | PASS (local mocks) |
| 6 | Francês | Preferred | PASS (local mocks) |
| 6 | Francês | Required | PASS (local mocks) |
| 6 | Francês | Excluded | PASS (local mocks) |
| 6 | Alemão | Preferred | PASS (local mocks) |
| 6 | Alemão | Required | PASS (local mocks) |
| 6 | Alemão | Excluded | PASS (local mocks) |
| 6 | Português | Preferred | PASS (local mocks) |
| 6 | Português | Required | PASS (local mocks) |
| 6 | Português | Excluded | PASS (local mocks) |
| 6 | Polonês | Preferred | PASS (local mocks) |
| 6 | Polonês | Required | PASS (local mocks) |
| 6 | Polonês | Excluded | PASS (local mocks) |
| 6 | Tcheco | Preferred | PASS (local mocks) |
| 6 | Tcheco | Required | PASS (local mocks) |
| 6 | Tcheco | Excluded | PASS (local mocks) |
| 6 | Latino | Preferred | PASS (local mocks) |
| 6 | Latino | Required | PASS (local mocks) |
| 6 | Latino | Excluded | PASS (local mocks) |
| 6 | Japonês | Preferred | PASS (local mocks) |
| 6 | Japonês | Required | PASS (local mocks) |
| 6 | Japonês | Excluded | PASS (local mocks) |
| 6 | Coreano | Preferred | PASS (local mocks) |
| 6 | Coreano | Required | PASS (local mocks) |
| 6 | Coreano | Excluded | PASS (local mocks) |
| 6 | Chinês | Preferred | PASS (local mocks) |
| 6 | Chinês | Required | PASS (local mocks) |
| 6 | Chinês | Excluded | PASS (local mocks) |
| 6 | Multi | Preferred | PASS (local mocks) |
| 6 | Multi | Required | PASS (local mocks) |
| 6 | Multi | Excluded | PASS (local mocks) |
| 6 | Unknown | Preferred | PASS (local mocks) |
| 6 | Unknown | Required | PASS (local mocks) |
| 6 | Unknown | Excluded | PASS (local mocks) |

# Stage B — Legacy provenance and integration

This inventory compares all **413 Android Native bindings** with all **354 original
Android TV control records** in the source audit at official commit
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`. The machine-readable
[complete comparison](stage-b-inventory.json) contains IDs, keys, native scalar
defaults, English choices, scope, category/gate, official counterparts and Native
code-consumer references. It never reads saved user preferences or credentials.

**125 controls are positively classified as additional and eligible for a Legacy
badge, subject to their existing compile/runtime visibility rules.** This is a
reviewed allowlist, not the audit's unmatched list. A badge does not imply a feature
has been newly implemented or certified on hardware. Shared parents remain
unbadged. Internal controls and unsupported experimental options remain hidden.

No preference IDs, keys, defaults, save/load functions, visibility gates, callback
implementations or backend consumers changed. No stored preferences were merged,
renamed or migrated. Each existing catalogue ID appears once. Existing UI controls
already serve shared original/Native features; a second original-branded copy is
not created. Similar names alone are not grounds to remove a control.

## Integration changes

- Cross-provider retry enable/order moved from Content & Discovery to Integrations,
  beside debrid services. Addon retry controls remain in Content & Discovery.
- Discord moved from Tracking to Integrations, as a connected presence service.
- IPTV/EPG controls remain under Content & Discovery; dual subtitle and player
  extras remain under Playback; glass/scale controls remain under Appearance;
  device render budgets remain under Advanced. No Legacy-only category exists.
- Badges use one neutral grey renderer in setting rows, the changed-settings list,
  the choice editor and both ordinary/best-result settings search rows. Width is
  reserved before title clipping; values retain their own column. The badge has
  no focus, pointer, key, save or service callback. `Legacy` is also a searchable
  provenance keyword. Ordinary labels and choices keep the Stage A English system.

## Equivalence corrections and deliberate non-consolidation

| Controls | Verified distinction / handling |
| --- | --- |
| Secondary audio vs secondary subtitles | Official audio uses `secondaryPreferredAudioLanguage`; the audit's subtitle-ID match is wrong. `AJ_AUD_LINGUA2` remains an unbadged audio fallback. No audio/subtitle preferences are combined. |
| Primary vs second subtitle styles | Official secondary language supplies ordered track-selection targets; Native has a simultaneous second renderer and separate style/offset. Only the extra style/offset/reset controls are badged. The ambiguous secondary-language control remains unbadged. |
| Hero vs poster ratings | `AJ_HERO_NOTAS` controls hero IMDb visibility; `AJ_NOTAS_HOME` controls poster/CW ratings. Both remain unbadged, independently saved and consumed. |
| Buffer preset vs off-heap mode/sliders | Native memory preset is not proof of official ExoPlayer native-memory/time-slider equivalence. Preserve its existing preset and defaults; no cosmetic additional toggles. |
| ASS renderer vs tunneling | The audit's ASS-render match to `AJ_TUNNELING` is wrong. Tunneling remains the existing unbadged playback feature; this does not implement an ASS renderer setting. |
| Metadata date source vs full-date format | `AJ_TMDB_DATAS` and `AJ_DET_DATA_CHEIA` are different keys/effects. Both are preserved and unbadged; no date-policy change is made. |
| Auto stream selection | Native `AJ_FONTE_MANUAL` uses 0=On for manual choice, the inverse wording of original autoplay. Preserve the saved representation and working getter rather than migrating it. |
| Primary subtitle opacity/font/delay | Related styling does not establish exclusive origin; preserve without a Legacy claim while the precise renderer/choice comparison is unresolved. |
| Secondary subtitle anchor vs offset | `AJ_LEG2_POS` selects the relative anchor; `AJ_LEG2_POSICAO` is an explicit vertical override. The existing reset/precedence behaviour is preserved; neither is deleted as a label duplicate. |
| Retained player vs cached link | Native retained pipeline/frame/lifetime controls are additional. The shared last-link reuse control remains unbadged. |
| Depth and poster provider settings | Both apps expose these features. Unmatched labels and hidden compatibility bindings are not automatically Legacy features. |

## Verified additional-control families

Every eligibility row below has the family's source evidence and classification
reason in the JSON comparison. Official links point to the pinned source commit;
Native links point to the working implementation. Evidence fingerprints identify
the reviewed files; line snippets locate per-setting readers/callbacks. Dynamic
account/forms use the existing shared handlers, including ordinal IPTV mappings.

### live-tv

Native adds its own Xtream, Stalker, M3U/XMLTV accounts and EPG/player controls. The official discovery hub exposes addons, media servers and plugins, not this IPTV account/guide stack. Addon live streams are not equivalent IPTV configuration controls.

Official: [ContentDiscoverySettingsContent.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/ContentDiscoverySettingsContent.kt), [SettingsCatalog.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/SettingsCatalog.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [iptvfontes.c](../../src/iptvfontes.c), [xtream.c](../../src/xtream.c), [stalker.c](../../src/stalker.c), [m3u.c](../../src/m3u.c), [guia.c](../../src/guia.c), [livetvdiag.c](../../src/livetvdiag.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_STALKER_PORTAL` | Stalker portal (MAC) | Content & Discovery |
| `AJ_STALKER_MAC` | Portal MAC | Content & Discovery |
| `AJ_XTREAM_SERVIDOR` | Xtream server | Content & Discovery |
| `AJ_XTREAM_USUARIO` | Xtream username | Content & Discovery |
| `AJ_XTREAM_SENHA` | Xtream password | Content & Discovery |
| `AJ_XTREAM_CONTA` | Xtream account | Content & Discovery |
| `AJ_EPG_PAIS` | TV guide source | Content & Discovery |
| `AJ_LIVETV_RES` | Main resolution | Content & Discovery |
| `AJ_LIVETV_FORMATO` | Xtream format | Content & Discovery |
| `AJ_LIVETV_ESPERA` | Wait for a channel to open | Content & Discovery |
| `AJ_LIVETV_DIAG` | Live TV diagnostics | Content & Discovery |
| `AJ_LIVETV_MODO` | Live TV player mode | Content & Discovery |
| `AJ_LIVETV_PROXY` | Live TV TS proxy | Content & Discovery |
| `AJ_URL_LISTA` | Lists and guides by URL | Content & Discovery |
| `AJ_URL_M3U_ADD` | Add M3U list | Content & Discovery |
| `AJ_URL_XMLTV_ADD` | Add XMLTV guide | Content & Discovery |
| `AJ_URL_REM` | Remove the chosen one | Content & Discovery |
| `AJ_LIVETV_SUGERIR` | Suggest channel categories | Content & Discovery |
| `AJ_XTREAM_CONEXOES` | Limited connections | Content & Discovery |
| `AJ_GUIA_FONTE` | TV guide sources | Content & Discovery |
| `AJ_GUIA_RECARREGAR` | Refresh channels and guide | Content & Discovery |
| `AJ_M3U_URL` | M3U playlist URL | Content & Discovery |
| `AJ_M3U_EPG` | XMLTV guide URL | Content & Discovery |
| `AJ_IPTV_CONTA` | Live TV account | Content & Discovery |
| `AJ_IPTV_ADD_XTREAM` | Add Xtream account | Content & Discovery |
| `AJ_IPTV_ADD_M3U` | Add M3U list | Content & Discovery |
| `AJ_IPTV_ADD_STALKER` | Add Stalker portal | Content & Discovery |
| `AJ_IPTV_NOME` | Account name | Content & Discovery |
| `AJ_IPTV_ATIVA` | Account enabled | Content & Discovery |
| `AJ_IPTV_REMOVER` | Remove Live TV account | Content & Discovery |
| `AJ_IPTV_LISTA_XTREAM` | Xtream | Content & Discovery |
| `AJ_IPTV_LISTA_M3U` | M3U | Content & Discovery |
| `AJ_IPTV_LISTA_STALKER` | Stalker | Content & Discovery |
| `AJ_IPTV_ACOES` | Live TV account | Content & Discovery |
| `AJ_IPTV_EXCLUIR` | Remove Live TV account | Content & Discovery |
| `AJ_GUIA_ORDEM` | Sort order | Content & Discovery |
| `AJ_IPTV_ORDEM` | Order | Content & Discovery |
| `AJ_EPG_MEMORIA` | Guide memory | Content & Discovery |
| `AJ_GUIA_CATEGORIAS` | Hide categories | Content & Discovery |

### glass

Native shader glass, blur/opacity/outline quality and accent-focus controls are additional to the official colour/theme and AMOLED controls; the shared colour-theme parent is not badged.

Official: [ThemeSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/ThemeSettingsScreen.kt), [CustomThemeEditor.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/CustomThemeEditor.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [gfx.c](../../src/gfx.c), [gpunivel.c](../../src/gpunivel.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_VIDRO` | Glass UI | Appearance |
| `AJ_VIDRO_CONTORNO` | Glass outline | Appearance |
| `AJ_VIDRO_OPAC` | Glass opacity | Appearance |
| `AJ_VIDRO_FOSCO` | Frosted glass | Appearance |
| `AJ_VIDRO_QUALIDADE` | Glass quality | Appearance |
| `AJ_FOCO_ACENTO` | Accent-colour focus | Appearance |
| `AJ_SUAVIZAR_DEGRADES` | Smooth gradients | Appearance |

### graphics-budget

Native controls the SDL render canvas, effect ladder, SGSR/FSR/Lanczos upscaling and image texture memory budget. Official player video resolution matching, RGB565 decoding and player buffer budgets have different consumers and are not equivalents.

Official: [NetworkSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/NetworkSettingsScreen.kt), [PlaybackVideoSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackVideoSettings.kt), [PlaybackBufferNetworkSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackBufferNetworkSettings.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [main.c](../../src/main.c), [gfx.c](../../src/gfx.c), [gpunivel.c](../../src/gpunivel.c), [tex_cache.c](../../src/tex_cache.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_RESOLUCAO` | Interface resolution | Advanced |
| `AJ_TEX_MB` | Memory for images | Advanced |
| `AJ_GPU_EFEITOS` | Visual effects | Advanced |
| `AJ_AMPLIAR_UI` | Scale interface | Advanced |
| `AJ_AMPLIACAO_ALGO` | Upscaling algorithm | Advanced |
| `AJ_AMPLIACAO_NITIDEZ` | Sharpness | Advanced |

### dual-subtitle-style

Native has independent styling, placement, timing and reset for a simultaneously rendered second subtitle. Official secondaryPreferredLanguage feeds ordered track selection with one shared subtitle style, not a second independently styled overlay. Primary subtitle styling remains unbadged.

Official: [PlaybackSubtitleSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackSubtitleSettings.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [legendasui.c](../../src/legendasui.c), [player.c](../../src/player.c), [legenda2.c](../../src/legenda2.c), [app.c](../../src/app.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_LEG2_POS` | Second subtitle position | Playback |
| `AJ_LEG2_TAMANHO` | Size | Playback |
| `AJ_LEG2_COR` | Color | Playback |
| `AJ_LEG2_FUNDO` | Background | Playback |
| `AJ_LEG2_BORDA` | Edge | Playback |
| `AJ_LEG2_FONTE` | Subtitle font | Playback |
| `AJ_LEG2_OPACIDADE` | Opacity | Playback |
| `AJ_LEG2_POSICAO` | Position | Playback |
| `AJ_LEG2_ATRASO` | Delay | Playback |
| `AJ_LEG2_NEGRITO` | Bold | Playback |
| `AJ_LEG2_COR_BORDA` | Outline color | Playback |
| `AJ_LEG2_RESTAURAR` | Restore default | Playback |

### alldebrid

Native adds AllDebrid credentials and explicit validation. Official connected-service credentials support Real-Debrid, TorBox and Premiumize; the shared providers are not badged.

Official: [DebridSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/DebridSettingsScreen.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [debrid.c](../../src/debrid.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_DEBRID_AD` | AllDebrid key | Integrations |
| `AJ_DEBRID_AD_TESTAR` | Test AllDebrid key | Integrations |

### provider-fallback

Native adds ordered cross-provider/addon failure retries and per-provider attempt limits through fontefalha. Official preferred resolver/provider and stream preferences do not expose this independent retry policy. No resolver behaviour is changed here.

Official: [DebridSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/DebridSettingsScreen.kt), [PlaybackAutoPlaySettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackAutoPlaySettings.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [ajustes_ux_interacao.inc](../../src/ajustes_ux_interacao.inc), [fontefalha.c](../../src/fontefalha.c), [streams.c](../../src/streams.c), [app.c](../../src/app.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_FALHA_PROVEDORES` | Switch provider on failure | Integrations |
| `AJ_FALHA_PROVEDOR_ORDEM` | Provider order and limits | Integrations |
| `AJ_FALHA_ADDONS` | Switch add-on on failure | Content & Discovery |
| `AJ_FALHA_ADDON_ORDEM` | Add-on order and limits | Content & Discovery |

### external-p2p

Native supports an external LAN P2P server URL and connection test in addition to any embedded engine. Official P2P settings configure its embedded torrent engine; the shared enable and cache controls are unbadged.

Official: [PlaybackP2pSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackP2pSettings.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [p2p.c](../../src/p2p.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_P2P_URL` | P2P server address | Playback |
| `AJ_P2P_TESTAR` | Test P2P server | Playback |

### seekr

Native adds Seekr service enable/key/test, seek-thumbnail filmstrip and synchronisation offset controls. Official playback settings have no Seekr service or matching thumbnail-configuration controls. This does not badge a generic shared seeking feature.

Official: [PlaybackSettingsSections.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackSettingsSections.kt), [PlaybackPlayerSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackPlayerSettings.kt), [PlaybackVideoSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackVideoSettings.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [seekr.c](../../src/seekr.c), [player.c](../../src/player.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_SEEKR_LIGADO` | Seek bar thumbnails | Playback |
| `AJ_SEEKR_CHAVE` | Seekr key | Playback |
| `AJ_SEEKR_TESTAR` | Test Seekr key | Playback |
| `AJ_SEEKR_FITA` | Thumbnail strip | Playback |
| `AJ_SEEKR_AJUSTE` | Thumbnail sync | Playback |

### retained-player

Native retains a player pipeline/frame after exit and controls its retention lifetime. Official stream-link reuse caches a link; it does not retain the Native player pipeline. Keep both behaviours and keys distinct.

Official: [PlaybackAutoPlaySettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackAutoPlaySettings.kt), [PlaybackPlayerSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackPlayerSettings.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [app.c](../../src/app.c), [player.c](../../src/player.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_MANTER_VIDEO` | Keep the video ready on exit | Playback |
| `AJ_RETOMADA_MORNA` | Quick resume on return | Playback |
| `AJ_MANTER_PRAZO` | Time to keep video ready | Playback |

### stats-layout

Native adds size and placement options to its live statistics panel. Official advanced playback statistics provides the main overlay control; that shared parent remains unbadged.

Official: [NetworkSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/NetworkSettingsScreen.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [estatvivoui.c](../../src/estatvivoui.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_ESTAT_TAMANHO` | Live statistics size | Playback |
| `AJ_ESTAT_LADO` | Live statistics side | Playback |

### native-layout

Native adds profile-picker backgrounds/resume cards, friends/Mural/collection/tab styles, row-title styles, top menu placement and specific guide/agenda/profile menu visibility, or Dynamic/Stage layout backgrounds. Official home layout, Discover/sidebar controls, profiles and colour-theme controls remain unbadged; these are distinct extra sub-controls.

Official: [LayoutHomeSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutHomeSettings.kt), [LayoutSettingsModel.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutSettingsModel.kt), [ProfileSettingsContent.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/ProfileSettingsContent.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [perfilsel.c](../../src/perfilsel.c), [home.c](../../src/home.c), [amigosfil_estilos.inc](../../src/amigosfil_estilos.inc), [vertudo.c](../../src/vertudo.c), [menu.c](../../src/menu.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_PS_FUNDO` | Profile picker background | Switch Profile |
| `AJ_MENU_GUIA` | TV Guide in the sidebar | Layout |
| `AJ_MENU_AGENDA` | Schedule in the sidebar | Layout |
| `AJ_MENU_PERFIL` | Profile & Stats in the sidebar | Layout |
| `AJ_PS_CONTINUAR` | Continue on profile picker | Switch Profile |
| `AJ_MENU_TOPO` | Menu position | Layout |
| `AJ_ABAS_ESTILO` | Tab style | Layout |
| `AJ_MURAL_TITULOS` | Titles in Mural | Layout |
| `AJ_AMIGOS_ESTILO` | Friends row style | Layout |
| `AJ_COLECAO_EXIBICAO` | Default collection view | Layout |
| `AJ_TITULO_FIL` | Row titles | Appearance |
| `AJ_FUNDO_DIN` | Dynamic layout background | Layout |
| `AJ_FUNDO_DIN_COR` | Background color | Layout |
| `AJ_PALCO_FUNDO` | Stage background | Appearance |
| `AJ_HERO_PASTA_DETALHES` | Show folder details on hero | Layout |

### feedback-polls

Native adds its credits-review prompt and in-app opinion polls. Official skip-credits, post-play recommendations, tracking comments and episode notifications serve different purposes and remain unbadged.

Official: [PlaybackAutoPlaySettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackAutoPlaySettings.kt), [TrackingSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/TrackingSettingsScreen.kt), [SettingsCatalog.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/SettingsCatalog.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [reacao.c](../../src/reacao.c), [enquete.c](../../src/enquete.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_REACAO_CREDITOS` | Ask what you thought at the credits | Playback |
| `AJ_ENQUETES` | Receive polls | Notifications |

### layout-detail-tuning

Native adds independent hero metadata visibility/size and row/title spacing controls. Official hero/catalogue/rating/layout controls have no matching per-field visibility and spacing preferences. Shared hero enable, layout and rating controls remain unbadged.

Official: [LayoutHomeSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutHomeSettings.kt), [LayoutDetailSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutDetailSettings.kt), [LayoutSettingsModel.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutSettingsModel.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [home.c](../../src/home.c), [detail.c](../../src/detail.c), [sinopse.c](../../src/sinopse.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_ESPACO_FILEIRAS` | Space between rows | Layout |
| `AJ_ESPACO_TITULOS` | Space between titles | Layout |
| `AJ_HERO_GENERO` | Show genre on hero | Layout |
| `AJ_HERO_SINOPSE` | Show synopsis on hero | Layout |
| `AJ_HERO_ANO` | Show year on hero | Layout |
| `AJ_HERO_DURACAO` | Show runtime on hero | Layout |
| `AJ_HERO_SINOPSE_TAM` | Synopsis size | Layout |
| `AJ_DET_ESPACO` | Row spacing | Appearance |
| `AJ_HERO_TAMANHO` | Hero size | Layout |

### ui-scale

Native adds independent canvas/text/settings scaling, animation reduction, in-app logo colour and clock-format controls. These are separate from official font family, launcher icon, theme colour and OSD-clock enable controls, which remain unbadged.

Official: [ThemeSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/ThemeSettingsScreen.kt), [LayoutSettingsModel.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutSettingsModel.kt), [PlaybackPlayerSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackPlayerSettings.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [gfx.c](../../src/gfx.c), [main.c](../../src/main.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_ANIM` | Animations | Appearance |
| `AJ_COR_LOGO` | Logo color | Appearance |
| `AJ_TAMANHO_UI` | Interface size | Appearance |
| `AJ_TAMANHO_AJUSTES` | Settings size | Appearance |
| `AJ_RELOGIO_12H` | Clock format | Appearance |
| `AJ_ESCALA_TEXTO` | Text and component scale | Appearance |

### extra-art-ratings

Native adds a fanart.tv API credential and visibility for Metacritic user score, Roger Ebert and MDBList aggregate score. Official TMDB/artwork/Trakt fanart URLs are not a fanart.tv API credential; its MDBList visibility model has no controls for these three extra scores. Shared rating sources remain unbadged.

Official: [MDBListSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/MDBListSettingsScreen.kt), [TmdbSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/TmdbSettingsScreen.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [notasui.c](../../src/notasui.c), [extras.c](../../src/extras.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_FANART_CHAVE` | fanart.tv key | Integrations |
| `AJ_NT_METAUSER` | Metacritic (users) | Integrations |
| `AJ_NT_EBERT` | Roger Ebert (critic) | Integrations |
| `AJ_NT_SCORE` | MDBList score | Integrations |

### source-verification

Native adds start-during-verification, connection warm-up and concurrent source verification. Official debrid link preparation and parallel HTTP playback connections use different pipelines and do not expose these controls. Preserve all policies; Stage B only classifies their UI.

Official: [PlaybackAutoPlaySettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackAutoPlaySettings.kt), [PlaybackBufferNetworkSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackBufferNetworkSettings.kt), [DebridSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/DebridSettingsScreen.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [app.c](../../src/app.c), [streams.c](../../src/streams.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_FONTE_TOCAR_CONFERINDO` | Play while checking the source | Playback |
| `AJ_FONTE_AQUECER` | Warm up connections when a title opens | Playback |
| `AJ_FONTE_CONFERIR_VARIAS` | Check several sources at once | Playback |

### extra-player

Native adds a separate anime player override, duplication of a retained-player island item in Continue Watching, and an HTTP local-network diagnostic-log server. Official default player, Continue Watching and Sentry reporting controls are not equivalents to these additional controls.

Official: [PlaybackPlayerSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/PlaybackPlayerSettings.kt), [LayoutCardSettings.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutCardSettings.kt), [NetworkSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/NetworkSettingsScreen.kt), [SentrySettingsDialog.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/SentrySettingsDialog.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [app.c](../../src/app.c), [player.c](../../src/player.c), [logrede.c](../../src/logrede.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_CW_RETIDO_TAMBEM` | Also in Continue watching | Tracking |
| `AJ_PLAYER_ANIME` | Anime player | Playback |
| `AJ_LOG_REDE` | Log on the local network | Advanced |

### discord-presence

Native Discord Rich Presence uses its own account/device link and publishes playback presence. Official tracking controls are Trakt/Simkl/library/progress, not a Discord presence control; community/support links do not implement it.

Official: [TrackingSettingsScreen.kt](https://github.com/NuvioMedia/NuvioTV/blob/99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f/app/src/main/java/com/nuvio/tv/ui/screens/settings/TrackingSettingsScreen.kt).

Native consumers/handlers: [ajustes.c](../../src/ajustes.c), [discord.c](../../src/discord.c).

| Binding | English setting | Category |
| --- | --- | --- |
| `AJ_DISCORD` | Discord | Integrations |

## Uncertain controls — retained, no badge

83 bindings remain deliberately unbadged because a reliable exclusive-control or
exact-equivalence conclusion is not established. This is a provenance limitation,
not removal or loss of functionality. They were compared with the full official
inventory, but further choice-by-choice/consumer comparison is required before a
badge or any consolidation is justified. Their defaults, choices, keys and Native
reader/callback evidence are retained in the complete comparison. In particular,
a related technology, partial audit match or a missing one-to-one audit match does
not establish exclusive origin. No unsupported feature is claimed complete.

| Binding | English setting | Category / availability |
| --- | --- | --- |
| `AJ_QUALIDADE` | Maximum quality | Playback |
| `AJ_ATMOS` | Dolby Atmos | Playback |
| `AJ_FONTE_AUTO` | Automatic source | Playback |
| `AJ_FONTE_TEXTO` | Source text | Playback |
| `AJ_HERO_FUNDO` | Hero background | Layout |
| `AJ_HERO_ARTE_DIF` | Hero with different art | Layout |
| `AJ_HERO_TRAILER` | Trailer on the hero | Layout |
| `AJ_FIL_LIMITE` | Row limit | Layout |
| `AJ_CW_OK` | OK on the card | Layout |
| `AJ_DET_VEU` | Backdrop dimming | Layout |
| `AJ_TRAILER_ASPECTO` | Trailer aspect | Layout |
| `AJ_TRAILER_FONTE` | Trailer source | Layout |
| `AJ_BORDA_FOCO` | Border on the focused card | Layout |
| `AJ_QUALIDADE_IMG` | Image quality | Advanced |
| `AJ_ENVIAR_LOG` | Send log | Advanced |
| `AJ_ESPACO` | Memory used by images | Advanced |
| `AJ_ITENS_FILEIRA` | Items per row | Layout |
| `AJ_POSTER_PROV` | Custom posters | Layout |
| `AJ_POSTER_TESTAR` | Test posters | Integrations |
| `AJ_DEBRID_RD_TESTAR` | Test Real-Debrid key | Integrations |
| `AJ_DEBRID_TB_TESTAR` | Test TorBox key | Integrations |
| `AJ_DEBRID_PM_TESTAR` | Test Premiumize key | Integrations |
| `AJ_DET_SO_CINEMETA` | Always use the Nuvio catalog | Layout |
| `AJ_ADDONS_PRINCIPAL` | Use the primary profile's add-ons | Content & Discovery |
| `AJ_HERO_TRANSICAO` | Hero transition | Layout |
| `AJ_ADDON_POSTER` | Addon posters | Layout |
| `AJ_ADDON_FUNDO` | Addon hero background | Layout |
| `AJ_ADDON_LOGO` | Addon logo | Layout |
| `AJ_COL_ARTE_CONTA` | Account folder artwork | Layout |
| `AJ_SELOS_CORES` | Colored badges | Playback |
| `AJ_SELO_VISTO` | Watched badge on posters | Layout |
| `AJ_SAIDA_PLAYER` | When leaving the player | Playback |
| `AJ_VER_REGISTRO` | View the log on screen | Advanced |
| `AJ_GUIA` | User guide | About |
| `AJ_LOGO_TRAILER` | Hide logo during trailer | Layout |
| `AJ_LEG_SYNC_AUDIO` | Sync by audio | Playback |
| `AJ_TRAILER_ZOOM_TPK` | Trailer and player zoom (experimental) | Layout |
| `AJ_PX_SERVIDOR` | Plex server | Content & Discovery |
| `AJ_FONTE_PRIORIDADE` | Automatic pick prioritizes | Playback |
| `AJ_FONTE_HDR` | HDR and Dolby Vision | Playback |
| `AJ_BUSCA_CINEMETA` | Search Cinemeta | Content & Discovery |
| `AJ_BRILHO_PLAYER` | Player interface brightness | Playback |
| `AJ_NOVIDADES20` | What's new in 2.0 | About |
| `AJ_DESCANSO_ESTILO` | Screensaver style | Appearance |
| `AJ_FONTE_OUTROS` | Use the others if there are none | Playback |
| `AJ_FONTE_REGEX` | Regex match | Playback |
| `AJ_FONTE_REGEX_MODELO` | Regex preset | Playback |
| `AJ_PROPORCAO_PADRAO` | Default aspect ratio | Playback |
| `AJ_DV_MKV` | Dolby Vision in MKV (experimental) | Playback |
| `AJ_HIST_CONTA` | Send history to the Nuvio account | Tracking |
| `AJ_SOCIAL` | Social features | Tracking |
| `AJ_FONTE_ORDEM` | Add-on order | Playback |
| `AJ_FONTE_ORDEM_USO` | Use the order | Playback |
| `AJ_BUSCA_NUVIO` | Nuvio catalog results in search | Content & Discovery |
| `AJ_BUSCA_ORIGEM` | Show the source on search results | Content & Discovery |
| `AJ_P2P_DESTINO` | P2P cache location | Playback |
| `AJ_PROXIMO_CONTAGEM` | Next episode countdown | Playback |
| `AJ_INTERFACE_MODO` | Interface mode | Appearance |
| `AJ_MIC_BUSCA` | Mic button in search | Layout |
| `AJ_TECLADO_ANDROID` | Keyboard | Appearance |
| `AJ_OCULTAR_FILMES_VISTOS` | Hide watched movies | Layout |
| `AJ_DICASCONTROLE` | Remote tips | About |
| `AJ_ABERTURA_ADDON` | Show add-on on opening | Playback |
| `AJ_CURADORIA_URL` | Curation address | Layout |
| `AJ_ABERTURA_FUNDO` | Opening screen backdrop | Playback |
| `AJ_EP_TAMANHO` | Episode card size | Layout |
| `AJ_PROVEDORES_ESCOPO` | Provider setup | Integrations |
| `AJ_FONTE_IDIOMA_AUDIO` | Prefer audio language | Playback |
| `AJ_BAIXAR_REGISTRO` | Download log | Advanced |
| `AJ_TRAILER_AUDIO` | Trailer audio | Layout |
| `AJ_ABERTURA_ESTILO` | Opening screen style | Playback |
| `AJ_NT_ORDEM` | Title line rating order | Integrations |
| `AJ_ARTE_TOM` | Art tone | Appearance |
| `AJ_AUDIO_OFFSET` | Audio offset (ms) | Playback |
| `AJ_IPTV_SALVAR` | Save | Content & Discovery |
| `AJ_IPTV_CANCELAR` | Cancel | Content & Discovery |
| `AJ_BIB_HOME` | Library row | Layout |
| `AJ_DV7` | Dolby Vision P7 to P8.1 (experimental) | Playback |
| `AJ_PERFIL_INFANTIL` | Kids profile | Switch Profile |
| `AJ_PAUSA_NOTAS` | Ratings when paused | Playback |
| `AJ_PAUSA_ELENCO` | Cast when paused | Playback |
| `AJ_PLR_FORMATO_POS` | Format position in the player | Playback |
| `AJ_PLR_CODEC` | Show the audio codec | Playback |

The remaining 33 internal/runtime-hidden bindings stay unbadged and retain all
existing gates. 153 bindings have original counterparts, nine have corrected
shared/related classifications, and ten have explicitly different related effects.
Those classifications do not assert every original option is implemented: missing
original controls remain recorded in the original audit. Resolver controls,
TorBox changes, metadata/date fixes and platform builds are deferred to Stages C–F.

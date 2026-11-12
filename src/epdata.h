// Stage E: episode release dates, countdown text, release alert and the
// Continue Watching progress label — a port of regular Nuvio (NuvioTV
// 99fbd65): core/util/EpisodeReleaseDateParser.kt, ui/util/AirDateUtils.kt,
// HomeViewModelContinueWatching.resolveNextUpReleaseState and
// ui/components/ContinueWatchingProgressLabel.kt. Same rules, no extras.
#ifndef NV_EPDATA_H
#define NV_EPDATA_H
#include <stddef.h>

typedef struct { int ano, mes, dia; } EpDia;   // calendar date (mes 1..12)

// parseEpisodeReleaseLocalDate: a plain date is kept as supplied; a zoned
// instant becomes the viewer's LOCAL date; a local date-time keeps its date;
// otherwise the first embedded YYYY-MM-DD. 0 when nothing parses.
int ep_dia_local(const char *raw, EpDia *out);

// parseEpisodeReleaseInstant: zoned -> exact instant; local date-time -> the
// viewer's zone; plain or embedded date -> UTC midnight. 0 when nothing parses.
int ep_instante_ms(const char *raw, long long *ms);

// isEpisodeReleaseAired: 1 aired, 0 not yet, -1 unknown (unparseable/empty).
int ep_ja_saiu(const char *raw, long long agoraMs);

// ChronoUnit.DAYS.between(today in the device zone, local release date).
// 0 when the release date does not parse.
int ep_dias_ate(const char *raw, long long agoraMs, long *dias);

// computeAirDateBadgeText (curto = 0) / computeAirDateBadgeTextShort (1):
// "Airs Today" / "Airs Tomorrow" / "Airs in N Days" (2..7) / "Airs <d MMM[ y]>"
// and the short "Today" / "Tomorrow" / "In N Days" / "<d MMM[ y]>". Returns 0
// (dst empty) when there is nothing to say: no date, already aired, past or
// unparseable — the card then falls back to "Upcoming".
int ep_rotulo_estreia(const char *raw, long long agoraMs, int curto, int idioma,
                      char *dst, size_t cap);

// resolveNextUpReleaseState.isReleaseAlert: aired, released after the seed was
// last watched, and less than 60 days ago.
int ep_alerta_lancamento(long long lancamentoMs, long long vistoMs, long long agoraMs);
// isNewSeasonRelease: an alert whose season differs from the seed's season
// (temporadaSemente -1 = unknown, as the official `seed.season == null`).
int ep_alerta_temporada(int alerta, int temporadaSemente, int temporadaProxima);

// Episode list artwork for an UNAIRED episode (owner request, 6 Nov 2026; not
// in regular Nuvio, whose list shows the still only): use the title's landscape
// backdrop when the episode has no still, or its "still" is portrait — the
// add-ons send the show's poster as the thumbnail before an episode airs.
// `aspecto` is the loaded still's width/height (0 = not loaded yet). A real
// landscape still is kept. 1 = draw the backdrop instead.
int ep_arte_usa_fundo(int inedito, const char *thumb, const char *poster,
                      const char *backdrop, float aspecto);

// Stage E artwork provider: the show's landscape art for a landscape card.
// `provedor` is the selected provider's card art and `aspecto` its loaded
// width/height (0 = not loaded); `metahub` the IMDb landscape backdrop;
// `catalogo` the catalog backdrop. A portrait provider image gives way to the
// metahub landscape. NULL when there is nothing.
const char *ep_arte_paisagem(const char *provedor, float aspecto, const char *metahub,
                             const char *catalogo);
// A portrait image (0 < aspect < 1) drawn in a landscape card is cropped to
// fill it (regular Nuvio: ContentScale.Crop), never pillarboxed.
int ep_arte_cobrir(float aspecto);

// formatContinueWatchingProgressLabel, from what Better's CatItem keeps:
// restanteMin > 0 (duration known) -> "Xh Ym left" / "Ym left" (at least 1);
// unknown duration -> "N% Watched" when N > 0, else "Resume".
void ep_rotulo_progresso(int restanteMin, int progresso, char *dst, size_t cap);

#endif

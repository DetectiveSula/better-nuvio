# Stage E — Episode air dates and artwork (investigation)

6 November 2026. Investigation only; no code changed. Stage J work preserved.
Reference: official NuvioTV `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`
(`~/Documents/Nuvio-Official`), current `codex/settings-restoration` tree.

## 1. Official implementation (what to reproduce)

### 1.1 Release-date parsing — `core/util/EpisodeReleaseDateParser.kt`

| Function | Rule |
|---|---|
| `parseEpisodeReleaseLocalDate(raw)` | Date-only (`2026-11-20`) kept **as supplied**. A zoned instant (`…Z`, `+01:00`) is converted to the **viewer's local** date. A local date-time without a zone is used as is. Otherwise the first embedded `YYYY-MM-DD`. |
| `isEpisodeReleaseAired(raw)` | Zoned timestamp → exact instant. **Date-only → UTC midnight.** Local date-time → viewer's zone. Aired when the instant ≤ now. `null` when unparseable. |
| `selectEpisodeReleaseValue` | Add-on `released` only. The TMDB `air_date` opt-in exists but every caller passes `useTmdbReleaseDates = false`, so **TMDB never replaces the add-on date**. |

### 1.2 Countdown text — `ui/util/AirDateUtils.kt`

`computeAirDateBadgeText` (long) and `computeAirDateBadgeTextShort` (short):

1. No `released` → `Airs <airDateLabel>` (short: `<airDateLabel>`), or nothing.
2. Already aired → nothing.
3. `daysUntil` = local release date − **today in the device zone**:
   - 0 → `Airs Today` / `Today`
   - 1 → `Airs Tomorrow` / `Tomorrow`
   - 2–7 → `Airs in N Days` / `In N Days` (plural "Day" for 1)
   - > 7 → `Airs <airDateLabel>` / `<airDateLabel>`
   - < 0 → nothing
4. If the result is null for an unaired next-up, the badge falls back to `Upcoming`.

`airDateLabel` (`formatEpisodeAirDateLabel`, `HomeViewModelContinueWatching.kt`):
locale best pattern for skeleton `dMMM` in the current year and `dMMMy` otherwise
(e.g. "20 Nov", "3 Jan 2027"). It's null once aired.

### 1.3 NEW EPISODE / NEW SEASON — `resolveNextUpReleaseState`

`isReleaseAlert = hasAired && releaseTimestamp != null && releaseTimestamp >
seed.lastWatched && (now − releaseTimestamp) < 60 days`.

- `isNewSeasonRelease = isReleaseAlert && seed.season != null && next.season != seed.season`.
- When it's a release alert, the sort timestamp is the **release time**; otherwise
  `lastWatched`.
- Badge text: `New Season`, else `New Episode` (short `New`). Colours: new episode
  `#1D4ED8`; new season `CwNewSeasonBadgeColor`; other badges background @ 80 %.
- Flags are **recomputed from the stored timestamp** on every cache read
  (`recalculateCachedReleaseBadge`), so a cached upcoming item flips to New
  Episode when it airs.

### 1.4 Continue Watching card — `ui/components/ContinueWatchingSection.kt`

- **Badge priority:** in-progress label (`Xh Ym left` / `Ym left` / `N% Watched` /
  `Resume`) → next-up badge (New Season / New Episode / air-date text / `Upcoming`
  / `Next Up`) → `Next Up`. Short labels (`Next`, `New`, `Today`, …) in **Wide
  and Poster** card styles.
- **Text:** title; `S1 E2` (`season_episode_format`); episode title. An unaired
  next-up with no episode title shows the long air-date text instead.
- **Artwork** (`continueWatchingImageModel`):
  - Card style, unaired next-up: custom landscape → backdrop → poster →
    thumbnail (only if thumbnails on). **The episode still is last for unaired
    episodes.**
  - Card style, aired next-up with thumbnails on: thumbnail → custom landscape →
    backdrop → poster.
  - Card style, in progress with thumbnails on: episode thumbnail → custom
    landscape → backdrop → poster. With thumbnails off, the thumbnail is skipped.
  - Poster and Wide styles: thumbnail (if on) → **poster** → backdrop. Thumbnails
    never apply to the Poster style.
  - On load failure: Poster/Wide → poster, backdrop; Card → backdrop, poster.
    Broken URLs are remembered and skipped. All artwork uses `ContentScale.Crop`.
- **Blur:** only when "blur unwatched" is on, thumbnails are on, the item isn't
  completed, and the resolved image *is* the episode thumbnail.
- **Unaired inclusion:** `showUnairedNextUp` setting; `Separate Upcoming Row` sort mode.

### 1.5 Episode list — `ui/screens/detail/EpisodesSection.kt`

- Card art: `episode.thumbnail` only, with `ContentScale.Crop`. Missing or failed →
  plain card background colour. **No poster or backdrop fallback, no countdown and
  no NEW badge.**
- Header `EPISODE n`; title (marquee on focus); overview.
- Meta row: runtime (`1h 5m` with clock icon) · IMDb rating · release date
  (`dMMMMy`, e.g. "20 November 2026", right-aligned).
- Badges: completed check; `UNAVAILABLE` (brown `#CC5D4037`) only when
  `episode.available == false`; a not-started badge. Blur unwatched optional.
- Card sizes 400×263 / 360×235 / 320×207 / 280×179 dp by scale.

## 2. Better Nuvio today

| Area | Better Nuvio (file) | Official | Status |
|---|---|---|---|
| Add-on `released` parsed per episode | `descoberta.c` → `CatEp.dataIso`, display `CatEp.data` | same source | **Exists** |
| Upcoming next-up in CW, `showUnairedNextUp`, Split/Streaming sort | `descoberta.c`, `cwordem.c` | same settings | **Exists** |
| Countdown text | `continuar.c`: `Airs <date>` always (`cwo_data_curta`); no Today/Tomorrow/In N Days | Today / Tomorrow / In N Days / date | **Missing (port)** |
| Date basis | `cwordem.c` uses **UTC calendar** (`gmtime_r`) for both dates | date-only as supplied; instant → **local** date; today in device zone | **Differs (port)** |
| Air-date label format | `idioma_data_curta` (day + short month, year if different) | locale `dMMM` / `dMMMy` | Close; verify wording per language |
| `Upcoming` fallback with no date | shows `Up next` | `Upcoming` | **Differs** |
| NEW EPISODE / NEW SEASON badge in CW | none; "Episódio novo" exists only in notifications/island (`avisos.c`, `ilha.c`) | `isReleaseAlert` rule, 60 days, blue badge | **Missing (port)** |
| Release-alert sort time | `descoberta.c:3714` already moves sort time to `released` when newer than last watched and under 60 days | identical rule | **Exists** (reuse for the badge) |
| In-progress label | `%dh %dmin left` / `%dmin left` | `%dh %dm left` / `%dm left` / `N% Watched` / `Resume` | **Wording differs**; `% Watched`/`Resume` cases missing |
| Episode line | `Season 1 · Episode 2` + episode name | `S1 E2` + episode title (air-date text when unaired and untitled) | **Differs** |
| Short labels for Wide/Poster styles | single label set (`continuar.c` ignores `AJ_CW_ESTILO`) | short set | **Missing** |
| CW artwork, thumbnails on | episode still for any series item, including unaired (`home.c` `arte_card_retomada`) | still **last** for unaired; poster-first for Poster/Wide | **Differs (port)** |
| CW blur | `home_proximo_desfocar`: next-up still when "blur next up" is on | requires thumbnails on and the image to be the thumbnail | Close; align conditions |
| Episode list art | still → series backdrop → poster on failure (`detail.c:5679-5697`) | still only; background colour on failure | **Better adds a fallback** (decision needed) |
| Episode list unaired badge | amber `AIRS` / `NOT AIRED` from TMDB schedule (`epNaoExibido`) | none (only `UNAVAILABLE` when `available == false`) | **Better-only** (decision needed) |
| Episode list date | long date `data` ("27 January 2023") | `dMMMMy` | **Exists** |
| Runtime · IMDb · date meta row | partly (date, ratings) | runtime + IMDb + date | Verify runtime format `1h 5m` |

## 3. What needs porting (no new rules)

1. **Date engine:** a C port of `parseEpisodeReleaseLocalDate`,
   `isEpisodeReleaseAired` and `daysUntil` with official semantics (local zone,
   date-only as supplied, aired at UTC midnight). It replaces the UTC-calendar
   countdown in `cwordem.c`, and is used by the CW card and the hero line.
2. **Countdown wording:** `Airs Today` / `Airs Tomorrow` / `Airs in N Day(s)` /
   `Airs <d MMM[ y]>`, short forms for Wide/Poster, and `Upcoming` when no date.
   New language-table keys are needed (sorted; all 29 tables).
3. **Release alert:** carry `releaseTimestamp`, `lastWatched` and the seed season
   per next-up. Compute `isReleaseAlert` / `isNewSeasonRelease` per the
   official rule and recompute on every publish. Show `New Episode` / `New Season`
   (short `New`) with the official colours.
4. **CW text:** `S1 E2` episode code; untitled unaired episodes show the long
   air-date text; progress wording `Xh Ym left`, `Ym left`, `N% Watched`, `Resume`.
5. **CW artwork order:** port `continueWatchingImageModel`, including style
   awareness (Card / Wide / Poster), unaired still-last, failure fallback order
   and blur conditions.

Decisions for the owner (the brief says "no extra features"): Better's
episode-list **backdrop/poster fallback** and amber **AIRS / NOT AIRED** badge are
not in official. Keep them, or remove them for parity?

Tests to add: a frozen clock across UTC/BST boundaries; date-only versus zoned
values; the 0/1/2–7/8-day and past cases; the release-alert window (59/61 days,
release before/after last watched, season change); artwork order per style
including broken URLs; blur conditions. Preserve the Continue Watching parity and
Stage J suites.

## 4. Implementation (6 November 2026, local only)

The official rules are ported directly; nothing was invented. The source of each
function is cited in its comment.

| Change | Files | Official source |
|---|---|---|
| Release-date engine: plain date as supplied; zoned instant → local date; local date-time in the viewer zone; first embedded date (an invalid first match is final); aired = instant ≤ now with plain dates at UTC midnight; `daysUntil` against today in the device zone | `src/epdata.c/.h` (pure, no UI text) | `EpisodeReleaseDateParser.kt` |
| Countdown text: `Airs Today` / `Airs Tomorrow` / `Airs in N Days` (2–7) / `Airs <d MMM[ y]>`, short `Today` / `Tomorrow` / `In N Days` / `<d MMM[ y]>`, nothing when aired, past or unknown | `src/eprotulo.c` | `AirDateUtils.kt`, `formatEpisodeAirDateLabel` |
| Progress label: `Xh Ym left`, `Ym left` (minimum 1), `N% Watched`, `Resume` | `src/eprotulo.c` | `ContinueWatchingProgressLabel.kt` |
| Release alert: aired, released after the seed's last watch, and **< 60 days** ago; New Season when the season differs. Recomputed at draw time from the stored instant, so a cached upcoming item flips when it airs | `src/cwordem.c` (`cwo_marcar_semente`, `cwo_alerta`), `src/descoberta.c` (records the seed before the existing sort rule) | `resolveNextUpReleaseState`, `recalculateCachedReleaseBadge` |
| The raw add-on `released` kept per next-up (`cwo_marcar_estreia_cru`) | `src/cwordem.c`, `src/trakt.c` | `selectEpisodeReleaseValue` (add-on value; TMDB opt-in is off in official) |
| Card badge (top right): progress label → New Season / New Episode → countdown or `Upcoming` → `Next Up`. Short forms (`New`, `Next`, `Today`, …) in Wide and Poster styles. Solid `#1D4ED8` (New Episode) / `#B45309` (New Season), white text | `src/continuar.c` (`continuar_selo`), `src/badges.c/.h` (`BADGE_NOVO_EPISODIO`, `BADGE_NOVA_TEMPORADA`) | `ContinueWatchingCard` |
| Episode line `S1 E2` above the title; episode title below; an **untitled unaired** next-up shows the long air-date text | `src/continuar.c` | `episodeStr`, `episodeTitle` |
| Poster-style CW row: next-up badge (short form) top right; progress labels hidden | `src/home.c` | `showBadgeInPoster` |
| Home hero line for a next-up: the same label, uppercased (NEW SEASON / NEW EPISODE / countdown / UPCOMING / NEXT UP) | `src/home.c` | `ModernHomeModels` |
| CW artwork (landscape card): an **unaired** next-up shows the title's landscape art first (custom → backdrop → poster) and its still only as the last resort; aired and in-progress keep the still first when "Episode thumbnail" is on | `src/home.c` (`arte_card_retomada`) | `continueWatchingImageModel` |
| 13 new language keys, all 29 tables synced (Arabic translated) | `src/idioma_*.h` | `strings.xml` `cw_*` |

Safety: an empty or unparseable date never shows a countdown. A next-up with no
date counts as aired, as the official `hasEpisodeAired` fallback does. Items that
are neither in progress nor next-up get no badge. Subset-linked builds use weak,
fail-closed fallbacks that keep the pre-Stage E behaviour
(`continuar_proximo_inedito` → "aired"; `cwo_marcar_estreia_cru` → instant only).

### Differences from regular Nuvio (remaining)

- **Seed season** isn't stored on a next-up item. It's derived as "the episode
  before SxE1 (x > 1) is in season x−1". This gives the official New Season
  result for normal next-up rollover.
- **Date order and format** use Better's per-language `idioma_data_curta`
  (English: "Nov 13", "Jan 3, 2027"), not Android's locale best-pattern.
- **In-progress labels** use Better's `restanteMin` (whole minutes). An item with
  under a minute left but a known duration therefore shows `N% Watched` rather
  than `1m left`.
- **Wide style** in Better is a landscape card. Official Wide uses a poster-shaped
  art strip; that layout wasn't changed (no redesign). It does get the short labels.
- **The hero for an in-progress item** keeps Better's "CONTINUE WHERE YOU LEFT
  OFF · N MIN" line; only the next-up line was ported.
- **Episode list:** Better's backdrop/poster fallback for a missing still, and its
  amber AIRS / NOT AIRED badge, are kept. Neither exists in official, which shows
  only the still (plain background when missing) and an UNAVAILABLE badge. No
  owner decision was received, so existing behaviour was left as is.
- The account season-rollover window (`cwo_virada_aceita`, 7 days) is existing
  Better logic and unchanged.

### Tests

- New `tests/stage_e_episodios.sh` (frozen clocks, London/Tokyo/New York zones,
  BST start and end): date parsing, aired boundaries, every countdown case (long
  and short), the 59/60/61-day alert window, watched-after-release, season
  change, progress labels, card badge and style per item state, untitled unaired
  title, `S1 E2`. **PASS.**
- `tests/nextup_art.c`: new case for unaired-next-up artwork (title art first,
  still last). **PASS.**
- Subset-linked tests compiling the touched files (124): no new failures after
  the fallbacks. `stream_parser` failed one timing check under parallel load and
  passes alone.
- Focused regression matrix after all Stage E changes: **41/41 PASS**
  (Stage G/I/J, settings A–D, Continue Watching parity, preservation, Home rows).
  Logs: `build/stage-e/`.

Not yet in an APK and not checked on a device; visual review on a TV is still
needed. No account sync, authentication, playback or Stage J code changed.

### Fix after BlueStacks review (6 November 2026)

The owner reported an unaired next-up (Abbott Elementary S6E6) showing "22m left".
Cause: a next-up item has `progresso` 0 but can still carry the episode runtime in
`restanteMin`, and the ported badge checked `restanteMin > 0` first. Official
treats InProgress and NextUp as different item types, so a next-up never shows a
progress label. Fix in `continuar_selo`: the progress label applies when
`progresso > 0`, or to a non-next-up with a known remaining time. A next-up
always takes the release/countdown path. The Poster-style badge now checks
`continuar_e_a_seguir`. Regression cases added to `stage_e_episodios`
(unaired next-up with runtime → "Airs in 3 Days" / "In 3 Days"; aired → "Next
Up"; non-next-up → "22m left"). PASS, as do `nextup_art`, `cwordem` and
`homepos`.

Rebuilt APK: `build/android/Nuvio-2.0.8-android-stage-e2-debug.apk`, 74,577,558
bytes, SHA-256 `c93ddddf71aaf878e72827fbd182d1072aa1f3b5ca2c6a19ccbbb163e7f4e98f`. Same package, version and signer; 12 libraries; 16 KB
aligned; no read-only guard. **Not installed.**

### Unaired episode artwork in the episode list (owner request, 6 November 2026)

The owner reported Abbott Elementary S6E6 (airs 11 November 2026) showing the
show's **portrait** poster pillarboxed inside the landscape episode card, and
asked for landscape art for unaired episodes. Before an episode airs, the add-ons
send the show's poster as its thumbnail. New rule (`ep_arte_usa_fundo`,
`src/epdata.c`, used in `desenhaEpisodioCorpo`, `src/detail.c`): for an
**unaired** episode (TMDB schedule `epNaoExibido`, or its `released` not yet
reached), use the title's landscape backdrop when the episode has no still, its
thumbnail is the show's poster URL, or the loaded image is portrait. A real
landscape still is kept; aired episodes and titles without a backdrop are
unchanged.

**This is a deliberate difference from regular Nuvio**, whose list always shows
the episode thumbnail. Tests: 7 new cases in `stage_e_episodios` PASS; the 9
subset-linked `detail.c` tests are unchanged versus baseline; 41/41 focused matrix
PASS.

APK: `build/android/Nuvio-2.0.8-android-stage-e3-debug.apk`, 74,577,191 bytes,
SHA-256 `b08d67b25efacfa803f78119b6d2a0ad883339a91cdde945e9f1de5c4bc5b92b`. Same
package, version and signer; 12 libraries; 16 KB aligned. Includes the earlier
badge fix. **Not installed.**

## 5. Air-date provider selection — comparison only (6 November 2026)

The owner asked for "Use TMDB for air dates" to work exactly as in regular Nuvio,
and for differences to be reported before any design change. No code was changed
for this section.

### Regular Nuvio (`99fbd65`; `origin/main` and `origin/dev` identical here)

- **There is no "Use TMDB for air dates" setting.** `selectEpisodeReleaseValue(
  addonReleased, tmdbAirDate, useTmdbReleaseDates)` exists
  (`EpisodeReleaseDateParser.kt`), but both callers hard-code
  `useTmdbReleaseDates = false` (`HomeViewModelContinueWatching.kt:2059`,
  `MetaDetailsViewModel.kt:1689`). It was introduced that way in `c7400171d` /
  `01f777058` (15 July 2026). The TMDB enrichment settings have no air-date
  option: "Episodes" covers titles, overviews, thumbnails and runtime only.
- **Effective rule:** the add-on's `released` is always used. A missing value
  stays missing: no TMDB fill, no countdown, and it counts as aired
  (`hasEpisodeAired` fallback).
- **Which add-on:** `MetaRepositoryImpl.getMetaFromAllAddons` takes the user's
  installed add-ons **in order**, preferring those supporting the type and ID
  prefix, then the top meta add-on. That is the user's configured metadata
  add-on (e.g. AIOMetadata), with its own settings and region honoured by the
  add-on itself.
- No region setting and no date shifting in the app.

### Better Nuvio today

| Consumer | Date source | Versus official |
|---|---|---|
| Stage E countdown, New Episode/Season, CW upcoming split (`trakt.c` → `cwo_marcar_estreia_cru`) | `/meta` from **`catalog.nuvio.tv`** (Nuvio's TMDB-backed catalog service, `metaprov.h`), with **Cinemeta** as fallback when an episode is missing | **Differs**: not the user's configured metadata add-on (AIOMetadata etc.) |
| Episode list dates (`CatEp.data`/`dataIso`, `descoberta.c` ~8000–8040) | Cinemeta's list versus each meta add-on's; **the one with more episodes wins**. When Cinemeta wins, the add-on only fills missing dates (`desc_mesclar_episodios`) | **Differs**: Cinemeta dates can win over the configured add-on's |
| "Release dates" TMDB toggle (`AJ_TMDB_DATAS`, `tmdb_use_release_dates`) | Movies only: TMDB `release_dates` for age rating; never episode dates | Not an air-date selector; official TV has no such toggle |
| Agenda / Schedule (`agenda.c`, `extras.c`) | TMDB `next_episode_to_air` / `last_episode_to_air` `air_date`; Cinemeta `released` date part | Better-only screen; also drives the episode list's amber AIRS badge (`epNaoExibido`) |
| Missing dates | No countdown; counts as aired | **Same** |
| One-day shifts / region settings | None | **Same** |

### Decisions needed before any change

1. A "Use TMDB for air dates" toggle **doesn't exist in regular Nuvio**. Adding
   one would be a new Better setting. The `selectEpisodeReleaseValue` semantics
   (TMDB when on, else add-on; add-on as fallback when TMDB has no date) could be
   reused. Should this be added?
2. To match official when the toggle is off, Continue Watching and the episode
   list would need to read dates from the **user's configured metadata add-on**
   (installed order, type and ID-prefix match) instead of
   `catalog.nuvio.tv`/Cinemeta and the episode-count pick. That changes existing
   metadata-source behaviour beyond dates (names, stills, episode lists), so it
   needs approval.

### 5.1 Implemented: air dates from the user's metadata add-on (6 November 2026)

The owner chose "Use my metadata add-on". No TMDB toggle was added (regular
Nuvio has none). Only the **date source** changed; names, stills and episode
lists are as before.

- **Add-on selection** (`metaAddonDasDatas`, `src/descoberta.c`), ported from
  `MetaRepositoryImpl.getMetaFromAllAddons`' first pass: installed add-ons **in
  order** that are switched on, have a read manifest, provide `meta`, and accept
  this type with this ID prefix or with no prefixes (`addons_aceita_id` ≠ 0). The
  first usable `/meta` wins. Cinemeta counts as an ordinary candidate, as in
  official. Bodies are shared with the existing meta cache (lock-protected).
- **Continue Watching** (`src/trakt.c`): the next-up's `released`/`firstAired`
  comes from that add-on and drives the countdown, New Episode/New Season, the
  upcoming split and the hero line. If the add-on lists the episode without a
  date, there's **no date** (counts as aired, official). Better's existing rule
  still applies: an undated account season rollover isn't offered.
- **Episode list** (`datasDoAddonNaLista`, after the existing list choice): each
  episode the add-on lists takes the add-on's date, including "no date"
  (`desc_datas_do_addon`).
- The add-on's values are used **as given**. The app adds no region setting and
  no one-day shift; plain dates stay as supplied, and instants become the
  viewer's local date (official rule).
- TMDB "Release dates" (`AJ_TMDB_DATAS`) still affects movies only, never
  episode dates.

**Remaining differences from regular Nuvio**

- Only official's **first selection pass** is ported. The later passes (inferred
  type, top meta add-on, legacy raw types) aren't. Add-ons whose manifest Better
  hasn't read yet are skipped until it is read.
- **Fallbacks kept from Better:** when no add-on answers, or the add-on doesn't
  list that episode, Continue Watching still uses `catalog.nuvio.tv` then
  Cinemeta, and the episode list keeps that episode's existing date. Official
  would have no date (or no such episode) there.
- Titles opened **from an add-on's catalogue** still use that add-on's `/meta`
  for their episode list (unchanged; official details also start from the source
  add-on).
- Better-only screens are unchanged: the Agenda/Schedule and the episode list's
  amber AIRS / NOT AIRED badge use TMDB's schedule.
- No "Use TMDB for air dates" toggle, so the "TMDB enabled" case doesn't exist.

**Tests** (in `stage_e_episodios`, PASS):
- the candidate rule;
- dates from two add-on styles (AIOMetadata-like zoned `released`, Cinemeta-like
  `firstAired`);
- a listed episode without a date (no date) and an unlisted episode (fallback);
- no add-on;
- TMDB "Release dates" on has no effect;
- no US shift (plain date unchanged in New York and London; zoned instant →
  local date);
- the episode-list overlay (wins, clears, keeps, idempotent).

Subset-linked tests: 124 run, no new failures (weak fallback for
`desc_lancamento_addon` in `trakt.c`). Focused matrix **41/41 PASS**. Not yet in
an APK.

## 6. Continue Watching refresh after watched changes (6 November 2026)

**Owner report:** after marking an episode watched or finishing it, Continue
Watching didn't update until a profile switch.

**Cause (two parts):**
1. **No trigger.** Marking or unmarking from the episode menu (`episodios.c` →
   `visto.c`), the Saved panel or the Agenda updated the local watched map and
   queued the account/Trakt/Simkl sends, but never rebuilt the row. Only the
   player exit, one card-menu path, the 10-minute Home timer and profile
   switches did.
2. **Stale seeds.** The account's next-up comes from `contalib_sementes_a_seguir`,
   built from the **server snapshot** of watched items (plus locally finished,
   not-yet-uploaded progress). A local mark or unmark wasn't in that snapshot,
   so even a rebuild kept the old next-up. After playback, once the progress
   upload cleared its "pending" flag, the seed fell back to the stale snapshot
   until the next account pull. A profile switch forces that pull, which is why
   it "fixed" it.

**Regular Nuvio** (`HomeViewModelContinueWatching.kt:276–324`): Continue
Watching is a `combine` of the **local** progress, next-up seeds and watched
items, `debounce`d, held while the player is open (`holdWhilePlayerOpen`). A
local mark updates the local store, and the row recomputes.

**Fix (same approach, targeted):**
- `src/contalib.c` `contalib_sementes_a_seguir`: seeds now come from the
  snapshot **plus this TV's journal**. Rows unmarked here later drop out (the
  existing `contapend_visto_oculto` gate, previously applied only to the watched
  map), and episode-level watched intents join, with the latest time winning
  (`contalib_vistos_com_locais`, pure).
- `src/contapend.c`: `contapend_revisao_gestos()` is bumped by each
  watched/unwatched gesture (not by sync reconciliation), and
  `contapend_vistos_marcados()` enumerates this profile's watched intents.
- `src/app.c`: wires the enumerator, and its loop (both TV and Mobile) calls
  `desc_refazer_continuar()` when the revision changes and the player is
  closed. Only this row is rebuilt, in one coalesced thread; no full Home reload
  and no extra sync cycle. The player exit keeps its existing rebuild.

Results: mark → next episode; finish → next episode (the journal intent keeps
the seed after the upload); unmark → back to that episode; returning Home shows
the latest state. Order, progress, badges and artwork come from the same
`montarContinuar` path as before.

**Tests:** `tests/contapend.c` (gesture bumps the revision; unmark bumps it;
watched intents enumerated and dropped after unmark; logged out = no bump) and
`tests/contalib.c` (snapshot S6E6 → mark S6E6 here → S6E7 with both anchor
rules; unmark S6E5 here → S6E5; merge has no duplicates and the latest time
wins). PASS. Subset tests compiling app/contapend/contalib: 39 run, no new
failures. Focused matrix **41/41**; Stage E, `cwordem` and `nextup_art` PASS.

**Differences from regular Nuvio:** Better rebuilds per gesture through its
existing coalesced worker rather than a 1:1 reactive stream. Trakt/Simkl
next-up lists still change only when those services are re-read (unchanged);
the fix covers the Nuvio account source and the local journal. Not yet built
into an APK; no device check.

## 7. Artwork provider consistency (6 November 2026)

The provider is Better's "Background do hero" setting (`AJ_HERO_FONTE`,
`ajustes_hero_fonte()`): Automatic, Catalog/Cinemeta, IMDb/Metahub, TMDB, Trakt,
Apple TV, fanart.tv, Anime. Regular Nuvio has no equivalent setting; this is a
Better feature made consistent.

**Investigation: who followed it.** Home landscape cards (`arte_por_formato` →
`artehero_url_card_fonte`), the Home hero for a non-Automatic source, the details
background, the player and the art picker already followed it, read every frame,
so a change shows without a reload (`homeestado.c` hashes it too). **Bypasses
found:**
1. **Episode list** (`detail.c` `desenhaEpisodioCorpo`): read the show's raw
   catalog/Cinemeta `backdrop` (then poster) directly, including for unaired
   episodes (section "Unaired episode artwork").
2. **Home hero, Automatic:** showed the episode "still" even for an unaired
   next-up (add-ons send the show's poster as that thumbnail).
3. **Landscape cards with no landscape image:** fell back to the poster and drew
   it with GFX_CARD's contain fallback (side bars).

**Fix:**
- `detail.c`: `epPaisagemSerie` gives the show's landscape art from the
  **selected provider**, then the IMDb/metahub landscape backdrop if the
  provider's image is portrait, then the catalog backdrop (`ep_arte_paisagem`,
  pure). It's used for episodes without a still, the failed-still fallback (then
  the poster, as before) and unaired episodes (`ep_arte_usa_fundo`). Aired
  episodes keep their still first.
- `home.c` `arte_hero_do_item`: an unaired next-up skips the still and shows the
  show's art (Automatic: `artehero_url_destaque`; other sources already did).
- `home.c` `arte_por_formato` (landscape): with no provider art, the metahub
  landscape comes before the poster.
- **No side bars:** a portrait image left in a landscape card is cropped to fill
  it (`ep_arte_cobrir` → `gfx_card_forcar_cover_atual`), in the episode list and
  in Continue Watching cards (regular Nuvio's `ContentScale.Crop`).
- **Refresh:** all of these resolve URLs at draw time, so changing the provider
  updates the Home rows, the hero, Continue Watching and an open episode list
  immediately, on TV and Mobile. No profile switch and no extra network
  requests (textures are cached by URL).

Coordination: the unaired checks reuse Stage E's `continuar_proximo_inedito` and
episode release rules (whose dates now come from the metadata add-on, §5.1). The
Continue Watching refresh (§6) is unaffected.

**Tests:** `stage_e_episodios` (provider landscape wins; portrait →
metahub; only portrait → kept and cropped; no provider → metahub → catalog →
none; crop rule) and `nextup_art` (unaired hero shows the show's art; aired hero
keeps the still). PASS. Subset tests compiling home/detail/trakt/descoberta:
131 run, no new failures. Focused matrix **41/41**. Not yet in an APK.

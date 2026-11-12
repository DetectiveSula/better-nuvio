# Stage E and H review — implementation deferred

Reviewed 5 November 2026. Stage J takes precedence. This document records scope,
not a claim that the requested features are implemented. No date, episode-card,
Home redesign or playback-control implementation was changed in this review.

## Stage E: episode dates and episode-card presentation

Requested scope:

- Accurate episode air dates and countdowns in Continue Watching and show details.
- NEW EPISODE badges with an explicit, consistent eligibility policy.
- Full-card portrait posters for unaired episodes, retaining readable air-date
  information, remote focus and accessibility.

Existing code parses addon `released` into `CatEp.dataIso` and the display date
in `src/descoberta.c`; Continue Watching has its own upcoming/next-up processing.
This is evidence of separate consumers, not proof of a particular date bug.
The pinned official repository is available at
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`.

Before changing date selection, capture one reproducibly wrong episode: title,
season/episode, provider/addon, raw `released`/`firstAired`/TMDB `air_date`, expected
UK display date and observed dates on both screens. Redact provider credentials.
Distinguish a date-only release from an actual instant; do not apply a blanket
BST/UTC offset or silently replace addon dates with TMDB. Preserve specials/anime
mapping, profile isolation, cached metadata refresh and completed-episode logic.

Define what NEW EPISODE means before coding: recently aired versus newly available,
unwatched eligibility, time window and timezone/date-only boundaries. Countdown
presentation must not imply that a date-only source supplies an exact release time.
Unknown dates need an honest fallback. Portrait poster fallback/cropping and badge
contrast should be reviewed visually on a 16:9 TV before implementation.

Tests should freeze the clock and cover UTC offsets, BST boundaries, date-only
values, malformed/missing dates, source priority, specials/anime identities, cache
refresh and parity between details and Continue Watching. Preserve all progress
formats and existing Continue Watching regression behaviour.

## Stage H: Apple TV-inspired presentation

Requested scope: Home and details pages, a simplified paused-player overlay and
cleaner playback controls. Keep the shared Native rendering architecture and TV
remote navigation; no framework replacement is proposed.

Modern / Default / Dynamic are distinct existing user choices. Their new
Better-to-Better sync binding is Stage J transport work, not a visual redesign.
Stage H must preserve all three choices, Stage G stable row identities/order,
profile-specific hybrid rows, Friends Watching, Last Session, 30+ rows, lazy
loading, paging, focus restoration and all extra Legacy controls.

Prepare reviewable Home/details/paused-player mockups first. Specify which paused
information remains visible, which moves behind an action, and the exact remote
shortcuts/Back behaviour. Do not remove subtitles, audio/sync controls, seek
thumbnails, stream/source selection, diagnostics or device-specific capabilities.

Validate 1920×1080 plus smaller displays, long titles, accessible text and focus,
loaded/missing artwork, playback/resume, input latency and platform-specific
Android TV/webOS/Tizen limits. Shared compilation is not real-device validation.

## Sequence

1. Finish and locally verify Stage J cleanup; keep backend writes disabled by default.
2. Obtain approval before installing the newly built APK or arming live tests.
3. Validate the new Home-layout binding and privacy/disarm changes on dedicated
   test accounts only. Do not access the main account's documents.
4. Review the reproducible Stage E example and Stage H mockups before their code work.

No separate Stage E or H specification file was found in
`docs/settings-restoration/` at review time. The older named audit files
`PRIORITY-FINDINGS.md`, `LEGACY-ADDITIONAL.md` and `settings-comparison.xlsx` were
not found in the project by filename search; no contents have been assumed.

## Stage H task: trailer flicker in zoom display modes (recorded 6 November 2026)

**Owner report:** when a trailer starts in Slight / Cinema / Ultra zoom, the video
sometimes flickers briefly. **Status: investigated (code reading only), NOT
implemented. To be done when Stage H begins.**

### Findings (Android path: Fire Stick, BlueStacks, Android TV/Mobile)

Zoom = `ajustes_trailer_zoom()` (Cinema 1.34, Slight 1.15, Ultra 1.55, Original
1.0). It's applied in `src/trailer.c` (around lines 364–400) through
`video_janela_fonte` (`src/video_android.c:1138`), which on Android **resizes and
moves the SurfaceView** to a window larger than the card and lets it clip.

1. **Default size first, then zoom: yes.** By design, `trailer.c` sends the
   **full frame** (no zoom) before playback starts, and applies the zoom crop
   only **after `playing` + 800 ms**. The comment cites a TV that ignored a crop
   requested before playing. The first visible frames are at Original/fit size,
   then the picture jumps to the zoomed size.
2. **Surface resize causes a blank: yes (most likely cause of the flash).** In
   `NvPlayer.kt` `aplicarJanela`, any real geometry change on **SDR** content
   calls `recriarSuperficie(350 ms)` (SurfaceView GONE → VISIBLE). It's a TCL
   workaround and is skipped only for MStar/Amlogic decoders (`semRecriar`,
   `RecriaHdr.kt`). On the Fire Stick (MediaTek) and BlueStacks, the
   full-frame → zoom change therefore recreates the surface about 350 ms later,
   giving a brief black frame on top of the size jump.
3. **Repeated application: partly.** After the crop, `video_recorte_reaplicar`
   re-sends the same window every 1.5 s for 6 s, and it bypasses the C-side
   dedupe (`janelaKt`, not `janelaSeMudou`). Kotlin's `aplicarJanela` returns
   early when nothing changed, so these are normally invisible. Rounding
   differences could still cause a further relayout; to verify. There is no zoom
   animation.
4. **Poster-to-video: contributes.** On Samsung (`NV_TPK`), `trailer_visivel`
   keeps the artwork up until the crop has settled. On Android it returns `1`
   immediately, so the video is shown before its final geometry. Hardware
   decoding: trailers are normally SDR, so the HDR/DV paths aren't involved; the
   SDR recreation above is.
5. **Player recreation: no.** The ExoPlayer instance isn't recreated; only the
   **Surface** is (point 2).

**Not yet confirmed on a device.** Confirm with the existing `[dvtrace]`,
`[aspect]` and `[video] android recorte` log lines while starting a trailer in
each zoom mode, on the Fire Stick and BlueStacks.

### Proposed fix (for Stage H; needs approval then)

- Compute the zoom window as soon as the video size is known and apply it
  **before the first visible frame**. Keep the artwork shown until the final
  geometry is applied and the surface has settled (`EV_SUPERFICIE_ESTAVEL`), as
  the Samsung path already does.
- Don't recreate the surface for trailer geometry changes on decoders that
  don't need the TCL workaround (or limit that workaround to the families
  measured to need it). Keep HDR/DV and the player's own aspect-mode behaviour
  unchanged.
- Route the periodic re-apply through the dedupe so identical geometry never
  relayouts.
- Keep the "full frame before playing" step only where it's proven necessary
  (the TV in the comment), not for every device.
- Tests: a pure state-machine test for the trailer visibility gate (art stays
  until crop applied and surface stable; no second geometry change); a Kotlin
  unit test that an unchanged or trailer-only geometry doesn't schedule
  `recriarSuperficie` on non-TCL decoders. Then a device check in every zoom
  mode on TV and Mobile, preserving autoplay, sound, controls and remote
  navigation.

### Implemented (6 November 2026, owner: "ok do it") — Android only

`src/trailer.c` (`#ifdef NV_ANDROID`; Samsung `.tpk`, webOS and web unchanged):
- **Zoom before the first visible frame:** the Slight/Cinema/Ultra crop is
  sent as soon as the frame size is known, with no full-frame step and no
  "playing + 800 ms" wait (that ordering came from webOS).
- **Artwork held until final geometry:** `trailer_mostra_video` reveals the
  video only when the crop has been sent **and** `video_janela_pronta()`
  confirms the native layout (applied, surface valid, no SurfaceView
  recreation pending or running). The opening resize and any TCL-workaround
  recreation therefore happen behind the artwork. Reveal is **sticky per
  source**. Fallbacks (never a stuck trailer): 2.5 s after playing, or 6 s after
  opening. Rule is `trailer_portao_android` (pure).
- **No repeated application:** the 1.5 s re-apply loop is skipped on Android
  (NvPlayer re-applies its last window itself after a recreation; identical
  requests are already no-ops there).
- **No Kotlin change was needed:** the recreation now happens while hidden, so
  the TCL/HDR/DV logic is untouched. The player isn't recreated.
- A bug found by the new test was fixed: the elapsed-time difference used
  unsigned arithmetic with `ticks|1`, which could wrap and reveal at once. It's
  now signed and clamped.
- Autoplay, sound, controls, remote navigation and Home-card behaviour
  (`noCartao` check) are unchanged. TV and Mobile share this Android path.

**Tests:** new `tests/trailer-android-zoom.sh`:
- the pure reveal rule;
- Cinema: no output before the size is known, one crop exactly, never a full
  frame, hidden until the layout is confirmed, sticky, no re-apply over 6 s;
- Ultra: the fallback reveal at 2.5 s;
- Original;
- resize re-crops once.

PASS. Existing `trailer-android-geometry` PASS. 30 trailer-related suites: no
new failures. Focused matrix 41/41. **Not yet built or checked on a device.**

### Follow-up: trailer subtitles cut off by zoom (investigated 6 November 2026)

- **Native trailers have no subtitle track:** Android/Fire Stick/BlueStacks and
  Samsung `.tpk` show none. Only the web/YouTube path loads captions
  (`trailer_js_legenda`). Subtitles seen on these devices are **burned into
  the video** (typical for foreign-language trailers).
- **The crop is centred vertically** (`trailer_recorte`, `sy = (vh - h) / 2`):
  - Cinema (1.34) trims ~12.5% top and bottom. This removes a 2.39:1 trailer's
    bars, so subtitles placed in the bottom bar are lost.
  - Ultra (1.55) trims ~18% each edge, reaching into the picture, so
    bottom-of-picture subtitles are lost too.
  - Slight (1.15) trims ~6.5% each edge.
  - The wide Home banner (Default layout, ~3.6:1) crops height heavily even in
    Original.
- **Detection:** burned-in subtitles can't be seen in the data. The available
  signal is the title's original language (`extras_idioma_original()`, TMDB)
  versus the app language. That flags "likely subtitled" trailers.
- **Decision pending (owner):** for likely-subtitled trailers, either
  (a) keep the zoom but take the vertical crop from the **top**, keeping the
  bottom subtitle band; or (b) show them **unzoomed** (Original framing).

### Implemented: subtitle-safe trailer crop (owner chose Option 1, 6 November 2026)

- **Signal (best effort, not a guarantee):** a trailer is "likely subtitled"
  when its title's TMDB original language is known, isn't English, and differs
  from the app language (`trailer_legenda_provavel`). The details page passes
  `extras_idioma_original()`. Home passes the language cached by the hero
  trailer lookup (`extras_idioma_original_de`, new 64-title cache in
  `extras.c`). Unknown → normal centred crop.
- **Crop** (`trailer_recorte_legenda`, `src/trailer.c`): same zoom, same width
  and same horizontal crop. Only the vertical trim moves up. The bottom keeps
  at most the standard bar trim (2.39:1 inside 16:9, ~12.8%), so no black strip
  appears, and nothing at all on matted frames. The rest comes off the top.
  Other trailers keep `trailer_recorte` (centred), unchanged.
- **Effect** (bottom trim, centred → subtitle-safe):

| Zoom | Frame | Destination | Before | After |
|---|---|---|---|---|
| Slight | 16:9 (IMDb) | hero 16:9 | 6.7% | 6.7% |
| Slight | 16:9 | banner 3.6:1 | 25.6% | 12.8% |
| Slight | matted 2.39 (Apple) | banner | 17.2% | 0% |
| Cinema | 16:9 | hero 16:9 | 12.8% | 12.8% |
| Cinema | 16:9 | banner | 25.6% | 12.8% |
| Cinema | matted | banner | 17.2% | 0% |
| Ultra | 16:9 | hero 16:9 | 17.8% | 12.8% |
| Ultra | 16:9 | banner | 25.6% | 12.8% |
| Ultra | matted | banner | 17.2% | 0% |
| any | matted | hero 16:9 | 0% | 0% (sides only) |

  Slight and Cinema in a full 16:9 hero already trimmed no more than the bar, so
  subtitles inside the picture were safe there. Subtitles printed *inside* the
  black bar of a 16:9 frame still can't be shown without bringing back a black
  strip; the owner ruled that out.
- **Flicker fix and compatibility kept:** the flag is read once, when the crop
  is sent (still before the first visible frame, still behind the artwork). A
  language arriving later never re-crops, so there's no extra geometry change
  or SurfaceView recreation. Kotlin, TCL, HDR and DV are untouched. Android,
  Samsung `.tpk` and the generic native path use the same rule; web/YouTube
  (JS zoom) isn't covered.
- **Also fixed:** `extras.h` now includes `<stddef.h>` (the new `size_t`
  declaration had broken subset builds that include it first).

**Tests** (`tests/trailer-android-zoom.sh`, PASS):
- the signal rule;
- all 3 zoom modes × 16:9/matted × hero/banner: same size, moved up only, never
  more bottom trim, no strip beyond the bar, matted bottom kept, `legenda` 0
  identical to the old crop;
- an end-to-end foreign-language trailer: one already-shifted crop, still
  gated, a later language doesn't re-crop;
- an English-app same-language trailer: centred.

Subset tests touching extras/trailer/home/detail plus trailer suites: 64 run, no
new failures. Focused matrix 41/41. Not built or installed.

### Investigation: full-card trailers that keep burned-in subtitles (6 November 2026)

Owner preference: Apple TV-style full card, no black bars, no permanent band.
No code changed.

**Device evidence (BlueStacks log):** recent trailers were IMDb 16:9 MP4s
(1920×1080 with the 2.39:1 picture letterboxed inside). Cinema cropped
`244,138 1432x804`, trimming the 138 px bars top and bottom. Apple (preferred
first on Android) didn't have these titles. Reveal at +0.9–1.3 s, "final
geometry" (flicker fix working).

1. **Bottom-biased crop without bars.** This works whenever the subtitle is
   **inside the picture**: Apple matted frames (1920×804) and the first line on
   IMDb frames. The shift is small (at most about 5% of the height comes off the
   top instead of the bottom in Ultra), so framing barely changes. It **can't**
   show a line printed **inside the black bar** of a letterboxed 16:9 frame:
   those rows are black across the full width, so any crop that includes them
   shows black. Line 2 of a two-line subtitle on IMDb frames is exactly that
   case.
2. **Position independent of zoom: yes.** The zoom fixes the crop **size**;
   `trailer_recorte_legenda` already sets the **position** separately.
3. **Moving the crop on Android:** the crop *is* the SurfaceView's position and
   size. Any change is a layout change, and on SDR with non-MStar/Amlogic
   decoders (the Fire Stick, BlueStacks) that triggers the TCL-oriented surface
   recreation, a visible flash once the video shows. A **fixed** position
   chosen before reveal is free (the current design). **Moving it during
   playback** isn't, unless recreation is skipped for position-only moves,
   which TCL panels need. A GL/SurfaceTexture trailer path would make crop
   changes free, but it's a pipeline change.
4. **Consistency:** one crop function serves Android, Samsung `.tpk` and the
   generic native path, for every destination (hero 16:9, details 16:9, full
   screen 16:9, wide banner 3.6:1). The banner crops the most height, so
   bottom bias helps most there. Web/YouTube (JS zoom) is separate.
5. **Detection without real-time processing:**
   - **Free:** frame shape at open (matted = subtitles inside the picture; 16:9
     = possibly letterboxed) and the original-language heuristic.
   - **Cheap, one-off:** a single-frame probe per trailer (Android
     `MediaMetadataRetriever` on the IMDb MP4, about 0.2–0.5 s plus one
     keyframe's download, cached per URL), during the existing ~1 s artwork
     hold. It would measure the exact bar height and whether bright text sits
     in the bottom bar.
   - **Not available:** live pixel analysis. The SurfaceView overlay can't be
     read back.

**Options compared** (Fire Stick / BlueStacks):

| Approach | Full card | Line 2 visible | Smoothness | Cost/risk |
|---|---|---|---|---|
| Current subtitle-safe crop | yes | no (letterboxed) | unchanged | done |
| Prefer Apple matted source for likely-subtitled titles | yes | yes (when Apple has it) | unchanged | low; often unavailable for foreign titles |
| Band (full screen/sound only) | no (band) | yes | unchanged | low; owner prefers not |
| **"Subtitle lift":** one-off probe + GL composite that copies the bar's text strip (black keyed out) onto the bottom of the zoomed picture | yes | yes | needs measuring; extra GPU pass | high: trailer rendering moves from SurfaceView to SurfaceTexture/GL (SDR trailers only, player unaffected); removes the recreation issue for trailers |
| Live repositioning when subtitles appear | yes | partly | flash risk on the Fire Stick | not recommended |

**Addendum: the Dynamic details "carousel" card (6 November 2026).** The large
rounded card the owner uses (Dynamic layout, details page; `detail.c`
`CAR_X 130`, `CAR_Y/CAR_BAIXO 26`, `CAR_VAO 34`, `CAR_RAIO 40`) is **1660 × 1028
in 1920×1080 units, ≈ 1.61:1 (about 16:10)**; ~1.64:1 measured on BlueStacks,
whose screen isn't exactly 16:9. It's a **rounded window over full-screen 16:9
art/video**: the details trailer opens at the full-screen rectangle
(`detail.c:3791`), so after the zoom crop the window hides a further 130 px
each side and 26 px top/bottom. The bottom-left carries the logo, Play, synopsis
and metadata over the `CAR_VEU 0.86` gradient, which centred bottom subtitles
overlap regardless of crop. **Implication:** on this screen, subtitle legibility
is limited by the page text as much as by the crop. Full-screen trailer playback
(no text over it) is where subtitles can realistically be read. Any subtitle
work (crop, lift or band) should prioritise full screen and treat this card as
best effort.

## Stage H task: carousel skips two cards while a trailer plays — paused (6 November 2026)

**Owner report:** pressing Right in the Dynamic details carousel while a trailer
plays sometimes lags and moves two cards. Investigation was paused by the owner
before a fix; no code changed.

**Evidence so far** (BlueStacks, adb single key events with instant down/up,
`[carrossel]`/`[quadro]` logs):
- **Duplicate processing: not reproduced.** 14 single Right presses moved
  exactly one card each, with trailers playing, starting or idle (Apple HLS and
  IMDb MP4). `carPasso` steps once per KEYDOWN; the carousel spring clamps
  `dt` (50 ms) and can't overshoot.
- **The lag is real.** Some presses took ~1.1 s instead of ~0.15 s. Each slow
  case logged a **~1.0 s frame stall spent entirely in the GPU buffer swap**:
  `[quadro] pior=1030.7ms | ev=0.1 … upd=0.4 … swap=1030.0`. That's the render
  thread blocked in `SwapWindow`, not input or update work. It's intermittent
  (about 1 in 3), more likely after the trailer has played for a while, and also
  seen once when the press only moved between buttons.
- **Likely link:** closing the card trailer (`carPasso` → `trailer_fechar` →
  `NvPlayer.parar()` → `liberar()` on Android's main thread: SurfaceView resized
  to 1×1 and hidden, cache hand-off) changes the video SurfaceView while SDL
  swaps its own surface. On recent Android, sibling-SurfaceView geometry
  changes are synchronised with the main thread (BLAST sync), so the swap waits.
  To confirm with an all-tags log capture (it was in progress when paused).
- **Why a real remote can skip two:** during the ~1 s freeze the UI gives no
  feedback, so presses made in that second (or a held key's firmware repeats,
  sent as separate KEYDOWNs per the `app.c` notes) queue and apply together. The
  idle-dimming wake swallow (`esmaecer_entrada`) is unrelated.

**Next when resumed:** confirm the stall source from the all-tags log; the
targeted fix is to avoid the synchronous surface change on carousel steps
(e.g. stop decoding and close the video hole immediately, defer the SurfaceView
resize/release until the carousel has settled or a new trailer reuses it),
not an input debounce. Regression tests: single press, held press and rapid
presses while a trailer closes.

## Proposed Stage H feature: Family Watching (replaces Friends Watching) — feasibility, not implemented

Recorded 6 November 2026. Investigation only; no code, account data, APK or
backend change. Owner mock-up: Option D (avatar + name, In Progress / Recently
Watched pill, large artwork, title, S/E line, time left, progress bar,
play/check icon).

### 1. Does the existing account API support it?

**Technically yes, with existing endpoints only; but there is no explicit
cross-profile *feature* or consent model in Nuvio.**

| Need | Existing call (already used by Better) | Notes |
|---|---|---|
| Other profiles' names/avatars | `sync_pull_profiles` (`src/perfis.c`) | `id`, `name`, `avatar_color_hex`, `avatar_id`, `avatar_url`, `profile_index`. |
| PIN-locked profiles | `sync_pull_profile_locks` (`src/perfis.c:372`) | One row per profile with `pin_enabled`. |
| In-progress item | `sync_pull_watch_progress` `{p_profile_id, p_limit}` (`src/syncprog.c:98`) | `content_id`, `content_type`, `video_id`, `season`, `episode`, `position(_ms)`, `duration(_ms)`, `last_watched(_at)`/`updated_at`. |
| Recently marked watched | `sync_pull_watched_items` (`src/sync.c:963`, parsed in `src/contalib.c:319`) | `content_id`, `season`, `episode`, `title`, `watched_at`. |

- All of these are **per profile** via `p_profile_id`, and the server accepts
  any profile id of the signed-in account with the **same account session**.
  That is the authorisation the profile switcher already relies on (Stage J's
  read-only probe read profiles 1 and 2 with one session). So reading another
  profile's latest activity needs **no new endpoint** and does not bypass the
  backend's isolation: the backend isolates *accounts*, not profiles.
- **Official Nuvio has no equivalent.** No cross-profile / family / household
  activity in the supplied official Android source; its sync services
  (`WatchProgressSyncService`, `WatchedItemsSyncService`) always pass the
  *active* profile's id. Family Watching would be a Better-only feature built on
  official endpoints used for a purpose official Nuvio doesn't use them for.
- **Cost:** two reads per other profile per refresh (progress with a small
  `p_limit`, watched items first page). With up to ~5 other profiles that is
  ~10 small RPCs; must be cached and throttled, never per frame.

### 2. Privacy, child profiles and content restrictions — what's missing

1. **Profile privacy.** There is no per-profile "share my activity" flag on the
   backend. The only existing privacy signal is the **profile PIN**. Proposal:
   - a PIN-locked profile is **never shown** (its PIN exists to keep it
     private) — unless the viewing profile is that profile itself, which it
     never is here;
   - a **local, per-device opt-in** setting "Family Watching" (default **off**
     until the owner decides) plus a per-profile "Hide my activity from Family
     Watching" toggle. Because there is no backend field, the per-profile hide
     can only be honoured on devices that know about it. To make it account-wide
     it needs either (a) a new backend field (out of scope: no new endpoints), or
     (b) storing it inside the profile's own existing synced settings document
     (the Stage J Better settings blob), which other devices already read. (b)
     uses existing storage but writes account data, so it needs owner approval
     and J4/J6 arming — not done now.
2. **Child profiles.** Neither Better nor official `UserProfile` has a kids /
   maturity flag, so "child profile" cannot be detected from data. Options:
   - owner marks child profiles locally (setting), stored like the hide flag;
   - when the **viewing** profile is a child profile: hide the whole row (safest)
     or show only titles that pass its content restriction;
   - a child profile's own activity can still show to adults.
3. **Content restrictions.** There is no account-wide rating/maturity
   restriction in the API either (only per-title parental guide data in
   official `ParentalGuideRepository`). Until a restriction exists, the safe
   default is: if the viewing profile is marked child, don't render the row.
4. **No copying.** Nothing from the other profile is written anywhere: no
   progress, no watched marks, no library or CW changes; reads only.

### 3. Proposed design (once the decisions in §4 are made)

- **Data (new `src/familia.c`, read-only):** on profile list load and on each
  normal sync cycle, for every *other* visible profile, fetch the newest
  progress row and the newest watched item, choose the most recent by
  timestamp, and keep one `FamItem { perfil, nome, avatar, tipo, imdb/tmdb,
  temporada, episodio, pct, restanteMin, quando, estado(EM_CURSO|VISTO) }`.
  Cached in memory and on disk per account fingerprint (no tokens/ids logged:
  log only counts and the 8-char fingerprint). Throttled (e.g. ≥ 2 min between
  pulls, immediate on Home open/resume) — refreshes without switching profile.
- **Metadata/artwork:** resolve title and art through the same path Continue
  Watching uses (`arte_card_retomada` / `ep_arte_paisagem` with the selected
  artwork provider and the metadata add-on), so the card matches CW. Episode
  still for in-progress episodes, series landscape for watched ones.
- **Row:** replace `FILEIRA_SOCIAL` ("Friends Watching", `amigosfil.c`) at the
  same Home position and the same layout id, renamed **Family Watching**. One
  card per other profile (Option D): avatar + name header, pill **In Progress**
  (progress bar + "22m left", play icon) or **Recently Watched** (check icon,
  "Watched 2h ago"), artwork, title, `S2 · E5` line. Profiles with no activity
  are omitted; the row hides when no card remains. Existing focus rules,
  remote navigation and row ordering (catordem/layout sync) unchanged.
- **Select:** OK opens the **details page under the current profile** (same as
  any catalog item) — no profile switch, no resume-from-their-position, no
  progress copy.
- **Friends Watching code:** kept in the tree behind the row type until Stage H
  is accepted, then retired; the Trakt/Nuvio-friends social panel is untouched.
- **Tests:** profile filtering (self, PIN-locked, hidden, child viewer),
  most-recent choice between progress and watched, card text/pill, navigation
  unchanged, no write RPCs issued (assert on a stubbed `contaRpc`).

### 4. Decisions needed from the owner before implementation

1. Accept using the existing per-profile RPCs to read other profiles' latest
   activity (same-account session; Better-only feature)?
2. Default **off** (opt-in per device) or on?
3. PIN-locked profiles: always excluded (proposed)?
4. Per-profile "hide my activity": local only, or stored in the profile's synced
   Better settings (needs approval to write account data)?
5. Child profiles: how to designate them, and hide the whole row for a child
   viewer (proposed) or filter titles?

### 5. Implemented locally (6 November 2026) — live reads and sync NOT enabled

Owner decisions: existing authorised account API; Option D; replace Friends
Watching; **Family Watching Off by default and "Share my watching activity" Off
by default, both synced** through the existing Better settings infrastructure;
PIN-locked and child profiles never shown; the row hidden on a child profile;
selecting a card opens details under the current profile.

**What is in the tree**
- `src/familia.h`, `src/familia.c` — pure rules and parsers: row visibility
  (`familia_linha_visivel`: on and not a child viewer), eligibility
  (`familia_perfil_elegivel`: another profile, no PIN, not child, opted in),
  newest-activity choice (in progress, or ≥ 90 % / marked watched = Recently
  Watched; ties prefer in progress), card build, newest-first order, labels
  ("In Progress" / "Recently Watched", "S2 E5 · 22m left", "Movie · 3 h ago"),
  and parsers for the existing `sync_pull_watch_progress` and
  `sync_pull_watched_items` response bodies.
- `src/familia_app.c` — state: main-thread snapshot of profiles and the gating
  settings (every 5 s; with the row Off nothing per profile is read, not even the
  consent flags); worker refresh in the Home catalogue cycle, throttled to one
  read per profile per 2 min, re-read when Home opens or a Family setting
  changes; cards dropped at once if their profile stops sharing, gets a PIN or is
  marked child; cleared on sign-out.
- `src/familia_ui.c` — Option D overlay on a 520×292 (16:9) card: avatar + name
  chip, status pill, title, S/E + time line, progress bar, play/check icon. The
  artwork is drawn by `home.c` through the ordinary landscape-card path, so the
  selected artwork provider applies; metadata through the same enrichment as
  Continue Watching (`descEnfeitar`).
- `src/home.c` — new `FILEIRA_FAMILIA` in the existing `social_activity` slot
  (same key, so Home order/visibility and the synced Home layout are unchanged);
  a plain details row: OK opens details, no resume, no card trailer, no watched
  badge or friends chip, no style menu. The empty Friends invite row is no longer
  injected; a Family row without cards is not shown.
- `src/descoberta.c` — the social worker now builds Family items instead of the
  Trakt/Nuvio friends feed. Items are **neutral** (no progress, no time left,
  plain title id), so the details page never sees another profile's state.
  `src/socialvis.c` ignores `familia:` items, so the friends panel is unchanged.
- Settings (`src/ajustes*.c/.inc`): `AJ_FAMILIA` "Family Watching" and
  `AJ_FAMILIA_PARTILHAR` "Share my watching activity", appended after Stage D,
  per profile, Off by default, in Home → rows next to the Friends style. Child
  designation reuses the existing "Child profile" mark (`perfilinfantil.c`).
  Another profile's consent is read from its own per-profile copy on this device
  (`ajustes_familia_partilha_perfil`).
- Sync: both keys are added to the portable Better map
  (`familyWatchingLocal`, `familyShareActivityLocal`; `JB_N` 91 → 93). Uploads
  still pass the J4 arming gate, which stays **disarmed**: nothing was written
  to any account. 3 new UI strings in all 29 tables.

**Deliberately NOT enabled (needs owner approval)**
1. **Live cross-profile reads.** No transport is registered
   (`familia_definir_busca` is never called), so the row has no cards on any
   device. Enabling = one function calling the two existing RPCs with the other
   profile's `p_profile_id` (`p_limit` small; first watched page), registered at
   start-up. Read-only, no new endpoint.
2. **Syncing the two keys live** = arming J4 on the test account and confirming
   they round-trip between Fire Stick and BlueStacks, as in Stage J.
3. **Consent across devices.** A profile's consent reaches another device only
   when that profile's Better settings are applied there (its own sign-in on
   that device). Reading another profile's settings document directly would be
   a second cross-profile read; not done.
4. **Home-open refresh** marks the data stale; the read itself happens in the
   next Home catalogue cycle. A Family-only fast refresh (like
   `desc_refazer_continuar`) is a follow-up once live reads exist.

**Tests:** `tests/familia.sh` (fixtures only; ASan/UBSan clean) and
`tests/settings_stage_h_familia.sh`; `tests/settings_stage_c.py` updated for the
two appended options. No device install.

### 6. Live reads enabled for the test account (owner-approved 7 Nov 2026)

- `src/familia_rede.c`: read-only transport over the two existing RPCs
  (`sync_pull_watch_progress` `{p_profile_id, p_limit:481}`,
  `sync_pull_watched_items` page 1 of 900) with the other profile's id and the
  current session. Registered only in **debug** builds and reads only for the
  account fingerprint `d21b69ba` (test account); any other account, signed out,
  HTTP errors or an account change mid-read give no cards. No write RPC exists
  in the file. Logs: fingerprint, profile number, HTTP status, row counts.
- `src/familia_app.c`: Home open / Family setting change starts a Family-only
  read on its own thread (at most one, throttled to 2 min unless a setting
  changed); Home is rebuilt only when the cards change. Fixed a wraparound in
  the "last read" time. One state log line when the row/eligibility changes.
- Tests: `tests/familia_rede.sh` (fake server) plus the updated
  `tests/familia.sh`, both ASan/UBSan clean.
- **BlueStacks (APK `c6f68708…18e8`)**, Profile 1 viewing, Profile 2 sharing:
  both RPCs HTTP 200, one card "Profile 2 · Recently Watched · MobLand · S1 E1 ·
  7 h ago" with the check icon and TMDB landscape art; OK opened MobLand details
  under Profile 1 (plain "Play", no profile switch, nothing written). Fire Stick
  untouched. Release builds still have no transport.

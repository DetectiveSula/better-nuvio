# Profile & Stats dashboard (A + C): implementation plan (Nuvio sync)

The design is the "Option A + C" board on the mock-up canvas:
https://claude.ai/artifact/2BVEKRYWmJDE7Vs2JfCcmU

What it is:
- One 1920×1080 screen with no scrolling.
- Header with five headline numbers.
- A 30-day bar chart (episodes and movies stacked), with a strip that shows the selected day.
- A 24-hour clock.
- Top-title posters.
- A "Your Mix" card (movies vs shows, top genres).
- Four small cards: longest binge, most active day, completion, on this day.

**Data source: Nuvio sync only.** Trakt and Simkl are not needed. Every logged-in Nuvio profile gets the dashboard.

Status (2026-11-07): **step 1 done, uncommitted.**
- New pure module: `src/perfilnuvio.c/.h`.
- `perfildados.h` gained the 30-day fields. The change is additive: the old fields are still filled, so the current screen could draw the snapshot as is.
- `app.c` now keeps the snapshot off the stack: worker `calloc`, main-thread copy `static`. The struct grew to about 70 KB.
- `tests/perfilnuvio.sh`: 53 checks pass, including with ASan/UBSan.

**Step 2 done, uncommitted.** In `app.c`:
- `pedirPerfil` copies watched items and this profile's progress rows on the main thread, picks the titles that need metadata (`perfilnuvio_ids_meta`), and pre-fills them from the library. The name and avatar come from `perfis_item_ativo()`.
- The worker completes metadata from a 64-entry cache, else `metaprov_meta` (at most 12 requests, 6 s each), then calls `perfilnuvio_montar`.
- Trakt/Simkl are used only when the user is not logged in, or when the account has no sync data and a tracker is on.
- While the screen is open, a new watched-items revision or a change in progress refreshes the screen. It checks every 5 s and rebuilds at most every 30 s.
- `tests/perfil_tracker` covers the Nuvio path, the metadata cache and stale replies.
- `perfilnuvio_meta_ler` parses Stremio `/meta`, with a test.

Not yet checked in the running app: no build or device run.

**Step 3 done, uncommitted.** No new shader mode was needed: `gfx_arco` (a `GFX_ANEL` arc) already draws an annulus sector.
- New `gfx_setor(cx, cy, r0, r1, ini, frac, rgba)` in `gfx.c`/`gfx.h`. It clips the quad to the sector's bounding box (`src/setor.h`, pure), so 24 wedges don't each fill a full square.
- `tests/setor.sh`: 2004 geometry checks.
- `tests/relogio_shot.sh`, GL: clipped = full quad pixel for pixel; peak, empty and gap colours checked. Capture: `docs/shots/profile-stats/step3-relogio.png`.
- Arc ends are now sharp (approved by the owner). The `GFX_ANEL` and `GFX_ANEL_GRAD` arc ends use a 1 px ramp measured in pixels along the arc, instead of 0.01 of a turn (3.6°, ~12 px of blur on a 200 px wedge). The ramp wraps correctly at the start angle.
  - `relogio_shot` asserts full colour 3 px inside an edge and background 3 px outside.
  - The player's progress ring and spinner use the same shader, so they get the same change. Capture: `docs/shots/profile-stats/step3-arco-player.png` (4×), clean and continuous across the top.

**Step 4 done, uncommitted.** The new screen is in `src/perfil_painel.inc`, included by `perfil.c`.
- **Layout:** header with a Friends pill; 5 numbers; day-by-day chart with the selected-day strip and a "Maior maratona" (longest binge) tag; 24-hour clock using `gfx_setor`; posters; mix; 4 facts.
- **Focus stops:**
  - Chart: Left/Right moves by day, starting on today. Day 0 is the left edge. OK opens the day sheet.
  - Posters: OK opens the title.
  - Friends pill: OK opens the comparison view.
- **Removed:** the calendar, the "most watched" list and the Friends card.
- **Kept unchanged:** the comparison view, the empty states and the footer notice.
- `perfil_definir_dados` validates every new index.
- **Screenshots:** `tests/perfil_shot.sh` builds its fixture through `perfilnuvio_montar` and saves `docs/shots/profile-stats/step4-*.png`: loading, dashboard, day, day sheet, posters, Friends pill, comparison view, no-identity (with ≈ and +marked), empty.
- **Tests updated to the new navigation:** `perfil_esquerda_barra`, `perfil_sair_roteador`, `perfil_pilula`. The left-edge (#371) and pill-offset guards still hold.
- **Known gaps:**
  - New strings are Portuguese keys with no translations yet (step 5). The English UI shows them in Portuguese until then.
  - The comparison view still labels my hours "assistidas neste mês"; they are now 30-day numbers.
  - Portrait/touch has rects and tap targets but has not been checked visually (step 5).

**Step 5 done, uncommitted.**
- **Translations:**
  - 52 new keys in `idioma_tab.h` plus all 29 sister tables (1508 translations), written in through `tools/idiomas.py --sincronizar`. The checker passes: 6447 entries × 29 languages.
  - The clock uses 24-hour numerals ("22:00", "00/06/12/18"), so it needs no key.
  - Episode codes use the existing `T%dE%d` key (English "S2E3").
  - The disconnected and empty-state copy now refers to Nuvio sync, in both `perfil.c` and `app.c`.
  - `tests/i18n.sh` has no hits in the profile files; its remaining hits are pre-existing, in other files.
- **Portrait/touch:** `NUVIO_SHOT_RETRATO=1 bash tests/perfil_shot.sh` builds with `-DNV_TOUCH_UI` and renders 1080×1920.
  - The header is now fixed above the scrolling column (before, the scroll clip cut it off).
  - Captures: `step5-retrato-*.png`.
- **English captures** (`NUVIO_SHOT_IDIOMA=1`): `step5-en-*.png`.

**Step 6: debug APK built, NOT installed.**
- APK: `build/android/Nuvio-2.0.8-android-profile-stats-debug.apk` (74.7 MB). SHA-256: `e763f0f6344eaf4637eecf088259736ec7cefc01d3baeb1c06ca5aa437e54c05`.
- arm64-v8a and armeabi-v7a both contain the dashboard strings.
- Built with `tools/android.sh --debug-only` (no release keys read), with two environment overrides on this Mac:
  - `JAVA_HOME=/opt/homebrew/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home`, because `~/.local/jdks` is empty;
  - rustup's toolchain first on `PATH` (`~/.rustup/toolchains/stable-aarch64-apple-darwin/bin`), because Homebrew's `rustc` has no Android targets.
- Debug signature: on a TV with the release APK it must be uninstalled first. Install only on the owner's "Install on BlueStacks".

Superseded: the "disconnected" message still says "link Trakt or Simkl". It now shows only when the user is logged out with no tracker. Reword it with the i18n step.

---

## 0. What Nuvio sync already gives us

`sync.c` pulls both of these for the active profile (`p_profile_id`). They are already in memory on the main thread.

| Source | RPC | In app | Fields we use |
|---|---|---|---|
| Watched items | `sync_pull_watched_items` (all pages, `puxarVistos`) | `contalib_n_vistos()` / `contalib_visto(i)` → `ContaVisto` | `id`, `tipo`, `temporada`, `episodio`, `vistoMs` (= `watched_at`) |
| Watch progress | `sync_pull_watch_progress` (`syncprog.c`) | `prog_ler()` → `ProgRegistro` | `contentId`, `tipo`, S/E, `posSeg`, `durSeg`, `lastWatchedMs`, `perfil` |
| Library | `sync_pull_library` | `contalib` items | `poster`, `genero` (only for saved titles) |
| Metadata | public meta add-on (`metaprov_meta`) | — | runtime, genres, poster for titles not in the library |

Progress rows are kept after 90% (Continue Watching only hides them). That makes `durSeg` the real runtime for most finished items.

### What Nuvio sync does NOT have, and how each stat copes

| Gap | Effect | Handling |
|---|---|---|
| One `watched_at` per item (no play log) | A rewatch replaces the earlier date; it doesn't add a play. | Count finished items, not plays. Label it "episodes" / "movies", never "plays". |
| `watched_at` is when it was marked watched (≈ end of play) | Start time unknown. | Start = `watched_at − runtime`. |
| **Bulk "mark as watched"** (a whole season at once) | Fake binges, fake 3 AM peaks, inflated hours. | Treat ≥3 episodes of one show within 120 s of each other as a bulk mark. Exclude them from hours, chart, clock and binge; show "+N marked watched" in the Episodes card. |
| No runtime on watched rows | Hours need a duration. | Fallback order: progress `durSeg` → meta `runtime` → 42 min per episode / 110 min per movie. If >20% of minutes are estimated, show a small "≈" on Hours Watched. |
| Partial watches | Only the latest position per episode is stored. | Count `posSeg` once for unfinished rows whose `lastWatchedMs` falls in the window. An approximation, but honest. |
| Genres | Not in sync rows. | Library `genero`, else meta for the top 12 titles by minutes; weighted by minutes. |

## 1. Decisions needed before coding

| # | Question | Recommendation |
|---|---|---|
| D1 | The current screen's **Friends card and "compare with a friend" view** (`desenharAmigos`, `desenharDuelo`). The mock-up has no room for it. | Keep the feature. A **"Friends" pill in the header** opens the existing comparison view. The comparison keeps reading `minutos` / `filmes`, which are now filled from Nuvio sync. |
| D2 | **Second colour:** `perfil.c` allows one accent colour; movies are amber in the mock-up. | Allow amber only as the movie data colour. Never use it for focus. |
| D3 | **Trakt/Simkl stats producers** (`trakt_perfil`, `simkl_perfil`). | Stop using them for this screen. Leave the code in place for one release, then remove it in a follow-up. Name and avatar come from the Nuvio profile (`perfis`). |
| D4 | Radial clock: `gfx` has **no arc/sector primitive**. | Add `gfx_setor()` (an annulus sector drawn as one triangle strip, ~2 h). Fallback: A's 24-bar strip. |
| D5 | **Bulk-mark rule** (≥3 episodes within 120 s). | Accept it; tune it against a test account's real data before release. |

## 2. Data

### 2.1 New pure module `src/perfilnuvio.c/.h` (no network, no SDL, fully testable)

```c
typedef struct { char id[24]; int minutos; char genero[160]; char poster[NV_ARTE_URL_MAX]; char titulo[128]; } PerfilMeta;

// Builds the 30-day snapshot. agoraMs and the local-time function are injected
// so tests pin the clock and timezone.
int perfilnuvio_montar(const ContaVisto *v, int nv,
                       const ProgRegistro *r, int nr, int perfil,
                       const PerfilMeta *m, int nm,
                       long long agoraMs, PerfilDados *d);
```

Steps inside:
1. **Events.** One event per finished item with `vistoMs` in [local midnight 29 days ago, now]. Then one partial event per unfinished progress row in the window that isn't also finished. Drop bulk marks (§0) and count them separately.
2. **Duration.** For each event: progress `durSeg` (key from `prog_chave`) → `PerfilMeta.minutos` → default. Flag estimated minutes.
3. **Totals.** `minutos`, `episodios`, `filmes`, `titulos` (distinct `contentId`), `minutosFilmes` / `minutosSeries`.
4. **Spreading minutes.** Each event's minutes go back from its end across local hours and days. This gives `minutosDia[30]`, `minutosFilmeDia[30]`, `minutosHora[24]`, `minutosSemana[7]` and `atividade[30]`.
5. **Streak and active days.** Computed over the rolling window (not reset on the 1st).
6. **Top titles.** Sort by minutes, keep 6. Poster: library `poster` → meta poster → metahub `poster/medium/<imdb>` (the pattern `trakt.c` already uses).
7. **Day items.** For each day, the top 3 titles by minutes, with episode range and first start time. Stored by index into `obras[]`.
8. **Binge.** Episodes of one show where each next start − previous end is under 30 min. Keep the longest.
9. **Completion.** Among progress rows touched in the window: share with `posSeg/durSeg ≥ 0.90`, or present in watched items.
10. **On this day.** Watched items with the same local month and day in earlier years; pick the newest.
11. **Coverage bits** (`cobertura`): `ESTIMADO` when estimated minutes are over 20%, `GENEROS` when at least one genre resolved.

### 2.2 `src/perfildados.h` v2
- Rolling 30 days: `PERFIL_DIAS 30` and `PERFIL_MAX_DESTAQUES 6`.
- New fields:
  - totals: `titulos`, `minutosFilmes`, `minutosSeries`, `marcados` (bulk marks)
  - per day / hour / weekday: `minutosDia[30]`, `minutosFilmeDia[30]`, `minutosHora[24]`, `minutosSemana[7]`
  - titles and day strip: `obras[24]`, `diaItens[30][3]`
  - single stats: `maratona`, `concluidas` / `iniciadas`, `hojeAnos`
  - coverage: `cobertura`
- Remove `primeiroDiaSemana`, `diasAtivosAno`, `parcial`, `anoCompleto`, `streakCompleto` and the month `periodo`.
- Size stays under 64 KB; it's copied by value from worker to screen.

### 2.3 Wiring (`src/app.c`, `carregarPerfil`)
1. **Main thread, on open.** Copy watched items (`contalib_visto` loop) and the profile's progress rows (`prog_ler`, filtered by `perfil`) into a `PerfilPedido`. These stores belong to the main thread / own their mutex, so copy before starting the worker.
2. **Worker.**
   - Find titles needing meta: the top 12 by provisional minutes plus any finished item lacking a duration, capped at 24.
   - Fetch with `metaprov_meta` (read-only, public add-on; reuse its cache).
   - Then call `perfilnuvio_montar`.
3. **Publish.** Keep the existing generation / profile / account checks before publishing.
4. **Refresh.** Re-run when `contalib_vistos_revisao()` changes or a progress push/pull lands while the screen is open (throttled to once per 30 s).
5. **Offline.** Build from the last pulled snapshot. If watched items were never pulled, show today's empty state with "Sign in to Nuvio sync".

## 3. UI (`src/perfil.c`)

### Layout
Rewrite to the board's measurements:
- Header: y 44, h 96, with the Friends pill on the right (D1).
- Numbers row: h 132.
- Middle row: h 372; chart 1150 wide, clock fills the rest.
- Bottom row: posters 840 wide, Mix 380 wide, highlights 2×2.

Keep `cartao`, `brilhoFoco`, `linhaFoco`, the `surge` cascade and `ajustes_conteudo_x()`. Delete `desenharAtividade` (the calendar).

### Focus stops

| Stop | Left/Right | OK | Up / Down |
|---|---|---|---|
| 1 Chart | day by day, starting on today; Left on day 0 → `sair` | day sheet: that day's ≤3 items, OK → detail (`perfil_item_selecionado`) | Up → Friends pill, else menu/clock as today; Down → stop 2 |
| 2 Top titles | poster by poster | detail (existing `escolhido`) | Up → stop 1 |
| 3 Friends pill | — | existing comparison view | Down → stop 1 |

Highlight cards are not focusable.

### Honest states
- Hours Watched shows "≈" when `ESTIMADO` is set.
- The Episodes card subtitle shows "+N marked watched" when there were bulk marks.
- A widget with no data shows a muted one-liner, not zeros. For example: "No viewing in the last 30 days", "Not enough history yet" (on this day).

### Other layouts
- **Portrait/touch** (`pfRetrato`): sections stack and the existing `toquerol` scroller handles scrolling.

## 4. Text and translations
About 25 new strings, for example:
- "Last 30 Days", "Hours Watched", "Titles", "Episodes", "Movies", "Current Streak", "%d of %d days active", "+%d marked watched"
- "Day by Day", "Time of Day", "peak", "Late night", "Evening", "Afternoon", "Morning"
- "Top Titles", "Your Mix", "Longest Binge", "Most Active Day", "Completion", "%d of %d finished", "On This Day", "Not enough history yet"
- "No viewing in the last 30 days"

Add them to `src/idioma_tab.h` and the 29 sister tables, then run `tools/idiomas.py`.

## 5. Tests
- **New `tests/perfilnuvio.c` (+ `.sh`):** fixtures of `ContaVisto` + `ProgRegistro`, with a pinned clock and timezone. Cover:
  - 30-day boundary
  - play crossing midnight
  - DST week
  - bulk mark excluded (and 2 episodes 100 s apart *not* treated as bulk)
  - duration fallback order and the ≈ threshold
  - partial rows counted once
  - binge gap of exactly 30 vs 31 min
  - hour buckets sum to `minutos`
  - on-this-day picks the newest earlier year
  - another profile's rows ignored
- **`perfil_tracker` test:** rewrite to assert the Nuvio path (no Trakt/Simkl calls) and the generation/profile guards.
- **Must still pass:** `perfil_pilula`, `perfil_esquerda_barra`, `perfil_sair_roteador`, `perfil_luz_arranque`, `stats_identity524`, `cw_stats_identity524`, `ordemperfil`, `amigoperfil_estados`.
- **Screenshots:** update `tests/perfil_shot.c` with fixture snapshots. Save to `docs/shots/profile-stats/`:
  - full
  - estimated/bulk-mark
  - empty
  - day sheet open

## 6. Order of work (each step builds and passes its tests on its own)
1. `perfilnuvio.c` + `perfildados.h` v2 + `tests/perfilnuvio` (no UI change).
2. `app.c` wiring (snapshot copy, meta fetch, refresh), with the `perfil_tracker` rewrite.
3. `gfx_setor` (or the strip fallback).
4. `perfil.c` layout, focus, day sheet, Friends pill.
5. i18n; portrait/touch pass.
6. Screenshots, full perfil suite, Android debug APK build. **No install without "Install on BlueStacks".**

Rough size: about 4 days. The data side is simpler than the Trakt version: no paging or API work, the inputs are already in memory.

## Owner rules that apply
- No commits, pushes or merges to master without an explicit ask.
- Never the main Nuvio account: fixtures for tests, a test account for manual checks. The feature only reads sync data and never writes to the backend.
- No APK installs without "Install on BlueStacks".

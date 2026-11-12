# Post-play page — official Nuvio TV layout (12 November 2026)

Owner asked for the post-play recommendations page to match official Nuvio
(`PostPlayRecommendationOverlay.kt`, `PostPlayOverlay.kt`, `PlayerScreen.kt`).

## Film ("More like this"), TV
- Video recedes to a 16:9 window, 32% wide, 64 px from the top-right corner (`player.c`, `destinoComRecuo`).
- The focused recommendation's backdrop fills the screen with official scrims (`posplay_fundo`).
- Bottom-left column: "Because you watched X", logo (Trakt) or title, genres • year • runtime • TMDB/Trakt rating, 3-line synopsis, then Details | ◀ ▶.
- Remote: Left/Right move between Details and the arrows (disabled arrow skipped); OK on an arrow changes recommendation (crossfade); Up focuses the video window, whose OK returns to the player; Down from the window returns to the buttons, from the buttons keeps the owner's "dismiss + controls"; Back dismisses.
- All 12 fetched recommendations are reachable (the old row stopped at 8).
- Difference from official: Details opens the title page; official's Play starts the film directly (no route from inside the player here).
- Trailer preview (Android, setting On) plays in a window mirroring the video's at the top-left, not full-screen, because it shares the Android layer with the receded video.
- Phone keeps the old poster row.

## Episode / post-credits / still watching
- Compact card bottom-LEFT (owner, 12/11; official 420 dp card mirrored): still, "Next episode", "S1 E4 • Name", "Playing in Ns", Play pill. Post-credits: Keep watching | Next episode. Still watching: "Are you still watching?" + Play.
- Fixed: Left/Right/Up used to fall through to the player and seek the video under the card.

## Data
- `extras.c` recommendations now also carry backdrop, synopsis, genres, rating (TMDB vote / Trakt rating) and, from Trakt, logo and runtime. Trakt related request uses `extended=full,images` (read-only).

## Tests / evidence
- `tests/posplay.sh` (new official-navigation and arrow-capture checks), `tests/dicacreditos.sh` (rewritten: card text fits in 31 languages × 5 states × 3 UI scales), `posplay_recuo`, `postplay_preview`, `extras_postplay`, `player_postplay_wiring`, `intro`: pass.
- `proxprefetch`, `velocidade`, `tpk_live_geometry`, `ponteiro_telas` (5 non-post-play screens) fail identically on committed HEAD. `player-parental-order-contract.cjs` not run (no node).
- Shots: `docs/shots/postplay/before/`, `docs/shots/postplay/after/` (`tests/player_glass_shot.sh`, `NUVIO_SHOT_IDIOMA=1`).
- APK: `build/postplay-official-20261112/` (not installed). Device behaviour unverified.

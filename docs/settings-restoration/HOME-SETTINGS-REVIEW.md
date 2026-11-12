# Home/settings defect review — 8 November 2026

Local changes on `codex/settings-restoration`; the existing dirty working tree is
preserved. No commit, branch switch, installation, publication, account access,
provider operation or backend mutation was performed for this review.

## Requested controls

| Control/problem | Finding and change | Verification |
| --- | --- | --- |
| Row spacing | Already consumed by `fileiraGap()` in Home row positioning, scrolling and Dynamic centring. Kept its existing factor, defaults and storage key. | Existing Home geometry tests exercise the spacing factors in all three layouts. |
| Modern sidebar flicker | Its opening spring never explicitly settled at 1, allowing tiny geometry changes to persist. Both animation endpoints now settle exactly, with opening animation retained. The actual sidebar also ignored its blur preference; the setting now selects frosted versus unblurred overlay material. This is a code-level stability correction, not confirmation of the owner's physical-device symptom. | Desktop/touch menu tests verify stable dimensions and velocity after settling, open/close endpoints and remote navigation. Physical Android TV/BlueStacks flicker check remains required. |
| Background previews overlap Logo colour | A fixed 150.75px thumbnail height was budgeted regardless of panel width, while rendering used responsive 16:9 previews. Measured height now follows the same width/aspect calculation, including captions and bottom padding. | Real settings interaction test checks narrow/expanded bounds. Native 1920×1080 screenshots inspected: previews/captions inside focus row, Logo colour below. |
| Hero catalogue stuck on Automatic | The chooser's remote navigation/persistence path works in the isolated interaction test. Fixed a stale-overlay edge case: edits were not reflected in the Home overlay when the J5 observer was detached. Such edits now update the local pending projection. Chooser opens on the selected Automatic/Random entry instead of Search. No claim that the precise installed-device failure was reproduced. | Actual OK/Down/Back input path selects Random and Automatic; two-device J5 mock checks catalogue selection with observer detached, then normal sync/profile/offline cases. |
| Hero preferences affect other cards | Ordinary landscape cards, Friends art, detail artwork/recommendations and paused-player backdrop read Home hero artwork preferences. Editorial/large-poster cards also read the hero genre toggle. Removed that coupling. Actual hero preferences keep their original consumers. | Home render tests keep poster/editorial genre, classification and synopsis unchanged when hero genre is Off; hero identity and recommendation-art regressions. |
| IMDb rating on posters | Previously used in Continue Watching but missing from other app-drawn Home card styles. Added one policy for all seven non-hero Home rating lookups, leaving the hero's separate rating control intact. | Actual editorial drawing with rating Off, plus policy and existing card regressions. Ratings baked into third-party poster images cannot be erased by this overlay setting; select a rating-free provider/template for those. |
| Tab style | Working, but scoped to streaming collections with catalogue tabs. Renamed English label to **Collection tab style** and clarified help; it does not control main app navigation. | Tests cover all five styles, unsupported/no-tabs fallback, top-menu geometry and input wiring. |
| Friends row style | Still controlled the old Friends renderer, while Family Watching uses a different renderer. Renamed English label **Family row style**. Family cards now consume the same saved six-value preference for feed, large landscape, portrait stories, presence strip, poster first and classic square layouts. Original Friends renderer remains available and wired. | Actual Family renderer exercised for all six styles: distinct dimensions, overlay visibility, text/card bounds and progress preservation. |
| Modern sidebar makes no difference in Modern layout | Floating geometry was selected entirely from the Home layout. Modern now uses the setting to choose floating sidebar versus docked drawer. Other Home layouts retain their menu geometry. | Real geometry selector checks On/Off on Modern; desktop/touch menu regressions. |
| Enriched context menu | Already functional: it gates the extended title-information card; Off retains the action menu. Non-title/row-only/channel menus intentionally have no extended card. | Actual production eligibility function tested On/Off, eligible title, missing IMDb, row/list menus and channels. No account/provider actions invoked. |
| Custom poster URL | Already implemented with HTTP(S) template validation, placeholders, persistence and redaction, but configuration was separated from its provider selector under Integrations. Moved the existing instance/token/parameters/key/template/test controls next to **Custom posters** in Layout; provider choice is now labelled **Custom URL template**. No new credential sync or settings IDs. | Actual settings URL setter/reload plus provider, long-template, encoded-placeholder, fallback and redaction tests. Use the poster service's image endpoint with supported placeholders such as `{imdb}`, `{imdb_id}`, `{tmdb}` or `{tmdb_id}`, not just its website URL. No live service validation performed. |

## Preservation and test evidence

Original 413 binding IDs, keys and default baseline remain unchanged. No new
preference IDs, sync document fields, account protocols or progress formats.
The custom-poster routing expectation explicitly changes only those six existing
controls from advanced Integrations to normal Layout; historical preservation
ledgers were not regenerated.

Evidence is under `build/settings-home-review-20261108/`. Screenshots:
`background-v2-fundo-opcao.png` and `background-v2-fundo-opcao-q90.png`.
The first sandboxed screenshot run could not initialise macOS video; the same
fixture ran successfully with native window access and fresh temporary data.

Two older harness defects were encountered and corrected without changing
application behaviour: long-poster input tests lacked the new input-test hooks;
the hero-identity unit unnecessarily invoked whole-account blob application,
pulling unlinked account/player transports into its narrow link set. It now
asserts the static CW-enabled baseline instead. The interaction test also exposed
missing icons for recently appended settings; explicit icons were added.
Initial failed logs are retained alongside reruns.

Final test results are recorded in STATUS.md and the local results manifest.
These tests do not certify every Better Nuvio setting or replace device testing.
The published/installed APK has not been rebuilt or updated with these changes.

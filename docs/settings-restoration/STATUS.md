# Settings restoration — Stage A

Updated 4 November 2026. Branch: `codex/settings-restoration`, based on
`fix/continue-watching-nuvio-parity` at
`09007f5893d63e61b42cdef86633f7bbef1cbf71`.
Official functional reference: `NuvioMedia/NuvioTV`, branch `dev`,
commit `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`.
Stage A is recorded in a local checkpoint commit on this branch. No master
changes, push, merge or release.

## References

See `references.json` for the completed source audit, comparison CSV/JSON, priority
findings, Native additional-control list, options and preservation ledgers.
`settings-comparison.xlsx` was unavailable; its CSV/JSON counterparts were used.
The screenshots inform presentation; the source/audit govern supported controls.
Reference paths are relative to the external audit bundle, with SHA-256 fingerprints;
personal machine paths and generated build artifacts are excluded from the checkpoint.

## Stage A implementation — complete locally

- Rebuilt the existing native SDL/gfx/text settings catalogue and landscape sidebar
  in this order: Account, Switch Profile, Appearance, Layout, Content & Discovery,
  Playback, Integrations, Tracking, Notifications, Advanced, About. Search and
  Changed settings remain utility entries above the categories. Existing submenus,
  modal editors and Panel/List preferences are retained.
- Watch Party is omitted because this client has no working implementation.
  Notifications contains existing in-app polls/notification preferences; this does
  not implement the official Android episode-alert/push service.
- Separated stable `AJS_*` scene identities and original artwork block identities
  from category positions in `ajustes_scenes.inc`. Updated generated previews,
  artwork lookup, category summaries, grouping, navigation and Live TV form routing.
  An option retains its original preview after moving category. Category summaries
  use the appropriate settings rather than an unrelated scene's summary.
- Added a dark sidebar with icons, rounded native panels and subtle blue focus.
  Right/OK enters a category; Left/Back returns to its sidebar row; Back/Left from
  the sidebar exits. Existing editor confirm/cancel, search return, dependencies,
  advanced-section reveal and focus restoration remain in the native event system.
  Sidebar rows remain readable and scroll the selected category into view when
  settings scale reduces the viewport. Existing option-list scrolling is retained.
  Pointer targets are clipped and disabled behind modal editors.
- Settings use English through the existing text system even if another app
  language is saved. Labels, help, static choices, language names, numeric units,
  feedback, add-on/plugin submenus and search output use English. English search uses translated labels,
  descriptions and choices; historical Portuguese search aliases remain accepted.
  A nested, thread-local rendering scope translates native text without changing
  saved app language, metadata language, language codes or worker-thread language.
- Added missing English translations for tab layouts, packs, a Continue Watching
  label/help, custom-poster help and platform/preview text. Guide settings paths
  derive from the live catalogue rather than the old category order; its settings
  introduction describes the new sidebar.
- Preserved all 413 Android binding IDs and keys, positional defaults and existing
  setting scopes. All 388 source presentation entries keep their platform gates,
  advanced/basic classification and original scene/art block. Internal/unsupported
  options remain hidden; no Legacy controls were removed or consolidated.

## Validation

The Stage A runner is `bash tests/settings_stage_a.sh` (local macOS SDL dependencies).
Its source tests compile the real catalogue/mapping for six configurations: native,
Android touch/keyboard, Android P2P motor, Tizen, Tizen 4.0 and Emscripten.
They check every presentation entry, category, gate, scene, artwork block and
advanced flag against the frozen preservation ledger. English coverage checks all
option labels, base help and static choices, with explicit technical-name exceptions.
The native interaction fixture checks remote category traversal/return, search,
English feedback and sidebar scrolling without starting an account session.

| Check | Result |
| --- | --- |
| `bash tests/settings_stage_a.sh` | PASS: 413 bindings/defaults/progress guard; 388 catalogue entries and all gates/scenes/art blocks/advanced flags across six configurations; English labels/base help/static choices; thread-local/nested English and unchanged worker language; native remote/search/scrolling/unit/guide routes |
| `bash tests/ajustes_secoes.sh` | PASS: 388 source entries, 11 categories, 85 advanced entries, each once |
| `bash tests/ajustes_padroes.sh` | PASS: defaults, setting scopes, option getters and persistence/migrations |
| `bash tests/ajustes_ux_dados.sh` | PASS: search, defaults, scopes, dependencies, current categories/art routing, private-value exclusion |
| `bash tests/ajustes_ux_interacao.sh` | PASS: editor drafts/cancel/confirm, persistence, dependencies, focus/Back, utility/category navigation and existing behaviour |
| `bash tests/cw_parity.sh` | PASS: progress, sync progress, account library, ordering/discovery, offline local progress, dismissal/unmarking and Simkl fixtures |
| `bash tests/cwordem.sh` | PASS: pure ordering, discovery ordering and home future-row split/rebuild/deduplication |
| Native / Android touch+keyboard / Tizen C syntax checks | PASS for settings, add-on and plugin UI sources |
| Native GL fixture screenshots | PASS: 1920×1080 layout, header/sidebar, editor/selectors and submenus captured |
| `git diff --check` | PASS |

The existing compiler warning about deprecated `ATOMIC_VAR_INIT` remains. Native
startup diagnostics report some deliberately hidden/platform-excluded options as
absent from TELA; the preservation matrix verifies their original gates rather than
exposing them to silence the diagnostics. These are not failed test assertions.

The older defaults/scope tests contained pre-existing expectations that disagreed
with the Continue Watching base. Scope expectations are now protected against a
frozen baseline from that unchanged commit, not changed in production. The CW
minimum remains the existing fixed 1% eligibility policy while preserving its
saved preference. Old tests now expect the restored categories rather than old
position-based artwork/grid routing. The CW dismissal fixture gained the missing
metadata-resolution stub required by its existing production consumer. The standalone
home-ordering fixture now configures only its two required settings through a test
initialisation shim, retaining real getters and next-up logic. This avoids pulling
account loading and GPU startup into an offline home-ordering test; blob/sync
compatibility remains exercised by the separate CW parity suite.

Native fixture screenshots: `build/settings-stage-a/settings-*.png`, including
`settings-english.png` at 1920×1080. They use bundled/sample data, not a personal
account. Homebrew SDL2, SDL2_image and SDL2_ttf were prepared locally for tests.

## Remaining work and boundaries

- Stage B: evidence-backed Legacy badge inventory, equivalence classification and
  any safe UI consolidation/migration. Stage A adds no Legacy badges.
- Stage C: Resolve Playable Links controls and actual resolver policy integration.
- Stage D: TorBox cache/playback investigation and explicit uncached-download flow.
- Stage E: reproducible episode-date investigation and focused fixes.
- Stage F: full platform builds, Android APK verification and physical-device tests.

No resolver, TorBox, episode-date or progress implementation was changed in Stage A.
No login/RPC/progress storage format, account configuration or provider credentials
were changed. No live TorBox operations were performed. No APK or release was made.
Original missing features remain missing; regrouping controls does not implement them.

The native screenshots and automated remote-event tests are desktop validation.
Android TV remote/hardware, LG Magic Remote/webOS and Samsung Tizen need real-device
validation. This work preserves the shared native architecture and conditional
controls; cross-platform catalogue/syntax checks do not establish device support or
screen-reader compatibility beyond the existing native text/focus model.

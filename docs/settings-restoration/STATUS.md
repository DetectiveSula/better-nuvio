# Settings restoration — Stages A, B, C and D

Updated 4 November 2026. Current focus: optional cached TorBox additions; see the final section. Branch: `codex/settings-restoration`, based on
`fix/continue-watching-nuvio-parity` at
`09007f5893d63e61b42cdef86633f7bbef1cbf71`.
Official functional reference: `NuvioMedia/NuvioTV`, branch `dev`,
commit `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`.
Stage A is recorded in a local checkpoint commit on this branch. No master
changes, push, merge or release. Stages B–D and the optional cached TorBox addition path are included in the local
checkpoint described in `CHECKPOINT-B-D.md`. Its parent is the Stage A checkpoint
`bd0b7edb120b042a8db2f2dd3980dd7b390ba403`; obtain its SHA from local Git history.

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

## Stage B — implemented locally, conservative provenance policy

The original source remains available locally at the pinned official SHA. The
comparison covers all 413 Android Native bindings and all 354 original control
records from the completed audit. See [LEGACY-BADGES.md](LEGACY-BADGES.md) and
[stage-b-inventory.json](stage-b-inventory.json) for the complete binding inventory,
defaults/choices/scopes, official counterparts, per-binding code references and
family classification evidence/fingerprints.

- Added an explicit allowlist for **125 verified additional Native controls**.
  Small neutral grey `Legacy` badges appear beside titles in native settings,
  changed-settings rows, choice-editor titles and ordinary/best settings search
  results. Width is reserved before clipping; option values retain their column.
  The badge registers no pointer/focus target and is a searchable English term.
- Original/shared controls remain unbadged, including theme/home layout, Trakt and
  Simkl, TMDB, primary subtitle controls, stream selection/preparation and existing
  shared providers. Unmatched audit rows are not automatically Legacy features.
- Moved cross-provider retry enable/order into Integrations beside debrid services;
  addon retry policies stay in Content & Discovery. Discord moved from Tracking
  to Integrations. All other additions were already integrated into the appropriate
  original categories in Stage A and remain there. No Legacy-only menu was added.
- Corrected audit equivalence assumptions for secondary audio, hero versus poster
  ratings, ASS versus tunneling and metadata date sources versus date formatting.
  Preserved independent controls when consumers differ. No storage consolidation,
  value migration, callback replacement or duplicate original-branded copy was
  introduced. Original inverse/manual-autoplay value semantics remain unchanged.
- Preserved all 413 IDs/keys/defaults and all 388 catalogue presentation entries,
  original scene/art identities, advanced/basic classification and platform gates.
  The original availability/visibility and save/load/mutation functions are frozen
  in the new Stage B wiring guard; backend consumers remain byte-identical.
- **83 bindings remain uncertain and deliberately unbadged.** The complete ledger
  lists each one. Further field-by-field/default/choice/consumer comparison is
  needed before claiming exclusive origin or merging it. This is an explicit
  remaining provenance limitation, not hidden removal of functionality. There
  are also 33 retained internal/runtime-hidden bindings, 153 with original
  counterparts, nine corrected shared/related controls and ten with explicitly
  different related behaviours. Counterpart mappings do not claim full runtime
  parity with every original Android option.

### Stage B verification

| Check | Result |
| --- | --- |
| `python3 tests/settings_preservation.py` | PASS: 413 existing bindings, keys, defaults and progress sources |
| `python3 tools/settings_legacy_policy.py` | PASS: generated switch matches explicit source-backed allowlist |
| `bash tests/settings_stage_b.sh` | PASS: complete inventory, badge positives/negatives and platform gates across six builds; frozen wiring/visibility; real isolated save/reload; equivalent-versus-distinct controls; Legacy search; moved-category remote focus/Back |
| `bash tests/settings_stage_a.sh` | PASS: original routing, scenes/art, English, scopes and remote/search/scrolling gates; three intentional category moves are separately recorded, original preservation ledger unchanged |
| `bash tests/ajustes_ux_dados.sh` | PASS: search, categories, defaults, scopes, dependencies and private-value exclusion |
| `bash tests/ajustes_ux_interacao.sh` | PASS: drafts, cancel/confirm, persistence, focus, dependencies and Back |
| `bash tests/cw_parity.sh` | PASS: progress, sync, local/offline data, tracking, completion/next-up/dismissal and Simkl regressions |
| `bash tests/cwordem.sh` | PASS: ordering, discovery and home future-row/rebuild tests |
| Android touch/keyboard and Tizen C syntax checks | PASS for settings and Spotlight sources |
| Native GL screenshots | PASS: generated and visually inspected at 1920×1080; grey badges fit between titles and scope/value text |
| `git diff --check` | PASS |

Native screenshot: `build/settings-stage-b/settings-english.png` (generated,
not tracked). No Android APK or platform application build was performed. Physical
Android TV/webOS/Tizen remote and rendering validation remains outstanding.
The Stage B guard deliberately freezes unrelated consumers during this stage;
later explicitly authorised implementation stages need focused compatibility tests
for intentional backend edits, not blind regeneration of this preservation baseline.

## Remaining work and boundaries

- Stage B provenance follow-up: resolve the 83 explicitly uncertain, unbadged
  controls before any further badge classification or consolidation. No settings
  were removed to hide uncertainty.
- Stage C: implemented locally for supported Native hash resolvers; see the results and bounded parity gaps below.
- Stage D: implemented locally, including cache labels and autoplay handoff; strict no-create account boundary documented below.
- Stage E: reproducible episode-date investigation and focused fixes.
- Stage F: full platform builds, Android APK verification and physical-device tests.

No resolver, TorBox, episode-date or progress implementation was changed in Stages A or B.
No login/RPC/progress storage format, account configuration or provider credentials
were changed. No live TorBox operations were performed. No APK or release was made.
Original missing features remain missing; regrouping controls does not implement them.

The native screenshots and automated remote-event tests are desktop validation.
Android TV remote/hardware, LG Magic Remote/webOS and Samsung Tizen need real-device
validation. This work preserves the shared native architecture and conditional
controls; cross-platform catalogue/syntax checks do not establish device support or
screen-reader compatibility beyond the existing native text/focus model.


## Footer correction before Stage C

- The category list now derives its bottom edge from the actual rounded list panel,
  reserving the footer height plus a 12 px gap. Scrolling uses that same edge;
  a row is drawn only when its complete height fits above the footer. This fixes
  the previous screen-relative edge that let the final row sit behind the footer.
- `bash tests/settings_stage_b.sh` passed immediately after this correction, before
  Stage C edits. Its interaction fixture tests the panel/footer boundary and rejects
  a partially obscured final row. Stage B passed again with Stage C integrated.

## Stage C — implemented locally

Functional reference remains official commit
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f` (local HEAD verified).
See [RESOLVE-PLAYABLE-LINKS.md](RESOLVE-PLAYABLE-LINKS.md) for the source evidence,
provider capability decision, manual exception and remaining parity gaps.

- Added Integrations → Connected Services → Resolve playable links (default Off),
  Resolve with, Prepare links and Links to prepare. These original controls have
  **no Legacy badges**. Existing Native provider configuration and all Legacy
  controls remain in place. Category/scene identities and remote return paths are
  retained; the provider editor excludes unavailable services for pointer and
  remote input, and refuses an unavailable value at the persistence boundary.
- Appended four control IDs after all original 413 Android bindings (417 total).
  Three new persisted keys are profile-local to this device. Prepare links is a
  derived toggle over the persisted count, not an independent boolean that can
  drift from it. Original IDs, keys, defaults, scopes and account/progress formats
  are unchanged. Provider credentials use their existing separate stores.
- Preparation defaults to 0; turning it On sets 2, Off sets 0, and count is clamped
  to 0–5. Disabled automatic resolution retains the chosen count/preference for
  re-enabling, but its effective preparation count is zero. No compatible configured
  provider disables the controls. Missing saved providers fall back to the first
  eligible service without erasing the saved preference. Failed saves roll back.
- Reused Native `debrid.c` through a central policy in `resolver_policy.c` for all
  automatic hash resolution, retry, renewal and preparation paths in `streams.c`.
  The selected automatic provider is TorBox or Premiumize, matching the pinned
  official visible resolver services. Unknown/cache-check failures fail closed;
  confirmed uncached hashes cannot enter automatic resolution. Cached addon URLs
  remain usable without forcing local hash resolution when the switch is Off. Read-only availability
  checks use no torrent creation, transfer, download or playable-link activation.
- Kept Real-Debrid/AllDebrid and Native provider order/manual P2P functionality behind
  an explicit per-source manual confirmation. The dialog defaults to Cancel and
  warns that allowing it can create provider items, start downloads and consume
  quota even with automatic resolution Off. Consent is single-use, expires, and
  is bound to the source-list generation and profile session. Back cancels. The
  confirmation renders above both Native sheet/island surfaces.
- New preparation count bounds the existing Native next-episode preparation path;
  prepared URLs are retained in its list and one winner is returned as before.
  Native provider/addon retry ceilings and filters still apply and may reduce
  the prepared count. Settings/context changes cancel stale work. Legacy source
  preselection and video preloading preferences retain their separate saved values.
- The Stage B provenance inventory still contains the original 413 records and 125
  positively classified Legacy badges; none were blanket reclassified. Original
  frozen defaults/ABI and progress hashes remain unchanged. The Stage B wiring
  check now allows only the explicit Stage C source snapshots in
  `tests/fixtures/settings-restoration/stage-c-wiring.json`, retaining its original
  before-snapshots and checking original function statements remain in order.

### Stage C verification

All provider API responses in the new resolver/availability tests are mocked;
no live debrid operation or provider download was performed. Tests use synthetic
keys and isolated temporary settings. The existing additional UX interaction
fixture attempted Trakt calls with synthetic credentials; restricted networking
rejected DNS resolution (zero response bytes). No user account or credentials
were loaded by these fixtures.

| Check | Result |
| --- | --- |
| Preservation guard | PASS: original 413 ordered IDs/keys, positional defaults prefix and progress source hashes |
| Stage A runner | PASS: category order, original 388 scene/art/presentation entries across six platform configurations, English choices/help and remote navigation; 392 current presentation entries |
| Stage B runner | PASS: footer boundary, 125 badges and negative cases, six platform configurations, scopes, original settings save/reload and moved-category navigation |
| `python3 tests/settings_stage_c.py` | PASS: exactly four appended IDs/defaults, all automatic call sites gated, read-only verification, one-shot consent routing above island surfaces |
| `tests/resolver_policy.c` | PASS: Off/no-provider/provider mismatch, unavailable preference fallback, cached/uncached/unknown, setting changes during calls and 0–5 clamp; provider functions mocked |
| `bash tests/debrid.sh` | PASS: all existing RD/TB/PM/AD fixtures plus strict hash/cache-response parsing, unknown/HTTP failures, read-only endpoint counts and credential redaction |
| `tests/settings_stage_c.c` native fixture | PASS: actual save/reload, no-provider/Off states, provider exclusion, persisted fallback, preparation toggle/count, save-failure rollback, scopes, category return and no Legacy badges |
| `tests/settings_stage_c_streams.c` native fixture | PASS: actual preparation limits 0–5, Legacy provider ceiling, disabled/stale work, read-only availability, remote Cancel/Allow and single-use/generation-bound consent; provider calls mocked |
| Existing `fontenota_preparo.c`, `stream_retry.c`, `ajustes_ux_dados.c`, `ajustes_ux_interacao.c` | PASS: native fixtures linked to current source objects; preparation fingerprint, direct URL/cache selection, retries, search/scopes, drafts/cancel/confirm/Back |
| `bash tests/cw_parity.sh` | PASS: progress, sync/account library, ordering/discovery, local/offline data, dismissal/completion/next-up and Simkl |
| Native, Android touch/keyboard and Tizen C syntax / `git diff --check` | PASS for the changed settings, policy, debrid, streams and app sources |

No Stage D TorBox root-cause/cache-label overhaul, episode-date change, APK/build
stage or real-device validation was undertaken. Linux, Android TV, webOS and Tizen
build/device validation remains Stage F. No commit, push, merge, release, master
change or Continue Watching branch change was made. HEAD remains the Stage A
checkpoint; these changes and the prior Stage B work are uncommitted.


## Stage D — cache status and autoplay implemented locally

See [TORBOX-STAGE-D.md](TORBOX-STAGE-D.md) for the traced flow, original-source
references, exact safety boundary and remaining device evidence needed.

### Root causes and changes

- Reproduced stale addon `foraCache` negatives being excluded before TorBox was
  checked. HTTP addon hashes in `hashTorrent` were also omitted from the Stage C
  verifier. Resolve now triggers an independent read-only annotation pass covering
  all pending hash sources, including addon-negative entries and URL-associated
  hashes. Original addon metadata is preserved byte-for-byte during annotation.
- Added explicit Cached/Uncached/Unknown/Error evidence and wired it into the actual
  source captions, badges, cached filter and effective autoplay eligibility. API
  failures/timeouts/missing or malformed data are Error, not Uncached; invalid or
  absent hashes are Unknown. Errors do not silently switch to another provider.
- TorBox cache checks follow the official read-only POST/object contract with up
  to 32 unique hashes per batch. Valid list responses remain supported. Strict
  root-level parsing prevents nested fields or malformed responses being mistaken
  for successful availability; an empty successful cache result means Uncached.
- Fixed a separate autoplay failure: Stage C's verifier could return success with
  no URL. The worker now resolves an existing ready account item to a playable URL
  before returning a candidate. Verified cached streams override stale addon
  negatives while retaining the original quality/decoder/source rules. Native
  quality caps and strict addon order are tested through the real selection path.
- Remembered/binge source selection now invalidates on stream-data revision as
  well as source-list size/generation. Cache updates can affect the preferred
  selection without waiting for another addon to add rows. Remote focus remains
  attached to its source through status-driven reordering.
- Tightened Stage C to meet the new **no automatic account mutation** rule:
  `add_only_if_cached` still creates an item, so automatic TorBox resolution now
  lists existing ready account items and requests a file link, using Native file
  selection. It never creates torrents/uploads magnets/adds downloads. Explicit
  manual confirmation remains the only stream-resolution exception.
- RD/AD remain manual-only for unsafe local-hash resolution. Existing service
  settings/order/manual operations are preserved; no unsafe automatic parity was
  added. Preferred provider, Off/defaults and all 413 original bindings plus the
  four Stage C controls retain their IDs, keys, scopes and defaults. No login,
  sync, tracking or progress format was modified.

### Stage D verification

| Check | Result |
| --- | --- |
| `bash tests/settings_stage_d.sh` | PASS: real addon → cache API → policy → actual caption → autoplay selection/URL; mixed Cached/Uncached/Unknown/Error, failures, provider preference/fallback, Resolve Off, quality cap/strict source order, stale list/settings/key responses and zero create/upload/add calls |
| Exact annotation metadata preservation | PASS: original Stream fields unchanged apart from the new transient cache-evidence fields |
| Native GL stream UI, both text modes | PASS: actual text-renderer assertions and visually inspected 1920×1080 screenshots showing all four states |
| `bash tests/settings_stage_a.sh` | PASS: complete settings routing/English/platform catalogue and native interaction regressions |
| `bash tests/settings_stage_b.sh` | PASS: Legacy provenance/badges, persistence, footer/focus and frozen settings wiring; Stage D exceptions retain the Stage C before hashes |
| `bash tests/settings_stage_c.sh` | PASS: resolver policy, mocked all-service backend, append-only controls/persistence and preparation/manual-consent regressions |
| `python3 tests/settings_preservation.py` | PASS: 413 original bindings/defaults and progress sources; no preference added during Stage D |
| `bash tests/stream_parser.sh`, `bash tests/torbox_aio.sh` | PASS: existing parser/root scoping, metadata and cached-mark regressions |
| `bash tests/cw_parity.sh` | PASS: progress/sync/account-library, ordering/discovery, offline data, dismissal/completion/next-up and tracking regressions |
| Native / Android touch-keyboard / Tizen C syntax and `git diff --check` | PASS |

Screenshots: `build/settings-stage-d/cache-status-native.png` and
`cache-status-addon.png` (generated/ignored). The Stage D provider fixtures supply
synthetic keys; no personal account is read. Provider HTTP is mocked and other
HTTP transport is disabled in the new integration/render fixture. No live provider
operation, torrent/download, commit, push, merge, release or master change occurred.

### Remaining limitations

- **Cached globally is not the same as already in the account.** Verified cached
  TorBox sources already present as ready account items can autoplay. An absent
  account item remains labelled Cached but requires an explicitly confirmed manual
  addition before Native can obtain its account-backed URL **in the read-only default**.
  The focused opt-in extension below now permits verified cached additions; this
  supersedes the initial Stage D prohibition when the new preference is enabled. The existing
  account scan is bounded to 800 items. Missing/unfinished/file-mismatched items do
  not turn a positive global cache result into Uncached.
- No raw failing device response/hash was supplied. The confirmed code defects are
  reproducible with mocked payloads; there is no claim that one explains every
  historical result in the user's account. Real Android TV playback/remote and
  webOS/Tizen hardware validation remains outstanding.
- Android's separate foreground preparation lifecycle remains unimplemented.
  Hashless opaque addon URLs without identifying metadata cannot be reliably
  classified from arbitrary URL text. Legacy file-browser operations are separate
  from this automatic cache/stream-resolution path.
- Stage E episode-date work and Stage F builds/APK work were not started. Changes
  remain local and uncommitted on `codex/settings-restoration`; HEAD remains
  `bd0b7edb120b042a8db2f2dd3980dd7b390ba403`. Continue Watching branch still points
  to `09007f5893d63e61b42cdef86633f7bbef1cbf71`.


## Focused Stage D extension — complete locally, validation passed

See [TORBOX-CACHED-ADDITION.md](TORBOX-CACHED-ADDITION.md) for the direct pinned-source
comparison, request contract, persistent duplicate handling and remaining limits.

- Added **Integrations → Connected Services → Allow cached TorBox additions**,
  default **Off**. The existing safe/read-only mode remains the default. Resolve
  must be enabled, TorBox must be the effective selected provider and autoplay must
  be enabled. Editing another profile cannot activate additions. The saved option
  survives restart but is ineffective when those conditions do not hold.
- Appended `AJ_TB_CACHED_ADD` / `torboxCachedAddLocal` after all previous 417 Android
  bindings (418 total). Original IDs, storage keys, defaults, scopes and functionality
  remain preserved. This new Better Nuvio policy control is not Legacy-origin and
  receives no Legacy badge. English help clearly discloses account changes, the
  possible download risk if provider cache state changes, and manual recovery.
- Reused Native multipart creation, file selection and link generation. After an
  authoritative account lookup, a fresh TorBox cache check must confirm Cached
  immediately before addition. Addon labels cannot authorise it. The request uses
  `add_only_if_cached=true` and `allow_zip=false`; there is no uncached fallback.
  Original/manual multipart defaults are retained.
- Serialised additions and added private, durable credential/hash-scoped receipts
  before POST. Pending/time-out/corrupt receipts prevent blind automatic resubmission;
  acknowledged IDs are reused. Receipt failure prevents addition. These runtime
  files do not contain plaintext credentials or signed URLs and are not synced.
- Newly added items must be ready and produce a non-empty HTTP(S) file URL before
  autoplay success. Queued/downloading/contradictory states, API errors, mismatched
  IDs/hashes and empty links fail safely. Provider/settings/profile ownership is
  checked throughout. Existing quality/source filters and autoplay semantics remain
  in control; manual uncached actions retain their explicit confirmation.
- Browser/Emscripten clients hide and disable this option because durable receipt
  semantics cannot be guaranteed there. Native clients also fail closed if app-data
  storage cannot durably record an attempt.

### Final validation — 4 November 2026

All commands below completed with exit status 0 against the current implementation.

| Check | Result |
| --- | --- |
| `bash tests/settings_stage_a.sh` | PASS: English labels/help, original routing/scenes/scopes/platform gates and remote navigation |
| `bash tests/settings_stage_b.sh` | PASS: original 413-control inventory, 125 Legacy badges, settings preservation, save/reload, footer/focus and explicit additive source snapshots |
| `bash tests/settings_stage_c.sh` | PASS: resolver policy, all-service mocked backend, preparation/manual-consent regressions, and real setting persistence including the default-Off opt-in |
| `bash tests/settings_stage_d.sh` | PASS: existing read-only cache statuses/autoplay, metadata preservation, provider selection, failure handling and zero automatic account additions with the option Off |
| `bash tests/torbox_cached_add.sh` | PASS: fresh provider verification, exact cached-only/ZIP-disabled multipart contract, globally cached item creation, acknowledged-ID readiness, non-empty autoplay URL, quality cap, concurrent duplicate suppression, corrupt/unwritable receipts, ambiguous timeout and reload recovery, and provider/Resolve/autoplay gates |
| `python3 tests/settings_preservation.py` | PASS: 413 original IDs/keys/defaults/progress sources; Stage C test also freezes the four existing additions and asserts exactly one new default-Off control |
| Resolver tests in Stage C (`tests/resolver_policy.c`, `bash tests/debrid.sh`) | PASS: safe-mode/provider/unknown/uncached gates and original RD/TB/PM/AD manual contracts preserved |
| `bash tests/cw_parity.sh` | PASS: progress/sync/account library, ordering/discovery, offline data, completion/dismissal/next-up and tracking regressions |
| `git diff --check` | PASS |

Verified explicitly:

- An absent saved preference loads **Off**. Enabling Resolve alone cannot add an
  item. The opt-in is ineffective with Premiumize selected, Resolve Off, autoplay
  Off, or while editing another profile.
- A fresh Uncached, Unknown or Error result causes **zero creation requests**,
  including a cache decline/error between the initial check and account addition.
- Ambiguous POST timeouts cause **one attempt only**, with suppression surviving
  settings reload/Off-On changes through the durable receipt. Six concurrent
  callers also produce one creation. Late accepted items can be found without
  another addition.
- A cached raw addon hash without an addon TorBox key can use the configured Native
  key, create its cached account item and return a playable URL through the real
  automatic selection path. A higher-resolution cached alternative outside the
  configured quality preference is not added just because it is available.
- The original inverse manual/autoplay setting and explicitly confirmed manual
  uncached-download path remain intact. Login, tracking and progress formats were
  not modified.

**Completion:** the focused implementation and requested automated validation are
complete. This does not claim a live-provider or physical-TV playback test. All test
provider requests used mocks; other HTTP transport is disabled in the integration
fixture. Public unauthenticated API-schema documentation was read for the contract,
but no authenticated/live account operations or real downloads were performed.

### Remaining limits and intentional differences

- Safe/read-only remains the default; regular Nuvio's cached-addition behaviour
  requires the extra opt-in here. Native also requires fresh cache evidence and
  explicit readiness, and retains its own file/episode-selection implementation.
- Pending or failed additions are not blindly retried. Some transient failures can
  therefore need explicit manual recovery; already accepted requests cannot be
  undone by switching the option Off. A late ready account item can be reused.
- Duplicate protection is local to this app data and credential. It cannot promise
  distributed exactly-once execution across different devices/API keys or other
  clients. Existing account discovery remains bounded to 800 items and readiness
  polling to three lookups; uncertain absence/readiness causes no automatic add or
  autoplay success.
- The addon must actually return the hash/source. Native cannot resurrect entries
  removed by AIOStreams' own Exclude Uncached filtering, and no addon configuration
  or API key is changed. Browser/Emscripten creation remains disabled. Native
  storage failures also prevent automatic additions.
- Physical Android TV/webOS/Tizen tests, platform builds/APK verification, Android's
  separate foreground preparation lifecycle/rate budgets and Stage E dates remain
  outside this focused change.

The completed work is included in the local Stages B–D checkpoint on
`codex/settings-restoration`; see `CHECKPOINT-B-D.md` for the reviewed file manifest
and fresh validation results. The Continue Watching branch remains at
`09007f5893d63e61b42cdef86633f7bbef1cbf71`. No push, merge, publication,
master change or destructive branch operation was performed. Stage E has not begun.

# Better Nuvio restoration — settings A–D and Home rows (Stage G)

Updated 8 November 2026. **Stage J is complete** (see "Stage J — complete" in the final section). Current work: focused pre-release validation approved by the user in place of exhaustive manual certification; see FOCUSED-RELEASE-CHECK.md. Individual unverified settings remain explicitly recorded. Existing Stage E/H work in the working tree has been preserved. Branch: `codex/settings-restoration`, based on
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


## Stage F — initial Android TV debug build and testing preparation

Completed locally on 4 November 2026, before Stage E. This is an initial build and
validation pass, not completion of every platform/device requirement in Stage F.

### Checkpoint and preservation

- Branch remained `codex/settings-restoration`; HEAD is the successful Stages B–D
  checkpoint `2bb366167fb9359b854303d6f096b85c80a1abf5`, with the requested commit
  message and all 59 reviewed files. Stage A is its parent
  `bd0b7edb120b042a8db2f2dd3980dd7b390ba403`.
- The original Continue Watching fix
  `09007f5893d63e61b42cdef86633f7bbef1cbf71` is an ancestor of HEAD, and the
  `fix/continue-watching-nuvio-parity` branch still points to that commit.
- No application source, setting IDs/keys/defaults, login/account/progress formats,
  signing configuration or package identity were changed for this build. Local
  configuration fingerprints, all branch refs and remote-tracking refs were checked
  unchanged. No reset, checkout, discard, commit, push, merge or publication occurred.
  Only this status document is modified; generated output stays ignored.

### Build configuration and result

Used the existing JDK 17, Android SDK platform/build-tools 35, NDK 27.2.12479018,
CMake 3.22.1, local Gradle/dependency cache, Rust compiler and installed Android
Rust targets. The existing ignored `build/android-local/build-debug.sh` configuration
provided workspace cache/Cargo/Gradle paths; no toolchains, keys or dependencies
were replaced. Build entry point was **`bash tools/android.sh --debug-only`**, with
`NUVIO_ANDROID_OFFLINE=1` and the existing local path overrides. No Lite option or
release task was enabled; Rust MKV core remained enabled for both ABIs.

The initial sandboxed attempt reached Gradle but was denied local socket access
(`java.net.SocketException: Operation not permitted`). The same offline build was
rerun with the necessary execution permission and succeeded. Gradle reported
`BUILD SUCCESSFUL in 43s`, followed by successful packaging verification. No
provider requests, account operations, real downloads or device installation occurred.
Existing SDL/native deprecation/compiler warnings did not prevent the build.

**Build limitation:** no embedded nuvio-engine P2P motor is configured or available
in this Mac setup, matching the prior build. The script reported P2P disabled and
both packaged libraries lack its marker. This is the standard non-Lite debug APK,
with Rust MKV and FFmpeg audio support, but it does not validate or supply the
optional embedded P2P engine. TorBox playable-URL playback uses the debrid path.

### APK verification

- Output: `build/android/Nuvio-2.0.8-android-debug.apk`.
- Application ID: `space.nuvio.nativelegacy.strictdebug` (unchanged).
- Version: `2.0.8-strictdebug`, version code `20008`; minimum Android API 24,
  target/compile API 35.
- Size: **74,492,446 bytes** (71.04 MiB).
- APK SHA-256: `792d50452bf28e21adfffed602b5646669e8d00ab8a9863098aa9a8178c8d9b3`.
- `apksigner verify --verbose --print-certs`: PASS, one RSA-2048 Android Debug
  signer, APK Signature Scheme v2 verified. Certificate SHA-256 matches the previous
  local debug APK; release signing was not read or used.
- Certificate SHA-256: `c0a433a85a1e3ca1fa9e1ef4b5998b36444b0cc8efc88c9a3ccb97d2f7c59bba`.
- ABIs: exactly `arm64-v8a` and `armeabi-v7a`. Each contains `libmain.so`,
  `libSDL2.so`, `libcurl.so`, `libjpeg.so`, `libwebp.so` and `libffmpegJNI.so`.
  ELF machine types match AArch64/ARM respectively.
- Android TV launcher: enabled, exported `space.nuvio.nativelegacy.Icone_original`
  alias targets `space.nuvio.nativelegacy.NuvioActivity`, with MAIN and
  LEANBACK_LAUNCHER intents and a TV banner. Touchscreen is not required.
- Assets: 1,086 artwork, 35 font and 8 licence files; P2P notices are present even
  though the optional motor is absent. The build's private-account/configuration
  filename exclusion check passed; ZIP integrity passed.
- `zipalign -c -P 16 -v 4`: PASS. `tools/android-apk-pages.py`: all six ARM64
  libraries passed 16 KB LOAD/RELRO checks; the six 32-bit libraries are outside
  that check's Android 16 KB requirement.
- Current source compilation lists include settings, streams/parser, resolver policy,
  TorBox cache/addition ledger and Continue Watching sources in both ABIs.
  Packaged `libmain.so` bytes exactly match the fresh Gradle stripped output.
  Both libraries expose the settings/Legacy, cache-verification, resolver, cached
  addition/receipt and autoplay entry points, and contain the new preference keys
  and UI strings. This verifies inclusion in the APK, not physical-TV behaviour.

### Fresh validation results

| Validation | Result |
| --- | --- |
| `python3 tests/settings_preservation.py` | PASS: original 413 bindings, defaults and progress sources preserved |
| `bash tests/settings_stage_a.sh` | PASS: category/scene routing, English, remote navigation and existing bindings |
| `bash tests/settings_stage_b.sh` | PASS: evidence-based Legacy badges, equivalence, platform gates, persistence and category interaction |
| `bash tests/settings_stage_c.sh` | PASS: resolver preferences, defaults, configured providers, persistence, read-only policy, 0–5 preparation and explicit manual override; includes `tests/debrid.sh` |
| `bash tests/settings_stage_d.sh` | PASS: addon/cache/policy, actual UI labels and autoplay URL; no automatic mutations in default mode |
| `bash tests/torbox_cached_add.sh` | PASS: default Off, selected-TorBox gates, fresh cache-only contract, Uncached/Unknown/Error rejection, readiness/URL, autoplay preferences and durable timeout/concurrency duplicate suppression |
| `bash tests/cw_parity.sh` | PASS: Continue Watching/progress/account regressions using fixtures |
| Android `testDebugUnitTest --rerun-tasks` | PASS: 61 tests, 0 failures, 0 errors, 0 skipped; 25 tasks executed, fresh run rather than only UP-TO-DATE |
| `bash tests/android_storage.sh` | PASS: durable user data, migration, APK install retention, LRU/clear/symlink handling and P2P budget fixture |
| `bash tests/player_android_retomada.sh` | PASS: exact resume, preparation, fallback, retained state, live, episode and profile behaviour with fixtures |
| `python3 tests/android_apk_cleanup.py` | PASS: startup/download/install APK retention |
| `python3 tests/android_interface_mode.py` | PASS: TV detection, saved choice, Auto, orientation and viewport |
| Native Android compilation and APK checks | PASS: both ABIs, signature continuity, assets, TV launcher, ZIP and 16 KB ELF alignment |

Android JVM tests were invoked with the same local Gradle properties (SDL sources,
staging/FFmpeg AAR and Rust MKV paths), offline mode and `testDebugUnitTest
--rerun-tasks`. Provider integration tests used mocks and disabled live HTTP transport;
no live TorBox or other debrid provider operations were used. Generated logs and
verification metadata are in ignored `build/stage-f/`, including
`android-build.log`, `android-unit-tests.log`, per-suite logs,
`native-android-checks.log`, `apk-verification.json`, `apk-signature.txt`,
`apk-manifest.txt`, `apk-badging.txt`, `apk-zipalign.txt` and
`apk-elf-alignment.txt`. They are local build evidence, not committed assets.

### Real Android TV checklist (user-run)

1. Open settings with the remote: check the sidebar order, English labels, submenus,
   footer clearance, scrolling/focus and Back. Shared/original controls have no badge;
   only verified additional controls have subtle Legacy badges.
2. Check existing Nuvio login, restart persistence, account syncing, profile switching
   and profile isolation. Confirm no saved preferences or progress have disappeared.
3. Resume a movie and an episode; check exact position, completed-item removal,
   next-up, dismiss/delete, ordering and offline-to-online progress sync.
4. Configure TorBox, enable Resolve playable links, choose TorBox as the resolver,
   and check Cached/Uncached/Unknown/Error labels. Verify no-provider disabled
   states and behaviour with Resolve switched Off.
5. With AIOStreams Exclude Uncached enabled and no TorBox key in the addon, use
   the app's configured key. Enable autoplay and check cached returned results play
   without the source picker, respecting quality/source/provider preferences.
6. Confirm Allow cached TorBox additions starts Off and persists after restart.
   Read-only mode can play ready existing account items; a globally cached item
   absent from the account may need the opt-in. Only enable it deliberately after
   reading the account-state/download-risk description. Check ready playable URL
   handoff and that ambiguity does not cause repeated additions. Avoid retrying an
   unresolved addition blindly; use the documented manual recovery path.
7. Record incorrect air-date examples in both Continue Watching and show details:
   series/provider ID, season/episode (including specials/anime numbering), displayed
   versus expected date, metadata/addon provider, timezone, and whether refresh
   changes the result. Redact credentials from any logs supplied for Stage E.

### Outstanding work

- This APK has not been installed, launched or played on a physical TV or emulator
  in this pass. Actual login/sync, remote performance, playback/codec/HDR and live
  provider behaviour require the user's device testing. No account mutation was
  used to validate cached additions; provider race risks and local-only duplicate
  receipt limitations documented above remain.
- Stage E is explicitly not started: episode air dates are unchanged and no date fix
  is claimed. Reproducible evidence from the checklist is still needed.
- Linux, webOS and Tizen builds/device checks were not performed in this initial
  Android pass. Broader Stage F platform/device validation remains open. The
  existing missing optional P2P engine is documented above, not silently supplied.
- `STATUS.md` is an uncommitted documentation update; the APK and logs are ignored.
  No push, merge, release, branch change, master modification or installation occurred.


## Post-Stage F — confirmed uncached source filtering parity

Implemented locally after the initial APK build, without starting Stage E. See
`STREAM-FILTERING.md` for pinned source paths, flow and evidence. Official HEAD
remains `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`; the repository/availability/
presentation/model files used were checked unchanged against that commit.

**Root cause:** original Nuvio's `StreamRepositoryImpl` annotates local cache
availability, then `DebridStreamPresentation.apply` removes locally resolved torrents
whose active provider confirms NOT_CACHED when Resolve playable links is enabled.
Native's Stage D correctly classified them and excluded them from autoplay, but its
visible source list still retained uncached rows at the bottom of each quality group.
AIOStreams' Exclude Uncached is a separate server-side preference and does not replace
this client-side presentation rule. No live addon response was captured, so no claim
is made about the exact server-side decision for each screenshot entry.

**Change:** a presentation-only predicate hides provider-confirmed uncached raw VOD
torrents without a ready URL. It uses active provider/profile/credential-scoped cache
evidence, not addon names or labels. It is shared by row filtering and provider counts;
these counts/layouts also invalidate on resolver On/Off, provider, credential and profile
changes. All underlying entries/classifications/metadata remain intact. Unknown/Error
and direct HTTP entries stay visible unless the user's existing explicit Cached-only
or source filters exclude them. Live filtering, manual resolution safeguards and the
working cached autoplay/cached-addition implementations are unchanged. No new setting,
key, account/progress format or default was added.

The existing Stage B/C/D/opt-in preservation snapshots were not rewritten. A separate
chained `stage-d-filter-wiring.json` records the reviewed UI-only change. The older
isolated source-sheet regression needed updated platform stubs, published revision
increments and the current row-height opening argument; its original ordering/focus/
layout assertions are retained and now pass.

| Fresh check | Result |
| --- | --- |
| `bash tests/stream_cache_filter.sh` | PASS: real mixed addon/cache/UI rows and counts, raw metadata/order, retained and removed focus, resolve Off/provider/key changes, HTTP/clientResolve/live exceptions, Unknown/Error/failures/timeouts/malformed/missing cache results, strict Cached chip and source filters; zero mutation calls |
| `bash tests/streamfit_sheet.sh` | PASS: stable groups, raw identities, freeze, incremental/focus, service rows and first-source behaviour |
| `bash tests/settings_stage_a.sh` | PASS |
| `bash tests/settings_stage_b.sh` | PASS: all 413 bindings/evidence classifications and explicit consumer-change chain |
| `bash tests/settings_stage_c.sh` | PASS: resolver controls, policy, preparation and mocked provider regressions |
| `bash tests/settings_stage_d.sh` | PASS: cache labels and cached autoplay URL pipeline |
| `bash tests/torbox_cached_add.sh` | PASS: opt-in gates, fresh cache verification, readiness, autoplay and duplicate suppression |
| `bash tests/cw_parity.sh` | PASS: Continue Watching regressions |
| `python3 tests/settings_preservation.py` | PASS: 413 original bindings/defaults/progress sources preserved |
| `git diff --check` | PASS |

All provider responses were mocked. No live TorBox operation, account mutation,
download, login/progress change, commit, push, merge, master change or device
installation occurred. The pre-existing Stage F documentation update is preserved.
The Continue Watching branch and the checkpoint HEAD remain unchanged.

**Build chronology:** the initial Stage F APK represented the checkpoint without
this filtering change. The subsequent rebuild below supersedes that file and includes
the filtering fix. On-device lists and any remaining direct-URL discrepancy need
testing with the same episode/addon configuration; retain credential-redacted example
objects if a mismatch remains. Stage E dates are unchanged.


## Android TV debug APK rebuilt with verified-uncached filtering

Rebuilt locally on 4 November 2026 on `codex/settings-restoration`, using checkpoint
`2bb366167fb9359b854303d6f096b85c80a1abf5` plus the preserved uncommitted filtering
fix/tests. The original Continue Watching commit remains included. No application
source or existing local configuration was changed during this rebuild.

Used `bash tools/android.sh --debug-only`, the existing local JDK/SDK/NDK/Rust,
Cargo/Gradle/dependency paths, and offline mode. Gradle completed successfully in
9 seconds, then the build script passed both-ABI packaging, private-configuration
exclusion and ELF alignment checks. Release signing/tasks were not used.

- APK: `build/android/Nuvio-2.0.8-android-debug.apk` (replaces the previous debug output).
- Package ID unchanged: `space.nuvio.nativelegacy.strictdebug`.
- Version: `2.0.8-strictdebug`, code `20008`; Android API 24 minimum, target/compile 35.
- Size: 74,492,609 bytes (71.04 MiB).
- SHA-256: `7d87a68d008a5b693b335b581a5339db38b083363566eea24123f2953583fdc2`.
- Signing: APK v2 verified; debug certificate SHA-256 matches the prior APK,
  `c0a433a85a1e3ca1fa9e1ef4b5998b36444b0cc8efc88c9a3ccb97d2f7c59bba`.
- Exactly `arm64-v8a` and `armeabi-v7a`, each with the six expected native libraries.
  Artwork, fonts, licence assets and enabled Android TV launcher/banner verified.
- ZIP integrity and `zipalign -c -P 16 -v 4` passed; all six ARM64 native libraries
  passed the 16 KB ELF LOAD/RELRO checks.

**Filtering-code inclusion:** both native debug outputs contain
`cacheOcultarConfirmada` and executable calls from the source-sheet filtering/count
paths (confirmed by DWARF and disassembly). Their ELF build IDs match the stripped
libraries, whose bytes match the libraries inside the APK. Both packaged `libmain.so`
hashes differ from the pre-filter APK. This checks the actual new code rather than
only the presence of settings strings.

| ABI | Native Build ID | Packaged `libmain.so` SHA-256 |
| --- | --- | --- |
| ARM64 | `d3a76b2d9e3545301d705ea691e8d434628cb443` | `a3eed9577c01a2c1f2b5ed11942e6367310aca1839f2d71b47c62879b3e901ea` |
| ARMv7 | `989e07096f2297c437c5fea9bad9276fb708a5ae` | `b053108ba6b561dac83c021974e947f339d168cd858e8094a61921627cb20e88` |

Android JVM unit tests were rerun with `testDebugUnitTest --rerun-tasks` in offline
mode: **61 passed, 0 failures/errors/skips**, all 25 tasks executed.

Fresh regression results (all exit code 0):

| Command | Result |
| --- | --- |
| `bash tests/stream_cache_filter.sh` | PASS: mixed cache states, actual filtered rows/counts, source order/metadata, focus, direct/live exceptions, failed checks, existing filters and zero mutations |
| `bash tests/streamfit_sheet.sh` | PASS: source-sheet grouping, identity, incremental updates and remote focus |
| `bash tests/settings_stage_a.sh` | PASS: categories, English and remote navigation |
| `bash tests/settings_stage_b.sh` | PASS: Legacy classification, original bindings, preservation chain and persistence |
| `bash tests/settings_stage_c.sh` | PASS: resolver/provider controls and gates, persistence, mocked debrid, manual consent and preparation limits |
| `bash tests/settings_stage_d.sh` | PASS: actual cache labels and cached autoplay URL handoff |
| `bash tests/torbox_cached_add.sh` | PASS: cached-addition opt-in, fresh cache checks, readiness/URL, autoplay preferences and duplicate/timeout suppression |
| `bash tests/cw_parity.sh` | PASS: Continue Watching regressions |
| `python3 tests/settings_preservation.py` | PASS: 413 existing bindings/defaults/progress sources |
| `git diff --check` | PASS |

Provider tests used mocked responses only. Final SHA-256 and source/configuration
fingerprints were rechecked after validation; only this status documentation was
changed by the rebuild task. Existing filtering implementation/test changes remain
uncommitted and preserved.

Build/signature/alignment/disassembly and per-suite evidence is in ignored
`build/stream-filter-apk/`. The local verification JSON records source hashes and
both packaged native hashes. No private configuration was added to version control.

No live provider request/account mutation, installation, commit, push, merge, branch
switch or master modification occurred. Stage E remains unstarted. The existing
configuration still has no optional embedded P2P engine; that prior limitation is
unchanged. Actual TV playback/remote/provider testing remains user-run.


## Stage G — Home row parity and duplicate fixes: validated locally

Updated 4 November 2026. Branch remains `codex/settings-restoration`, HEAD
`2bb366167fb9359b854303d6f096b85c80a1abf5`. The original Continue Watching fix
`09007f5893d63e61b42cdef86633f7bbef1cbf71` is still included; its branch and all
other refs are unchanged. Existing uncommitted stream filtering/build documentation
were preserved. See `STAGE-G.md` for the findings/risks recorded before implementation,
the approved hybrid merge strategy and pinned original-source references.

### Root causes and implemented behaviour

- Native's saved local order/visibility and small catalogue quota could override
  synced rows. Original Nuvio stores home order/disabled keys per profile and applies
  the `home_catalog_shared` payload. Native now uses a read-only runtime projection
  for shared states/order, retaining Native-only row slots and local customisations.
- Repeated exact row keys were accepted from saved preferences, giving one apparent
  catalogue multiple positions/quotas. Reads now fold identical keys only, preserve
  the first occurrence and leave the original file unchanged. An explicit later
  normalising save requires an exact `.pre-stage-g` backup. Different names are not
  identities; matching names with different IDs/types/configured instances remain.
- The editor formerly labelled every entry after the first queued item as queued,
  including later app/collection rows. It now partitions by actual state and uses
  the same effective order as discovery/Home, retaining focused row identity.
- Local fallback exclusions and collection membership could suppress a shared
  catalogue explicitly enabled by the profile. Synced state now wins both manifest
  quota selection and fetch eligibility. Ordinary content and collection rows stay
  separate; an enabled ordinary shared row is not silently consumed by a folder.
- Correct row keys alone were insufficient when Native's primary-addon override
  fetched another profile's sources. The official account inheritance flag now
  selects the home scope/cache. Valid synced authority also selects that matching
  addon source; the stored Native override remains a fallback. A cold first pull
  uses the existing retry mechanism to re-read the newly authoritative source.
- Source context includes selected/effective profile; received blobs carry their
  requested scope. Existing owner/session-generation checks and cache envelopes
  remain. Malformed, missing, failed and empty pulls preserve the last valid
  configuration; a matching cached snapshot is used offline, otherwise Native
  preferences apply. No new RPC, row upload or account-sync format was introduced.

The named Streaming Services, Genres, Top 25 Movies/Series, Trending Movies/Series
and Popular Movies/Series use their real configured catalogue/collection identities.
Tests include those example labels under synthetic fixture IDs; no hard-coded
replacement rows, guessed user IDs or title-based deduplication were introduced.
Friends Watching and other Native-exclusive rows keep selected-profile visibility,
ordering anchors, styles/sizes and persistence, including disabled preferences.
Shared enable/order controls explain their synced ownership; Native-only controls
remain editable. All original 413 bindings/IDs/defaults and progress formats survive.

### Removal of the small Android quota

No fixed ten-row storage/renderer maximum was found. Ten can be a saved Native
quota (default seven), which caused otherwise enabled catalogue rows to queue.
Android now loads enabled rows independently of that small saved quota, keeping the
old value untouched for recovery and showing Automatic in its informational limit
control. This does not force previously disabled Native-exclusive rows On.

Existing constraints remain: 200 ordinary catalogue rows within 224 catalogue slots,
8,000 item slots, 768 known row identities, plus explicit emergency session memory
protection. Non-Android TV allocation/quota behaviour is unchanged. Existing request
scheduling remains three concurrent requests and Android windows of up to 24, with
cached responses, on-demand pagination and lazy visible/nearby artwork. Initial
catalogue pages load progressively; no strictly viewport-only fetch scheduler or
unmeasured performance claim was added.

### Final regression results

| Validation | Result |
| --- | --- |
| `bash tests/home_rows_stage_g.sh` | PASS: 10/15/20/30 shared rows, 33 enabled rows/34 editor entries at maximum; exact shared ordering/visibility, Friends Watching and exclusive anchors/states/styles, collection distinction, real editor partition/focus under emergency queue, duplicate recovery, profile/inherited scope/cache/restart/offline fallback and exact Native-file preservation |
| `bash tests/syncaddons.sh` | PASS: actual sync worker/mock account transport, fresh home authority changes source from primary to selected profile 2, correct addon result, zero home/addon uploads; existing addon-sync cases retained |
| `bash tests/fileiras.sh` | PASS: existing Native row registry/ordering/persistence regression suite |
| `bash tests/fileiras_android.sh` | PASS: Android/other-TV bounds, 150 Android rows with no queue, emergency safety cap and bounded 24-row/3-request scheduling |
| `bash tests/catordem.sh` | PASS: order/enabled parsing, stable identities, large snapshots; empty pulls retain valid state to match pinned Android semantics |
| `bash tests/catordemcache.sh` | PASS: owner/profile cache isolation and recovery |
| `bash tests/colfileiras_sync.sh` | PASS: actual collection reconciliation, identity, shared visibility and profile isolation; runtime order checked without requiring Native storage reorder |
| `bash tests/addons_proveniencia.sh` | PASS: full configured-instance identity, same-name distinct catalogue IDs/types, repeated requests and stream provenance |
| `bash tests/homepos.sh` | PASS: Home rebuild/focus-position persistence and cache-versus-network behaviour |
| `bash tests/settings_stage_a.sh` | PASS: categories/scenes/English and remote routing |
| `bash tests/settings_stage_b.sh` | PASS: Legacy inventory, persistence and preserved consumer chain, with explicit reviewed Stage G scope evidence |
| `bash tests/settings_stage_c.sh` | PASS: resolver/preferences/providers, manual consent and preparation limits |
| `bash tests/settings_stage_d.sh` | PASS: cache labels and cached autoplay URL pipeline |
| `bash tests/torbox_cached_add.sh` | PASS: opt-in gates, fresh checks, readiness/URL, autoplay and timeout/duplicate safety |
| `bash tests/stream_cache_filter.sh` | PASS: actual visible rows/counts, mixed states, source preferences, order/metadata and focus |
| `bash tests/cw_parity.sh` | PASS: Continue Watching/progress/account regressions |
| `python3 tests/settings_preservation.py` | PASS: original 413 bindings/defaults/progress sources |
| `git diff --check` | PASS |

All executable suites above returned exit code 0. The Stage G queue/focus fixture
was additionally extended and rerun after the full suite pass; production sources
remained unchanged. Logs and input fingerprints are in ignored `build/stage-g/`.
All account/provider traffic in validation used fixtures/mocks; no live account
mutations, real provider operations or downloads were performed.

### Older Home integration harness — pre-existing link failure

`tests/colfileiras_home.sh` is not claimed to pass. It exits 1 with **62 undefined
service symbols**, including settings-loading/platform service dependencies. The
original script was also run against a **complete pristine export of checkpoint
2bb366167fb9359b854303d6f096b85c80a1abf5**, without switching/resetting this branch.
That build fails on the **identical set of 62 symbols**. Stage G did not introduce
this link failure. No permissive linker workaround or blanket mocks were used to
hide it. Evidence: `build/stage-g/home-harness-comparison.json` and the two harness
logs. The focused real editor/row model, collection sync, sync worker and Home
position suites provide passing coverage; this older harness remains a coverage gap.

### APK readiness and remaining limits

**Ready for a new Android TV debug APK build and device testing of the implemented
paths.** No Stage G APK was built or installed. The existing
`build/android/Nuvio-2.0.8-android-debug.apk` includes the stream-filtering fix but
predates Stage G. Rebuild it before expecting these Home changes on TV.

Thirty shared rows plus three Native/collection rows are preserved in automated
configuration/plan/editor tests, and the Android capacity fixture exercises 150.
This is not a claim that 33 rows have been rendered/scrolled on a physical TV.
Real-device focus/scroll/pagination, memory/performance and exact account catalogue
availability remain to be tested. Missing/empty/unsupported addon resources do not
receive fabricated content. Existing optional P2P engine/build limitations remain.

Native collection presentation remains. Non-default original Follow-addons-order
layout mode and remote custom row-title import were not added by this shared-order/
visibility phase. Multiple configurations of an addon retain existing URL-based
instance keys; the remote legacy key addresses the first instance, so independently
syncing every parameterised variant is not claimed. No identity-format migration or
unsupported remote payload was invented. Other-platform builds/devices remain
unverified; genuine structural/emergency limits still apply.

All changes remain local/uncommitted. No commit, push, merge, publication, master
modification or original Continue Watching branch change occurred. Apple TV visual
redesign and Stages E, H and I were not started.

## Stage I — cross-device settings sync (4 November 2026)

Implemented a **verified subset of compatible bidirectional settings sync** on
`codex/settings-restoration`, retaining the existing Stage G changes. HEAD remains
`2bb366167fb9359b854303d6f096b85c80a1abf5`; the original Continue Watching commit
`09007f5893d63e61b42cdef86633f7bbef1cbf71` remains an ancestor. Nothing was committed.

The pinned original `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f` was examined before
implementation. [STAGE-I.md](STAGE-I.md) records the actual profile-settings RPCs,
original export/exclusion rules, Native compatibility risks and implementation
policy. [stage-i-official-sync.json](stage-i-official-sync.json) captures all 13
original feature families and explicit device/credential exclusions.

### Implemented and preserved

* **31 original-format boolean controls** synchronise per selected profile:
  17 layout/Continue Watching/display controls, pause overlay and player clock,
  and 12 existing TMDB switches. See
  [stage-i-inventory.json](stage-i-inventory.json) for every key, original default
  expression, Native getter and actual runtime consumer. No new settings bindings,
  changed defaults, enums, storage keys or server fields were introduced.
* Incoming values use exact feature/key/type boundaries. Outbound updates use
  `sync_push_profile_settings_blob`, `p_profile_id`, `p_platform: tv`,
  `p_settings_json` and the original `p_origin_client_id`, reusing Native's existing
  persisted client identity. Only pending shared edits are merged into a fresh
  successfully pulled blob. Unknown remote data survives; new Native/device cloud
  snapshots and credential uploads are not generated.
* Durable account/profile journals preserve pending edits and last-known values
  across offline operation, restarts and switches. Missing/malformed/partial data
  does not reset settings. In-flight edits survive acknowledgements. Fresh matching
  reads resolve ambiguous write timeouts without duplicate uploads. Old-session
  responses cannot publish into another context. Imported values do not echo.
* Existing account language/badge/source-rule read paths are retained with stricter
  boundaries; local language choices and source-rule protection remain. Native-only
  settings, provider secrets and device-specific controls remain local.
* Stage G hybrid row ordering, Friends Watching/exclusive rows, Native preferences,
  collections, source scope and >30-row capacity are retained. No home preferences
  are uploaded or replaced. Resolver/TorBox/filtering/login/progress formats were
  not changed. Original 413 bindings/defaults and appended resolver controls remain.

### Validation

PASS: `tests/settings_stage_i.sh` (production bridge plus actual sync worker with
mocked HTTP), `tests/settings_preservation.py`, complete `settings_stage_a.sh`,
`settings_stage_b.sh`, `settings_stage_c.sh`, `settings_stage_d.sh`,
`torbox_cached_add.sh`, `stream_cache_filter.sh`, `cw_parity.sh`,
`home_rows_stage_g.sh`, `syncaddons.sh`, `fileiras.sh`, `fileiras_android.sh`,
`catordem.sh`, `catordemcache.sh`, `colfileiras_sync.sh`,
`addons_proveniencia.sh` and `homepos.sh`. Stage I also checks fresh-process restart,
inactive-profile edits, per-field merging, original-format writes, device/Native
preservation, malformed envelopes, offline/retry and timeout acknowledgement.
The reviewed Stage I wiring overlay retains every original persistence statement.
Evidence/logs are under ignored `build/stage-i/`.

### Remaining differences and build readiness

**Full original settings sync parity is not complete.** Unverified string/enum/unit
conversions, theme variants, subtitle styles, explicit language choices, autoplay
mode/source variants, resolver policy, tracking and other unlisted original fields
remain local/read-only. Stage G home sync remains read-only; no additional remote
home writes were added. Three candidates (`AJ_NOME_ADDON`, `AJ_GRAD_CLASSICO`,
`AJ_DET_TRAILER`) lack a traced runtime consumer outside settings and are excluded.
Their existing controls/bindings were preserved. No cosmetic functionality was added.

The pinned whole-blob protocol has no client-visible revision/CAS: it cannot prevent
another device writing between pull and push. Missing defaults are not inferred;
remote reset-by-deletion does not clear Native. Original 1.5-second observer timing
is not reproduced: existing Native account-sync scheduling/backoff is used.
Historical automatic Native/device snapshot tests (`ajustes_sync.sh`, `syncjank.sh`)
assert the unsupported cloud-extension behaviour now deliberately bypassed; they
are not claimed to pass unchanged. Replacement protocol tests cover the new path.
The older Home harness's pre-existing link failure remains documented in Stage G.

**The implemented Stage G + I paths are ready for an Android TV debug APK build and
device testing, with these explicit sync limitations.** No APK was rebuilt or
installed. Real regular-Nuvio/Better-Nuvio cross-device account testing, focus/scroll
under sync updates, offline reconnection and other-platform device validation remain.
All account/provider validation used mocks; no live account mutations occurred.
No commit, push, merge, release, master/original Continue Watching branch change,
or Stage E/H work was performed.

## Stage G + I Android TV debug APK build (2026-11-04)

**Result: PASS.** Debug APK built from the current working tree (branch
`codex/settings-restoration`, uncommitted Stage A–D, G and I changes included).

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,502,791 bytes (71 MB) |
| SHA-256 | `2348402c69f186332ed490f32991651dd1bc3512998f0848d685fc194f8a4163` |
| Built | 2026-11-04 21:14 GMT, `tools/android.sh --debug-only` |
| Package | `space.nuvio.nativelegacy.strictdebug`, versionCode 20008, versionName `2.0.8-strictdebug`, debuggable |
| SDK | minSdk 24, targetSdk/compileSdk 35 |
| ABIs | `arm64-v8a`, `armeabi-v7a` (libmain, SDL2, curl, jpeg, webp, ffmpegJNI in each) |

Pre-build regression gate (`build/stage-gi-apk/tests.json`): all 10 suites exit 0 —
`settings_stage_i`, `home_rows_stage_g`, `settings_stage_a`–`d`, `torbox_cached_add`,
`stream_cache_filter`, `cw_parity`, `syncaddons`.

### Build recovery

The first attempt (`build/stage-gi-apk/build.log`) stopped at step 3/5: the native
dependency prefix `~/.cache/nuvio-android/prefix` had never been built on this Mac
(`arm64-v8a/libcurl.so ausente`). `tools/android/deps.sh` was run (curl 8.22.0 +
mbedTLS 3.6.7, libjpeg-turbo 3.1.4.1, libwebp 1.6.0 for both ABIs; `deps.sh: OK`).
The second attempt (`build-rerun.log`) failed in the Rust mkv core:
`tools/rust-mkvcore.sh` defaults `RUSTUP_HOME`/`CARGO_HOME` to an absent external
SSD, so Homebrew's standalone `rustc` (no Android std) was used. The final build
(`build-rerun2.log`) ran with environment overrides only — `JAVA_HOME` = Homebrew
OpenJDK 17.0.21, `RUSTUP_HOME=~/.rustup`, rustup proxies from
`/opt/homebrew/opt/rustup/bin` first on `PATH` (the Android Rust targets were already
installed there). No source, script or Gradle file was changed.

### Verification

* **Stage G and Stage I in both ABIs:** each `lib/<abi>/libmain.so` contains the Stage G
  strings (`Synced Home rows follow this profile…`, `Managed by your synced profile…`,
  `Enabled Home rows load automatically…`) and the Stage I shared-settings keys from
  `src/ajustes_shared.inc` (`modern_hero_full_screen_backdrop`,
  `focused_poster_backdrop_expand_enabled`, `fast_horizontal_navigation_enabled`); none
  exist in HEAD. Both `libmain.so` were relinked at 21:13–21:14, after the last source edit.
* **Signing:** `apksigner verify` passes, v2 scheme, one signer, Android Debug
  certificate (SHA-256 `c0a433a8…c59bba`). Not the release key; the `.strictdebug`
  suffix installs beside, not over, the release app.
* **Launcher:** enabled alias `Icone_original` -> `NuvioActivity` with MAIN +
  LAUNCHER + LEANBACK_LAUNCHER; app icon and TV banner set; the nine other icon-switch
  aliases ship disabled. Touchscreen and leanback marked not required.
* **Assets:** 1086 `assets/art`, 35 `assets/fonts`, 8 `assets/licencas` entries; the
  script's personal-data/secret filter found 0 matches.
* **Alignment:** `zipalign -c -P 16 4` passes; all six arm64 `.so` have 16 KB LOAD
  alignment (`android-apk-pages`: 6 libraries, 0 failed).
* **Not included:** P2P motor (no motor folder on this Mac, `motor P2P: DESLIGADO`).
  This is the script's normal no-folder behaviour, not a Stage G/I change.

Not installed or device-tested. No commit, push, merge, release, master change, live
account mutation or new development stage was performed.

## Stage G + I real-device fixes (2026-11-04)

The first Stage G/I debug APK failed on the TCL: Home rows did not match regular
Nuvio, and a setting changed in regular Nuvio (Resolve Playable Links) never
reached Better Nuvio. Investigated against the pinned NuvioTV source
(`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`) and Better Nuvio's runtime paths.
No live account was read or written; all evidence is source and mocked tests.

### Root causes

**Stage I (settings).**
1. *Resolve Playable Links was never synchronised.* It is NuvioTV's
   `debrid_settings.debrid_enabled` (DebridSettingsDataStore, profile-scoped,
   default false). `debrid_settings` is a synced feature; only the provider API
   keys are excluded. It was not one of the 31 Stage I fields, so the value
   regular Nuvio pushed was received and ignored. This is the definitive cause of
   the reported test.
2. *Refresh timing.* Better Nuvio read the account only at startup and every
   5 minutes, never on resume or after playback. On Android TV, reopening an app
   whose process is alive only resumes it. (Pinned NuvioTV itself pulls profile
   settings and the Home snapshot only in its startup/forced sync; its
   resume/15-minute pulls cover watch state and library only, and
   `requestForegroundPull`/`requestRealtimeSurfacePull` have no caller at this
   commit.)
3. *No diagnostics.* An applied, skipped or refused account document left no log
   line, so device testing could not tell which happened.

**Stage G (Home).**
1. *Hybrid order was the inverse of regular Nuvio.* Native order came first and
   synced rows were only shuffled among the slots they already held.
   NuvioTV (`HomeViewModel.rebuildCatalogOrder`) uses the synced order, then
   unsaved catalogues in addon/manifest order, then unsaved collections.
2. *Follow Addons Order* (`layout_settings.follow_addons_order`, selected
   profile) was not implemented.
3. *`showInHome: false`* manifest catalogues were not excluded (official
   `shouldShowOnHome`); they appeared as extra rows outside the synced list.
4. *pinToTop collections* (Collection.pinToTop) were not placed above all ordered
   rows as in `computedHomeRows`.
5. *Identity.* Synced keys lower-cased every type, and manifest types were cut to
   7 bytes; NuvioTV's `catalogTypeKey` lower-cases only movie/series/channel/tv.
6. *Unlisted rows* kept Native's automatic hiding (collection membership,
   account-visibility marks) although regular Nuvio shows them.
7. A synced "disabled" was only honoured for rows already registered locally.

**Why the earlier tests passed.** Stage G tests injected synthetic row keys
directly into the Native rows file and built the synced snapshot from the same
keys, so identity always matched; they never used real manifests, `showInHome`,
follow order or pinned collections, and they asserted the hybrid order itself.
Stage I tests used hand-written payloads with stubbed account/profile context and
never exercised resume, playback or which settings a user actually changes.

### Fixes

* `src/ajustes_shared.inc`: 32nd field `{AJ_RESOLVE_LINKS, debrid_settings,
  debrid_enabled}` (stored choice syncs; effective state still needs a configured
  TorBox/Premiumize resolver on each side). Upload merge now covers every mapped
  feature. `follow_addons_order` imported read-only and kept in the per
  account/profile journal. One privacy-safe `[ajustes-conta]` line per document
  (counts and official field names only) and per upload; refusals name a fixed
  reason (`ajustes_compartilhados_motivo`).
* `src/sync.c`, `src/main.c`, `src/app.c`: `sync_pedir_atualizacao` on
  `SDL_APP_DIDENTERFOREGROUND` and on the player-closed frame, using
  ProfileSettingsSyncService's foreground timing (2.5 s delay, at most once a
  minute since the last cycle start), never during playback, respecting backoff.
* `src/fileiras.c`: `contaOficial` reproduces `rebuildCatalogOrder` (both modes,
  including `normalizeCollectionBoundaries`) and pinned collections. Continue
  Watching leads; Native-exclusive rows (Friends Watching, Last Session,
  non-account catalogues) follow the shared row they followed in the Native
  order. Unlisted shared rows are shown unless hidden by an explicit TV choice.
  Native files, row sizes/styles and offline cache are unchanged.
* `src/descoberta.c`, `src/home.c`: no remote pre-sort while the projection is
  active (fil_unir owns the order); `showInHome:false` handled like search-only
  catalogues; full-width type for row identity; same unlisted-row visibility.
* `src/catordem.h/.c`, `src/addons.c`: shared `catordem_tipo_chave` identity on
  both sides. `src/colecoes.*`, `src/colfileiras.c`: pinToTop parsed and published.

### Safe mode and strict payload rejection

Safe mode cannot undo a synced setting: its undo list (`RISCOS`) holds only
resource-heavy controls, and synced values do not pass through `seguro_mudou`.
Two crashes within 60 s without a culprit do cap Home at 12 rows for the next
session (`[seguro] ... PERFIL SEGURO`), which would make a long Home differ from
regular Nuvio for that session; the safety check is unchanged. The strict
validator accepts a complete 13-feature NuvioTV export (all value types,
exponents, 64-bit integers, escapes, braces inside strings, JSON-in-string);
refusals are now logged with their reason. Neither explains the reported failures.

### Tests

New: `tests/settings_stage_i_official.c` (full NuvioTV export, Resolve Playable
Links both ways, follow-order persistence/isolation, refusal reasons, log
privacy), `tests/home_rows_official.sh` (realistic Cinemeta/AIOMetadata/TMDB
manifests and a `buildHomeCatalogSyncPayload` payload through the real
catordem/fileiras code, compared with a port of the official ordering, both
follow modes, plus a TV-personal-hide variant), the `primeiroplano` scenario in
`tests/syncaddons.c` (real sync worker: resume, 2.5 s delay, 1-minute limit,
player close), and wiring checks. Updated: the Stage G order assertion (Friends
Watching is now anchored, not fixed at index 2), stubs in `syncaddons.c`,
`settings_stage_i.c` and `fileiras_removidas.c` (its `catordem_estado` stub was
missing since Stage G).

Prioritised regression (83 scripts; the full 222 were not rerun): **51 pass,
including every test of changed code** (`settings_stage_i`, `home_rows_official`,
`home_rows_stage_g`, `syncaddons`, `fileiras*`, `catordem*`, `colecoes`,
`colordem`, `collections`, `colfileiras_sync`, `homepos`, `ordemperfil`,
`desc_*`, Stages A–D and interactions, `torbox_cached_add`,
`stream_cache_filter`, `cw_parity`, `seguro_ajustes`, `settings_preservation`).
**32 fail, none introduced today**, each compared with a clean `git archive HEAD`:

| Failures | Cause |
|---|---|
| 30 also failing at HEAD: link errors (`abrirtitulo`, `addonurl`, `aiostream_instances`, `addonslista`, `colfileiras`, `synccolecoes`, `syncordem`, `pinrows`, `colecoes_teto`, `home`, `home_progressiva`, `homerefresh`, `sync_aplicar_perf`, `sync_resumo_corrida`, `troca_perfil_espera_conta`, `contaoffline`, `contaoffline_limites`, `montagem_cedo`, `montagem_estrutura`), Portuguese-UI/scope assertions after Stage A/C (`ajustes_aviso_legal`, `ajustes_creditos`, `ajustes_escopo`, `ajustes_fonte`, `ajustes_idioma_disco`, `ajustes_iptv`), stale source greps (`troca_perfil_esquece_contalib`, `troca_perfil_repete_desc`), `sync_prioridade` (debrid.h prototype conflict), `tmdb_idioma_conta`, `legconta378` | Pre-existing fixture/source drift; not fixed (unrelated) |
| `ajustes_sync`, `syncjank` (pass at HEAD) | Assert automatic `c_settings_v1`/`c_devices_v1` snapshot upload/restore, deliberately removed by Stage I (documented above) |

Where a link failure also lists one of today's new symbols, the fixture was
already missing Stage G/I or older symbols. Logs: `build/stage-gi-fix/`.

### Still requires real-device verification

Rebuild the debug APK and on the TCL, same account/profile in both apps:
toggle Resolve Playable Links in regular Nuvio, return to Better Nuvio and wait
about 3 s (look for `[ajustes-conta] ... aplicados 1 [debrid_enabled]`); toggle
it back in Better Nuvio and restart regular Nuvio; compare Home rows top to
bottom with Follow Addons Order off and on; check pinned collections, Friends
Watching and Last Session placement; confirm no `[seguro] PERFIL SEGURO` line.
Regular Nuvio applies Better Nuvio's changes only on its own startup sync.
Rows over the Android 200-row ceiling, empty catalogues and manifest types
longer than 7 bytes (row identity now correct, request type still 7 bytes) can
still differ. No APK was built or installed; nothing committed or pushed.

## Stage G + I fix debug APK build (2026-11-04)

**Result: PASS.** Built from the working tree with the real-device fixes above,
including the 32nd shared setting (Resolve Playable Links). Same environment as
the first Stage G/I build: Homebrew OpenJDK 17.0.21, `RUSTUP_HOME=~/.rustup` with
the installed Android Rust targets, the existing `~/.cache/nuvio-android` native
dependencies, `tools/android.sh --debug-only`. No source, script or Gradle change.

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,507,824 bytes (71 MB) |
| SHA-256 | `efa8a42cac5299d226079e72b0cf7b6edd7293973e51ad0472a3f00009491e64` |
| Built | 2026-11-04 22:18 GMT (log: `build/stage-gi-fix/apk-build.log`) |
| Replaces | the 21:14 Stage G/I APK, SHA-256 `2348402c…4a8a4163` |
| Package | `space.nuvio.nativelegacy.strictdebug`, versionCode 20008, `2.0.8-strictdebug`, debuggable |
| SDK / ABIs | minSdk 24, targetSdk 35; `arm64-v8a` + `armeabi-v7a` |

Verification:

* **Latest code in both ABIs.** Both `libmain.so` were relinked at 22:18, after the
  last source edit (21:55). Each contains `debrid_enabled`, `follow_addons_order`,
  the `[ajustes-conta] perfil %d` and `documento recusado` diagnostics, the
  `duplicate JSON key` refusal reason, `showInHome` and the
  `voltou ao primeiro plano` resume refresh. None is present in the previous APK.
  The original Stage G/I strings are retained.
* **Signing.** `apksigner` verifies: v2, one signer, Android Debug certificate
  (SHA-256 `c0a433a8…c59bba`), unchanged; `.strictdebug` package ID unchanged.
* **Alignment.** `zipalign -c -P 16 4` passes; all six arm64 libraries have 16 KB
  LOAD alignment (`android-apk-pages`: 0 failed).
* **Launcher.** Enabled `Icone_original` -> `NuvioActivity` with LAUNCHER and
  LEANBACK_LAUNCHER, app icon and TV banner.
* **Assets.** 1086 art, 35 fonts, 8 licence entries; libmain, SDL2, curl, jpeg,
  webp and ffmpegJNI in each ABI; the build's personal-data/secret filter passed.
  P2P motor not included (no motor folder on this Mac, as before).

Not installed or device-tested; the TCL checks listed in "Stage G + I real-device
fixes" remain. No commit, push, merge, live account change or master change.

## Per-profile Home visibility: second addon instances (2026-11-04)

Report: one regular Nuvio profile shows the Indian catalogue rows; on the other
profiles they are hidden in regular Nuvio but still appeared in Better Nuvio.

**Profile scope, caching and switching are correct.** Both apps number profiles by
`profile_index`; a non-primary profile with `uses_primary_addons` shows the
primary's rows in both (official LayoutPreferenceDataStore; Native
`perfis_home_config`), every other profile its own synced blob. On a switch
Native clears the in-memory snapshot (`catordem_cache_carregar` starts with
`catordem_esquecer`), loads only that profile's owner/profile-scoped cache, drops
the previous projection (`fil_definir_perfil`) and refuses another profile's addon
list or late pull. No global visibility cache exists.

**Root cause: row identity of a second addon instance.** Regular Nuvio keys every
installed instance's catalogues as `<manifest id>_<type>_<catalogue>`
(`homeCatalogKey`; an exact collision keeps the first instance). Better Nuvio keys
every catalogue of a second or later instance of the same addon id by its URL
fingerprint (`addons_catalogo_chave`). Stage G recorded this as a limitation, but
it means a catalogue that regular Nuvio disables on a profile never matches, so it
falls back to the shared/Native "visible" state on **every** profile. On the
profile where the rows are enabled this looks correct, so the defect looks
profile-dependent. The Indian rows come from a regional catalogue addon (Bharat
Binge / streaming catalogues), the typical case of an addon installed twice with
different configurations.

**Fix (`src/fileiras.c`).** The account addon list now caches each instance's
fingerprint; `chaveOficial` translates a fingerprint key to the official key for
synced visibility (`contaEstado`) and synced order (`contaOficial`). Stored Native
keys, per-profile row files, styles and sizes are unchanged; nothing is hidden
globally. A second-instance catalogue whose official key collides with the first
instance follows that key's synced state, as the single official row does.
`[fileiras] perfil N: snapshot … casadas, … por chave oficial de 2a instancia, …
desligadas pela conta; … da conta fora do snapshot: <up to 5 keys>` is logged on
every snapshot or addon-list change (keys only, never URLs or credentials).

**Test.** `tests/home_rows_profiles.sh` (real fileiras/catordem/cache, sync.c's
switch order): US and India instances of one addon, profile 1 India on, profile 2
off with stale Native "visible" rows, profile 3 inheriting primary, profile 4
mixed; switches 1→2→3→4→2→1 apply each profile's own visibility and order, and
Native files keep their keys. With the mapping disabled the test fails on
profile 2 exactly as reported. Rerun and passing: `home_rows_profiles`,
`home_rows_official`, `home_rows_stage_g`, `fileiras`, `fileiras_android`,
`fileiras_removidas`, `fileiras_navegacao`, `colfileiras_sync`, `homepos`,
`catordem`, `catordemcache`, `ordemperfil`, `colordem`, `collections`,
`colecoes`, `desc_sessao_recovery`, `desc_sessao_pin`, `desc_iniciar_corrida`,
`syncaddons`, `settings_stage_i`.

**Limits / device check.** The mapping applies once discovery has read the
profile's manifests in a round (the account addon list); before that, on a cold
start, a second-instance row can briefly follow its Native-local state. That
the Indian rows really are a second instance is inferred, not observed: confirm
on the TV with the `[fileiras] perfil N:` line (expect "por chave oficial de 2a
instancia" > 0 and the India keys absent from "fora do snapshot"). If they are
listed there instead, regular Nuvio's blob for that profile does not contain them
and the line names them. The 22:18 APK predates this fix; a rebuild is needed.

## Per-profile visibility fix debug APK build (2026-11-04)

**Result: PASS.** Rebuilt with the second-instance identity fix above, same
environment and `tools/android.sh --debug-only` (log:
`build/stage-gi-fix/apk-build-profiles.log`, exit 0 at 22:37).

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,508,739 bytes (71 MB) |
| SHA-256 | `ab2429331e132d264ea05542b21030745883f091a216bc9ba503d30920d613a6` |
| Replaces | the 22:18 build, SHA-256 `efa8a42c…09491e64` |
| Package | `space.nuvio.nativelegacy.strictdebug`, 20008, `2.0.8-strictdebug`, debuggable; minSdk 24, targetSdk 35 |

* Both `libmain.so` (arm64-v8a, armeabi-v7a) relinked at 22:37, after the last
  `src/fileiras.c` edit (22:35). Each contains the new per-profile diagnostic
  (`por chave oficial de 2a instancia`, absent from the 22:18 build) and every
  earlier Stage G/I marker (`debrid_enabled`, `follow_addons_order`,
  `[ajustes-conta] perfil %d`, `showInHome`, resume refresh, Stage G/I strings).
* `apksigner` verifies (v2, one signer, Android Debug certificate `c0a433a8…c59bba`,
  unchanged); `zipalign -c -P 16 4` passes; six arm64 libraries 16 KB aligned.
* Launcher `Icone_original` -> `NuvioActivity` (LAUNCHER + LEANBACK_LAUNCHER), icon
  and TV banner; 1086 art, 35 fonts, 8 licence entries; 12 native libraries; the
  personal-data/secret filter passed. P2P motor not included, as before.

Not installed or device-tested. No commit, push, merge or account change.

**Device result (2026-11-04, reported by the owner):** the 22:37 APK
(`ab242933…20d613a6`) on the TCL Android TV shows the Indian catalogue rows only on
the profile where regular Nuvio enables them and hides them on the profiles where
regular Nuvio hides them. Per-profile Home visibility confirmed on device.

## Stage J — cross-device synchronisation (audit and design, 2026-11-04)

Audit and design only; **not implemented, awaiting approval**. See
[STAGE-J.md](STAGE-J.md) and [stage-j-inventory.json](stage-j-inventory.json).

* Traced today: 32 settings sync both ways through the `"tv"` profile blob;
  profiles, collections and Home order are read-only from Better Nuvio; addon
  membership stays local (only enable/disable toggles upload); plugins, provider
  credentials, watch progress, watched items and the library sync both ways.
  Native row presentation and 103 Better-only portable settings do not sync.
* The `"tv"` blob cannot hold Better-only data (regular NuvioTV replaces it on
  every push); `home_catalog_shared` is also unsafe. Proposed: a per-profile
  Better Nuvio document under `p_platform:"better_nuvio"` on the existing RPCs,
  with per-field revisions, pull–merge–push, no default uploads and strict
  allow/deny lists. Server acceptance of that namespace is unverified (stage J0
  needs approval, preferably on a test account).
* Staged plan J0–J7 and a two-device test plan are in STAGE-J.md.
* No code, APK, install, commit, push, merge or live account access in this pass.

### Stage J0 — namespace investigation (2026-11-04)

Source and documentation only; no backend or account request. Regular Nuvio's
clients already keep separate per-profile documents in the same RPCs under
`"tv"`, `"mobile"` and `"home_catalog_shared"`, with different schemas, and the
last was introduced client-side in both clients on 2026-06-11. A
`"better_nuvio"` namespace is therefore **very likely supported but not proven**.
A read-then-write probe with risks, cleanup and isolation checks is specified in
STAGE-J.md section 8.2 and awaits approval (preferably on a test account).
Alternatives if unsupported are in 8.4. J1–J7 not started.

### Stage J0 probe (2026-11-04)

Test account chosen. Implemented a debug-only, token-gated three-phase probe
(`src/j0sonda.c`, deep links `nuvio-legacy://debug/j0-probe[/write|/clean]`)
with mock-server and deep-link tests (`tests/j0sonda.sh`, `tests/deeplink.sh`:
pass). Release builds compile it out. Not run; no APK built. Procedure in
STAGE-J.md 8.6.

### Stage J0 probe debug APK (2026-11-04)

**Built and verified; the probe has not been run.** Same environment and
`tools/android.sh --debug-only` (log `build/stage-j0/apk-build.log`, exit 0, 22:59).

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,513,810 bytes (71 MB) |
| SHA-256 | `f12d763f69fb497958dd2dbb67c7026d16fa835a24aa41667969320c562a5609` |
| Replaces | the 22:37 per-profile build (`ab242933…20d613a6`) |
| Package | `space.nuvio.nativelegacy.strictdebug`, 20008, debuggable; minSdk 24, targetSdk 35 |

* Tests before the build: `tests/j0sonda.sh` and `tests/deeplink.sh` pass
  (`build/stage-j0/`).
* Probe present in both ABIs: each `libmain.so` (relinked 22:59, after the last
  probe source edit at 22:57) contains the probe schema `better-nuvio-probe`, the
  `better_nuvio` namespace, the `j0-probe` link path, the phase results and the
  exported `j0sonda_pedir`; none is in the 22:37 build. Stage G/I markers retained.
* Release exclusion: no release keystore here, so `j0sonda.c` and `deeplink.c`
  were compiled with the NDK using the release variant's flags (`-O2 -g -DNDEBUG
  -DNV_ANDROID`) for arm64 and armv7: 0 probe strings, 0 account-call references,
  no `j0-probe` link handling. The same files with debug flags contain all three.
* Signing (Android Debug `c0a433a8…c59bba`), `zipalign -P 16`, 16 KB arm64
  alignment, launcher, `nuvio-legacy` deep-link scheme and assets unchanged.

Not installed; no backend contacted; no commit, push or merge. Run the probe on a
separate test account as in STAGE-J.md 8.6.

### Stage J0 — live result (2026-11-04)

**Passed** on BlueStacks with a dedicated test account: `better_nuvio` read HTTP
200, write HTTP 204, read-back verified, and `tv`, `home_catalog_shared` and
`mobile` unchanged. The per-profile Better Nuvio namespace is supported by the
existing backend. Details: STAGE-J.md 8.7.

### Stage J1 — final settings inventory (2026-11-04)

Inventory finalised in `stage-j-inventory.json` (generators in
`tools/settings-restoration/`): 32 already synced; 57 official-tier candidates for
J6 (regular Nuvio exports an equivalent key); 26 official counterparts with an
unresolved/composite key (local until J6); **90 portable Better Nuvio-exclusive
settings** for the `better_nuvio` namespace; 87 device-local; 126 non-preferences.
Native Home presentation (row shape/size, Friends Watching/Last Session
placement, TV-only hides, hero source) is also portable and goes to
`better_nuvio` keyed by official row keys. Mappings were resolved from regular
Nuvio's stored keys and reviewed by hand. No sync code, APK, commit or push.
Three decisions before J2 are listed in STAGE-J.md 9.5.


### Stage J2 — generic engine, isolated validation (2026-11-05)

Implemented `src/bnsettings.c` / `.h`: typed versioned documents, per-owner/profile
state, no-default-upload restoration, per-field logical revisions/device tie-break,
fresh-pull upload gate, offline checkpoint/restore, snapshot acknowledgements,
late-edit protection, ambiguous-write recovery and loop prevention. Portable
allowlists/type/range checks, credential/device key guards, UTF-8 validation,
revision bounds and a 64 KiB JSON cap fail closed. No Native settings are bound.

PASS: `tests/bnsettings.sh` (isolated mock documents, strict compiler warnings,
ASan/UBSan), Stage G, Stage I, preservation, Stage A/B static regressions. Existing
production source and `stage-j-inventory.json` are unchanged. All 26 unresolved
settings remain local; the 90 portable settings are not enrolled until J4.

This is the engine layer only: raw response decoding, RPC transport and durable
filesystem adapters remain J3; Native bindings J4; Home J5; official candidates/
unresolved settings J6; device validation/default enablement J7. Unknown fields/
future versions currently stop preparation instead of being erased. Whole-blob
server races remain possible; revision repair requires a retained winning copy.
See STAGE-J.md section 10 for API boundaries, tests and limitations.

No live account mutation or transport was enabled, no APK was built/installed,
and no commit, push, merge, publication, master change or J3–J7 work occurred.


### Stage J3 — isolated Better-tier transport and checkpoints (2026-11-05)

Implemented `bnaccount` worker APIs over the existing authenticated Nuvio account
transport and a strict `bncodec` JSON/checkpoint layer. Calls are hardcoded to
per-profile `better_nuvio`; regular `tv`, `mobile` and `home_catalog_shared`
documents are not touched. Existing client identity, token refresh and transient
backoff services are reused without changing login or server protocols.

New profiles/devices default to **Better-tier writes Off**. Explicit local opt-in
is durable and private, never imported from the account. Checkpoints are explicit
versioned JSON, atomically saved with `dados_gravar`, scoped to exact owner/profile/
client. Defaults remain unobserved; a fresh pull always precedes an upload. Failed
saves preserve prior state; offline edits, late edits, retries and ambiguous writes
recover through the J2 revision/acknowledgement rules. Old-session responses cannot
publish into a new account/profile. Future/corrupt data blocks writing rather than
being dropped or reset. No secrets or raw response bodies are persisted/logged.

PASS: `tests/bnaccount.sh` with strict warnings and ASan/UBSan: file-backed mock
backend, authenticated payloads, separate-process restart and second device,
default-Off and opt-in-before-first-pull restoration, conflicts, auth retry/backoff,
account/profile switches, malformed/Unicode data, persistence failure, ambiguous
commit and regular-document preservation. Also PASS: J2 engine, Stage G, Stage I,
preservation and Stage A/B static suites. Logs: `build/stage-j3/`.

Stage G, Stage I, J2, existing 32 shared settings, J1's 418 classifications and all
pre-existing uncommitted implementation files remain unchanged. **No production
settings registry, Native observer, visible control or automatic scheduling was
added**: the callable transport and local permission APIs are ready for J4 binding.
The 90 portable settings are not enrolled and the 26 unresolved settings remain
local until J6. Existing Stage I/progress sync is unaffected by this permission.

Limitations: raw future/unknown sections and incompatible checkpoints fail closed;
J4 needs explicit registry migrations. A leftover J0 probe is not auto-overwritten.
The server still has no CAS, and an already dispatched request cannot be recalled.
Real-backend/device validation is deferred. See STAGE-J.md section 11.

No live requests/writes, APK build/install, commits, pushes, merges, master changes
or J4–J7 implementation occurred in this stage.


## Stage J4 — 90 portable settings bound (5 November 2026)

Complete locally and mock-tested on `codex/settings-restoration`; no commit made.
Exactly the 90 reviewed J1 portable controls now use the existing per-profile
`better_nuvio` engine/transport. Native saves capture batched edits; the existing
worker reads/synchronises and the SDL thread applies validated changes without
loops. Writes remain default **Off**, including across first login and restarts.
No new user-facing permission control was introduced.

The 32 Stage I shared controls retain their existing ownership and transport.
The 26 unresolved controls, 57 official candidates and 87 device-local controls
are not enrolled. No credentials, tokens, consent, hardware settings or new Home
sections enter the document. Stage G's hybrid rows and Native-exclusive features
remain intact. No IDs, defaults or Native storage keys changed.

Includes typed conversion for every binding, stable language/font identifiers,
missing-font fallback, durable offline recovery, registry expansion, account/profile
ownership isolation, inactive-profile editing and nonblocking late-response safety.
See [STAGE-J.md](STAGE-J.md#12-stage-j4--portable-native-settings-bindings-5-november-2026)
and [stage-j4-bindings.json](stage-j4-bindings.json) for all bindings and limitations.

PASS: J4 real-Native mocked binding suite; J2/J3 engine/transport (ASan/UBSan);
settings A–D and I; Stage G; syncaddons; TorBox cached-addition/autoplay;
verified-uncached stream filtering; Continue Watching; preservation and static
routing/English/binding guards. Logs/manifests: ignored `build/stage-j4/`.
An optional sanitizer run of the full Native harness stalled before the binding
checks with macOS service errors and was stopped; it is not a sanitizer pass.
Normal full-Native tests pass. Real TV/backend validation remains outstanding;
missing font files are not transferred, and server conflict guarantees remain
limited by the existing non-CAS protocol.

No live backend/provider operations, APK build/install, commit, push, merge,
publication or master changes. J5–J7, Stage E and Stage H were not begun.


## Stage J5 — portable Home presentation sync (5 November 2026)

Implemented locally on `codex/settings-restoration`; no commit made. Uses the
existing per-account/selected-profile `better_nuvio` document. Writes remain
**Off by default**. No regular Nuvio document or sync protocol was replaced.

Synchronises approved per-row shape, size, collection item order, portable personal
hides/shows, Friends Watching / Last Session anchors and selected hero source.
Sparse per-property revisions avoid uploading unrelated defaults. Stable identities
include official catalogue keys/type/parameters and distinct duplicate-instance
family context; names and URL fingerprints are not sent. Unavailable preferences
remain stored, never create catalogues, and hero/anchor rendering falls back safely.

Stage G still owns shared row order/enable states; J5 hides cannot enable an
originally disabled source. Native-exclusive rows and 30+ row support remain.
Native recovery files are retained, remote projection uses an overlay, and private
unobserved baselines preserve offline preferences without becoming uploads.
Account/profile switching, offline/restart recovery, conflicts and no-loop behaviour
are covered by two-device mocks. A fresh-install loading-gate fix enables the
same restoration when no Native settings file exists yet.

PASS: J5 two-device ASan/UBSan and real Native integration; J2/J3; all 90 J4 bindings;
32 Stage I controls; Stage G original/profile/hybrid tests; settings A–D;
TorBox/autoplay; stream filtering; Continue Watching; syncaddons; row-limit,
navigation and collection/Native row regressions; preservation and English/static
guards. All **413 original bindings** remain preserved. The 26 unresolved and
87 device-local controls remain excluded, along with credentials/tokens/consent.
See `build/stage-j5/` for local test logs and final validation manifest.

See [STAGE-J.md section 13](STAGE-J.md#13-stage-j5--portable-home-presentation-5-november-2026)
and [stage-j5-bindings.json](stage-j5-bindings.json). Limits: complete source
identities are required for safe duplicate matching; changing duplicate-instance
configurations is not a proven identity migration. Storage/parser bounds were
expanded to fit the existing structural row registry (4,096 fields / 1 MiB), with
no new enabled-row cap. Actual TV memory/performance, backend large-document
acceptance and remote-device UI behaviour remain unverified. Missing resources are
not transferred; failed local persistence is retried but total storage failure is
not guaranteed durable across process death. J2/J3's non-CAS limitation remains.

No live requests/account mutations, APK build/install, commit, push, merge,
publication or master changes. J6/J7 and Stages E/H were not begun.

## Stage J6 — official-tier settings (5 November 2026)

Completed from Codex's partial work; see [STAGE-J.md section 14](STAGE-J.md) and
[stage-j6-audit.json](stage-j6-audit.json). **40 settings now sync through
regular Nuvio's `"tv"` document** (38 of the 57 J1 candidates plus Show subtitles
and the focused-poster trailer), with explicit lossless conversions. Account writes
for them stay **Off by default**. Rejected 26 (different meaning, second owner, or
device-local in regular Nuvio); kept 16 local as uncertain/composite and 1
device-local. One owner per setting holds across Stage I (32), J4 (90), J5 Home,
Stage G and J6.

Fixed in the handed-over code: a legacy device predicate that disabled 9 mappings
and aborted the J6 test; `hide_unreleased_content` mapped twice (Stage G owns it);
an enabled trailer without a stored target being rejected; the missing audit file.
Added regression cases for language imports (the earlier audio-language problem)
and Stage G ownership.

PASS: J6 integration (plain and UBSan) and 27 regression suites including Stage I,
all 413 original bindings, J4's 90, J5, J2/J3, settings A–D, TorBox/autoplay, stream
filtering, Continue Watching and Stage G Home tests. ASan could not run because
Homebrew's `sdl2-compat` opens a blocking dialog under ASan (environment limit).

No live backend contact, account mutation, APK, commit, push, merge or master
change. J7 not started.

## Stage J7 — real-device validation plan (5 November 2026)

Plan only; see [STAGE-J.md section 15](STAGE-J.md). Findings from inspection:
J4/J5 (`better_nuvio`) and J6 writes have **no way to be enabled on a device**
(their permission functions are only called by tests), so device A → B cannot be
tested yet; Stage I's 32 fields still upload automatically when edited (existing
behaviour); Better-tier write permission persists per profile while J6's resets on
restart; the Better tier and J6 log nothing on device. Proposed J7a: debug-only,
token-gated write arming limited to the enrolled test owner, privacy-safe sync logs,
an extended snapshot probe, and a combined two-device mock test; then 14 live
scenarios on the test account (TCL + BlueStacks). Decisions D1 (Stage I writes)
and D2 (permission persistence) await you. No code change, APK, live contact,
commit or push.

## Stage J7a — debug arming, diagnostics, two-device mock test (5 November 2026)

Approved J7a only, with D1 and D2. Details: [STAGE-J.md section 16](STAGE-J.md).

* **D1:** Stage I's 32 `"tv"` fields keep uploading automatically. This is the one
  documented exception to the J4/J5/J6 default-Off policy.
* **D2:** J4/J5 and J6 upload permission is remembered per profile, for the enrolled
  test account only, for at most 4 hours (wall clock; enforced after restart;
  tampered or far-future expiries refused; a malformed state file means off).
* **Gate:** `j7arme_permite` sits in front of every J4/J5/J6 upload (bnaccount
  `rpc`/`bna_cycle`, J6 merge/pending) on top of the existing flags, so stored flags
  cannot upload by themselves. Android release builds contain no arming code: the gate
  is a constant refusal and there is no deep-link route (checked on NDK objects).
* **Activation (debug builds only):** `nuvio-legacy://debug/j7/arm` shows on screen a
  10-minute random code bound to the signed-in account (hash fingerprint) and profile.
  `…/arm?token=` enrols and arms. Another account, or signing out, erases enrolment
  and stops uploads at once. `…/disarm` and `…/snapshot` are also available. The code
  is never logged.
* **Logs:** `[bn-sync]`, `[ajustes-oficial]` and `[j7]` lines carry setting names,
  counts, HTTP codes and fingerprints only.
* **Snapshot:** gives fingerprints of `tv`, `better_nuvio`, `home_catalog_shared` and
  `mobile`. It counts fields outside the allowlist, sensitive patterns, and J4 keys
  found in `tv`, ending `limpo` or `VAZAMENTO`.
* **Mock test:** `tests/settings_stage_j7.sh` runs two devices as separate processes
  on one file-backed mock account, covering 21 steps (STAGE-J.md 16.4).

Results:
* **J7a suite:** PASS plain and with UBSan.
* **Regressions:** 40 of 41 suites PASS.
* **Expected failure:** `ajustes_sync`, which tests the device-snapshot upload
  Stage I removed (already documented).
* **Android:** all touched files compile for NDK debug and release; release contains
  no arming code.
* **Fixed during J7a:** older test builds that do not link `j7arme.c` now use inline
  stubs or a weak refusal fallback.

Remaining limitations:
* There has been no device or live-backend run (J7b).
* ASan is blocked locally.
* The arming code sits in private app storage for up to 10 minutes on debug builds.
* Enrolment proves "this signed-in account", not "the test account"; checking the
  fingerprint on screen is the safeguard.
* J4/J5 same-field conflicts use per-field revision/device ordering; J6/Stage I use their three-way remote-conflict rule (corrected during the live Fire Stick conflict test).
* Stage I still uploads on any account, as before.

No live backend contact, APK, live writes, commit, push, merge or master change.
Awaiting separate approval for a debug APK and for live J7b runs.

### Stage J7a debug APK (2026-11-05)

**Built and verified. Not installed, not armed, no backend contacted.** Built from
`codex/settings-restoration` with all uncommitted changes, using
`tools/android.sh --debug-only` (log `build/stage-j7a/apk-build.log`, exit 0, 11:45).
`git status` and the working-tree diff were identical before and after the build.

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,562,410 bytes (71 MB) |
| SHA-256 | `94f4c656e9c9cd88334c31c5f7876c260afef760c23f7566e8a8b387619fa04f` |
| Replaces | the J0 probe build (`f12d763f…c562a5609`) |
| Package | `space.nuvio.nativelegacy.strictdebug`, versionCode 20008, versionName `2.0.8-strictdebug`, debuggable; minSdk 24, targetSdk 35; arm64-v8a + armeabi-v7a |
| Signing | Android Debug, certificate SHA-256 `c0a433a8…c59bba`; `zipalign -P 16` OK; 16 KB arm64 alignment |

**Real gate, not the fallback.** Both `libmain.so` files export `j7arme_permite` as
a GLOBAL function of 304 bytes (arm64) and 296 bytes (armv7). It takes the J7 mutex,
so it is not the 0-returning weak fallback; there is no WEAK copy. In arm64 it is
called from `bna_cycle` (2 calls: the write RPC and the push decision) and from
`ajustes_compartilhados_mesclar` (2) and `ajustes_compartilhados_pendentes` (1), the
J6 paths. `j7arme_pedir`/`j7arme_passo` are called from `app_atualizar`. Both ABIs
contain the arm/disarm/snapshot route, the on-screen activation text, the snapshot
leak summary and the name-only `[bn-sync]`/`[ajustes-oficial]` logs. The J0 probe is
still present.

**Off by default.**
* A fresh install has no `j7-arme.txt`, so no account is enrolled and the gate
  refuses.
* The J4/J5 and J6 flags also start off.
* The J7 suite's "default Off" step, run on the same tree as the APK, passes: J4,
  J6 and J5 edits stay local, stored flags cannot bypass the gate, and only Stage I
  uploads.

**Release cannot arm.** I built the real release native library with
`externalNativeBuildRelease` (RelWithDebInfo, `-O2 -g -DNDEBUG`, no signing keys
read; log `build/stage-j7a/release-native.log`). In both ABIs it has:
* 0 `j7arme_*` symbols;
* no activation, arming, expiry or leak-summary strings and no `disarm` route;
* a release `deeplink.c.o` with no J7 route strings.

The gate there is the inline constant 0, so the J4/J5/J6 upload paths cannot pass.
(A `snapshot` string found elsewhere in the library comes from unrelated debug-info
names, not from the route.)

**Stage I unchanged (D1).** Stage I's 32 fields still upload automatically.
`ajustes_shared.inc` was not changed by J7a, and the J7 test asserts exactly one
Stage I `tv` upload while J4/J5/J6 stay off.

**Checks on the built tree.** PASS: `settings_stage_j7` (plain and UBSan),
`j0sonda`, `deeplink` (host and debug variants), `settings_stage_i`,
`settings_stage_j6`, `settings_stage_j5` and `bnaccount` (logs in `build/stage-j7a/`).
The wider 41-suite run from J7a still stands, since the sources have not changed
since.

**`ajustes_sync` is obsolete, not a J7 regression.** It fails in 4 of 7 scenarios
(roundtrip, empty, legacy, quota) on its first check, "display override excluded
from shared preferences". That check requires every account save to write the
Native `c_settings_v1`/`c_devices_v1` snapshot into the `tv` document. Stage I
deliberately removed that automatic extension upload (STAGE-I.md: "automatic
Native extension cloud backup/restore is no longer used"). The test passed at
HEAD, which predates Stage I, and it failed identically before J7a.

Its remaining intentions are already covered elsewhere:
* no default push on login, cloud applied before any push, unknown features
  preserved, logout leaves the cloud intact: `settings_stage_i`;
* per-device display settings staying local: `settings_stage_j7` (local-only step);
* Better-tier restore: J4/J5.

The quota and music-restore assertions depended on the retired device snapshot. I
recommend retiring it, or rewriting it later as a Stage I-era test, as a separate,
test-only change. It was not changed here.

Not installed. Uploads not armed. No live backend contact, commit, push or merge.
Awaiting approval for installation and live J7b tests.

## Stage J7b — started; J0 probe blocker fixed (2026-11-05)

* **BlueStacks (device B).** The J7a APK installed and its checksum verified
  (`94f4c656…`). Read-only first cycle: uploads off, `tv` empty (0/32 Stage I,
  0/40 J6).
* **Baseline fingerprints** (profile 1):
  * `tv`, `home_catalog_shared` and `mobile` empty (`9b8a3e00c63ea86f`);
  * `better_nuvio` 150 B (`bb09e12d232c709a`);
  * leak check clean.
* **Android TV (device A)** is not connected yet. Nothing armed or written.
* **Blocker:** the leftover J0 probe in `better_nuvio` was refused as an unknown
  document, and the refusal ended the cycle, so J4/J5 could never start on that
  profile.
* **Fix (approved option 1):** the codec now reads exactly the historical J0 probe,
  and the J0 clean result `{}`, as "not initialised". Every other unknown or
  malformed document is still refused. The probe is never written directly; it is
  replaced only by a normal armed upload of real local edits.
* **Tests:**
  * new `tests/bn_j0probe.sh`;
  * 7 new two-device steps in `settings_stage_j7`: first sync, second device,
    offline recovery, follow, profile isolation, unknown schema;
  * 41 suites pass.
* **Replacement debug APK:** `build/android/Nuvio-2.0.8-android-debug.apk`,
  74,564,017 bytes, SHA-256
  `fae8d7fb8a6b17dc2597f44cc97fa1536f9b58febf243e29388ff2deca1dd6ab`,
  `space.nuvio.nativelegacy.strictdebug` 2.0.8-strictdebug (20008). It has the real
  J7 gate in both ABIs. Not installed.
* **Finding:** the older `[sessao]` and `[perfis]` log lines write the account id
  to logcat. This predates J7; J7b reads only redacted or name-only lines.

No upload armed, no backend write, no commit, push or merge. Awaiting approval to
install the replacement APK.

### J7b — replacement APK on BlueStacks, read-only (2026-11-05)

* **Installed and verified:** `fae8d7fb…1dd6ab`, checked before and after
  installing; app data and the test-account sign-in kept.
* **Probe recognised:** the J0 probe is read as uninitialised, and the Better tier's
  cycle completes (pull HTTP 200, 0 fields, nothing applied).
* **Uploads off; no backend writes.**
* **Unchanged:** all four fingerprints are identical to the baseline, the probe is
  unmodified and the leak check is clean.
* **Not touched:** the TV. Nothing armed.

### J7b — separate J4/J5 and J6 permissions; replacement APK (2026-11-05)

* **Default activation is J4/J5 only.** `…/j7/arm` arms J4/J5 (`better_nuvio`).
  J6 official `tv` writes need their own code via `…/j7/arm-j6`. Codes are bound
  to their scope; per-profile 4-hour expiry; disarm clears both; a J7a-format state
  file means J4/J5 only.
* **J6 pending visible while off.** After every pull, names and counts of J6
  changes waiting while J6 is off are logged (no values). Previously this line could
  not be reached.
* **`tv` isolation tested:** J4/J5 armed with J6 pending gives no `tv` write; a
  Stage I edit (D1, unchanged) writes only Stage I's field; J6 armed alone never
  writes `better_nuvio`; profile switching and restarts keep scopes separate.
* **Tests:** 41 suites pass.
* **APK:** `build/android/Nuvio-2.0.8-android-debug.apk`, 74,565,837 bytes, SHA-256
  `a5fed5e294f4e83e8f91bea14dc290a42335760658f52c60d9094be0e555288c`,
  `space.nuvio.nativelegacy.strictdebug` 2.0.8-strictdebug (20008). It has both real
  scope gates in both ABIs (disassembly checked).
* **Not installed.** BlueStacks still runs `fae8d7fb…`, not armed.

No arming, no backend writes, no commit, push or merge.
* 2026-11-05: separate-scope APK `a5fed5e2…288c` installed on BlueStacks and its
  checksum verified.
  * Read-only check: both scopes off, 0 J4/J5 and 0 J6 changes waiting.
  * Snapshot identical to the baseline; leak check clean.
  * No arming and no writes.

### J7b — J4 test stopped before arming; activation notice fixed (2026-11-05)

* **Stopped before arming.** The one-line notice cut off the activation code, so the
  first J4 attempt stopped before arming. Nothing was armed or written, and the
  pre-test snapshot matched the baseline.
* **Fix.** The code now comes first in the notice (characters 17–24). It is still
  never logged, and a new test checks its visible position.
* **Tests:** 41 suites pass.
* **New debug APK** `b73a55636764a11a445bde833689fd3e31f9f8f2da59426adfd413042f77420b`
  (74,566,099 bytes), `space.nuvio.nativelegacy.strictdebug` 2.0.8-strictdebug
  (20008). Not installed. BlueStacks still runs `a5fed5e2…`, not armed.
* 2026-11-05, second J4 attempt **stopped before arming**.
  * The switched setting was the Stage I "Hero section" (`hero_section_enabled`),
    not the J4 "Show genre on hero". Stage I uploaded it automatically (D1).
  * The test account's `tv` document went from empty to 193 B
    (`36dd6d0e82efc08d`).
  * `better_nuvio`, `home_catalog_shared` and `mobile` are unchanged; the leak
    check is clean.
  * Nothing armed.

### J7b — first real J4 sync over the J0 probe: PASSED (2026-11-05)

On BlueStacks with the test account, `b73a5563…`:
* **Before arming:** the "Show genre on hero" change waited locally with uploads
  off.
* **Arming:** J4/J5 only (J6 off).
* **Upload:** one upload, `enviando 1 [heroGeneroLocal]`, `push HTTP 204`.
* **Documents:** `better_nuvio` went from the probe to a 256 B profile document
  (`46d82a0dd8ba9954`). `tv` (`36dd6d0e…`), `home_catalog_shared` and `mobile` are
  unchanged; the leak check is clean.
* **Restart:** arming kept, no echo.
* **Disarm:** both scopes off, 0 uploads after.
* **Remaining J7b work:** TV device, two-device scenarios, offline edits,
  conflicts, profiles, account switching, local-only checks and J6.

No commit, push or merge.


### J7b — TV connected; approved installation verified (5 November 2026)

* Device A is the connected Xiaomi MiTV-AFMU0 (Android 14), ADB
  `192.168.0.43:5555`, authorised by the user. BlueStacks remains connected
  (`emulator-5554`); its latest filtered J7 log reports both upload scopes off.
* With explicit **installation-only** approval, installed the existing approved
  APK, `space.nuvio.nativelegacy.strictdebug` 20008 / `2.0.8-strictdebug`.
  Read back the installed APK: exact SHA-256 match
  `b73a55636764a11a445bde833689fd3e31f9f8f2da59426adfd413042f77420b`.
  Regular Nuvio remains separate.
* No app launch; process not running. TV has no persisted J7 arming file, so both
  scopes default off. Test-account verification on the TV remains pending.
* No login, setting change, arming, backend snapshot/write or new live sync test.
  Remaining two-device scenarios and approval boundaries are prepared in
  `build/stage-j7b/TV-READONLY-PLAN.md`; see STAGE-J.md 17.12.

Existing work/evidence retained. No source implementation change, commit, push,
merge or master change in this connection/install session.


### J7b — first TV receive-only restore PASSED (5 November 2026)

User confirmed completion of the dedicated-test-account/profile-1 sign-in.
TV received and applied BlueStacks' existing `heroGeneroLocal`; the next pull
applied zero fields. Both scopes remain off. The current TV process logged zero
Better, J6 or Stage I sends/uploads, including during the read-only J7 snapshot.
All four namespace fingerprints match BlueStacks' post-test baseline; leak counts
are all zero. Evidence: `build/stage-j7b/tv-first-restore-filtered.txt`.

Initial B→A receive/no-echo passes. New A→B writes, offline/conflict/profile tests
remain pending explicit approval. No setting edits, arming, source changes,
commits, pushes, merges or master changes in this receive-only check.


### J7b — approved TV → BlueStacks J4 transfer PASSED (5 November 2026)

User changed **Show genre on hero** on the TV while uploads were off. After
private test-account/profile/scope verification, armed TV J4/J5 only, received
one `heroGeneroLocal` HTTP 204 upload, then disarmed the TV. BlueStacks (unarmed)
received/applied that field with no upload logged. No J6 or Stage I send occurred.

Both devices now show both scopes off. Final snapshots match: `better_nuvio`
`41160f64fdb0e802`; `tv`, `home_catalog_shared` and `mobile` unchanged; all leak
counters zero. Filtered evidence is in `build/stage-j7b/*a-to-b*` and
`bluestacks-receive-from-tv.txt`; see STAGE-J.md 17.14.

Single-field J4 transfers pass both directions. Wider Home/offline/conflict/profile
and J6 scenarios remain unapproved/unperformed. No source change, commit, push,
merge or master modification.


### J7b — offline recovery PASSED (disarmed-edit path, 5 November 2026)

User disconnected TV Wi-Fi, changed `heroGeneroLocal`, force-stopped/reopened the
app and confirmed the edit survived offline. ADB observed the outage. After
reconnection, the app remained disarmed and retained one pending local field
through a successful server pull. Explicitly armed TV J4/J5 for the approved
transfer: one field, one HTTP 204; immediately disarmed. Unarmed BlueStacks
received/applied it without an upload. J6 and Stage I made no send.

Both final snapshots match (`better_nuvio` `2b1fd615437388a7`); the other three
namespaces are unchanged, leak checks clean, both scopes off on both devices.
Evidence: `build/stage-j7b/offline-*.txt`; STAGE-J.md 17.15.

Offline persistence/restart/reconnect recovery passes for the disarmed-edit path.
Automatic armed reconnect remains untested. Conflict, profile/account isolation,
J5 Home and J6 live tests remain pending approval. No source changes or Git
checkpoint/publication actions.


### J7b — Fire Stick replacement installed, not launched (5 November 2026)

User requested Fire Stick testing in place of the Xiaomi TV. Amazon AFTSS,
API 28 / ARMv7, connected at `192.168.0.242:5555`. Approved install initially
failed for storage, then succeeded after the user requested retry. Independently
pulled installed APK matches approved `b73a5563…7420b`, strictdebug 20008.
No app process or persisted J7 arming file; both scopes default off.
No launch/login/settings/arming/backend operations. Dedicated-test-account
verification and Fire Stick testing remain pending. Earlier TV results are not
Fire Stick validation. See STAGE-J.md 17.16 and
`build/stage-j7b/firestick-install-verified.txt`. No source or Git publication changes.


### J7 — Fire Stick armed offline recovery passed (5 November 2026)

Dedicated test account/profile 1 confirmed. With explicit J4/J5-only approval,
Fire Stick saved the user's offline `heroGeneroLocal` edit across a timed app
restart, then automatically uploaded once (HTTP 204) after Wi-Fi recovery.
Unarmed BlueStacks applied the field without an echo upload. Both devices were
subsequently disarmed; J6 remained off. Final snapshots agree, unrelated `tv`,
`home_catalog_shared` and `mobile` fingerprints are unchanged, leak counters zero.
See STAGE-J.md 17.17 and `build/stage-j7-final/01-*`.
No application code/APK changes or commit/push/merge operations. Remaining live
conflict, profile/account, J5 and J6 scenarios are pending specific approval.


### J7 — live same-field conflict passed (5 November 2026)

Approved Fire Stick/BlueStacks profile-1 Hero size conflict converged to
Extra large. Each device uploaded its pending field once (204); three subsequent
fresh app restarts/pulls per device while J4/J5-armed produced no uploads.
Both devices are now disarmed; J6 stayed off. Unrelated backend fingerprints
unchanged; final snapshots match and leak checks are clean.

Corrected the earlier first-writer claim: J4/J5 uses revision/device ordering;
J6/Stage I uses its separate three-way policy. Application engine/APK unchanged;
only documentation and the mock suite's descriptive output were corrected.
Engine ASan/UBSan, J7 two-device mocks and 413-binding preservation all PASS.
See STAGE-J.md 17.18 and `build/stage-j7-final/02-*`.
Profile-2 availability remains unverified; no profile/account changes were made.
Remaining live profile/account isolation, J5, J6 and local-only isolation tests
are not yet complete. No commit, push, merge or master change.

### J7 — handover from Codex; profile-2 test state recovered (2026-11-05 17:13)

* **What Codex did:** set Fire Stick profile 2's Hero size to Large (pending
  locally) and armed J4/J5 for profile 2. It was interrupted at the profile
  chooser before any sync.
* **Server:** a fresh snapshot shows profile 2 still empty and profile 1 unchanged.
  **Nothing was uploaded.**
* **BlueStacks:** disarmed since 16:52.
* **Fire Stick:** its app was force-stopped for safety (no backend write). The
  profile-2 arming expires on its own (about 21:07 GMT); the Hero size edit
  remains pending locally.
* **Finding:** debug links wait until a profile is chosen.
* Details: STAGE-J.md 17.19.

### J7 — profile 2 isolation passed (2026-11-05 17:20)

* **Upload:** Fire Stick profile 2 sent `heroSizeLocal` once (HTTP 204) when the
  profile was chosen; J4/J5 only, no J6 or Stage I send.
* **Disarmed:** the Fire Stick was disarmed immediately.
* **Receive:** unarmed BlueStacks profile 2 applied the change with no echo.
* **Documents:**
  * profile 2's `better_nuvio` is now `8f4e1d5814a9bbb5` on both devices;
  * profile 1 is unchanged (`a5db70c7…`, nothing applied);
  * all `tv`, `home_catalog_shared` and `mobile` documents are unchanged;
  * leak checks are clean.
* **Now:** both devices disarmed.
* **Remaining:** account isolation (needs a second test account), J5 Home live, J6
  live, different-field conflict, and a live device-only/credential check.

### J7 — J5 Home row shape passed (2026-11-05 17:27)

* **Upload:** BlueStacks profile 1's Trending Movies changed from Automatic to
  Posters. With only J4/J5 armed, it uploaded one field,
  `home_7fe97440…_s` (HTTP 204).
* **Receive:** the unarmed Fire Stick applied it, shows Posters, and sent nothing
  back.
* **Documents:** profile 1's `better_nuvio` is `df8695e6d89776a0` on both devices.
  `tv`, `home_catalog_shared` and `mobile` are unchanged; leak checks are clean.
* **Now:** both devices disarmed.
* **Remaining:** J6 live, different-field conflict, live device-only/credential
  check, and account isolation (needs a second test account).

### J7 — J6 official tv setting passed, with one limitation (2026-11-05 17:37)

* **Upload:** BlueStacks profile 1 switched "Add-on name in the row" Off → On.
  Before arming, the J6 pending line showed it while uploads were off. With only J6
  armed, it uploaded one field, `catalog_addon_name_enabled`; there was no Stage I
  or J4/J5 send.
* **Documents:** `tv` went from 193 to 259 B (`9dcfd1563588074c`) on both devices.
  Other documents are unchanged; leak checks are clean.
* **Receive:** the Fire Stick was already On, so it received the value without a
  visible change and sent nothing back.
* **Now:** both devices disarmed.
* **Limitation:** a visible apply on the second device is not shown; the reverse
  direction needs approval.
* **Handling note:** an account identifier from a private journal header appeared
  in Claude's terminal output once; it was not saved anywhere.

### J7 — J6 reverse leg passed (2026-11-05 17:45)

* **Upload:** the Fire Stick switched "Add-on name in the row" On → Off; it waited
  while uploads were off. With only J6 armed, it uploaded one field.
* **Receive:** unarmed BlueStacks **applied it (On → Off)** and sent nothing back.
* **Documents:** `tv` is 260 B (`f25096afddf566a6`) on both devices; the other
  documents are unchanged and leak checks are clean.
* **Now:** both devices disarmed. J6 now passes in both directions, which closes
  the 17.22 limitation.
* **Remaining J7:** different-field conflict, live device-only/credential check,
  and account isolation (needs a second test account).
* Correction to the remaining list: J5 Home has passed **one direction only**
  (BlueStacks → Fire Stick, 17.21). Codex's plan entry "J5 Home bidirectional"
  is recorded as partial in `build/stage-j7-final/session-plan.json`, which now
  has one entry per planned test.

### J7b — remaining live tests passed; only account isolation left (2026-11-05 18:10)

Run under your standing approval (details: STAGE-J.md 17.24–17.28).

* **J5 reverse:** the Fire Stick's row size change reached BlueStacks (one `_z`
  field, no echo).
* **Different-field conflict:** both edits survived on both devices, with no
  ping-pong.
* **Device-only and consent:** Items per row and Receive polls caused 0 sends even
  with J4/J5 and J6 both armed; all documents were unchanged; both settings were
  restored.
* **Final checks:** 41/41 regression suites pass. Final documents are identical on
  both devices (`tv` `f25096af…`, `better_nuvio` `975ad35e…`, others empty). Leak
  checks are clean. Both devices are disarmed and run APK `b73a5563…`.
* **Not run:**
  * account isolation, which needs a second test account signed in by you;
  * typing a real credential into the app (covered by the automated suite and the
    leak scans).
* **Five findings recorded for J7c** (STAGE-J.md 17.28). No code change, commit,
  push or merge.

### J7b — account isolation stopped (2026-11-05)

* **Not a test account.** "Account B" on BlueStacks turned out to be a non-test
  account, as you confirmed. The test stopped before any profile was chosen, any
  snapshot taken, any sync run or anything armed. No Account B data was read or
  written by J7.
* **Account A** on the Fire Stick is unchanged and disarmed.
* **Handling note.** Account B's ID appeared once in Claude's terminal output (not
  saved anywhere); the filter is fixed.
* **To finish account isolation:** sign a second *dedicated* test account in on one
  device; the mock suite covers it meanwhile.
* **Status:** every other J7b scenario has passed (STAGE-J.md 17.28).
* 2026-11-05: account isolation, **read-only part passed; stopped before arming**.
  * **Isolation so far:** Account B (`db5b47a6`) received 0 Better fields and
    inherited no pending changes from Account A (`d21b69ba`); 0 sends. Account A
    is unchanged.
  * **Why it stopped:** Account B's snapshot flagged one Better key name in its
    60 KB `tv` (likely an old Native `c_settings_v1` snapshot; not verified), and
    its documents look like a real, heavily used account.
  * **Not done:** no arming and no upload. STAGE-J.md 17.30.

### J7b — account isolation passed (2026-11-05)

* **Upload:** with the upload made on test Account A (`d21b69ba`, J4/J5 only,
  `heroSinopseLocal`, HTTP 204), only A's `better_nuvio` changed
  (`fdb3931a…`).
* **Observer:** Account B (`db5b47a6`, unarmed) received 0 Better fields and sent
  nothing; all four of its documents are byte-identical before and after.
* **Now:** both devices disarmed.
* **Open item:** B's `tv` has one Better key name that predates J7 (likely an old
  Native snapshot; not verified, no change made).
* **Status:** with this, **every planned J7b scenario has passed**; credentials are
  covered by the automated suite and leak scans. STAGE-J.md 17.31.

### Follow-up work in progress (2026-11-05, interrupted by usage limit)

**Done in code (not yet in an APK; not committed):**
* **Privacy, logs:** `[sessao]` and `[perfis]` log lines now print an 8-hex account
  fingerprint (`sessao_impressao`, header-only in `sessao.h`) instead of the account
  ID.
* **Privacy, J6 journal:** the header is now `v2 <hash>`. Legacy `v1` files (raw ID)
  still load with pending edits kept, and are rewritten as `v2` when the profile
  opens.
* **Home rows editor message:** it now depends on `ajustes_better_envia()`
  (whether J4/J5 uploads are on): "Better Nuvio sync is on: your other devices
  follow" when they are, and the old text otherwise.
* **Language tables:** synced, +1 line each; Arabic translated; `tools/idiomas.py`
  reports ok.
* **Tests:** a new `privacy` step in `tests/settings_stage_j7.sh` checks the
  combined logs and J6 journals hold no account ID, the v1 → v2 migration, and that
  the message follows the upload state. The J7 suite passes.
* **Not yet verified:** the full local test run (1,141 safe scripts, background)
  had not finished; failures are not yet triaged.

**Investigation results (no change made):**
* **Home layout (Modern/Default/Dynamic, `homeLayoutLocal`) is not synced by any
  tier.** J1 classed it as "official key unresolved"; J6 left it local (no lossless
  mapping to regular Nuvio's MODERN/CLASSIC/GRID). It is not among J4's 90
  bindings. Smallest safe fix: add `{AJ_HOME_LAYOUT,"homeLayoutLocal"}` to the J4
  `better_nuvio` bindings (Better Nuvio devices only; regular Nuvio's `tv`
  untouched), and update the binding counts, inventory and tests.
* **Why Settings differ:** BlueStacks' Interface mode is Automatic, and BlueStacks is
  not an Android TV device, so it runs the Mobile interface (`telefoneui_ativo`),
  which forces the Settings "List" view (`ajLayoutSelecionavel`). The Fire Stick
  runs the TV interface and offers Panel. Smallest fix, with no code change: set
  BlueStacks' Appearance → Interface mode to **TV** (device-only setting).
* **Devices:** both disarmed (Fire Stick: no enrolment; BlueStacks Account B: J4/J5
  and J6 off, 0 sends).

### Stage J cleanup and Home layout sync — locally verified (5 November 2026)

Preserved Claude's work and historical J7b results. Fixed the interrupted
`sessao.c` compile error; verified fingerprint-only session/profile logs and
J6 v1 → hashed-v2 migration with pending edits retained and malformed/cross-scope
headers rejected. Debug **disarm only** now works before profile selection;
activation/snapshots retain their gates. No devices were armed or installed.
Fire Stick's local arming file has no armed entries; BlueStacks/main-account
backend documents were not accessed.

**Home layout now syncs through `better_nuvio` only:** Modern/Default/Dynamic,
existing `homeLayoutLocal` enum, per account/profile. Original 90 J4 bindings
remain, plus this single reviewed extension (91 total). The official `tv`
document/schema and 32 Stage I owners are unchanged. All three modes, offline
recovery, old-checkpoint expansion, profile isolation and no-default/no-echo
behaviour pass mocked transport tests. All 413 original bindings remain preserved.

**Validation:** 41/41 relevant regression suites PASS (including J7 UBSan),
language consistency PASS, Android JVM **61/61 tests** PASS. Initial Stage B
log-only contract failure was fixed with an exact privacy-line allowance and the
full Stage B suite passed again. Debug build and APK verification PASS:

- `build/android/Nuvio-2.0.8-android-debug.apk`, **74,567,092 bytes**.
- SHA-256: `a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`.
- Unchanged `space.nuvio.nativelegacy.strictdebug` and signing certificate.
- ARM64 + ARMv7 code/data verified against unstripped outputs: 91-entry Better
  map including Home layout, and early-disarm implementation in both.
- Same 12 libraries, 1,129 assets, TV launcher, ZIP alignment and ARM64 16 KB ELF
  alignment verified. Previous approved APK preserved. Nothing installed.

**Remaining:** live validation of this new APK/binding/privacy/disarm behaviour
requires installation and scope-specific approval on dedicated test accounts.
Quick-foreground refresh timing remains a known limitation. Existing absent
optional P2P engine and untested webOS/Tizen/device behaviour remain.
Claude's older broad 1,141-script sweep reported **717 failures**; its detailed
ledger was not located, so it is not claimed fixed or replaced by the focused
41-suite pass. Broader sweep triage remains outstanding.

Reviewed Stage E (dates/countdowns, NEW EPISODE badges, portrait unaired cards)
and Stage H (Home/details/paused-player/control visuals) in
`NEXT-STAGES-REVIEW.md`. **Neither implemented.** See STAGE-J.md 17.32–17.33 and
`build/stage-j-cleanup/` for evidence. Branch unchanged; HEAD remains
`2bb366167fb9359b854303d6f096b85c80a1abf5`. No commit/push/merge/master changes.

### Broad sweep triaged; dedicated-device validation prepared (5 November 2026)

Recovered the original detailed ledger: **1,141 runs, 424 passed, 717 failed**.
All 717 are classified in `BROAD-FAILURE-TRIAGE.json`; see
`BROAD-FAILURE-TRIAGE.md` for causes and remaining maintenance.

- **399** first failed on the interrupted `sessao.c` compile edit, now fixed;
  not all 399 are claimed rerun/passing.
- **111** link-fixture failures, **38** other compile/tool failures, **15**
  assertions and **154** runtime/harness/prerequisite failures were separated.
- **41 current/checkpoint comparisons (82 runs)** were completed with OS-denied
  networking and temporary data. Only the profile-selector harness and the
  deliberately obsolete device-snapshot sync test differed in pass/fail outcome.
- Fixed the profile-selector fixture and added meaningful Stage G source-scope
  assertions: PASS plain and ASan/UBSan. Kept production behaviour intact.
- Fixed offline Android TS JDK discovery and javadoc-jar selection: **3/3 PASS**.
- Eleven original assertion failures and four sampled renderer crashes also
  fail at the unchanged checkpoint; legacy/platform/localisation maintenance
  remains. The overall sweep is **not green** and is not a release approval.

**Triage complete.** Focused **41/41** and Android **61/61** results remain valid;
Home-profile and **413-binding preservation** checks pass. No application source
or APK changes in this triage; existing verified `a9cd562e…aad5ad` candidate is
unchanged and needs no rebuild.

`DEDICATED-DEVICE-VALIDATION.md` defines the next approvals: confirm both apps use
only dedicated test accounts, then installation only, then separately scoped live
launch/sync tests. BlueStacks may still hold the main account and was not accessed.
No device installation/arming, backend access/writes, commit/push/merge/master
change. Stages E/H remain deferred. See STAGE-J.md 17.34.

### Cleanup APK installed on both dedicated test devices (5 November 2026)

User approved installation only after confirming dedicated test accounts.
Fire Stick and BlueStacks now both have the exact approved APK:
`a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`.
Installed bytes/package/version verified: unchanged strictdebug, 20008 /
`2.0.8-strictdebug`. Replacement installs retained app data; no clearing/uninstall.
Fire Stick's first streamed attempt failed for storage; non-streamed retry
succeeded without removing anything.

**Both apps remain stopped.** No launch, sync arming, account/settings change or
backend operations. Live validation is still pending separate approval; these
installation results do not constitute functional device-test passes. Confirm the
same dedicated account before cross-device testing and disclose existing Stage I
automatic sync in the launch approval. Evidence: `build/stage-j-cleanup/install-only/`;
STAGE-J.md 17.35. No commit/push/merge/master changes; Stage E/H deferred.

### Final Stage J read-only preflight — partial (5 November 2026)

Dedicated test-account identity confirmed locally on Fire and from Blue's latest
login. Fire selected profile 1; Blue's current profile remains unverified: older
sync lines belonged to a prior account within the same process, and are excluded
from current test evidence. Blue was stopped; both apps now stopped.

Read-only, allowlisted RPCs using Fire's existing test token confirmed profiles
1/2 and current profile-1 backend fingerprints. Better has six fields and no
`homeLayoutLocal`. No write RPCs, token refresh, app launch or arming. No main
account's backend document was accessed. Privacy checks show no raw identities or
tokens in checked current-account log lines. Fire J6 p1 journal is v2/hash-only;
p2 is still unopened legacy v1, so live migration is not complete.

**Automatic approval review blocked online app launch/snapshot** because Stage I
may write while J4/J5/J6 gates are off. No workaround launch was attempted.
Completing current-profile, live migration and foreground checks requires a
strictly read-only test build or separate permission for existing Stage I automatic
settings sync on the dedicated test account. No J4/J5 activation is authorised.

Both-direction layout tests are prepared in `HOME-LAYOUT-LIVE-VALIDATION.md`;
not executed. See STAGE-J.md 17.36 and `build/stage-j-final-live/` (earlier Blue
uncorrelated summaries are superseded). No application code/build/install/Git
publication changes. J6 stays disabled; Stage E/H deferred.

### Read-only validation build prepared — 5 November 2026

User selected a debug-only read-only preflight build. Account mutations (including
Stage I), auth refresh and unknown endpoints are denied before network; native
HTTP mutation requests are guarded and J4/J5/J6 arming cannot become effective.
Normal builds retain their original policy. See `READ-ONLY-VALIDATION.md` and
STAGE-J.md 17.37 for the exact boundary and limitations.

Build/native verification PASS; Android unit tests **61/61 PASS**. Guard and
permission/native-HTTP mock tests PASS. Focused regression matrix **41/41 PASS** (ordinary policy): Stage G/I, settings
A–D, TorBox/autoplay/filtering, Continue Watching and preservation.
The extra native HTTP/TLS and buffer tests pass after repairing a confirmed
checkpoint-existing test callback collision; subsequent web JS tests need a Node CLI, unavailable locally (the bundled REPL disallows
their dynamic Function construction). No new application functionality beyond the debug guard.

APK: `build/android/Nuvio-2.0.8-android-readonly-validation.apk`
Size **74,565,640 bytes**; SHA-256 `573b0192a787a6370e04b3ff0d4d604dc3640d74cf9f641a5c48197ac4e4d10e`.
Unchanged package/certificate; both ABIs, TV launcher, assets and alignment verified.
Previous approved artifact preserved. **Not installed.** No device launch,
arming, backend access/write or main-account access. Live profile-2 migration and
foreground checks remain pending. Outbound Home-layout tests cannot run in this
artifact and require separate approvals. No commit/push/merge/master changes.

Final read-only artifact checks: **blocked refresh preserves the stored session
byte-for-byte and keeps the account logged in**. Local denials deliberately use
503 rather than an authentication rejection, so they cannot cause the existing
refresh code to erase credentials. Both devices were observed connected with
their apps stopped. No install or live account operations occurred. Final
evidence: `build/stage-j-readonly/FINAL.json`. The separate broad legacy-test
maintenance backlog is not declared resolved by these focused passes.

### Read-only live checks and sweep triage complete (6 November 2026)

Codex had already installed the read-only APK `573b0192…4d10e` on both devices
(verified, data kept). Claude ran the remaining read-only checks (STAGE-J.md 17.38):

- BlueStacks: test account `d21b69ba`, profiles 1 and 2 open and pull normally.
- Fire Stick: profiles 1 and 2 open normally. Profile 2's J6 journal migrated
  **v1 → v2** with the same 40 rows, raw ID removed, 0 pending.
- Read-only guard: one Stage I upload attempt (BlueStacks p1
  `hero_section_enabled`) blocked locally with HTTP 503. Otherwise 0 sends.
  All 8 backend documents byte-identical before and after.
- Foreground refresh: return under 60 s is skipped by design; return after 75 s
  re-reads. Privacy scan: no raw IDs, tokens, emails or codes in Nuvio logs.
- Both apps disarmed and stopped; nothing armed.

Sweep: of the original 717 failures, 311 now pass, 405 are pre-existing (they
fail identically with today's edits reverted), and 1 is a load-timing flake.
**0 regressions from today's changes.** The sweep is still not green (legacy
maintenance). J7 suite incl. privacy step PASS plain and ASan/UBSan.

**Before the final bidirectional Home-layout test:** resolve or accept the
pending local edits (BlueStacks p1 `hero_section_enabled`; Fire p2 unsynced
local settings); approve reinstalling the writable APK and Stage I automatic
sync on the test account; approve J4/J5 arming per direction. No commit, push,
merge or master change.

**Update (6 November 2026):** BlueStacks profile 1's pending `hero_section_enabled`
edit was discarded locally at the user's request (Show hero set back to the
account value, Off). Pending count is now 0, with no sends and backend unchanged. Still open before the
Home-layout test: Fire Stick profile 2's local-only settings, writable APK and
Stage I approval, and per-direction J4/J5 arming. See STAGE-J.md 17.38.

**Update (6 November 2026):** Fire Stick profile 2's "local settings" checked
read-only: 0 pending in Stage I, J6 and J4/J5. The log line is only a protection
flag; three local-only values (`heroSizeLocal` already synced, `heroAnoLocal`,
`catalogAddonNameEnabled`) will not upload. No pending edits remain on either
device. See STAGE-J.md 17.38.

**Update (6 November 2026):** Writable APK `a9cd562e…aad5ad` reinstalled on the
Fire Stick and BlueStacks (install only, data kept, bytes verified, both apps
stopped). Next: approval to launch (Stage I automatic sync becomes active) and
per-direction J4/J5 arming for the Home-layout test.

**Update (6 November 2026): Home layout Fire Stick → BlueStacks PASSED.** With
J4/J5 armed on the Fire Stick only, Default → Modern uploaded one field
(`homeLayoutLocal`, HTTP 204). BlueStacks applied it on foreground refresh (Modern
shown in Settings and on Home) with no echo. The backend changed only in profile 1
`better_nuvio` (6 → 7 fields). Fire Stick disarmed; both apps stopped. Remaining:
reverse leg BlueStacks → Fire Stick (needs J4/J5 arming on BlueStacks), plus a
stale "This profile only, on this TV" label on the Home layout panel. See
STAGE-J.md 17.38.

**Update (6 November 2026): Home layout BlueStacks → Fire Stick PASSED.** With
J4/J5 armed on BlueStacks only, Modern → Dynamic uploaded one field (HTTP 204).
The Fire Stick applied it on foreground refresh with no echo; Settings shows
Dynamic. Backend changed only in profile 1 `better_nuvio`. **Bidirectional Home
layout sync is verified.** Both devices disarmed and apps stopped. Open
follow-ups: the stale "This profile only, on this TV" label on the Home layout
panel, and touch taps on the Modern side rail don't respond on BlueStacks. No
commit, push, merge or master change.

**Update (6 November 2026):** Fixed the stale Home layout "Applies to" label:
J4-bound settings now read "Account/profile" while this profile's Better Nuvio
uploads are on (otherwise unchanged; release unaffected). 41/41 focused suites
and `ajustes_ux_dados` PASS. Source change only, not yet in an APK. No more live
tests are required for Stage J. No commit, push, merge or master change.

## Stage J — complete (6 November 2026)

The user marked Stage J complete. Summary of what was verified:

- **J0–J6 implemented:** official `tv` tier (Stage I's 32 fields plus J6's 40),
  Better Nuvio tier (J4's 91 bindings including Home layout, plus J5 Home
  presentation). Device-only settings, credentials and consent preferences stay
  local. Better and J6 writes are off by default; in debug they need the J7 gate,
  and release builds contain no J7 code.
- **J7 live validation on dedicated test accounts (Fire Stick + BlueStacks):**
  restore, transfers in both directions, offline (armed and unarmed), same-field
  and different-field conflicts, profile isolation, J5 Home rows, J6 in both
  directions with separate permission, account isolation, and **Home layout sync
  in both directions** (Default → Modern, Fire Stick → BlueStacks; Modern →
  Dynamic, BlueStacks → Fire Stick). Each run had one upload per change, no
  echo, and backend changes limited to the intended document.
- **Read-only checks:** profiles 1 and 2 on both devices, J6 journal v1 → v2
  migration, foreground refresh (60 s minimum by design), privacy (no raw
  account IDs or tokens in logs), backend unchanged.
- **Tests:** 41/41 focused suites (including J7 plain and ASan/UBSan) and 61/61
  Android unit tests. Broad sweep: 0 regressions from Stage J. Its 405
  pre-existing legacy failures remain maintenance work, not Stage J scope.
- **Polish included:** privacy fingerprints in logs and the J6 journal; Home-rows
  notice and Home layout "Applies to" label follow the real sync state.

Known limitations carried forward: quick foreground returns (under 60 s) don't
re-read; webOS/Tizen untested; the label fix is in source only (not yet in an APK).
Devices end on the writable `a9cd562e…` APK, disarmed, apps stopped, both on
Home layout Dynamic. Branch HEAD unchanged; no commit, push, merge or master
change. Details: STAGE-J.md 17.38.

### BlueStacks Modern side-rail taps — no defect (6 November 2026)

Investigated the reported "rail taps don't respond in Modern" on BlueStacks.
**There is no bug; the original observation was a test error.** In the earlier
session the Settings gear tap *did* open Settings. The next tap, aimed at the
Settings "Layout" category using Dynamic-layout coordinates, landed on the rail's
Search icon, because in Modern the rail pushes the category list right. That sent
the screen back to Search, so the gear tap looked as if it had failed.

Evidence:

- A local harness drawing the real `menu.c` in Modern with the rail closed: every
  rail icon (including Settings) lies inside its own tap target.
- On BlueStacks, with the user's approval, profile 1 was switched briefly to Modern
  and back. With pointer traces, every tap hit the right target and navigated
  correctly: gear from Home; Search icon; gear from Search; gear from Search with
  the text keyboard open; gear after a tap on an empty area.
- Dynamic layout: pill → menu → Settings by touch over Search also works.

No code change. Side effect, as disclosed beforehand: BlueStacks profile 1's Home
layout is back on **Dynamic**, but its J4 slot is now marked changed. It will
re-upload "Dynamic" (same value) the next time J4/J5 is armed there. 0 sends
during the test; backend byte-identical; app stopped. Ready for Stage E.

### Stage E — episode air dates and artwork implemented (6 November 2026)

Ported regular Nuvio's episode rules (see `STAGE-E.md` §4):
- Release-date engine (official local-date and aired semantics).
- `Airs Today` / `Airs Tomorrow` / `Airs in N Days` / `Airs <date>` countdown,
  with short forms.
- New Episode / New Season badges (`#1D4ED8` / `#B45309`, 60-day window,
  recomputed live).
- `S1 E2` episode line; `Xh Ym left` / `N% Watched` / `Resume` progress labels.
- Poster-style CW badges; hero next-up line.
- Unaired next-up artwork: title art first, still last.

New `src/epdata.c`, `src/eprotulo.c`; changes in `continuar.c`, `cwordem.c`,
`home.c`, `trakt.c`, `descoberta.c`, `badges.c`; 13 language keys (29 tables ok).
Tests: new `stage_e_episodios` PASS, `nextup_art` extended PASS, 41/41 focused
matrix PASS, no new failures across 124 subset-linked tests. Remaining
differences are listed in `STAGE-E.md`. Not built into an APK; no device check;
no commit/push/merge/master change.

**Update (6 November 2026): Stage E debug APK built, not installed.**
`build/android/Nuvio-2.0.8-android-stage-e-debug.apk`, 74,578,105 bytes, SHA-256
`cc618f4e228130fe48f9dfdbc13f6c671dd2b46d421a20e6729ce08eba24ed17`.
Same package/version (`space.nuvio.nativelegacy.strictdebug`, 20008 /
2.0.8-strictdebug) and signing certificate as the installed `a9cd562e…` build; it
installs over it with data kept. Both ABIs, 12 native libraries and 1,129 assets
(identical sets to `a9cd`), 16 KB zip alignment. Stage E code present; read-only
validation guard absent (normal writable build: Stage I automatic sync active,
J4/J5/J6 still gated by J7 arming). It also contains the earlier Home layout
label fix. Build environment notes:
`tools/rust-mkvcore.sh` defaults to an unmounted external SSD toolchain, so the
build used `RUSTUP_HOME=~/.rustup`, the rustup stable toolchain on `PATH`, and
Homebrew JDK 17. Previous APKs backed up in `build/stage-e/apk-before/`.
Evidence: `build/stage-e/apk-verification.json`.

**Update (6 November 2026): Stage E APK installed on BlueStacks only.** At the
user's request ("just bluestacks"), `cc618f4e…24ed17` replaced `a9cd562e…` on
BlueStacks (model and package confirmed first; `install -r`). Installed bytes
match; version 20008 / 2.0.8-strictdebug. App data kept: first-install time still
4 November, this update at 12:44. App **not launched**, nothing armed. The Fire
Stick is unchanged on `a9cd562e…`, stopped. Note: launching on BlueStacks
re-enables Stage I automatic sync (writable build). BlueStacks profile 1 still has
the disclosed J4 Home layout slot marked changed (same value, Dynamic), which
uploads only if J4/J5 is armed there. Evidence:
`build/stage-e/install-blue/results.json`.

**Update (6 November 2026): Stage E badge fix installed on BlueStacks only.** The
user approved "Install on BlueStacks". `c93ddddf…e4f98f` replaced `cc618f4e…` on
BlueStacks (`install -r`, data kept). Installed bytes verified; app not launched,
nothing armed. The fix: an unaired next-up carrying a runtime shows its
countdown instead of "22m left" (STAGE-E.md). The Fire Stick is unchanged on
`a9cd562e…`. Evidence: `build/stage-e/install-blue2/results.json`.

**Update (6 November 2026): Stage E unaired-art APK installed on BlueStacks only.**
The user approved "Install on BlueStacks". `b08d67b2…c5b92b` replaced
`c93ddddf…` (`install -r`, first-install time still 4 November, so data kept).
Installed bytes verified; app not launched, nothing armed. It contains the
countdown-badge fix and landscape backdrops for unaired episodes in the episode
list (STAGE-E.md). The Fire Stick is unchanged on `a9cd562e…`. Evidence:
`build/stage-e/install-blue3/results.json`.

**Update (6 November 2026): air dates from the metadata add-on — built and
installed on BlueStacks only.** The user approved "Build and install".
`build/android/Nuvio-2.0.8-android-stage-e4-debug.apk`, 74,579,401 bytes, SHA-256
`32e8982908ef38deec095f5f50b412c74c66c4ed295ac5d506da35bcfddd5b44`. Same
package, version and signer; both ABIs; 12 libraries; 16 KB aligned; no
read-only guard. It replaced `b08d67b2…` on BlueStacks (`install -r`,
first-install time still 4 November, so data kept). Installed bytes verified.
App not launched, nothing armed; the Fire Stick is unchanged on `a9cd562e…`.
Episode air dates (CW countdown, New Episode/Season, upcoming, episode list) now
come from the user's metadata add-on, chosen as regular Nuvio does
(STAGE-E.md §5.1). Evidence: `build/stage-e/install-blue4/results.json`.

**Update (6 November 2026): Continue Watching refresh after watched changes.**
Cause: watched/unwatched gestures never triggered a rebuild, and the account's
next-up seeds used only the server snapshot. Fix: seeds include this TV's
journal (marks join, later unmarks drop), and a gesture revision triggers one
coalesced Continue Watching rebuild (TV and Mobile, deferred while playing), as
regular Nuvio's local-first combine does. Tests: `contapend`, `contalib` new
cases PASS; 41/41 focused matrix; no new subset failures. Not in an APK; no
install, backend access or commit. See STAGE-E.md §6.

**Update (6 November 2026): Continue Watching refresh fix — built and installed
on BlueStacks only.** The user approved "Build and install".
`build/android/Nuvio-2.0.8-android-stage-e5-debug.apk`, 74,579,837 bytes, SHA-256
`9bf8209533f20a4e86f3c1523aa559de35475067b39d53e508c053b2426ff1a9`. Same
package, version and signer; both ABIs; 12 libraries; 16 KB aligned; refresh
code present; no read-only guard. BlueStacks had restarted and dropped adb; it
was reconnected and the install retried. It replaced `32e89829…` (`install -r`,
first-install time still 4 November, so data kept). Installed bytes verified;
app not launched, nothing armed. The Fire Stick is unchanged on `a9cd562e…`.
Evidence: `build/stage-e/install-blue5/results.json`.

**Update (6 November 2026): artwork provider consistency (Stage E §7).** The
episode list now uses the selected "Background do hero" provider for show art
(it read the raw Cinemeta backdrop before), including unaired episodes. The hero
skips the poster-as-still for unaired next-ups. Portrait images in landscape
cards are cropped, not pillarboxed, and landscape cards prefer the metahub
landscape over the poster. Changes apply at draw time, so they show without a
reload. Tests PASS; 41/41 matrix; no new subset failures. Not in an APK; no
install or commit.

**Update (6 November 2026): artwork provider consistency — built and installed on
BlueStacks only.** The user approved "Build and install".
`build/android/Nuvio-2.0.8-android-stage-e6-debug.apk`, 74,580,088 bytes, SHA-256
`943548ee17b4ecfd694fd7292bc3cb1e7a100c7f8ead84ed45992c4d59e56ce6`. Same
package, version and signer; both ABIs; 12 libraries; 16 KB aligned; earlier
Stage E fixes present; no read-only guard. It replaced `9bf82095…` (`install -r`,
first-install time still 4 November, so data kept). Installed bytes verified;
app not launched, nothing armed. The Fire Stick is unchanged on `a9cd562e…`.
Evidence: `build/stage-e/install-blue6/results.json`.

**Recorded (6 November 2026): Stage H playback polish — trailer flicker in zoom
modes.** Investigated, not implemented (owner: wait for Stage H). Likely cause on
Android: the trailer is shown at full frame first and zoomed only at playing +
800 ms; that geometry change triggers the TCL-oriented SurfaceView recreation
(GONE → VISIBLE) on non-MStar/Amlogic decoders such as the Fire Stick, a brief
black flash. Android also shows the video before the crop settles (Samsung
waits). The player isn't recreated. Details and proposed fix:
`NEXT-STAGES-REVIEW.md` ("Stage H task: trailer flicker"). Needs device log
confirmation. No code changed.

**Update (6 November 2026): trailer flicker fix implemented (Android).** The owner
approved doing it now. The zoom crop goes out before the first visible frame,
the artwork is held until the native layout confirms it (so any surface
recreation is hidden), the reveal is sticky, the re-apply loop is gone on
Android, and the fallbacks are 2.5 s / 6 s. A new test caught and fixed an
unsigned-time wrap. Tests: `trailer-android-zoom` PASS, no new trailer-suite
failures, 41/41 matrix. Not built, installed or committed. See
NEXT-STAGES-REVIEW.md.

**Update (6 November 2026): trailer flicker fix — built and installed on
BlueStacks only.** The user approved "Build and install".
`build/android/Nuvio-2.0.8-android-stage-e7-debug.apk`, 74,580,319 bytes, SHA-256
`3cfa886f3fe6227a96a33ebe97998f245bd9b305bc2e085c5fd84f7675562e19`. Same
package, version and signer; both ABIs; 12 libraries; 16 KB aligned; trailer and
earlier Stage E fixes present; no read-only guard. It replaced `943548ee…`
(`install -r`, first-install time still 4 November, so data kept). Installed
bytes verified; app not launched, nothing armed. The Fire Stick is unchanged on
`a9cd562e…`. Evidence: `build/stage-e/install-blue7/results.json`.

**Update (6 November 2026): subtitle-safe trailer crop (owner: Option 1).** For
likely-subtitled trailers (title's original language not English and not the
app language; best effort), the zoom crop moves its vertical trim up, keeping
the bottom band with no black strip and the same zoom mode. Other trailers stay
centred. The flicker fix is preserved (crop still applied once before the first
frame; no re-crop or surface recreation). Tests PASS; 64 related tests with no
new failures; 41/41 matrix. Not built or installed. See NEXT-STAGES-REVIEW.md.

**Update (6 November 2026): subtitle-safe trailer crop — built; install
pending.** `build/android/Nuvio-2.0.8-android-stage-e8-debug.apk`, 74,580,657
bytes, SHA-256 `91a7bc0080624847871c03eb48479198ca339adc4988df806652c95480e6bdb1`.
Same package, version and signer; both ABIs; 12 libraries; 16 KB aligned; no
read-only guard. The BlueStacks install was approved but not done: BlueStacks
wasn't running (no adb port), and the install script stopped at its target
check. Nothing changed on either device. The Fire Stick is unchanged on
`a9cd562e…`; BlueStacks still has `3cfa886f…`.

**Update (6 November 2026): subtitle-safe trailer crop installed on BlueStacks.**
Once BlueStacks was open, `91a7bc00…6bdb1` replaced `3cfa886f…` (`install -r`,
first-install time still 4 November, so data kept). Installed bytes verified;
app not launched, nothing armed. The Fire Stick is unchanged on `a9cd562e…`.
Evidence: `build/stage-e/install-blue8/results.json`.

**Recorded (6 November 2026): proposed Stage H feature — Family Watching**
(replaces Friends Watching). Feasibility only, not implemented. The existing
per-profile RPCs (`sync_pull_profiles`, `sync_pull_profile_locks`,
`sync_pull_watch_progress`, `sync_pull_watched_items`, all with
`p_profile_id`) can read other profiles of the same account with the current
session; no new endpoint needed. There is no backend privacy/share flag, no
child-profile flag and no content-restriction field; official Nuvio has no
cross-profile feature. Design, privacy proposal and the owner decisions needed
are in `NEXT-STAGES-REVIEW.md` ("Proposed Stage H feature: Family Watching").

**6 November 2026: Stage H Family Watching implemented locally (not live).**
Replaces Friends Watching in the `social_activity` slot (Option D). Family
Watching and "Share my watching activity" are per profile, Off by default and
added to the portable Better map; J4 stays disarmed, so nothing was uploaded.
PIN-locked and child profiles are excluded and a child viewer gets no row.
Cross-profile reads are **not registered**: the row stays empty until the owner
approves the read transport. Details and the approval list are in
`NEXT-STAGES-REVIEW.md` ("Family Watching", section 5). Debug APK built, not
installed.


## Requested settings expansion — 8 November 2026

Implementation and evidence: [SETTINGS-EXPANSION.md](SETTINGS-EXPANSION.md),
[per-setting report](SETTINGS-VERIFICATION.md), machine-readable
`settings-expansion-ledger.json` and `settings-expansion-tests.json`.

- Added Hide modern sidebar pill (the user's clarified target), independent
  detail/post-play trailer delays, opt-in Android post-play previews, movie
  recommendation enable/percentage, still-watching episode interval, and full
  automatic subtitles with saved/manual None preferences honoured.
- Reused App Icon & Banner with its existing supporter restriction; linked
  icon/banner selection and package identity are unchanged.
- Added original named card width/radius presets; restored renderer consumers
  for edge glow/coverage and all five surface switches. Corrected zero-focus
  episode cards and actual preview display/dependency wiring.
- Added functional Filters & Sorting: preferred/required/excluded choices for
  seven dimensions, result/resolution/quality limits, minimum/maximum size,
  six stable sorts, and required/excluded release groups. Rendered counts and
  automatic candidate paths use the same filters; availability safeguards remain.
- Unattended episode counts survive automatic close/open to the expected next
  episode and reset on manual exit/profile changes. Prompt requires existing
  automatic-next enabled; no progress keys/formats or account RPCs changed.
- Fourteen append-only IDs: eight profile-local preferences and six virtual
  editors using the existing source-configuration store. All 413 original
  bindings/defaults and progress fingerprints pass preservation. Existing J4
  Family/Home extension fixtures corrected from 91 to the already-implemented
  93; no new sync enrollment, schema or write permission was added.

Validation: **42 relevant regression suites PASS**, including settings A–D,
Stage G/I/J, TorBox, stream filtering and Continue Watching. **64 Android JVM
unit tests PASS**. ARM64 and ARMv7 `main` libraries compile/link from the final
sources. Actual local OpenGL framebuffer and ten launcher resource-pair checks
pass. No new APK packaged or installed; no device settings, account/provider
operations, uploads or synchronisation permissions changed. Branch remains
`codex/settings-restoration`, HEAD `2bb366167fb9359b854303d6f096b85c80a1abf5`;
existing Continue Watching commit remains an ancestor. No commits/push/merge.

**Coverage is not exhaustive functional certification.** All 434 Android
bindings have ledger entries; 341 host value contracts, 30 focused functional
bindings and 228 filter subchoices were checked. Other controls are explicitly
uncertified individually. The older repository-wide failures are not claimed
fixed. Real remote navigation, launcher refresh, real subtitles and the Android
software preview's composition/decode/CPU behaviour remain untested. Preview
supports direct IMDb AVC on supported Android devices only and defaults Off;
LG/Tizen/desktop secondary previews are unsupported. Sizes use whole decimal GB;
preferred tags are unordered sets, not ordered official preference lists.


### BlueStacks installation — 8 November 2026

User authorised installation. Built the latest settings expansion using
`tools/android.sh --debug-only` with `NUVIO_VALIDATION_READONLY=1` and offline
inputs. Existing rustup proxies were required ahead of Homebrew Rust on PATH;
no new targets/dependencies downloaded. Original strictdebug package and debug
certificate retained. Both ABI libraries contain the latest settings keys and
read-only guard. Signature, ZIP/16KB native alignment, assets and TV launcher
checks pass; mocked read-only transport and original preservation checks pass.

Installed with `adb install -r` on BlueStacks only, preserving app data.
Installed package was read back and matched the built APK byte-for-byte.
APK: `build/settings-expansion/Better-Nuvio-settings-readonly-debug.apk`
SHA-256: `f003828efcf835f3549a3449b2c520f95c158142483038636288e7373ae9a3b3`.
No app launch, setting edits, account inspection, sync arming, live backend/provider
operations or Fire Stick changes performed. This build blocks account mutations
and session refresh; login/refresh/sync-write checks cannot be certified in it.
Real-device settings tests have not yet started. No commit, push or merge.


### BlueStacks login-capable validation replacement — 8 November 2026

The user could not sign in because the first strict read-only APK intentionally
blocked authentication and session refresh. After the user agreed to a replacement,
added separate debug-only `NUVIO_VALIDATION_LOGIN=1` opt-in, requiring
`NUVIO_VALIDATION_READONLY=1`. Normal and strict read-only build behaviour is
unchanged. No authentication/activation checks were bypassed.

Source-traced `sessao.c` allowlist: existing password/refresh/anonymous grants,
start/poll TV login RPCs and `tv-logins-exchange`. QR anonymous bootstrap signup
permits only its exact fixed body, rejecting email signup bodies and extra bytes.
Both account and native HTTP boundaries use the policy; exact account origin,
method and path checks remain. Settings/progress/library/home/provider mutations
and J4/J5/J6 arming remain blocked. The build permits authentication session changes,
so it is no longer described as completely read-only. No live authentication
was performed by the agent; user must sign in using the dedicated test account.

Mock verification PASS: ordinary/strict/login-enabled boundary modes, exact
signup body, denied methods/origins/extra queries, no settings or provider writes,
release/unguarded-login compile rejection; login-enabled J7 native mutation and
arming gate; existing email and refresh concurrency tests; original 413-binding
preservation. Email tests use only an isolated localhost mock server. Logs are
`build/settings-expansion/login-*.log`. Both Android native ABIs rebuilt; APK
signature, alignment, package, latest settings and validation markers checked.

Installed replacement on BlueStacks only using `adb install -r`, retaining data;
read-back package matched SHA-256 byte-for-byte. Same package/certificate as prior
installation. APK: `build/settings-expansion/Better-Nuvio-settings-login-validation.apk`
SHA-256: `00d1b16a15b198cf0697d5d06b84258d964e7395079998de2e29c151185f1f72`.
No app launch, account-data inspection, setting edits, live auth/backend writes,
Fire Stick changes, commit, push or merge. Actual login/server success remains
for the user to verify. Live sync-write behaviour cannot be certified in this build.


### Initial approved BlueStacks settings checks — 8 November 2026

User approved local preference testing/restoration only; excluded destructive,
consent, purchase and account/provider mutations remain excluded. See
`BLUESTACKS-SETTINGS-TESTS.md` and the per-binding
`bluestacks-settings-tests.json` checklist. This initial round is not exhaustive:
language/background editors, actual accent colour and three Home layouts were
observed. All changed preferences restored (language Automatic, accent Petrol,
background Frost, Home Dynamic); Show hero remained Off. No setting restart
persistence or media playback tests have been completed on BlueStacks yet.

Found Modern + Show hero Off still reserves a large empty hero area. Fixed
Modern row anchor, clipping and hero displacement in `src/home.c`; preserved
hero-On geometry and other layouts. New direct production geometry assertions
fail on the original code and pass after correction across all three layouts and
normal/125% text scale. `tests/home.sh`, menu_tvos, menu_geometry,
home_rows_stage_g, home_rows_profiles, homepos and cw_parity pass. Original
413-binding/default/progress preservation passes.

Built both Android ABIs offline. Signature/certificate/alignment and packaged
login-validation/latest-settings markers verified. APK prepared, NOT installed
pending separate approval: `build/settings-expansion/Better-Nuvio-settings-hero-off-fix.apk`.
SHA-256 `f0a8de997c59ddf67cb25b62371c3fc70911d0052bd90619a1d9a044a785e75f`.
Device verification of the correction remains pending. Pointer clicks appeared
offset; cause unresolved, so pointer navigation is not certified. Directional
keys/OK and Back worked once inside the menu/settings. No backend/provider writes,
permission arming, main-account inspection, commit, push or merge performed.


### Approved BlueStacks update and second settings round — 8 November 2026

Approved `f0a8de…85e75f` validation APK installed on BlueStacks only, data retained;
installed bytes verified. **Modern + Show hero Off correction now verified on the
actual app**, including app exit/reopen. Hide pill also works on actual Modern
Home, survives exit/reopen, keeps edge navigation accessible, and restores Off.
Actual Compact width and Sharp corners affect real Home posters; original 134 dp
and 12 dp restored. Depth master/five surface switches exercised individually at
UI level, without claiming every actual media surface verified.

All **228 metadata filter checkboxes** individually actuated On/Off through the
Android UI across 21 sheets; exact group counts and None restoration checked.
All six sorting choices, all five numeric limits/size editors and both release
group text editors exercised and restored. Final root count 0 active rules.
Full stream consumer effects remain covered by mocks, not live playback here.
Current dedicated setup has no eligible provider; disabled resolver controls are
correct but real provider tests remain blocked. No provider credentials entered.

See `BLUESTACKS-SETTINGS-TESTS.md` and `bluestacks-settings-tests.json` for per-binding
scope/evidence and untested entries. Original preferences restored, including
Dynamic Home, Modern sidebar On, Hide pill Off, depth master/surfaces On, width
134 dp, radius 12 dp; original language/accent/background remain restored.
Account/settings uploads and arming remain blocked. No media playback or provider mutations, destructive/consent actions,
main-account inspection, commit/push/merge. Local preference writes were the
approved test actions; authentication refresh remains permitted by this build.

Not complete: every original setting is NOT individually certified. Remaining
preview mismatch: animated card-width/corner illustrations do not depict selected
geometry reliably, although real Home responds. Singular filter-rule wording
also needs correction. Neither was changed in the installed APK in this round.

## 8 November 2026 — BlueStacks playback retry and further settings checks

After the user's source-addon setup, Scrubs from Continue Watching plays, with
English embedded dialogue subtitles selected automatically before manual choice.
Actual subtitle Off/English selection and yellow dialogue rendering verified.
Primary subtitle style controls exercised individually with original values
restored; some evidence is the rendered style sample rather than actual timed
cues. Do not generalise this to every format, secondary track or global setting.

Further UI checks: next-episode toggle/countdown, post-play recommendations and
percentage, still-watching interval, automatic full subtitles and subtitle-list
mode. All tested values restored. End-of-media effects remain pending. Actual
interface font, scale, settings Panel/List, clock visibility and 12/24-hour
format changes observed and restored. App Icon & Banner remains supporter-gated;
no entitlement/security bypass or purchase performed.

Local corrections (not yet in approved installed APK): correct Subtitle font
label in player; correct English singular filter/episode wording; poster width
and radius illustration now reflects the selected geometry including radius 0,
instead of free-running width/corner animation. Other illustrative card calls
retain their prior default radius. New actual-GL framebuffer regression proves
selected geometry changes pixels and remains stable across elapsed idle time.

Regression results PASS: settings native callbacks/persistence, Stage A, subtitle
automatic selection, in-player style controls, both subtitle selector suites,
Continue Watching parity, 413-binding preservation, read-only/login validation
boundaries, actual poster-preview pixels, speed list/clock/real Android bridge
and PCM restoration mocks. Speed harness initially needed its supported
NUVIO_TEST_JAVA override for the existing JDK; rerun passes. Both subtitle
selector harnesses initially missed production legenda_idioma at link time;
linking existing legenda.c fixes the harness dependency and both pass. Initial
preview run lacked Mac graphics-service access; graphics-enabled rerun passes.
Ignored evidence: build/settings-expansion/playback-*.log.

Device caveats: no objective audio/speed measurement. Around speed changes,
reprepare and E-AC-3→AAC metadata transition observed; subtitle/track retention
needs controlled reproduction, not a speculative playback fix. Local progress
advanced/restarted/seeked during approved playback. Private-file backup was
blocked by BlueStacks; user informed, no security changes attempted. Exact
progress/title-choice restoration cannot be claimed. Settings/progress/provider
writes and sync arming remain blocked; authentication refresh permitted.

BLUESTACKS-SETTINGS-TESTS.md and bluestacks-settings-tests.json now record **51/434
binding observations**, explicitly distinguishing consumer effects, UI-only and
blocked results. **All-settings certification remains incomplete**; unvisited
entries stay NOT TESTED. No new APK installed, no commits/push/merge, branch stays
codex/settings-restoration. Existing work preserved. Remaining: approved updated
APK for local UI fixes, remaining individual device/format/end-of-playback tests,
and supported hardware/provider tests within their existing approval boundaries.

## 8 November — approved validation APK installed; individual testing ongoing

User approved installing the corrected read-only APK and continuing local tests.
Installed package readback matches SHA-256
`197cc1d01066c3712a67283cef95203e81ac5e569a286bd7f6e5eb11a776b048`.
Package/certificate/data retained; both ABI validation/login flags verified.
No settings/progress/provider uploads or sync arming; authentication refresh is
permitted. No Fire Stick actions, main-account inspection, commit/push/merge.

Actual BlueStacks consumer confirmations added: Editorial English Home headings;
all five profile-picker background modes (remote artwork branch excluded);
profile-picker programme preview Off/On; Continue Watching row visibility;
episode-versus-series artwork; upright Continue Watching Poster mode; corrected
selected-width/sharp-radius previews; corrected Subtitle font label; global
subtitle size reaches player and restores to 120%; automatic external English
subtitles; manual dual external subtitle rendering; four independent sidebar
visibility controls and restoration. Full details and limitations are recorded in
BLUESTACKS-SETTINGS-TESTS.md and bluestacks-settings-tests.json. Observation counts
include partial and blocked entries and are **not full functional passes**.

Quality preference: 720p device trial reused/resumed 1080p; actual source policy
is a preference with only-source fallback, not strict exclusion. Added regression
for all four persisted resolution choices and fallback. No speculative resolution
policy change made. Secondary-language preference does not establish automatic
secondary activation; manual dual rendering works. Pause-panel Off kept paused
controls stable and Back exited; original On restored. Idle-overlay/pointer-hover
timing with On remains a device verification concern, not a confirmed fixed bug.

Local PASS: 413 preservation; Continue Watching parity; Stage A (earlier in this
round); Stage B, C and D; native settings callbacks/independent persistence/Back/
category routing/subtitle safety; automatic subtitle mocks; Home layout/title
consumer regression; poster-provider and real metadata-root parser; source-quality
selector; complete Back suite including paused case, webOS/sheets/timeouts and
both Tizen shells. Existing Node runtime used for Tizen, no dependency installed.
Top-menu regression PASS with graphics access after fixing the obsolete Search
fixture's missing Nuvio source identity. Poster parser link repaired by stripping
unexercised functions, without stubbing exercised behaviour. Home trailer browser
harness remains blocked by its hard-coded unavailable Emscripten location.

Testing remains in progress. Screensaver temporarily Off still needs restoration
to original 2 min. Remaining unvisited controls, hardware/provider-specific tests,
additional global secondary subtitle consumers and end-of-playback behaviour are
not certified. Do not report every setting tested or complete.


## 8 November — further individual Home and subtitle checks

BlueStacks observations now cover **91/434 binding entries**, including partial
and blocked entries; this is not 91 full functional passes or all-settings
certification. Dynamic background Color/Ocean produced navy Home; Immersive
produced artwork-toned surfaces. Original Dark gray restored. Hero ratings,
year and runtime were each toggled independently and their actual metadata
changed on the same Abbott Elementary hero. All three hero sizes changed real
Home geometry. Original ratings On/year Off/runtime On/Extra large and Show hero
Off restored. Folder-details toggle saved/restored, but the tested episode hero
is not a folder hero: consumer verification remains partial.

**All pending temporary preferences from these tests are restored**, including
Screensaver **2 min**, Showcase/Catalog and Dynamic background **Dark gray**.
Subtitle-language editor inspected then cancelled without changing its original
From account selection. Trailer button is On, but inspected titles had no known
trailer: no trailer-button consumer pass is claimed. No account/provider/settings
or progress uploads, sync arming, Fire Stick operations or APK installation.

Additional isolated functional PASS:

- Main subtitles: all **10 ordinary controls / 1,268 legal choice or slider-step
  values**, through actual Settings callback, player-style consumer, preference
  file reload and independent restoration. Includes all 1,201 delay steps.
- Secondary subtitles: **76 style choices** reach the actual renderer style;
  representative actual GL pixel effects and exact restoration for all ten
  fields; secondary timing moves cues out of/into the real renderer window.
- Card depth: actual GL framebuffer changes for all five independent surfaces,
  master Off, all three edge-strength presets and coverage modes, sheen strengths.
- Post-play: real automatic-next control flow, all 20 still-watching intervals,
  multi-session count carry and manual reset, acknowledgement, autoplay Off and
  all 100 recommendation percentages/master Off. Preview all 301 delay choices,
  cancellation/error/timeout/no-retry behaviour also pass.
- Source filtering: all preferred/required/excluded choices, sort/size/quota and
  stable 20,000-source selection; audio selection, dual-subtitle state under
  ASan/UBSan, subtitle delay sign and 413-binding preservation pass.
- Hero artwork: native and Tizen policies under ASan/UBSan pass after correcting
  the unchanged-checkpoint fixture to include the existing virtual TMDB logo
  fallback and its definitive-failure branch. No production artwork change.

Harness fixes: subtitle-border test now links the real font/ASS modules; black
subtitle-background pixels are measured on a neutral video-like surface; current
subtitle delay fixtures include the sign-version marker and reset the cached
reader per simulated restart. No application bug was established by those
initial harness failures. Existing optional screenshot input used the stale
secondary-size key; corrected to legenda2EscalaLocal. Missing host LG Display
and Droid Sans font files limit exact font-face rendering claims; fallback does
not certify those faces on a TV.

Evidence: build/settings-expansion/remaining-*.log, secondary-consumer-shots/;
structured detail in bluestacks-settings-tests.json. Existing device-only,
credential/provider/account actions and unvisited controls remain uncertified.
No AMOLED/pure-black-named setting was found in the current settings source;
this is a requested-feature gap, not a passed control. No commit/push/merge or
production implementation changes in this additional round.


### Further filter and navigation checks (8 November continuation)

Settings Panel/List layout works through the TV remote header: List removes the
inspector, widens rows, survives category navigation and retains the remembered
Playback row; Panel and first Content row restored. Header pointer clicks did not
activate in this TV build; this does not certify mouse/touch operation.

Expanded source-filter tests now classify realistic filename tags before
exercising all 76 options in Preferred/Required/Excluded modes. Language codes
are tested as explicit tags; `It.2017.mkv` is not Italian. New
`tests/stream_filter_metadata.sh` links the actual Stream conversion and filter
consumers: all seven dimensions, release groups, structured height/MiB fallback,
unchanged source metadata, Unknown/live/cached classification and rejection of
URL text as media-tag evidence PASS. No production source change.

Final reruns PASS: Stage A category/scene/English checks across six builds,
Stage B 413 provenance records/125 verified Legacy controls/gates, Stage C
resolver/default-Off safety contract, Continue Watching parity and read-only
validation transport guard. Full log manifest is in
bluestacks-settings-tests.json. Its **91 device observations** include partial or
blocked cases and do not mean every control is functionally certified.
All temporary preferences restored. No APK update, backend/provider mutation,
commit, push, merge or branch switch in this continuation.


## 8 November — continued testing and real addon-label defect fixed

Continued automatically after user requested no pauses between batches.
Individual device observations now cover **97/434 entries**, including partial
cases. Unvisited entries remain NOT TESTED; this is not complete certification.

New actual device checks: poster labels On/Off remove/restore text beneath
portrait cards without removing in-card landscape text; row type suffix adds
`- Movie` to Movies by Year and avoids duplicating Series; focused-poster
expansion Off retains portrait geometry, On expands after the original 2 s dwell;
focused-card border Off removes the ring while size still indicates focus.
Original labels On/suffix Off/expansion On/delay 2 s/border On restored.

**Confirmed application defect:** Add-on name in the row persisted but its getter
had no consumers. Compared pinned official commit
99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f, CatalogRowSection.kt showAddonName
and catalog_from_addon (`from %1$s`). Added a locked display-name copy for the
exact catalogue key and a runtime-only Home row snapshot. Home now reads the
existing setting on each draw and shows a subdued `from <addon>` beside catalogue
headings. Native fixed header height, stable IDs, row order, counts and navigation
are unchanged. No URLs used as fallback, app/collection rows excluded; no new
stored keys, defaults, sync fields or protocol changes. Original app places the
caption below the heading; Native keeps it inline to preserve row geometry.

Local PASS: actual heading consumer in all four title styles (On/Off/unknown-name,
identity unchanged); duplicate-title/instance display-name and profile isolation;
full Home harness; Stage G 10/15/20/30 rows and Native preservation; J5 inventory;
Stage A English/scenes; 413 preservation. Evidence addon-heading-*.log in
build/settings-expansion/. Build initially rejected the new translation entry
because parallel tables were not aligned; corrected only the new entry across
29 tables, existing translations retained. Complete idiomas checker PASS.

**User-approved APK update installed and read back from BlueStacks:**
build/settings-expansion/Better-Nuvio-addon-heading-validation.apk
SHA-256 24875b3ee997d220aa26e9c8fc9e440e3834e2d4d954d762b3216895b7c3af6f
Size 74,670,880 bytes. Same space.nuvio.nativelegacy.strictdebug package and
certificate c0a433a85a1e3ca1fa9e1ef4b5998b36444b0cc8efc88c9a3ccb97d2f7c59bba.
Both ARM64/ARMv7 libmain contain the new caption and locked-copy function;
1,129 asset entries, TV launcher, signature and zip/16 KiB native alignment PASS.
Both ABI read-only and login flags verified. No automatic writes/arming enabled.
The launch needed a second/double launcher activation, not a code or security
change; profile picker and existing profile 1 loaded with data retained.

**Fix verified on the actual installed APK:** On shows `from Nuvio Catalog Addon`
beside Trending Movies and Trending Series, without a caption on Continue Watching
or Family Watching, and without counter overlap. Reopening settings retains On;
restoring Off removes captions immediately. No live provider/account mutations,
Fire Stick operations, commit/push/merge or branch changes. Other existing work
preserved. Testing continues; do not report all 434 settings verified.


## 8 November — focused pre-release check completed

User approved moving from exhaustive manual settings certification to a focused
critical-flow smoke check plus automated regressions. **15/15 relevant suites
PASS**: settings A/B, original 413 preservation, C resolver/default/provider and
stream paths, TorBox D/cached-addition/autoplay, verified-uncached filtering,
metadata conversion, Continue Watching, settings persistence, subtitle automatic
selection, player preferences, read-only safety and Stage G profile/Home rows.
Player persistence initially failed at SDL macOS display initialisation in the
sandbox, before assertions. Dummy-display rerun passed saved-value recovery in
separate processes and one-time delay migration; no production changes needed.
Original and rerun logs retained in build/settings-expansion/focused-release/.
Existing unchanged Android build evidence: 64 unit tests, six suites, no failures.

BlueStacks: Scrubs resumed from Continue Watching with automatic English dialogue
subtitles; advancing playback, Pause/Continue, exit/reopen, retained login, profiles
and saved appearance values observed. Android Back exited cleanly; keyboard
Escape around pause overlays remained timing-sensitive. Returned to profile 1
Home with no temporary preference edits pending. Local progress advanced during
approved playback; exact pre-test restoration is not claimed.

See FOCUSED-RELEASE-CHECK.md and bluestacks-settings-tests.json for evidence and
limits. Counts remain 98/434 individual observations including partial/blocked
cases, 336 unvisited: this is NOT exhaustive certification. TorBox operations are
mocked; physical TV/hardware variations and real write/sync paths remain outside
this read-only check. No new installation, account/provider writes or sync arming.

Approved installed APK checksum rechecked:
24875b3ee997d220aa26e9c8fc9e440e3834e2d4d954d762b3216895b7c3af6f.
This artifact is deliberately read-only and is not a public-release build. The
next step is release preparation: review intended uncommitted changes and secret/
artifact exclusions, produce normal-build validation and accurate release notes.
User said a GitHub release can be considered after testing; no commit, push, merge
or publication has been performed or authorised by that statement.


## 8 November — local release preparation

Built a normal, write-capable strictdebug preview with validation flags OFF in
both ABIs; no install/launch. Package and signing certificate unchanged. APK:
`build/release-preparation/Better-Nuvio-2.0.8-strictdebug-normal-preview.apk`,
74,670,667 bytes, SHA-256
`b98b067ea1a74b9ba1040a42088ad6e48ddd32c85ab4feca13e3421bbf9b2784`.
Signature, assets, TV launcher, both current packaged native libraries, ZIP and
16 KiB alignment PASS. Fresh release-tooling/J7 security mocks PASS; normal Android
unit task PASS (64 tests/six suites), focused regressions remain 15/15 PASS.

Draft release notes and detailed review in RELEASE-PREPARATION.md. Current-text
heuristic scan found only a synthetic credential-URL negative test. Two generated
logs excluded from proposed checkpoint; nothing staged. Internal documents and
visual references still need publication-scope/privacy review.

Confirmed public-source blocker: reachable history contains non-empty historical
personal credential files at `4ea949d33baa`. Values not printed; credentials not
used, validated or changed; no history rewrite. Resolve credential rotation and
safe publication strategy before pushing this ancestry publicly. Production
package/signing/version and unchanged Android J4/J5/J6 release write gates also
need an explicit release decision. No commit, push, merge, master change, backend
mutation, APK installation or publication. All previous work preserved.

## 8 November — APK-only prerelease bundle prepared

User selected APK-only strictdebug prerelease. Local bundle and exact proposed
GitHub title/tag/anchor/assets documented in RELEASE-PREPARATION.md. Destination
is public; existing remote anchor verified read-only. Bundle contains only APK,
checksums, notes/install instructions and a local manifest; no local source/history.
GitHub's automatic archives of the old anchor are explicitly not this APK's source.
Checksum reverified. No release/tag created, upload, commit, push, merge, install,
account mutation or master modification. Publication approval remains pending.


## 8 November — approved APK-only prerelease published

User explicitly approved public publication of the three reviewed assets, release
notes and tag at the existing remote anchor. Release published at 14:42:29 UTC:
https://github.com/DetectiveSula/better-nuvio/releases/tag/v2.0.8-better-nuvio-preview.20261108.1

GitHub API confirms prerelease=true, draft=false, exact approved tag anchor
`7cac4f25d11931b56c41cfefb952e4e183c45c1a`, and unchanged remote master. Release body
matches approved local notes. Downloaded all three public assets after publishing:
APK, SHA256SUMS and INSTALL.txt; every byte count and SHA-256 matches local approved
files and GitHub asset digests. APK: 74,670,667 bytes; SHA-256
`b98b067ea1a74b9ba1040a42088ad6e48ddd32c85ab4feca13e3421bbf9b2784`.

Evidence: build/release-preparation/published-verification.json and
published-prerelease.jpg. The APK-only local ZIP remains the original prepublication
preparation snapshot. No local source/history uploaded, local commits/pushes/merges,
branch movement, device install, live provider operation or application-account
mutation. GitHub's automatic old-anchor source archives are explicitly disclosed
as not matching the APK's uncommitted working-tree source. Historical credential
cleanup remains separate and unresolved.

Upload initially required manual user-enabled ChatGPT extension file-URL access;
browser policy blocked automating chrome://extensions and no bypass was attempted.
After user confirmation, uploaded only the approved checksum/install files and
published. Existing local work on codex/settings-restoration is preserved.

## 8 November — Better Nuvio Android updater implemented locally

User requested repointing updater. Android strictdebug now discovers compatible
Better Nuvio prereleases, with exact repository/tag/variant and per-asset SHA-256
checks. Android adds explicit package, newer-version-code and exact-signer checks
before opening the system installer. Production Android stable feed points to
Better Nuvio; other platforms unchanged. No security/arming gates weakened.

New full/Lite preview mocks, existing production variant/general/scheduling tests,
69 Android unit tests, 413 preservation, Continue Watching and read-only guard
PASS. Normal local APK built/verified with unchanged strictdebug package/cert:
`build/updater-better-nuvio/Better-Nuvio-2.0.8-strictdebug-updater.apk`, SHA-256
`dcfe666a8f04960a236907e0c199478c1b9f41f7b6182f0d7e18c38c94c1b404`.

See BETTER-NUVIO-UPDATER.md for feed/asset conventions, results and limitations.
Published APK is unchanged and still has the old updater; initial updater-enabled
APK needs a manual install. Future updates must bump numeric version/versionCode;
same-version recuts are not upgrades. No new publication, install, commit, push,
merge, branch switch, master modification or live application-account operation.


## 8 November — updater preview 2 published with explicit approval

Published v2.0.8-better-nuvio-preview.20261108.2 at 15:31:35 UTC:
https://github.com/DetectiveSula/better-nuvio/releases/tag/v2.0.8-better-nuvio-preview.20261108.2

GitHub API confirms prerelease=true, draft=false, exact approved existing anchor
7cac4f25d11931b56c41cfefb952e4e183c45c1a and unchanged remote master. Preview 1 remains
available with its original APK digest. Downloaded APK, SHA256SUMS and INSTALL.txt
from the public release; all byte counts and hashes match the local approved
files and GitHub digests. Body matches the prepared notes.

APK SHA-256: dcfe666a8f04960a236907e0c199478c1b9f41f7b6182f0d7e18c38c94c1b404.
Size 74,671,919 bytes, unchanged strictdebug package/certificate and version 20008.
One manual install is needed to bootstrap the Better Nuvio updater; later in-app
updates must increase numeric version/versionCode. Physical installer validation
remains outstanding. No automatic installation, local source/history upload,
commit, push, merge, master modification, sync arming or provider/account mutation.
Evidence: build/updater-better-nuvio/published-verification-2.json and
published-prerelease-2.jpg. Previous local work and preview 1 preserved.

## 8 November — Home/settings defect fixes (local, no APK update)

Completed the requested control review and local fixes. See
`HOME-SETTINGS-REVIEW.md` for the per-control findings, source scope, tests and
remaining limitations.

- Background preview height now follows responsive 16:9 card dimensions and
  includes caption/padding space. Inspected native 1920×1080 screenshots confirm
  Logo colour no longer overlaps the expanded Background chooser.
- Modern sidebar opening/closing spring settles exactly; Modern sidebar On/Off
  now changes floating/docked geometry. Its actual glass material respects the
  blur preference. Android/BlueStacks flicker has not been rechecked on-device.
- Removed Home hero artwork/genre preference leakage into ordinary landscape,
  editorial and large poster cards, Friends art, detail/recommendation artwork
  and paused-player backdrop. Hero-specific controls retain their own consumers.
- All seven non-hero Home poster-rating lookups now honour IMDb rating on posters;
  hero ratings stay separate. A rating baked into an external poster image remains
  part of that image and needs a rating-free provider/template.
- Hero catalogue chooser tested using real OK/Down/Back dispatch; fixed detached
  J5 observer/stale Automatic projection edge case and initial selected focus.
  Exact installed-device failure was not reproduced; verify in the next APK.
- Renamed Friends row style to Family row style, preserving friendsRowStyleLocal
  and its six integer values. Family renderer now implements six distinct card
  shapes/presentations; original Friends implementation remains available.
- Row spacing already affects layout/scrolling in all three Home modes. Collection
  tab style (clarified label/help) and Enriched context menu have active consumers;
  tests confirm their scope/gates. No cosmetic replacement toggles added.
- Existing custom-poster URL/instance/token/key/parameter/test controls moved beside
  Custom posters in Layout. Custom URL template is an explicit provider choice;
  URL validation, placeholders, local persistence, fallback and redaction retained.
- Added missing explicit icons for recently appended settings exposed by the
  interaction suite. Fixed two narrow legacy test harnesses, without changing
  account or input application behaviour.

PASS: Stage A/B, 413 preservation, settings interaction (responsive row bounds,
hero catalogue remote input and custom URL persistence), J5 two-device mocks and
Native J5 projection, all six Family rendering styles, enriched-menu eligibility,
desktop/touch sidebar geometry and settled animation, all five collection tab
styles, poster providers/long templates/redaction, Home geometry/rendering,
hero identity, recommendation artwork (27 checks), Continue Watching parity,
Stage C controls/resolution/preparation, Stage D cached classification/autoplay,
TorBox cached-addition/duplicate suppression and verified-uncached stream filtering.
All provider/account operations in these tests used isolated fixtures/mocks.

Evidence: `build/settings-home-review-20261108/results.json`, associated logs and
`background-v2-fundo-opcao.png` / `background-v2-fundo-opcao-q90.png`.
Initial failures are retained separately; final selected runs all returned zero.
The sandboxed screenshot attempt could not initialise macOS video; the isolated
fixture succeeded with native window access and fresh temporary data.

No binding IDs, storage keys, default baseline, sync schema or progress formats
changed. Existing working-tree changes preserved on codex/settings-restoration.
No Android APK build/install, publication, commit, push, merge, master change,
account access, sync arming or backend mutation. Published/installed APKs still
contain the previous code. Remaining: build a new APK and verify physical remote
navigation, responsive layout, catalogue selection and sidebar flicker on the
actual Android/BlueStacks device. This review does not certify all 434 settings.

## 8 November — fresh settings-fixes Android TV APK built and verified

Built with the existing local SDK/NDK/Java/Rust configuration using
`bash tools/android.sh --debug-only`, offline dependency mode. Build SUCCESSFUL;
ARM64 and ARMv7 native targets rebuilt. Normal debug build, validation-readonly
and validation-login both OFF; no signing, package or version changes.

APK:
`build/settings-home-review-20261108/apk/Better-Nuvio-2.0.8-strictdebug-settings-fixes-20261108.apk`

- Size: 74,673,350 bytes (71.21 MiB).
- SHA-256: `1c5b879df9b645d2291139d28a012ba97b6779b3b9cf0b423afb323031da70f1`.
- Package: `space.nuvio.nativelegacy.strictdebug`, version 2.0.8-strictdebug / 20008.
- Certificate SHA-256 unchanged:
  `c0a433a85a1e3ca1fa9e1ef4b5998b36444b0cc8efc88c9a3ccb97d2f7c59bba`.
- APK v2 signature verified; ZIP alignment and ARM64 ELF 16 KB alignment PASS.
- ARM64/ARMv7 libraries match the built stripped artifacts byte-for-byte; both
  contain the new settings help/presentation markers. Source hashes recorded.
- TV launcher categories, banner resources, art/fonts, ZIP integrity and exclusion
  of named local credential/signing files checked.
- Android unit task SUCCESSFUL: 69 tests, zero failures/errors/skips; Gradle reused
  up-to-date results because JVM sources/test inputs were unchanged.
- Existing 11 final host regression groups remain PASS; reran the 413-binding
  preservation guard successfully before building.

Evidence: `build/settings-home-review-20261108/apk/verification.json`, SHA256SUMS,
Android build/unit logs, manifest, signature and alignment reports.

Not installed or published. No account/provider operations, sync arming, commit,
push, merge or master changes. The currently installed/published APK remains
unchanged. Physical-device confirmation of sidebar flicker and hero catalogue
selection remains outstanding. This same-version local APK is for manual update;
the updater requires a higher numeric version to offer a future release.

## 8 November — settings-fixes preview 3 published at the user's request

Published APK-only prerelease `v2.0.8-better-nuvio-preview.20261108.3` at
18:16:42 UTC (release ID 438428675):
https://github.com/DetectiveSula/better-nuvio/releases/tag/v2.0.8-better-nuvio-preview.20261108.3

The public release contains the verified settings-fixes APK (74,673,350 bytes),
SHA256SUMS and INSTALL.txt. Downloaded and hashed all three public assets; each
matches the prepared local file and GitHub digest. APK SHA-256:
`1c5b879df9b645d2291139d28a012ba97b6779b3b9cf0b423afb323031da70f1`.
Release body matches the prepared notes after normalising GitHub CRLF endings.

Confirmed draft=false, prerelease=true, and tag anchor
`7cac4f25d11931b56c41cfefb952e4e183c45c1a`. Remote master remains at that same
commit. No local source/history push, source commit, branch movement, merge or
installation. Prior previews remain available. Release notes explicitly disclose
that automatic source archives describe the old anchor rather than this binary's
uncommitted source. Same numeric version/code requires manual installation; the
in-app updater will not offer this recut. Physical-device checks remain pending.

Evidence: `build/settings-home-review-20261108/prerelease-3/` includes the reviewed
bundle, `published-verification.json` and `published-release.jpg`.

## 8 November — details sidebar transitions and film post-play fixes

Owner confirmed sidebar flicker only during opening/closing on details, and the
post-play failure was a film. See `DETAILS-POSTPLAY-REVIEW.md` for source findings,
behaviour, evidence and device limitations.

- Modern details overlay now fades at full panel size, removing its changing
  frost sampling footprint. Home's rail expansion remains. Native GL blur,
  open/close and actual SDL Back tests PASS; physical Android confirmation pending.
- Direct Continue Watching movie opening now requests the existing related-title
  metadata when recommendations are enabled; details is no longer the only path.
- EOF retains a visible recommendation/presence panel instead of exiting
  immediately. Dismissal releases it normally. A confirmed fractional EOF supports
  the 100% threshold; early/truncated EOF is rejected and other percentages keep
  their existing semantics. Available related metadata is still required.
- Are you still watching verified for all 20 intervals, Off/autoplay Off,
  automatic episode close/reopen, persistence while awaiting confirmation, and
  manual-stop reset. Repeated/held OK cannot confirm the prompt. Requires next
  episode autoplay; manual Next/seek resets an unattended sequence by design.

PASS: settings A/B; 413-binding preservation; actual post-play panel, EOF and
metadata wiring; player preference persistence and recuo; Continue Watching;
TorBox cache/cached-addition/autoplay; verified-uncached stream filtering; menu
regressions and native details GL rendering. Actual panel tests cover 10–100%,
late/empty results, master Off and dismissal; policy tests cover all 100 values.
Post-play HTTP transport disabled in its isolated fixture; provider tests mocked.
Initial fixture failures retained; final selected regressions all PASS.

Built/verified a new Android debug APK; all 69 Android JVM tests rerun and PASS:
`build/details-postplay-review-20261108/apk/Better-Nuvio-2.0.8-strictdebug-details-postplay-fix-20261108.apk`

Size 74,673,018 bytes. SHA-256:
`66b3967ba529aea2384607c8af7577efe1f3000a4c8c0ebd833676f32db26ea3`.
Package `space.nuvio.nativelegacy.strictdebug`, version 2.0.8-strictdebug / 20008;
unchanged debug certificate. ARM64/ARMv7 packaged libraries match built artifacts;
signature, ZIP/ELF 16 KB alignment, TV launcher/assets and ZIP integrity PASS.
Normal write-capable build; no sync arming or live account/provider mutations.

Evidence: `build/details-postplay-review-20261108/`, `results.json`, native
screenshots and `apk/verification.json` / SHA256SUMS.

All existing local changes preserved on codex/settings-restoration. No install,
commit, source push, merge, publication or master modification. GitHub preview 3
remains unchanged and does not contain these new fixes. A device install needs
approval, followed by confirmation of transition flicker and real playback.

## 8 November — details/post-play preview 4 published at the user's request

Published APK-only prerelease `v2.0.8-better-nuvio-preview.20261108.4` at
19:10:33 UTC (release ID 438458586):
https://github.com/DetectiveSula/better-nuvio/releases/tag/v2.0.8-better-nuvio-preview.20261108.4

Public APK (74,673,018 bytes), INSTALL.txt and SHA256SUMS all downloaded and
verified against their local prepared bytes and GitHub digests. APK SHA-256:
`66b3967ba529aea2384607c8af7577efe1f3000a4c8c0ebd833676f32db26ea3`.
Release notes match after normalising line endings; draft=false, prerelease=true.

Tag anchors the existing remote commit
`7cac4f25d11931b56c41cfefb952e4e183c45c1a`; remote master is unchanged at that
commit. No local source/history was uploaded, no source commit/branch movement,
merge, installation, sync arming or account/provider mutation. Previous previews
remain available. Source archives describe the old anchor, not the current APK
source, as disclosed in the notes. Manual installation remains required because
numeric version/code is unchanged. Physical-device confirmation remains pending.

Evidence: `build/details-postplay-review-20261108/prerelease-4/`, including
`published-verification.json` and `published-release.jpg`.


## 8 November 2026 — BlueStacks details/Coco follow-up and local TMDB key

Branch remains `codex/settings-restoration`; all existing work preserved. No
commit, source push, merge, master change or new GitHub publication.

Observed on BlueStacks preview 4 (installed checksum verified): foreground
Ratings content in the sidebar blur over scrolled details. Coco at 1:36:24 of
1:45:02 reached the actual credits-marker recommendation gate but displayed no
recommendation panel. The issue is not fixed by adjusting its percentage.
The earlier top-of-details test did not validate Ratings/Cast. See
`DETAILS-POSTPLAY-REVIEW.md` for observations, source trace and remaining limits.

Implemented locally:
- Reserve the details sidebar strip before page content is drawn; use a strict
  background-only blur source instead of switching to live foreground captures.
  Retain existing blur preferences, Home behaviour, focus and Back routing.
- Add the user-approved **Integrations → TMDB → TMDB API key on this device**
  action. Appended ID/default only; no existing binding renumbered. A blank,
  masked keyboard accepts a 32-character hexadecimal v3 key or explicit removal.
  Private atomic 0600 storage is account/profile scoped on this device. No raw
  key is shown, logged, included in preferences or synchronised. No Legacy
  badge is assigned to this new credential action. Existing account/app-key
  fallback and master Off semantics remain intact.
- Invalidate only metadata requests after a key edit, preserving MDBList and
  other account credentials. Reopen the film after saving to refresh results.
- Fix the separate request gate where movie recommendations alone could not
  fetch a TMDB movie response with other extras Off. Reuse the existing HTTP
  and parsing path; same-title request reuse now includes recommendation policy.
- Replace per-frame empty-list logging with safe state-change counts.

The APK/build configuration contains no bundled TMDB key or key asset. Account
credentials were not inspected; their absence is not assumed. Recommendations
still require a usable enabled metadata source and a non-empty response. The
new credential field makes local configuration possible without putting a
personal key into a distributed APK. No personal key was requested in chat.

Validation passed:
- Original 413-binding/default/progress-format preservation.
- Settings A/B (including category routing/English/focus) and resolver C/D.
- Actual threaded movie metadata request/JSON parser with canned HTTP, independent
  post-play request gate, same-title refresh/idempotence and provider master Off.
- TMDB local-key validation, masked keyboard/status, 0600 persistence/reload,
  account/profile isolation, removal/fallback, exclusion from J4 allowlists and
  ordinary preferences, and preservation of the MDBList credential override.
- Post-play recommendations/countdown/presence tests, player opening/EOF wiring,
  TorBox cached addition/autoplay, verified-uncached UI filtering and CW parity.
- Actual native Ratings/Cast scenes, sidebar blur-source assertions, Back, and
  GL pixel checks for first/new regions, foreground cards/veils and repeated
  frames. The region-expansion fixture now reserves its overlay before content,
  matching the real application contract. No synthetic pattern covers the page.
- All 69 Android JVM tests: zero failures/errors. ARM64 and ARMv7 implementations,
  TV launcher/assets, unchanged strictdebug package/certificate, signature,
  ZIP alignment and 64-bit ELF 16 KB alignment verified.

Initial failures are retained in evidence: missing SDL window-server access,
English/append-only frozen-file guards needing explicit additions for the new
credential action, a post-play test matching the old diagnostic text, and an
Android unit invocation missing ANDROID_HOME. Corrected configuration/fixtures
and reran the affected checks. No original preservation baseline regenerated.

Final APK:
`build/details-device-followup-20261108/apk/Better-Nuvio-2.0.8-strictdebug-sidebar-tmdb-final-20261108.apk`

Size: 74,676,268 bytes. SHA-256:
`c4ece159579bacca8e4e655ca6613d178b615acbe59e0189086eea1690f4e93d`.
Package `space.nuvio.nativelegacy.strictdebug`, numeric version 20008 and signing
certificate unchanged. This is the ordinary write-capable build, not a new
read-only validation variant. Existing sync permission gates remain unchanged.

Device verification remains pending. The owner approved installation only of
an earlier prepared checksum; final review corrected metadata invalidation to
retain MDBList credentials, so that APK was superseded. Separate final-checksum
installation approval has been requested; the final APK has not been installed
or launched. Do not enter a key, arm sync or perform backend writes automatically.
After approved installation, the owner must enter a valid key locally if needed,
then repeat Coco and scrolled Ratings/Cast checks before device success is claimed.
No live-provider operations or account settings writes were performed by the
regressions; all new HTTP/key tests use isolated synthetic data.

Evidence: `build/details-device-followup-20261108/results.json`, final regression
logs, device screenshots, native screenshots and `apk/verification-final.json`.
Stages E/H were not begun; this is not certification of all settings or all TVs.


### Final APK installed on BlueStacks — 8 November 2026

The owner explicitly approved the final checksum
`c4ece159579bacca8e4e655ca6613d178b615acbe59e0189086eea1690f4e93d`.
Installed that exact APK on BlueStacks only (`127.0.0.1:5555`) using `adb install -r`:
Success. Verified the installed base APK SHA-256 matches exactly, package remains
`space.nuvio.nativelegacy.strictdebug`, versionCode 20008. Existing app data was
retained; no uninstall/clear, automatic launch, key entry, settings edit, sync
arming, backend write, Fire TV action, commit, push or publication was performed.
The earlier pending-install note is superseded by this installation evidence.
Actual sidebar and configured live recommendation behaviour still await retesting.
Evidence: `build/details-device-followup-20261108/apk/bluestacks-installed.json`.

## Automatic settings sync — 12 November 2026

Owner decision: settings and layout "should automatically sync" between TVs. This
replaces the J4/J5/J6 default-Off policy and the J7 debug-arming requirement.

* **Cause found:** release Android compiled `j7arme_permite_j4/j6` to a constant 0
  (`src/j7arme.h`), and the J4/J5 and J6 permissions had no UI and defaulted Off.
  The release app on BlueStacks logged 18 J4/J5 and 5 J6 edits waiting with uploads off;
  `[bn-sync] ... 0 campo(s) recebidos` meant no TV had ever uploaded the `better_nuvio` blob.
* **Change:** every build lets the signed-in account/profile upload (`src/j7arme.h`,
  `src/j7arme.c` `permite`/`j7arme_passo`); J4/J5 writes start On per context
  (`src/ajustes_better.inc`) and J6 permission starts On (`src/ajustes_official.inc`),
  through `NV_SYNC_AUTO` (`src/ajustes.h`, 1 by default; 0 only in the J4/J6 engine
  suites). The read-only validation build still refuses. Merge rules are unchanged:
  only fields edited on this TV, against a fresh pull, with the existing conflict rules.
* **Bug fixed on the way:** a fresh install returned from `ajustes_dir` before writing
  the one-time migration markers, so its second launch reset theme/logo colour/depth/
  scale/profile background etc. With sync on, that reset would have uploaded and undone
  the same choices on every TV. Fresh installs now mark those migrations as done.
* **Still read-only:** regular Nuvio's `home_catalog_shared` row order (Stage G). Better
  Nuvio's own Home layout (J5) syncs through `better_nuvio`.
* **Known:** the retired key `trailerDetalheSomLocal` (#509) is normalised Off once per
  boot if another TV stores it On; this converges after one upload.
* **Tests:** new `tests/settings_sync_auto.sh` (two devices, no arming, plus read-only
  build). `settings_stage_j7.sh` default-Off/arming flow retired (read-only mode kept).
  Stage B guard gained `sync-auto-wiring.json` for the `ajustes_dir` addition.
  `settings_stage_i.sh` fails on consumer lines in `descoberta.c`/`extras.c`, which
  predates this change.
* No device install, live backend contact, commit or push.

## Autoplay speed: one TorBox library read per search — 12 November 2026

BlueStacks log: "Checking the source" took 28.1 s; 7 hash-carrying links each paged
the whole TorBox library (`resolverExistente`, up to 8 x 100, bypass_cache) and failed
silently before the 8th played. The existing-item lookup now reads the library once per
search (debrid session + search generation, same TorBox key, 60 s at most) and reuses
it for every link; a TorBox add clears it, and the add-if-cached path always re-reads
(an item it just added can appear late). New logs: `[debrid] TorBox library read …`
(counts, pages, ms) and one `[resolver] …` line per check (path, result, ms; never the
hash). Test: `tests/torbox_library_once.sh`; Stage D, TorBox cached add, debrid,
TorBox AIO, Stage C and the TB add-on-link autoplay suite pass (Stage D now starts a new
search before emptying its fixture library). Debug APK
`build/android/Nuvio-2.0.8-android-autoplay-speed-debug.apk` (SHA-256 `c5c9dbcc…bff9`)
installed on BlueStacks only. The "P2P | Not Cached" link counted as cached is not
fixed yet: it needs the new logs from a real run.

## TorBox Instant: bare hashes no longer use up autoplay attempts — 12 November 2026

BlueStacks: "Could not open the source" on every play. AIOStreams (TorBox Instant) sent
53 sources; the top 8 by score were bare torrent hashes, cached on TorBox but not in the
account. With "Allow cached TorBox additions" Off (default) they cannot play, they used
all 8 automatic attempts, and the playable add-on links below were never tried. Such
checks are now free (`primeiraComGratis` in `src/streams.c`, still serial, #130); real
attempts keep the usual limit, and the queue holds up to `VER_MAX` (16) when cached
additions are Off. Log: `[fonte] N torrent(s) not in the TorBox account skipped …`.
Test: `tests/autoplay_instant_hashes.sh`; `tests/fonteauto.sh` guard accepts the new
serial loop. Debug APK `build/android/Nuvio-2.0.8-android-instant-debug.apk`
(SHA-256 `95fd2b9e…f128`) installed on BlueStacks only. Pre-existing failures at HEAD
(link errors/guards): fonteparalela, autoplay_alvo, fontesmortas, fontesmortas_orcamento,
fontecache_vod, fontefalha_worker, fontefalha_playback, fonte_ordem_addon.

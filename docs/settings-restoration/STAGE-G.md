# Stage G — Home row parity: findings and compatibility boundaries

Reference: official NuvioTV commit `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`.
This initial findings section was written before implementing the Stage G changes.

## Verified source behaviour and root causes

- Official `LayoutPreferenceDataStore` uses `ProfileDataStoreFactory` under
  `layout_settings`: `home_catalog_order_keys`, `disabled_home_catalog_keys` and
  custom titles. Row flows read profile 1 when a non-primary profile inherits
  primary addons. Otherwise they read the selected profile. Remote items are
  sorted by `order`; `enabled=false` is separate from sorting.
- `HomeCatalogSettingsSyncService` pulls the existing `home_catalog_shared` payload;
  no row/malformed/temporary failure does not mean an empty valid configuration.
  `HomeCatalogSyncSupport` uses addon ID + API content type + catalogue ID for sync
  keys and `collection_<id>` for collections. Display/runtime keys also include
  the configured addon base, so identical names are not an identity.
- Native already pulls the same read-only platform payload and caches it per
  account/profile (`catordemcache.c`). Its home settings are device-local but
  already have per-profile files, not one undifferentiated global configuration.
  `descoberta.c` and `home.c` apply local row order after remote order; explicit
  local visibility and a seven-row default quota can override the synced choice.
- Native's row editor `filMontarLista` sets one queued separator at the first
  queued item and then appends all later rows indiscriminately, including app or
  collection rows that are not queued. Its display derives the status from the
  separator rather than the row's actual state. This misclassifies later rows.
- `fileiras.c:carregar` accepts repeated `linha` keys without deduplicating them.
  Such records have independent positions and can count twice against the quota,
  placing one apparent catalogue both on Home and in the queue. Registration only
  searches the first key, leaving the duplicate alive across refreshes/reloads.
- Addon row identities currently use the legacy sync key for the first installed
  instance and a configured-base fingerprint for additional instances. Keep
  distinct configured sources/types/catalogue IDs even when their titles match;
  never merge by a translated display title or strip configuration parameters.

## Approved authority and migration policy

The user confirmed: a valid synced configuration governs ordering, enabled states
and catalogue selection; preserve Native customisations for offline fallback and
recovery, never overwrite them merely by reading synced rows. Use a matching
account/profile last-known valid cache when network sync fails. No new account
protocol, remote write or live account request is authorised in this stage.

Compatibility risks to cover before claiming complete parity:

- Home is assembled in both discovery and rendering and includes app/collection
  rows. Changing only one ordering pass leaves inconsistent quotas and settings.
- Native has extra row shapes, sizes, item-order settings and local additions;
  their stored values must survive a remote projection. Local files are recovery
  inputs, not a safe destination for an authoritative remote rewrite.
- Parent-addon inheritance must choose the correct existing profile cache/read
  scope without changing login, account protocols or progress ownership.
- Late profile/account responses must never repopulate another profile's rows.
- Missing/malformed/failed sync must not erase a valid cached row configuration.
- Platform/memory caps and missing/empty/unsupported catalogue resources can
  prevent literal visual parity; report those limits instead of inventing rows.
- The requested Streaming Services, Genres, Top 25, Trending and Popular rows
  must follow actual configured catalogue/collection identities. No title-based
  hard-coded replacement rows or guessed episode/date behaviour will be added.

Implementation and validation results will be recorded below and in STATUS.md.

## Hybrid merge and capacity strategy (before implementation)

User clarification: preserve Friends Watching (`social_activity`), all Native-only
rows and their local state/shape/size, and their positions relative to shared rows.
A synced snapshot is a read-only runtime projection, never a replacement Native file.

- Build the Native order first. Only slots whose exact identities occur in a valid
  synced snapshot are reordered according to that snapshot. Unmatched Native-only
  rows keep their slots; shared disabled states override local shared visibility.
  Unknown/new addon rows remain available (the pinned original appends unsaved
  catalogue keys too). Collections retain `collection_` identities and membership.
- Last-known valid, owner/profile-scoped synced caches remain in use through failed
  refreshes. A profile transition clears in-memory projection before loading the
  correct cached scope. Inherited addons use profile 1 home-row configuration, as
  the official profile flows do; local exclusive preferences remain selected-profile.
- Prevent identical stored row keys from occupying two slots; first occurrence wins
  deterministically in memory. Keep the original file unchanged during reads;
  preserve an exact recovery copy before a later explicit save normalises duplicates.
  Distinct addon configurations, IDs, types and catalogues are not title-deduplicated.
- On Home and Queued are partitioned by each row's state, then rendered in the
  effective hybrid order. A separator position cannot redefine another row's state.

**Capacity finding:** no hard-coded ten-row renderer/storage maximum was found.
Native has a stored catalogue-row quota (default seven, user-configurable) that can
be ten. Android already has 224 catalogue row slots, 8,000 item slots, and a 200-row
catalogue safety ceiling; other TV builds have 40 slots and a smaller item budget.
Discovery already uses three concurrent catalogue requests and Android batches of
24, with existing caching, pagination and visible/nearby artwork loading. These
structural/platform ceilings are genuine, not a ten-row UX requirement.

Android's old small saved quota must stop preventing enabled rows from appearing;
retain that stored value for recovery rather than rewriting it during migration.
Keep the existing 200-row structural ceiling and explicit emergency session memory
cap. Do not advertise infinite rows or remove platform allocation bounds. Existing
legacy per-row shapes, item ordering, visibility and configuration values survive.
Test 10/15/20/30 rows plus Native-exclusive rows; no new arbitrary low ceiling or
parallel request fan-out is introduced. Other-platform budgets remain explicit.

## Implemented hybrid projection

- `fileiras.c` retains Native preferences and their storage keys/defaults. A separate
  in-memory profile projection controls shared visibility and reorders shared slots
  over the Native order. Unknown/new catalogue keys are not title-matched or deleted;
  Native-exclusive rows keep local visibility, shapes, sizes, item ordering and slots.
- `fil_unir` is shared by discovery, Home assembly and the editor. The editor now
  partitions actual Home/Queued states rather than assigning every later row to the
  queue. It preserves the focused registry identity when rebuilding the list.
- Source selection in `descoberta.c` now honours the synced shared state both before
  quota selection and before fetching content. Local fallback exclusions cannot
  suppress an enabled shared row. Ordinary catalogue rows and `collection_` rows
  remain distinct; collection membership cannot override explicit synced catalogue
  selection. Legacy wrapping still applies to unmatched Native-managed rows.
- `fileiras.c:carregar` folds repeated exact keys in memory, retaining the first
  occurrence and leaving the original file untouched during reads. An explicit
  later local edit makes an exact `.pre-stage-g` recovery copy before normalising
  the file. This also handles the old unscoped file seeding profile 1. A backup
  failure prevents the save; existing backup files are not overwritten.
- Automatic account reconciliation, mirroring and old quota migration do not
  rewrite the Native recovery preferences while a valid shared projection is
  active. Explicit local edits to Native-only styles/size/visibility still persist.
  Shared row enable/order controls explain that they are managed by the synced
  profile; they do not claim to upload a local change.
- Android's small saved catalogue quota is retained in storage but no longer limits
  enabled rows. The row-limit display is informational Automatic. The existing
  200-catalogue structural ceiling and emergency session cap remain. Other platform
  quotas are unchanged; their different allocation limits are documented above.

## Profile source scope and sync safety

`perfis_home_config` follows the official account flag `uses_primary_addons`, using
profile 1 only when the account says the selected non-primary profile inherits it.
Home settings are read/cached from that effective scope. Native-only choices remain
in the selected profile's own local file.

A valid projection also takes precedence over Native's device-level "use primary
profile addons" fallback override, otherwise the app could load correct profile 2
row keys but fetch only profile 1's catalogues. The Native override's stored value
is preserved and resumes its original role without a valid synced configuration.
Settings explain this precedence. On a cold first pull, the existing sync retry
mechanism reads the newly authoritative addon scope after the first cycle completes;
no new RPC or sync payload format is introduced.

Cached context tracks both selected and effective home scope. Incoming home blobs
carry their requested scope and are applied only while it still matches. Existing
account/session-generation guards remain in place. Cache envelopes remain the
existing owner/profile-scoped `NVCATORDER 1` format. Missing, malformed, failed or
empty pulls preserve the last valid snapshot, matching the pinned Android service;
logout/profile reset remains an explicit operation. All-disabled items are non-empty
valid configuration, not mistaken for an empty pull.

No Native-only row is exported to the home sync service. This phase adds no remote
write operation. Verification uses synthetic local files and mocked account/provider
responses, never the user's live account.

## Validation coverage and remaining limits

Final suite results are in STATUS.md. The new fixture compiles the actual editor
partitioner, profile scope selectors and manifest-priority function from their source,
and links the real Native row registry, remote parser and owner/profile cache. It
covers 10/15/20/30 shared rows plus Friends Watching, a Native-only catalogue and a
separate Native collection: 33 enabled rows and 34 editor entries at the largest
configuration. It verifies enabled/disabled exclusive states across profiles, primary
inheritance, refreshes, restart/reload, matching caches, account mismatch, invalid and
empty input, local fallback, exact Native-file preservation and recovery backups.
It uses synthetic catalogue identities (including the eight requested example names),
not guessed identities for the user's real addons.

The real sync worker fixture additionally verifies a fresh profile 2 home snapshot
causes an initial primary-addon fallback to be re-read from profile 2, selects the
correct addon source and sends neither addon nor home settings. Existing provenance
tests now explicitly exercise catalogue keys for distinct configured instance URLs,
content types and catalogue IDs; matching display names are retained.

Remaining limits:

- A new Android TV APK is needed. No Stage G APK was built or installed. Plan/editor
  counts and row identities are automated evidence; actual 30+ row TV rendering,
  scrolling, playback, memory use and server-provided catalogue availability still
  require device testing.
- Existing discovery is progressive: three simultaneous catalogue requests, Android
  windows of at most 24, cached responses, on-demand catalogue pagination and lazy
  visible/nearby artwork. Initial pages of all enabled rows eventually load; this
  is not a new strictly viewport-only catalogue-fetch scheduler. Device performance
  measurements are not inferred from local fixture success.
- Native collection presentation is retained. A configured service/genre/Top 25/
  Trending/Popular row must be supplied by its actual installed addon or collection;
  unavailable/empty/unsupported sources do not gain fabricated content. Non-default
  official Follow-addons-order layout mode and remote custom row-title import are
  not added by this shared-order/visibility change.
- The existing registry (768 identities), catalogue/item allocation ceilings and
  emergency memory protection remain. Over-capacity catalogues may still queue;
  this phase removes the small Android quota, not every safety bound.
- Multiple configured instances of one addon retain their existing full-URL-based
  disambiguation. The shared remote legacy key still identifies the first instance;
  additional instances remain Native-side identities. No ambiguous remote mapping
  or change to saved identity formats is guessed.
- The older `colfileiras_home.sh` monolithic integration fixture has unresolved
  service dependencies. Its failure was reproduced from a pristine archive of
  checkpoint `2bb366167fb9359b854303d6f096b85c80a1abf5`, without switching or modifying
  the working branch. Exact comparison results are recorded in STATUS.md. Focused
  row/editor, real collection reconciliation, sync worker and Home position tests
  provide passing coverage; the legacy harness is not claimed to pass.

No Apple TV redesign, Stage E dates, Stage H/I work, commit, push, merge, publication
or live account mutation is part of Stage G.


## Final validation — 4 November 2026

The final executable suites all exited 0: `home_rows_stage_g.sh`, `syncaddons.sh`,
`fileiras.sh`, `fileiras_android.sh`, `catordem.sh`, `catordemcache.sh`,
`colfileiras_sync.sh`, `addons_proveniencia.sh`, `homepos.sh`, settings Stages A–D,
TorBox cached additions/autoplay, stream filtering and Continue Watching parity.
The 413-binding preservation guard and diff whitespace check passed.

The row fixture also forces the existing 12-row emergency cap to exercise a genuine
queued section. A collection after queued catalogues stays in On Home, all editor
entries match their actual Home/Queued states, and focus follows its registry identity
through the repartition. The 10/15/20/30 normal configurations have no small-quota
queue or configuration/editor truncation. Native-exclusive sizes and disabled states
persist, and pre-profile-file migration keeps the exact original and a recovery copy.

**Legacy harness diagnosis:** `tests/colfileiras_home.sh` exits 1 in both the current
tree and a complete pristine `git archive` of checkpoint
`2bb366167fb9359b854303d6f096b85c80a1abf5` (`src/` and `tests/`, original recipe).
Both fail to link on the **identical set of 62 undefined service symbols**. This is a
pre-existing isolated-fixture dependency problem, not a Stage G-introduced unresolved
symbol. It was not hidden with permissive linker flags or blanket mocks. The legacy
harness remains unavailable; the focused actual row/editor, collection reconciler,
real mock-transport sync worker and Home-position suites pass.

Evidence is in ignored `build/stage-g/`: per-suite logs, pristine/current harness
logs, `home-harness-comparison.json` and source/test fingerprints. The profile source
scope fixture uses the actual sync worker: it reads primary fallback once, reads
selected profile 2 once after fresh home authority, selects profile 2's addon result,
and records zero home/addon setting pushes.

**Readiness:** ready to build a new Android TV debug APK for device validation of
these supported paths. This is not physical-TV certification or complete parity
with every non-default original layout mode. No new APK/build/install was performed
in Stage G. The existing APK predates all Stage G changes. All work remains local and
uncommitted on `codex/settings-restoration`; the checkpoint and Continue Watching
branch references are unchanged. Stages E, H and I were not started.

## Real-device follow-up (2026-11-04)

The hybrid projection above was replaced: regular Nuvio's synced order is now primary
(`fileiras.c contaOficial`), with Follow Addons Order, `showInHome:false`, pinToTop
collections, official type identity and Native-exclusive anchoring. Root causes,
fixes and test results are in STATUS.md, "Stage G + I real-device fixes".

## Second addon instances (2026-11-04)

The limitation above ("additional instances remain Native-side identities") caused
profile-hidden rows to stay visible. Synced visibility and order now reach a second
instance through the official key (`fileiras.c chaveOficial`); see STATUS.md,
"Per-profile Home visibility: second addon instances".

# Stage I — compatible profile settings synchronisation

## Source investigation (before implementation, 4 November 2026)

Reference: `~/Documents/Nuvio-Official`, commit
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`.

`core/sync/ProfileSettingsSyncService.kt` exports version 1 typed preference
objects under `features`, using `sync_pull_profile_settings_blob` and
`sync_push_profile_settings_blob`. Both RPCs use `p_profile_id`, `p_platform: tv`;
the push uses `p_settings_json` and `p_origin_client_id`. Native reuses its existing
`dados_cliente_id()` persisted client identity for this original origin parameter.
This is a **profile** blob, not an account-wide
preference document. `data/local/ProfileDataStoreFactory.kt` gives each profile
its own feature store (profile 1 uses the unsuffixed filename).

Exported feature families are theme, layout, experience mode, player, stream
badges, plugins, trailers, TMDB, MDBList, Trakt, debrid, AnimeSkip and track
preference. Values have an explicit `type` and `value` (boolean, string,
int/long/float/double or string_set). Presence alone does not establish a Native
equivalent. The original observer drops its initial emission, debounces and
deduplicates, suppresses remote-import echoes, and captures the profile before
an asynchronous push. Pull and push share a mutex and refresh expired JWTs.
Missing blobs/features are not an instruction to reset a Native preference.

Home catalogue order, disabled keys and titles use the separate
`HomeCatalogSettingsSyncService.kt` protocol. Stage G already consumes that
protocol. Stage I must leave its hybrid projection, Native row stores, collection
membership and addon scope unchanged; **no home configuration uploads** are added.

The official generic exporter excludes device player/decoder/audio/frame-rate,
buffer/cache/performance preferences and some layout/trailer implementation
preferences. Provider secrets have separate `ProviderCredentialSyncService.kt`
RPCs and exclusions from generic blobs. Stage I does not expand credential sync,
upload credentials, or change existing login/provider protocols. No account-wide
settings update is justified by this profile-blob protocol.

## Compatibility risks and implementation policy

Native's previous automatic path recursively matched names without a feature
boundary, protected the whole blob after any local edit, and generated extra
`c_settings_v1` / `c_devices_v1` snapshots. These extensions are not the pinned
official settings contract. The new automatic path must not generate them,
restore Native/device files from them, or replace the Native settings file with
remote data. Existing legacy import/export helpers can remain for compatibility
tests; automatic account sync will use a separate typed allowlist.

Start with shared layout controls whose actual Native consumers have boolean
equivalence. Keep unverified conversions, Native-only controls and all hardware
choices local. Map wire names explicitly; do not rename storage keys or bindings.
Use per-account/profile durable, per-field baselines and pending edits. Apply
incoming fields independently, preserve absent/invalid fields, and merge only
pending shared edits into a freshly fetched original-format blob. Preserve
unknown remote features. Never upload an entire local snapshot on first login.

Without a server revision/CAS in the pinned client RPC contract there is no
provable global last-writer ordering. Use conservative three-way conflict
handling: when both sides changed from a known common baseline, accept the
fresh remote value rather than overwrite it. An edit made before any common
baseline is likewise deferred to an existing remote value. Missing remote values
do not erase pending local edits. Acknowledgement must match the value sent,
preserving edits made during a request. Session/profile guards reject responses
from a previous context. A residual concurrent server write between pull and
push cannot be eliminated client-side and must be reported.

## Implemented result

The automatic account path now uses `src/ajustes_shared.inc`, through the existing
sync worker, RPC authentication/retry infrastructure and account/session guards.
There are **31 bidirectional controls**: 17 layout, two player display controls
and 12 TMDB switches. All have original-format boolean values and a traced Native
runtime consumer. No setting IDs, defaults, dropdown values or storage keys were
renumbered or translated internally. No new UI/category or remote schema was added.

The complete wire/native mapping, pinned original default expressions, getter
locations and actual runtime call sites are in [stage-i-inventory.json](stage-i-inventory.json).
The definitive original export predicate, all 13 feature families, 67 excluded
player keys and other exclusions are recorded in
[stage-i-official-sync.json](stage-i-official-sync.json). These describe the original
contract, not a claim that every exported original preference is implemented here.

| Shared controls | Original feature | Scope/status |
|---|---|---|
| Sidebar collapsed/modern; landscape posters; hero enabled/full-screen; poster labels; catalogue type suffix; focused poster expansion; unwatched-episode blur; Continue Watching enabled/thumbnails/next-up blur/unaired/furthest-episode calculation; external detail metadata; full release date display; fast horizontal navigation | `layout_settings` | Selected profile; bidirectional |
| Pause overlay and on-screen clock | `player_settings` | Selected profile; bidirectional |
| TMDB enabled; artwork, basic info, details, credits, productions, networks, episodes, trailers, similar titles, collections and Continue Watching enrichment | `tmdb_settings` | Selected profile; bidirectional existing switches only |
| Account-preferred primary/secondary audio/subtitle languages, TMDB language hints, source regex/selected addon/plugin lists and stream badge rules | Existing typed feature fields | Existing read-only imports retained with feature/type boundaries; missing/malformed values preserve current values. Explicit Native language choices win. Existing local source-rule protection remains; no new outbound conversion is claimed. |
| Home order, enabled rows, catalogue sources, collections and Native-exclusive row placement | Separate `home_catalog_shared` protocol | Existing Stage G hybrid read path unchanged; no new uploads |

Source analysis also found three apparent candidates without runtime consumers
outside settings: `AJ_NOME_ADDON`, `AJ_GRAD_CLASSICO`, `AJ_DET_TRAILER`. Their bindings
and controls are preserved but excluded from the new sync allowlist. A getter or
visible toggle alone is not evidence of working presentation behaviour. Wiring
these features is outside this settings-sync change.

## Persistence, conflicts and offline behaviour

* Account/profile journals are named `settings-shared-<owner-hash>-p<N>.txt` in the
  existing private data directory. The header checks the exact owner and profile;
  entries use original field names and only integer preference/baseline state.
  They contain no credentials or tokens. Existing atomic `dados_gravar` is used.
* Activation does not mark defaults as edits. Native's successful local save
  observes the allowlisted fields and persists pending values. The inactive-profile
  editor journals only changed fields for its target, without changing the active
  upload context. On profile selection/restart the journal restores that profile's
  pending values or last-known valid synced values.
* Missing remote keys/features/rows do not clear preferences. Malformed documents,
  duplicate JSON keys, unsupported schema versions/types, nested impostor fields,
  truncated or multi-row RPC envelopes do not supply a fresh upload base. Confirmed
  `[]`/`settings_json: null` can supply an empty base, but do not themselves upload
  defaults. Explicit local edits can then create only supported original fields.
* Each outbound merge requires a successful fresh pull in that cycle. Unknown
  remote fields/features, including existing historical extensions, are retained
  without adding new Native/device snapshots. A pending local field wins over an
  unchanged common baseline; an existing remote value wins an unbased conflict.
  A matching remote value acknowledges a pending edit without another upload.
* Only fields actually sent are acknowledged. Changing a preference back during
  HTTP leaves the later value pending. A failed write retains pending work; after
  an ambiguous timeout, a fresh matching remote value avoids duplicate uploads.
  Repeated pulls and imported values do not generate echo uploads.
* Previous-session responses cannot publish into another account/profile. The
  existing periodic/manual account-sync scheduling and backoff are retained;
  this is not a new 1.5-second foreground observer. Offline changes are persisted
  immediately and wait for an eligible account-sync cycle.
* The pinned protocol has no client-visible revision/CAS precondition for these
  whole-blob writes. Client-side stale session responses are rejected, but a second
  device writing between a successful pull and push remains a server race. Global
  last-writer ordering is **not guaranteed**. No revision field was invented.

## Native preservation and unsupported differences

The original 413 bindings/defaults and Stage C/D appended controls remain. Native
home/hero customisation, Friends Watching, Live TV/EPG, dual subtitles, source
filters, provider credentials, logging and all device/platform controls remain
available through their existing implementations. The new allowlist never
replaces Native row, collection, resolver, provider, progress or other preference
files. Stage G source scope, profile inheritance, stable identities, focus model
and >30-row capacity were not rewritten. Its Native offline/recovery preferences
remain intact.

Hardware/decoder/audio output/frame-rate/buffer/cache/memory/remote settings remain
local. Provider keys/tokens remain on their existing credential path; this work
neither uploads them nor expands credential sync. No new account-wide preference
write was added. Original primary-plugin inheritance is a feature-specific rule,
not permission to turn arbitrary profile preferences into account-wide fields.

**Full original settings sync parity is not claimed.** Fixed/dynamic Native themes,
layout mode/units, numeric/enum conversions, subtitle styles and explicit language
choices, autoplay mode/source variants, resolver/provider policy, tracking source,
trailer settings, experience mode, MDBList/AnimeSkip and remaining original fields
are not on this bidirectional allowlist. A safe equivalent conversion has not been
established for each. They remain functional locally; outbound sync is deferred.
The Native-only controls retain their local saved values, without inventing remote
fields. The old `c_settings_v1`/`c_devices_v1` helpers remain available for explicit
compatibility/import tests; **automatic Native extension cloud backup/restore is
no longer used**. Previously uploaded extension data is not deleted remotely.

Additional limitations: original defaults absent from the wire are deliberately
not inferred, a remote reset expressed by removing fields does not reset Native,
read-only account language/rule imports have not gained the new bidirectional
journal, and Stage G home configuration remains read-only to the account. Independent
sync of parameterised addon variants and remote row titles retains Stage G's
already documented limitations. No episode-date source policy was changed.

## Validation (4 November 2026)

All tests used isolated local data and mocked account/provider operations.

| Test | Result |
|---|---|
| `bash tests/settings_stage_i.sh` | PASS: 31 wire mappings/default/consumer evidence; actual production bridge; real sync worker with mocked HTTP; inbound/outbound original RPC payloads; per-field edits; missing/malformed/partial/duplicate/wrong-version data; account/profile isolation; inactive editor; fresh-process restart; offline/retry; in-flight edits; ambiguous timeout deduplication; stale-session rejection; Native/device/credential/row preservation |
| `python3 tests/settings_preservation.py` | PASS: original 413 bindings/defaults/progress sources |
| `bash tests/settings_stage_a.sh` | PASS: categories, stable scenes, English, remote navigation/Back |
| `bash tests/settings_stage_b.sh` | PASS: Legacy classification, binding/wiring chain, settings persistence |
| `bash tests/settings_stage_c.sh` | PASS: resolver controls, providers, persistence, gates and preparation limits |
| `bash tests/settings_stage_d.sh` | PASS: mocked cache states, real UI labels and autoplay |
| `bash tests/torbox_cached_add.sh` | PASS: opt-in defaults/provider gates, fresh checks, readiness, concurrency and durable duplicate/timeout prevention |
| `bash tests/stream_cache_filter.sh` | PASS: verified-uncached filtering, Unknown/Error retained and source ordering |
| `bash tests/cw_parity.sh` | PASS: Continue Watching/progress regressions |
| `bash tests/home_rows_stage_g.sh` | PASS: 10/15/20/30 shared rows plus three Native/collection rows, duplicate prevention, profile ordering, offline projection, editor and focus model |
| `bash tests/syncaddons.sh` | PASS: real sync worker's profile addon/source scope and safe Home retry |
| `fileiras.sh`, `fileiras_android.sh`, `catordem.sh`, `catordemcache.sh`, `colfileiras_sync.sh`, `addons_proveniencia.sh`, `homepos.sh` | PASS: existing Stage G support regressions |

The only change admitted to the frozen settings-persistence function is explicit
shared-field observation and inactive-profile journalling. The reviewed Stage I
preservation overlay retains all original statements and side effects. No original
consumer/default/progress baseline was regenerated. Test logs and run manifests
are under ignored `build/stage-i/`.

The old automatic Native snapshot round-trip expectations in `ajustes_sync.sh` /
`syncjank.sh` predate this protocol restriction: they require cloud backup/restoration
of Native/device files, which this stage deliberately stops doing automatically.
They are not claimed to pass unchanged. Stage I supplies replacement automatic-sync
coverage rather than reinstating unsupported fields to satisfy those expectations.
The pre-existing older Home integration harness link failure remains documented
in Stage G; it was not hidden or attributed to Stage I.

## Android TV readiness

The implemented **Stage G + Stage I supported paths are ready for a debug APK build
and real-device testing**, subject to the explicit remaining sync differences above.
No APK was rebuilt or installed. Real two-device/account interoperability, remote
focus/scroll under updates, offline reconnection and LG/webOS/Tizen device behaviour
remain unverified. Shared native test compilation is not TV validation.

Work remains local on `codex/settings-restoration`. No live account mutation,
provider operation, commit, push, merge, publication or master/Continue Watching
branch modification was performed. Stages E and H were not begun.


## Real-device follow-up (2026-11-04)

Resolve Playable Links (`debrid_settings.debrid_enabled`) is now the 32nd synchronised
field; `follow_addons_order` is imported read-only for Stage G; the account is
refreshed on resume and after playback; every received or refused document is
logged without values. Details in STATUS.md, "Stage G + I real-device fixes".

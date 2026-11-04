# Optional cached TorBox account additions

Pinned comparison: NuvioTV dev
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f` (local HEAD verified).
This extends Stage D; it does not replace its read-only default.

## Setting and policy

Integrations → Connected Services → **Allow cached TorBox additions** defaults to
**Off**. Resolve playable links must be enabled, the effective selected resolver
must be TorBox, and autoplay must be enabled. Selecting Premiumize, disabling
Resolve/autoplay, editing another profile, or losing TorBox eligibility makes the
addition policy ineffective without erasing the saved choice. Manual confirmed
resolution retains its existing separate behaviour.

The visible English help discloses that On can add account items and may start a
download if provider cache state changes. It also explains the fresh provider-check
requirement and that failed/timed-out additions require manual confirmation.
The opt-in itself is the explicit permission; turning Resolve on alone grants none.

One ID is appended after all 417 existing Android controls: `AJ_TB_CACHED_ADD`
(index 417, 418 total). `torboxCachedAddLocal` uses the existing profile-local
persistence and Native 0=On / 1=Off convention. Original 413 bindings and the four
Stage C controls retain their IDs, keys, defaults and scopes. No account JSON/RPC,
login, tracking or progress format changes. This is a new Better Nuvio safety
option, not a Native Legacy-origin control, so it has no Legacy badge.

Browser/Emscripten builds hide and disable this option because their asynchronous
storage cannot provide the required durable write-before-send receipt. Native
filesystem clients fail closed if a receipt cannot be persisted.

## Actual comparison with original Nuvio

Paths are relative to `app/src/main/java/com/nuvio/tv/` in the pinned repository.

| Step | Pinned original | Better Nuvio |
| --- | --- | --- |
| Enable resolution | `domain/model/DebridSettings.kt`, `DebridSettingsDataStore.kt`: Resolve defaults Off | Existing Stage C setting retained, plus separate default-Off addition consent |
| Cache annotation | `LocalDebridAvailabilityService.kt`, `LocalDebridService.kt` use selected credentials/provider cache evidence | Existing Stage D read-only batches retained; addon Cached alone never authorises a write |
| Create | `TorboxDirectDebridResolver.kt` calls `TorboxApi.createTorrent` with cached-only and ZIP-disabled multipart parts | Reuses Native multipart builder with the same parts only in the opted-in TorBox path, after a fresh check |
| ID parsing | `TorboxDto.kt` accepts `data.torrent_id` or `data.id` | Supports both, requires a valid successful envelope and positive ID |
| Item/file lookup | Original fetches the returned ID, selects a file and requests its URL | Reuses Native file selection/link handling; verifies the acknowledged ID, matching hash when supplied, and explicit ready flags; rejects queued/downloading states |
| Autoplay success | Original requires a non-empty URL | Existing Native selection receives a non-empty HTTP(S) URL before success; existing quality/source/provider preferences remain authoritative |
| Duplicate/failure policy | This resolver does not implement the durable receipt policy below | Reuses ready account items first, serialises additions, records attempts before POST, and never blindly resubmits an ambiguous write |

The public [TorBox OpenAPI schema](https://api.torbox.app/openapi.json) was consulted
as documentation only, alongside the pinned source. The relevant contract is the
multipart `torrents/createtorrent` endpoint with `magnet`, `add_only_if_cached=true`
and `allow_zip=false`, followed by item lookup and a playable-link request. All
provider operations in tests are mocked; no authenticated/live account request or
actual download was made. No server guarantee is inferred from an addon label.

## Execution and duplicate prevention

1. The existing source selection and Stage C policy gates run first. Off, provider
   mismatch, Uncached, Unknown and Error cannot enter automatic creation.
2. A per-process TorBox mutex serialises the account lookup/addition sequence.
   Existing ready items use the read-only path; known unfinished items are not
   duplicated. Incomplete, malformed or failed account scans cannot prove absence.
3. Only after confirmed absence does the provider receive another fresh cache
   check. A decline or error aborts. Consent, autoplay, provider, credential context
   and profile-session ownership are checked again before writing.
4. A private receipt is claimed with exclusive creation and flushed, including the
   directory entry, **before** the POST. The filename is SHA-256 of the credential
   and normalised hash; it contains no plaintext API key. The directory is private.
5. The cached-only request is sent once. It is never downgraded to an uncached
   addition, retried on timeout, or followed by an automatic deletion/download
   fallback. Acknowledged IDs are saved atomically even if the setting/context
   changes during the request, so subsequent attempts can reuse them.
6. A confirmed ID gets up to three bounded readiness lookups. Files alone do not
   imply readiness. Missing/contradictory ready flags, wrong IDs/hashes, API failures
   and empty/error URLs cannot become autoplay success.
7. Pending, corrupt or unconfirmed receipts prevent another automatic POST, including
   after settings reload/restart. If an item accepted after a timeout later appears
   in the account, it can be discovered and played without creating another item.
   Explicitly confirmed manual resolution remains available for recovery.

Receipts live in the existing private app data directory under
`torbox-cached-additions/`; they are local runtime state, not synced settings or
progress data. The mode does not store signed playback URLs or credentials in them.
Manual Native multipart defaults and all RD/AD manual capabilities remain unchanged.
Preparation snapshots also include this effective policy, invalidating stale work
when it changes. Cache annotation itself remains read-only in both modes.

## Validation and remaining differences

`tests/torbox_cached_add.sh` exercises the real Native parser, policy, API parsing,
file/link handling, durable receipt implementation and autoplay selection with mock
TorBox responses. Other HTTP transport is disabled. Coverage includes default safe
mode, provider and autoplay gates, fresh cache decline/error, malformed responses,
409, timeout, queued/not-ready items, optional/mismatched hashes, corrupt/unwritable
receipts, repeated/reloaded requests, six concurrent callers, late accepted items,
setting changes during requests, and cached autoplay respecting a quality cap with
no addon TorBox key. Stage A–D, preservation, resolver and CW suites remain required.

Intentional differences/limits:

- This is compatible with the original cached-addition flow, not an identical
  implementation. It adds explicit consent, a fresh provider check, stronger
  readiness/response checks and durable duplicate suppression. Original Nuvio can
  accept an item based on its file response where Native conservatively waits.
- A failed or ambiguous addition is not automatically retried until an existing
  item can be discovered or an acknowledged ID becomes ready. Recovery may require
  explicit manual confirmation. The app cannot undo a request already accepted
  while the setting was being turned off.
- Local receipts protect callers/processes sharing this app data and credential.
  They do not provide distributed exactly-once execution across other devices,
  different API keys, external clients or provider-side behaviour. No unsupported
  server idempotency guarantee is claimed.
- Account discovery remains bounded to eight pages of 100 items. If absence cannot
  be established, no automatic addition occurs. Readiness polling is bounded;
  slow/queued items may need a later attempt using the saved ID or manual action.
- It needs a hash actually returned by AIOStreams/addons. Native cannot recover
  streams that an addon's own Exclude Uncached filter omitted entirely, or infer
  missing hashes from arbitrary opaque URLs. No addon credential/configuration is
  changed, and no addon TorBox API key is required for returned hash sources.
- Native file/episode selection is retained. Original Android's separate foreground
  preparation lifecycle and rate budgets remain earlier parity gaps. Physical
  Android TV/webOS/Tizen playback validation and APK work remain outstanding.

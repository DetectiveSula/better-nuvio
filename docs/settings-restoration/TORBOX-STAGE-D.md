# Stage D — TorBox cache status and autoplay

The default read-only behaviour below is retained. The completed optional extension
in [TORBOX-CACHED-ADDITION.md](TORBOX-CACHED-ADDITION.md) now allows fresh-verified
cached account additions only after separate user opt-in.

Reference: local NuvioTV dev commit
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`, unchanged.
Official reference files under `app/src/main/java/com/nuvio/tv/`:
`core/debrid/LocalDebridAvailabilityService.kt`, `LocalDebridService.kt`,
`DirectDebridResolver.kt`, `TorboxDirectDebridResolver.kt`, and
`data/remote/api/TorboxApi.kt`.

## Confirmed defects and evidence

The exact Android test-build response/log was not supplied. These are source-level
faults reproduced with synthetic addon/provider responses, rather than a claim to
have inspected the user's TorBox account or confirmed one exclusive device cause.

| Fault before Stage D | Effect | Correction |
| --- | --- | --- |
| Native `foraCache` addon negatives were filtered before the cache probe | An addon-marked Uncached stream could never acquire fresher positive TorBox evidence | Independent read-only annotation covers negative streams before autoplay filtering |
| HTTP streams keep their hash in `hashTorrent`; Stage C verification only used `infoHash` | An addon activation URL's hash was not verified, so stale addon labels survived | Probe either hash while preserving the original URL/hash distinction and all other metadata |
| Cache checks were tied to Native candidate verification rather than the Resolve setting and source-list lifecycle | Enabling Resolve alone did not refresh all stream labels | A background, batched status pass runs when eligible streams arrive or configuration/credentials change |
| The legacy positive-cache helper did not clear operational `foraCache` | A source could acquire a Cached indicator while still being rejected by autoplay | Provider-scoped read-only evidence feeds effective eligibility; successful URL resolution also clears the stale operational hint |
| Stage C's `verificarUma` returned success after a cache check with no playback URL | Autoplay could choose a hash-only source whose URL remained empty | Verification must return an actual safe playable URL before publishing a winning source |
| Legacy TorBox cache parsing conflated missing `data` with absent cache; Stage C only supported list responses | Missing/malformed responses and format differences could not be represented accurately throughout the UI | Strict root-level JSON parsing, official object response plus supported list response, and explicit Unknown/Error states |
| Remembered/binge source selection was cached by list size/generation only | Annotation of the same list could leave the remembered selection stale | App selection invalidates those caches on source-data revision too |

Official Android separates `LocalDebridAvailabilityService` from playable-link
resolution. Its API uses read-only POST `torrents/checkcached?format=object` with
`hashes` in JSON. Native now uses that same contract, in batches of at most 32
unique pending hashes, and accepts both hash-keyed maps and valid list responses.

## Current flow

1. `stream_parse.c` preserves addon names, filenames, quality, description, audio,
   hints and source URLs. Root-scoped hash lookup distinguishes raw torrents from
   URLs carrying a torrent hash. A `clientResolve` object with no hash is explicitly
   unverified and cannot activate an opaque URL automatically.
2. With Resolve enabled and an eligible preferred provider, `streams_cache.inc`
   checks pending hashes in a worker. Duplicate hashes share a result. No resolver,
   torrent creation or addon activation URL is called during this pass. Network
   I/O never runs on the UI thread or while holding the stream-list mutex.
3. Results are transient and scoped to provider, credential generation, profile
   session and source-list generation. Stale responses are discarded. A provider
   switch does not reuse another provider's result; Off restores addon-derived
   display and makes no native cache request. Completed results refresh after
   60 seconds while the source list remains active.
4. The actual source caption and row badge use the same four-state classifier:
   **Cached**, **Uncached**, **Unknown**, **Error**. Only an authoritative successful
   response lacking the requested hash means Uncached. Missing/invalid hashes
   mean Unknown. HTTP failures, timeouts, missing data, false success envelopes
   and malformed JSON mean Error. Error/Unknown fail closed for opaque hash
   sources without being labelled Uncached.
5. Positive local evidence overrides an old addon negative for cached filters,
   quality ranking and automatic eligibility, while leaving the original addon
   fields intact. Existing quality, source order, regex, decoder and source-policy
   constraints still apply. Status revisions refresh the source UI and remembered
   autoplay choice; focus stays attached to the same source through reordering.
6. Automatic selection calls the existing Stage C policy. It checks cache, then
   uses Native file selection and link handling to obtain a URL for an **existing
   ready TorBox account item**. A successful URL is stored before the candidate is
   returned to the existing app/player handoff. An availability check alone is
   never playback success.

No setting ID, default, persistence key, account RPC, login or progress format was
changed. The existing inverse `escolherFonteManual` / `AJ_FONTE_MANUAL` semantics
remain: choosing sources manually prevents app autoplay dispatch; background cache
annotation still works for the manual source screen.

## No automatic account creation

Stage D deliberately tightens Stage C. `add_only_if_cached=true` still creates a
TorBox account item, so it does **not** satisfy the new restriction.
`resolver_policy.c` now uses `debrid_resolver_existente`, not the old mutating
cache-only resolver. TorBox automatic operations are limited to cache checks,
listing existing items and requesting an existing ready file's playable URL.
File and torrent IDs are read from the correct root object, so nested file IDs
cannot masquerade as a torrent ID. Readiness and hash matching are verified.
The account scan is bounded to eight pages of 100 items.

A globally cached torrent absent from the account is still correctly labelled
Cached, but cannot be turned into an account-backed playable URL automatically
under this restriction. It stays available for explicitly confirmed manual
resolution. This is a necessary difference from official Android, whose resolver
creates a cached account item. No fallback silently creates one or starts a download.

Real-Debrid and AllDebrid retain their existing explicit manual paths. Their
magnet upload/create operations remain outside automatic eligibility. They were
not added to the Stage C preferred resolver list. Premiumize retains its cached
`cache/check` → `transfer/directdl` URL-return path; no `transfer/create` is used.
Native provider order and the single-use, expiring, source/list/profile-bound manual
confirmation remain in place. Hash-associated addon URLs also require that manual
confirmation before direct manual activation.

## Tests and limitations

`bash tests/settings_stage_d.sh` uses the real addon parser, stream list,
classification, Stage C policy, debrid API parsing, file selection and automatic
selection worker. Provider HTTP is mocked; the rest of the transport cannot load
libcurl. An unexpected provider request or any create/upload request fails the test.
Fixtures cover:

- stale addon-negative → verified Cached → automatic selection → non-empty URL;
- authoritative uncached maps/lists, mixed states, duplicate hashes, missing and
  invalid hashes, strict JSON/root fields, whitespace, HTTP failure, timeout,
  missing data and malformed responses;
- exact preservation of all original `Stream` metadata fields during annotation;
- configured TB/PM/RD/AD together, preferred PM, fallback from missing PM to TB,
  no safe provider, Resolve Off, key rotation and stale list/settings responses;
- existing quality caps, strict addon order and unchanged manual/autoplay boolean;
- globally cached but absent account item: Cached label retained, no playable URL,
  no account mutation; failed/malformed playable-link responses also keep cache
  evidence separate, clear the URL and never start a download;
- actual caption strings used by the renderer, not just backend return codes.

Optional GL mode (`bash tests/settings_stage_d.sh build/settings-stage-d/cache-status`)
opens a hidden native window, spies on captions passed to the real text renderer
and saves 1920×1080 PNGs for both Native and addon text modes. All four labels were
rendered and the screenshots were visually inspected. Generated images remain
under ignored `build/settings-stage-d/`.

Device playback/decoder validation remains outstanding. The test proves automatic
selection and the populated URL handoff with mocked service responses, rather
than playing a live TorBox file on Android TV. The exact user account/hash/API
response would still be needed to attribute every historical Uncached result to
a particular provider payload. No such live operation was performed.

The separate Android foreground preparation lifecycle and rolling budgets remain
Stage C parity gaps. Hashless opaque addon URLs that omit `clientResolve` as well
as hash/cache metadata cannot be reliably identified from arbitrary URLs alone;
this change makes no claim to control undocumented upstream addon behaviour.
Legacy file-browser operations are separate from automatic cache/stream resolution.
Stage E dates and Stage F builds/APK/device work were not started.

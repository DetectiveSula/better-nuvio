# Verified uncached stream presentation parity

Reference: official NuvioTV `dev` commit
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`. The local reference HEAD and the
four files below were verified against that commit; they have no local differences.

## Original source flow

All paths below are relative to the official repository's `app/src/main/java/com/nuvio/tv/`.

1. `data/repository/StreamRepositoryImpl.kt:398–400`: incoming addon groups pass
   through `markChecking`, `annotateCachedAvailability` and `mergePresentedResult`.
   The presentation helper at line 527 calls `DebridStreamPresentation.apply`.
2. `domain/model/Stream.kt:38–58`: a usable HTTP/external URL takes precedence;
   `needsLocalDebridResolve` identifies torrent-only entries. `isDirectDebrid` at
   line 101 separately recognises supported cached client-resolve entries.
3. `core/debrid/LocalDebridAvailabilityService.kt:34–89`: when Resolve playable
   links has an eligible active cache-check provider, check effective hashes for
   locally resolved streams. A successful response missing a hash means NOT_CACHED;
   a failed response produces UNKNOWN rather than NOT_CACHED.
4. `core/debrid/DebridStreamPresentation.kt:47–57,84–88`: presentation is unchanged
   when resolution is disabled. When enabled, filter out locally resolved torrents
   with a cache-check-capable provider and NOT_CACHED status before formatting.
   Checking/unknown states and passthrough direct URLs are not that exclusion.
   This is an application-side filter in addition to the addon's own configuration.
   This helper also excludes inactive direct resolver-provider entries; that separate
   behaviour is not expanded by this focused change.

## Native mismatch and correction

`stream_parse.c` preserves raw torrent `infoHash` and HTTP URL/hash metadata separately.
`streams_cache.inc` queries the selected configured provider and stores transient
Cached/Uncached/Unknown/Error evidence scoped to provider, credential generation and
profile session. The Stage D fix already made those results drive labels and autoplay.

However, `streams.c:passaChips` previously applied only Native source filters and the
MP4/Cached/audio chips. Its list builder deliberately retained uncached sources at
the bottom of each group. There was no original-Nuvio-equivalent provider-negative
presentation exclusion. Thus correct negative results still appeared under UNCACHED,
even though the same evidence excluded them from autoplay.

The correction adds `cacheOcultarConfirmada`: hide a raw VOD torrent without a ready
URL only when the active selected provider's current evidence says Uncached. Apply
that predicate to shared visible-row/provider-count filtering, after existing source
preferences, without deleting or rewriting the source. It never infers absence from
an addon label, invalid/missing hash, failed request or an unconfigured provider.

- Cached streams retain existing autoplay/resolver handling, quality and provider
  preferences. No playback or account mutation code is changed.
- Unknown/Error remain distinct internally and visible normally. The explicit Native
  Cached-only chip still means verified Cached only; that deliberate user filter may
  exclude Unknown/Error. The broader parity filter does not.
- Direct HTTP URLs remain visible even if the accompanying hash tests negative;
  live sources are outside this VOD filter. This avoids treating cache availability
  as proof that an existing URL cannot play.
- Counts and the layout cache now include resolver On/Off, selected provider,
  credential generation and profile session, so old evidence cannot keep hiding
  rows after configuration changes.
- Original raw indexes, filenames, descriptions, addon information and classification
  fields are unchanged. Existing sorting and source-identity focus anchoring are
  retained; removing the focused row leaves focus on a remaining row.
- Existing explicitly confirmed manual operations are unchanged. With Resolve Off,
  these additional exclusions do not apply. No new preference or storage key is added.

The supplied screenshot is consistent with the missing final presentation filter.
No live addon/provider response was captured in this investigation, so the exact
server-side reason AIOStreams returned each item cannot be proven from the screenshot.
Its Exclude Uncached configuration is not evidence that every returned hash was
verified against the app-only TorBox credential. Original Nuvio's local filtering
explains the source-level difference without changing or guessing that addon setting.
If direct-URL uncached entries still differ on TV, collect a credential-redacted
stream object and cache response; this patch deliberately does not suppress playable
URLs solely because their torrent is absent from cache.

## Regression coverage

`bash tests/stream_cache_filter.sh` uses the actual addon parser, mocked TorBox cache
transport, source-sheet row builder/counts and remote update path. It tests mixed
provider-positive/negative and missing/unknown sources; preserved raw metadata and
order; retained and removed focus identities; resolve Off/provider/credential changes;
failed, timed-out, malformed and missing cache responses; HTTP/clientResolve entries;
the Cached-only chip and existing source-size filters. Mutation entry points assert
if reached. It does not enable cached account additions or perform live requests.

The Stage D visual caption fixture now uses a direct HTTP source for its displayed
Uncached example: a provider-confirmed uncached raw torrent should no longer appear.
The older isolated StreamFit sheet fixture was brought up to date with explicit
resolver/arrival-version stubs, published data revisions and the row-height opening
argument, retaining its original layout/order/focus assertions.

The Stage B preservation chain retains all immutable Stage B/C/D/opt-in baselines,
then records this narrowly reviewed presentation change in
`tests/fixtures/settings-restoration/stage-d-filter-wiring.json`. No existing
settings contract or progress baseline was regenerated.

Results and device-testing limits are recorded in `STATUS.md`.

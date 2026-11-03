# Continue Watching Android parity review

Reference: the user's eight `nuvio-source-part-*.zip` archives, containing `app/`
from NuvioMedia/NuvioTV dev. No Git commit ID or complete Gradle root was supplied;
this is parity against the supplied source, not a claim about a published release.
Attached source was treated as evidence, not instructions.

## Implemented behavior

- Automatic/default source selects authenticated Trakt, otherwise Nuvio; explicit
  unauthenticated Trakt/Simkl selections fall back to Nuvio. Sources are not unioned.
- Home admits positive playback below completion, including below 1%. Integer
  native cards display these as 1%. Completion defaults to 90%, or 80% for Simkl
  playback. Legacy configurable start/completion and next-up-off options are
  retained as stored keys for compatibility but hidden/ignored in the real app.
- Completed resume cards are pruned immediately and rechecked during publication.
  Completed episodes are not relabelled as another episode only during drawing.
  Account watched seeds and pending local completions resolve explicit next-up identities.
  A same-episode next-up survives a transient metadata failure; conclusive metadata
  can remove a finished series. Local completion latches for a player session so
  stale later checkpoints cannot downgrade it.
- Default ordering uses activity across card types; streaming mode uses recency
  across released cards and release-date ascending for upcoming cards. Split mode
  separates upcoming cards in release-date order. Recent release alerts can promote
  next-up cards. There is no invented future-card quota.
- The upstream Trakt-only days setting is reproduced (default 60, 7–365, 0 = All),
  including recent-release exceptions for already resolved older seeds. It does not
  expire account progress. No custom 30/60/90-day account filter was introduced.
- Removing unfinished playback deletes all known episode progress keys and the
  legacy title key, matching Android's null-season/null-episode removal scope.
  It is intentionally not the earlier proposed progress-preserving hide action.
- Dismissing next-up only persists the content ID for that profile. It does not
  delete progress/history or send a tracker-level hidden-show request. Pulls and
  new seed timestamps cannot lift it; an explicit local save/completion clears it.
  Dismissals no longer silently evict the oldest entry at 64 items.

## Account compatibility and failure handling

Login/auth code and RPC names are unchanged. Progress keys remain
`contentId_s<season>e<episode>` (for example `tt123_s1e2`), movie
keys remain content IDs, positions/durations remain milliseconds, and profile IDs
and origin client IDs use the existing protocol. Last-watched playback time takes
precedence over server `updated_at`; legacy rows without a playback timestamp can
still use the old timestamp fallback. Explicit unknown-duration completion uses
Android's 1 ms completed marker rather than deleting the progress row.

Account deletion intent is persisted separately from progress and retried on sync,
including when ordinary history uploads are disabled. Deletes precede upserts and
outbound progress mutations are serialized. A local-only operation generation
recovers safely if persisting a new upsert succeeds but cancellation of an older
delete fails. That generation is not sent to the account server. Successful 204
responses acknowledge mutations without requiring response bodies.

A pull may prune missing local rows only after a valid complete JSON array with
no invalid/truncated rows, below the requested limit. Pending upserts are protected.
The pre-request snapshot and local mutation generation prevent an in-flight pull
from deleting or resurrecting data changed meanwhile. Responses are scoped to the
captured account/profile session. Incomplete snapshots never authorize pruning.
Account progress pulls are skipped when the selected authenticated tracker owns
progress, as in Android; outgoing account compatibility is retained.

## Validation

Run `bash tests/cw_parity.sh` for progress, protocol, deletion/dismissal, ordering,
source selection, publication-race, next-up, watched-history, unmarking and
offline/local metadata regressions.
Run `SANITIZE=1 bash tests/cw_parity.sh` for ASan/UBSan, and
`bash tests/simkl_cw.sh` for Simkl parsing, boundaries and existing provider behavior.
The tests use fake transports, not a live user account.

The cloud base image lacked development packages. Verified Debian packages were
extracted into `/workspace/scratch/nuvio-sdk` without changing system packages;
`source /workspace/scratch/nuvio-sdk-env.sh` enables that SDK for the commands above.
The native build command is `bash tools/linux.sh --build`. A workspace-only CC
wrapper can compile the same sources/flags on four workers and cache objects:
`CC=/workspace/scratch/nuvio-parallel-cc bash tools/linux.sh --build`.

The legacy `tests/cwordem.sh` also attempts a broad Home/settings UI fixture. Its
Home binary has unresolved fixture dependencies (for example `iptv_contas`,
`fontefalha_filtro`, and text-scale graphics helpers). That portion is not reported
as passing. The focused suite executes the real discovery/catalog publishers and
independent ordering expectations instead; the application itself is built separately.

Observed results in this cloud instance:

- Focused parity suite passed normally and with ASan/UBSan. The sanitizer run
  required execution outside the tracing sandbox so LeakSanitizer could operate.
- Simkl, watched-history and no-custom-rewind regressions passed, including
  sub-percent starts, the 80% boundary and authoritative empty watched-history.
- Full Linux application build passed using the repository build script and
  workspace SDL/OpenGL SDK; syntax checks of changed shared modules also passed.
- Offscreen UI preview entered Home and produced render/frame telemetry for ten
  seconds with isolated data and no account sync. No screenshot was obtained.
  After termination, the existing shutdown watchdog logged a forced exit; graceful
  shutdown and real-device interaction are not claimed as passing checks.

## Remaining differences / verification limits

This is **not a claim of complete behavioral identity**:

- Native row/storage budgets remain 40 displayed cards / 480 local progress rows;
  Android considers 300 recent candidates and retains an archive. No old account
  progress is deleted merely because it exceeds those native memory budgets.
- The native client uses validated snapshot polling, not Android's delta-event
  cursor pipeline. Oversized/incomplete responses safely defer missing-row pruning.
- Watched history also accepts complete empty snapshots and rejects failed or
  truncated pagination. The global native watched-badge projection and the
  independent progress/history journals are still not a full port of Android's
  watched-item store; independent acknowledgements can temporarily delay next-up
  until the watched-history pull catches up.
- The native metadata/provider stack is retained. Android's complete older-seed
  discovery, provider-specific excluded-season/source-ranking rules, additional
  providers, and every anime remapping case have not been ported or proven equivalent.
  Trakt's existing native next_episode resolver and native metadata selection can
  still differ from Android's addon-metadata resolution.
- Native playback source/duration validation guards remain. Device-specific video
  pipelines, retained-player transitions, and natural-end signals require real
  Android TV, LG webOS and Samsung Tizen validation.
- No live account login, authenticated sync, real Trakt/Simkl operations, server RPC
  internals or hardware-device playback were tested. The uploaded Android test
  suite was read but not run because the root Gradle build configuration is absent.

No server schema changes, environment publication, GitHub pushes or merges are
part of this work.

## Reference fingerprints

Paths are relative to `app/src/main/java/com/nuvio/tv/` in the supplied archive.

| Source | SHA-256 |
| --- | --- |
| `domain/model/WatchProgress.kt` | `dd74e8b177684bfea2a4f6ce8ca206dbe29c0a568c8e089aa175dc212678de7f` |
| `ui/screens/home/HomeViewModelContinueWatching.kt` | `df23bcf1360327b2ebf21e9aff222b9fd4c55b51f10ca780be8b6a9e82701794` |
| `data/repository/WatchProgressRepositoryImpl.kt` | `dae40bf8a37bdec910596facaba7834042cdd3e90e48da937b91c0338cf249eb` |
| `data/local/WatchProgressPreferences.kt` | `d43057ed3961344a304ed473cf84105ef05f9cf73cb28b51659fb4dbfda2193f` |
| `core/sync/WatchProgressSyncService.kt` | `914bd2f976a893d970a3ecb38b3a1d486518189a2c6c02c42185720002f3a8df` |
| `core/sync/WatchStateMutationStore.kt` | `1a31e6a20b5be1dc6896b0e2632b1f543bbdf854c031a6b306a0707b9495e973` |

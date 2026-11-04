# Resolve Playable Links — Stage C

Stage D supersedes the original cached-item creation path and cache-status handling
below. See [TORBOX-STAGE-D.md](TORBOX-STAGE-D.md): automatic TorBox resolution now
uses existing ready account items only; the earlier `add_only_if_cached` behaviour
is retained in the Legacy/manual backend and is also available through the new
default-Off [cached-addition opt-in](TORBOX-CACHED-ADDITION.md).

Official source: NuvioTV dev, commit
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`.
Paths below are relative to `app/src/main/java/com/nuvio/tv/` in that repository.

| Behaviour | Official evidence | Native implementation |
| --- | --- | --- |
| Default Off, preferred resolver | `domain/model/DebridSettings.kt`; `data/local/DebridSettingsDataStore.kt` | Append-only Native preferences; no account protocol change |
| Connected Services controls | `ui/screens/settings/DebridSettingsScreen.kt` | Integrations → Connected Services, four unbadged controls |
| Prepare On → 2, Off → 0 | `ui/screens/settings/DebridSettingsViewModel.kt`, `setInstantPlaybackPreparationEnabled` | Derived toggle writes the count atomically using existing Native persistence |
| Count 0–5 | `data/local/DebridSettingsDataStore.kt`, normalisation; `core/debrid/DirectDebridStreamPreparer.kt` | Existing Native preparation path with an additional count ceiling |
| Visible resolver-capable providers | `core/debrid/DebridProvider.kt` | Configured, locally available TorBox/Premiumize only; deterministic preference fallback |
| Prepared cached streams and bounded candidates | `core/debrid/DirectDebridStreamPreparer.kt`; `DirectDebridResolver.kt` | Existing `streams.c` candidates/filters/preparation list and `debrid.c` resolver; no parallel resolver |

## Storage and preservation

Four appended IDs: `AJ_RESOLVE_LINKS`, `AJ_RESOLVE_PROVIDER`,
`AJ_RESOLVE_PREPARE`, `AJ_RESOLVE_LIMIT`.
Their Android numeric positions are 413–416, after the existing 0–412 bindings.
The derived toggle has no serialized value; its `-` key follows Native's existing
non-persisted control convention. Three keys are new:

- `resolvePlayableLinksLocal`: 1 = Off (the existing Native boolean convention).
- `preferredResolverProviderLocal`: 0 = Automatic, 1 = TorBox, 2 = Premiumize.
- `playableLinkPreparationLimitLocal`: 0–5, default 0.

They persist in the existing profile-local/device settings system. They do not
invent server JSON fields, change provider-key storage or rewrite existing values.
Unavailable preferred providers fall back at runtime without rewriting the saved
selection. Unavailable choices cannot be selected by remote, pointer or setter.
Prepare On resets the count to 2, matching the official action; turning automatic
resolution Off retains the saved count but makes its effective value zero.

## Safety and capability boundary

`resolver_policy.c` gates every automatic resolver call and reuses
`debrid_resolver_servico_cache`. A new read-only `debrid_cache_consultar` probes
TorBox `torrents/checkcached` and Premiumize `cache/check`, returning confirmed
cached, confirmed absent or unknown. Invalid hashes, malformed/missing responses,
unsupported services and HTTP failures return unknown. Only confirmed cached
hashes reach resolution. Automatic TorBox resolution retains its existing
`add_only_if_cached` request, verified by the mocked Native resolver tests.
Actual cached URL resolution can create a cached provider item; an availability
check cannot. Background verification no longer performs that resolution step or
GETs hash-associated opaque addon activation URLs.

Native RD's old “cache-only” path uploads a magnet/selects files before it knows
whether the torrent is downloaded. Native AD similarly uploads a magnet, then
checks its state. Those APIs were not silently presented as safe automatic
providers. Their existing implementations remain available through explicit manual
consent and existing Native provider order. Manual consent also covers sources
marked uncached whose addon HTTP URL can trigger a download. Confirmation warns
about provider mutations/quota and is specific to the source, list generation and
profile, single-use and time-limited. Back and the default action cancel.

No live provider calls are part of the new tests. Mocked cache checks prove that
no create/download/transfer endpoint is called, including absent/unknown/error
cases. Existing all-service resolver tests still exercise the original manual APIs
with mocked transport and verify that credentials never enter logs.

## Bounded parity gaps

- This is the supported Native **local torrent-hash** resolver integration.
  Original Android's client-resolve/direct-debrid URL families are not all
  implemented by Native. Opaque addon HTTP URLs cannot be reliably classified
  solely from a URL; their upstream behaviour is not a new resolver guarantee.
  Known uncached automatic sources remain excluded. Manual exceptions are explicit.
- Native's existing preparation lifecycle is for next-episode preparation. The
  new limit controls that working path; it does not add Android's separate
  foreground stream-list preparation lifecycle or its rolling minute/hour budget.
  Native retry ceilings, addon/source filters and candidate order remain authoritative,
  so count is a maximum rather than a promise to prepare that many links. No
  provider safety limit or Legacy preference is increased to satisfy the count.
- Automatic RD/AD local-hash support needs a demonstrably read-only cache contract
  before it can be offered here. No cosmetic unavailable provider option was added.
- Original 83 uncertain Legacy provenance records remain unbadged. This feature
  does not resolve their classification or merge distinct preselection/preloading
  settings into the new preparation control.
- Stage D remains responsible for the full TorBox cache-state/source-label and
  playback investigation. Stage E dates and Stage F platform/APK/device work are
  untouched. Desktop fixture compilation is not TV hardware validation.

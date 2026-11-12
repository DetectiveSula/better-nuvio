# Settings expansion — 8 November 2026

Local implementation on `codex/settings-restoration`. Existing uncommitted work
was retained. No installation, app launch on a device, live account/provider
operation, commit, push or merge was performed. No APK was packaged.

This report distinguishes implemented behaviour from test coverage. **It does
not certify every original setting as working merely because it has a control,
a getter or a persistence test.** The complete binding-by-binding result is in
[SETTINGS-VERIFICATION.md](SETTINGS-VERIFICATION.md) and
[settings-expansion-ledger.json](settings-expansion-ledger.json).

## Implemented controls and real consumers

| Area | Result | Default / compatibility |
|---|---|---|
| Appearance: App Icon & Banner | Reuses the existing linked icon/banner selector, Android activity aliases and `iconeapp.c`. Entry is visible; existing supporter entitlement still disables unauthorised changes. All 10 alias/resource pairs are checked. | Original icon retained. The icon and banner change together; this is not an independent custom-image uploader. Launcher refresh requires a device check. LG/Tizen package branding remains fixed at installation. |
| Layout: Hide modern sidebar pill | Hides the closed modern icon rail and Dynamic pill. Expanded navigation and the existing pointer-edge target remain available. | Off. The old collapse/fixed sidebar setting is retained: modern sidebar mode ignores that old setting, so it was not an equivalent control. Disabled with top navigation or without the applicable sidebar. Content reservation is preserved. |
| Hero trailers | Existing delay retained and exposed in Layout → Trailers / More options. | Existing key, default and range retained; tenths converted to milliseconds. |
| Detail trailers | Replaces the fixed 2500 ms start delay with a persisted 0–30 s control, in 0.1 s steps. | 2.5 s. Existing automatic-trailer switch remains the prerequisite. Adoption of an already-playing Home trailer is unchanged. |
| Post-play trailers | New opt-in muted IMDb preview and independent delay in focused recommendation / next-episode cards. Separate Android software-only ExoPlayer/TextureView; no `NvPlayer`, progress callbacks or hardware decoder reuse. | Off, 2.5 s. No preview on low-RAM devices or without a software AVC decoder. Poster remains on failure. Android-only; unsupported controls are absent on other platforms. Actual Android composition/decode/CPU behaviour remains unvalidated. |
| Poster card width | Named Compact 104, Dense 112, Standard 120, Balanced 126, Comfort 134 and Large 140 dp presets displayed by the existing numeric editor. | Existing stored dp values and custom 72–200 dp range retained. Actual Home geometry is tested; these controls apply where the existing custom poster-size path is used. |
| Poster corner radius | Sharp 0, Subtle 4, Classic 8, Rounded 12 and Pill 16 dp names. | Existing 0–40 dp custom range retained. Names/values follow pinned official `LayoutCardSettings.kt`; “Pill” means that original 16 dp preset. |
| Card depth | Restored edge glow, sheen, coverage and independent Posters / Continue Watching / Episode cards / Cast / Trailers switches. Focus is carried into the real card/avatar shaders. | Existing master, keys, defaults and values retained. Edge presets Subtle 28%, Balanced 42%, Bold 56%; coverage Top only 0%, Half 50%, Full outline 100%. Sheen keeps the existing 0–100% strength range. Effects follow focus, rather than lighting every unfocused card. |
| Integrations → Filters & Sorting | One shared 29-row sheet: max results, six sort modes, per-resolution/quality limits, min/max size, Preferred/Required/Excluded for all seven dimensions, required/excluded release groups. | New limits/masks default to zero / no restriction. Existing exclusions, max size and excluded groups are retained. Uses the existing atomic source-configuration store, not a new resolver. |
| Playback: Post-play recommendations | Movie recommendations enable plus 0–100% trigger slider. | On; 0 keeps the existing credits/estimated-end policy. 1–100 overrides the movie trigger percentage. Episode autoplay remains independently controlled. |
| Playback: Are you still watching? | Prompt after 1–20 consecutive unattended episodes; countdown stops until OK. Post-play interaction, seeking, manual Next/source reload and profile changes reset the automatic sequence. Automatic player close/open pairs carry the counter only to the expected next episode. | 0 / Off. App restart starts a new unattended sequence. Existing autoplay Off remains authoritative. |
| Playback: Automatically load full subtitles | Uses saved title/language choices, falling back to app language when unset; selects full dialogue even with same-language audio. Preloading uses the same empty-preference fallback. | On for the new policy. Explicit None and saved title choices win. Off restores the existing forced-only policy. The old forced-only control remains stored and is disabled with explanatory help while full-auto is On. |

Original reference: commit `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`, especially
`app/src/main/java/com/nuvio/tv/ui/screens/settings/LayoutCardSettings.kt` and
`app/src/main/java/com/nuvio/tv/core/debrid/DirectDebridStreamFilter.kt`.

## Settings that previously had no effect or prevented the requested behaviour

1. **Edge strength, edge coverage and five surface switches had no renderer
   consumers.** The 2.0.5 renderer read only global sheen. Reconnected all seven
   controls and tagged the actual Home, detail, player-episode, cast, trailer
   and post-play surfaces. The player episode list also supplied zero focus;
   it now supplies the real row focus. Local framebuffer tests verify effects.
2. **Detail delay was a hard-coded constant.** It now uses the new persisted
   control. During testing, the list could display 25 s for stored 25 tenths;
   the list and editor now share correct 2.5 s formatting.
3. **Full automatic subtitles could select nothing with an unset language, or
   only forced subtitles with same-language audio.** Added the explicit full-auto
   policy and language fallback, preserving manual None and title history.
4. **Filters supported exclusions only.** Added real requirements/preferences,
   quotas, minimum size and sorting. All options feed filtering/selection, not
   just the settings sheet. Counts use the same batch policy as rendered rows.
   Verified-uncached entries cannot consume an automatic result quota ahead of
   an eligible cached source. First-in-list mode moves all candidate vectors and
   the preferred index together when applying custom order.
5. **Post-play had no safe trailer path.** Added an isolated Android preview
   rather than using the feature-film decoder. Delay/first-frame/failure/focus
   cancellation and decoder policy pass local tests; device validation is still
   required before declaring this preview working on a TV.
6. **App icon/banner was already functional behind an entitlement and hidden
   from non-supporters.** Its existing feature was not rebuilt or unlocked.
   Showing the disabled entry makes the actual requirement visible.

## Filter semantics and safety

- Required choices are OR within a dimension and AND across dimensions.
  Exclusions retain the existing Native behaviour. For languages, a source is
  excluded only when all its declared languages are excluded; other dimensions
  exclude on any matching tag. Unknown remains an explicit choice.
- Preferred choices are unordered sets, with one soft score per matching
  dimension. They are not the official app's ordered preference lists.
- Sorting uses stable O(n log n) merging; a 20,000-source case verifies stable
  ties and a requested 10,000-result limit without an arbitrary input truncation.
- Sorting choices: Original order, Resolution, Quality, Size smallest first,
  Size largest first, Preferred tags. Ties preserve addon order. Unknown sizes
  sort last. First-in-list honours custom ordering within existing provider,
  scope, quality and availability safeguards; Best source retains its established
  quality/provider scoring with soft preferences.
- Zero result/per-group limits mean unlimited; zero minimum/maximum size disables
  that bound. Sizes use decimal GB. Unknown sizes remain eligible unless another
  rule excludes them. Unknown resolution/quality has its own quota bucket, matching the pinned
  official limiter; it does not consume a known group's quota. Cache Unknown/Error
  is a separate availability state and is not relabelled by metadata quotas.
- Release groups are case-insensitive exact CSV entries. Requirements reject
  unknown groups; exclusions retain unknown groups. Live TV bypasses these VOD
  restrictions and does not consume quotas.
- Cache Unknown/Error are not converted to Uncached. They may remain visible
  while being ineligible for automatic playback. Sorting does not make them
  playable. No new resolver or provider mutation path was introduced.

## Persistence and preservation

The Android catalogue is now **434 bindings**: original 413, Stage C four,
Stage D one, pre-existing Family two, then **14 append-only local bindings**.
The desktop harness has 433 because Android's log-export action is absent.

Eight new normal preferences are profile-local and excluded from account
settings documents. Six additional IDs are virtual filter subeditors; masks and
filter fields live in the existing source configuration, respecting its existing
shared-versus-per-profile selection. No new remote schema or sync enrollment was
introduced. The unwired compatibility exporter still includes non-sensitive local
preferences in its existing same-device `c_devices_v1` backup; that is separate
from the portable section and is not called by current account sync. Original defaults, IDs, keys and progress sources pass the unchanged
413-binding preservation guard. Existing custom card values are retained.

The earlier Family implementation already added two approved portable controls,
so J4's actual allowlist was 93 while some tests/docs still expected 91. Updated
those extension records and tests, including 90 → 93 checkpoint restoration;
this does not add new transport fields, enable writes or change J1 history.

New controls are not automatically marked Legacy. Existing badge evidence and
125-control allowlist are unchanged. Restored original depth controls retain no
Legacy badge. No internal controls were exposed automatically.

## Verification results

| Verification | Result |
|---|---|
| Relevant regression suites | 42 distinct suites PASS after targeted fixes and final reruns: settings A–D, G, I, J4–J7, TorBox cached additions, filtering, Continue Watching, Home, menu, post-play and subtitles. |
| Original preservation guard | PASS: all 413 original IDs/keys/defaults and progress-source fingerprints unchanged. |
| Native settings UI/persistence harness | PASS: 433 host entries enumerated; 341 value contracts; all legal values of eight new normal preferences saved/reloaded; profile isolation; filter subeditor callbacks/Back; new entries excluded from all three current sync maps. |
| Filter subchoices | PASS: all 228 dimension/choice/mode combinations, limits, six sorts, groups and 20,000-source stable-order case. |
| Actual local OpenGL renderer | PASS: shader compilation and framebuffer changes for edge presets, coverage, sheen, master and five surface masks; production focus wiring checked. |
| App branding resources | PASS: all 10 icon/banner/TV launcher pairs; original default and entitlement preserved. |
| Android JVM unit tests | PASS: 64 tests, zero failures/errors/skips; Gradle offline rerun, all 25 tasks executed. |
| Android ARM64 / ARMv7 native components | PASS: both `main` shared libraries rebuilt and linked from the final sources. This is not a packaged APK or device validation. |
| APK / real-device checks | Not performed. No installation, launcher refresh, live subtitle/provider actions or Android preview playback was attempted. |

Machine-readable results: `settings-expansion-tests.json`. Per-control evidence:
`SETTINGS-VERIFICATION.md` and `settings-expansion-ledger.json`. Logs and the
source-only pre-change snapshot are under ignored `build/settings-expansion/`.
The per-binding ledger separates boundary contracts from functional consumer tests
and hardware checks. Final standalone native, source guards, preservation and
both ABI checks were repeated after the last dependency/help adjustment.

Relevant older harness problems were corrected without changing application
behaviour: missing linked source/stub dependencies, Bash 3 empty-array handling,
old hidden-depth/category assumptions, stale 91-versus-93 extension counts,
old empty Friends-invite expectations, and reused post-play tests retaining new
prompt/autoplay settings between cases. The Home harness still checks every
real catalogue identity and absence of truncation/duplicates.

## Remaining validation and differences

- No real TV/BlueStacks installation or settings changes were authorised for
  this implementation. Launcher alias refresh, remote/pointer feel, final
  screen fit, subtitles with real files/providers, and post-play decoding /
  composition / CPU load remain untested on devices.
- The new post-play preview uses direct IMDb AVC trailers only, muted, and is
  separate from the existing Home/detail trailer source preference. Apple/YouTube
  previews and LG/Tizen secondary-player backends are not implemented.
- App icon and banner remain linked and entitlement-gated. Independent custom
  icon/banner uploads are not implemented.
- **Every original setting has a ledger entry; not every original control has
  been individually functionally certified by this run.** The host harness checks
  341 value contracts. Focused consumer tests cover 30 bindings and all 228
  filter subchoices; other controls retain their existing regression coverage
  and are explicitly marked uncertified in the ledger. Actions involving login,
  provider accounts, consent, destructive operations and platform hardware were
  not executed live. The older repository-wide sweep is not a green all-tests
  result; its pre-existing failures remain documented in `STAGE-J.md`.


## BlueStacks validation artifact

The first strict read-only APK blocked login as well as uploads. It was replaced
with a debug-only login-capable validation APK after the user reported this and
agreed to replacement. Authentication uses the existing server flow; settings,
progress and provider mutations remain blocked, as do sync-arm permissions.
Package/certificate/data retained; installed bytes verified. See the final
`STATUS.md` entry for SHA-256 and boundary tests. The agent did not launch or
sign into the app. This artifact cannot certify outbound synchronisation or
provider-account mutation features; those require separately approved testing.

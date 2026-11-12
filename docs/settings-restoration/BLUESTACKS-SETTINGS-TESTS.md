# BlueStacks settings checks — 8 November 2026

**Initial round only; not all-settings certification.** The user authorised local
preference changes, restarts and restoration. Purchases, consent, destructive
operations and account/provider mutations remain excluded. The login-enabled
validation build blocks settings/progress/provider uploads and sync arming.

## Individually observed controls

| Control | Original / test / restored | Result |
|---|---|---|
| App language | Automatic → English → Automatic | Editor, preview and save feedback pass. No whole-app language/restart certification. |
| Accent colour | Petrol → Ocean → Petrol | Actual focused-row/header/preview colours changed and restored. Remaining colours/modes and restart pending. |
| Background | Frost → Blurred art → Frost | Editor, preview, summary and restoration pass; real image-backed backgrounds not certified. |
| Home layout | Dynamic → Default → Modern → Dynamic | Actual Home/navigation changes observed. Modern exposed the hero-Off defect below. Dynamic restored. |
| Show hero | Off throughout | Interaction defect found, not an On/Off toggle test. |

All four changed preferences were visibly restored before code investigation.
Machine-readable checklist: `bluestacks-settings-tests.json`, with all 434 bindings;
unvisited controls explicitly remain NOT TESTED on BlueStacks. Earlier local
regression coverage remains in `SETTINGS-VERIFICATION.md` and does not imply a
device pass. No restarts, media playback or account data changes performed here.

## Defect found: Modern + Show hero Off

Modern reserved its normal hero row anchor, clip boundary and scroll displacement
even though hero drawing was disabled. The Home screen had a large empty region
and Continue Watching began near the bottom. Default/Dynamic did not show this.

Corrected Modern's three geometry calculations in `src/home.c` to release that
space with hero disabled. Existing hero-On geometry is unchanged. Added direct
production-function assertions to `tests/home_layout.c` across all three layouts
and normal/125% text scale. The new assertion failed before the fix and passes
after it. Full `tests/home.sh` passes. Device confirmation requires the rebuilt
APK; do not label this defect verified fixed on BlueStacks yet.

## Navigation observation

Edge gesture opened the sidebar. Directional keys/OK worked through menu,
settings categories and editors; Back returned to sidebar and Home. Direct
pointer clicks appeared offset while the visible pointer was at the target.
The cause (tool/emulator coordinate mapping versus app) remains unresolved;
no pointer-navigation pass is claimed.


## Approved corrected APK — second device round

Installed approved APK `f0a8de997c59ddf67cb25b62371c3fc70911d0052bd90619a1d9a044a785e75f`
with data retained and read-back hash verified. **Modern hero-Off fix PASS on
BlueStacks**, including app exit/reopen. Continue Watching now begins near the
top instead of below a large blank band. All changes below were restored.

| Setting/subsetting | Device evidence | Limits |
|---|---|---|
| Hide modern sidebar pill | On hides actual Home rail; retained across app exit/reopen; edge opens expanded menu; Off restores rail | Modern verified; Dynamic hidden-state case pending |
| Modern sidebar | Off/On, Hide pill dependency disabled/enabled | Full classic navigation not certified |
| Poster width | Actual Home geometry changed at Compact 104 dp; restored Comfort 134 dp | Other presets/restart pending; transient 74 dp also visited |
| Poster corners | Sharp 0 dp makes actual Home/expanded card square; restored Rounded 12 dp | Other presets/restart pending |
| Depth master | Off disables child controls; On restored | Actual effects on every media surface not yet device-certified |
| Posters / Continue Watching / Episodes / Cast / Trailers depth | Each individually Off then On | UI only; existing local framebuffer tests are separate evidence |
| All six sort choices | Each confirmed and reflected in sheet | Actual stream sorting remains mocked |
| Maximum results, per-resolution/quality, minimum/maximum size | Each 0 → 1 → 0, correct sheet return and rule counts | Actual source filtering remains mocked |
| All 228 filter checkboxes | Every choice individually On/Off through UI; all 21 group counts match, all restored None | No live stream selection/playback |
| Required / excluded release groups | Native TV keyboard saved temporary literal `0`; reopened and cleared, empty rule restored | CSV matching mocked; host text injection did not insert text |
| Resolve / preferred provider / preparation / TorBox cached additions | Disabled with No eligible provider, as expected for current setup | BLOCKED for actual provider test; no credentials entered |

Checkbox counts per mode (Preferred, Required, Excluded): resolutions 8, quality
13, visual tags 16, audio tags 13, channels 5, encodes 6, languages 15. Total
`(8+13+16+13+5+6+15)×3 = 228`. Required/Excluded audio were separately repeated
when initial batch screenshots were ambiguous; only the verified reruns count.
Final Filters & Sorting summary: **0 active rules**, matching initial state.

Two remaining UI findings: card-width/corner previews are animated illustrations
and do not reliably depict the selected geometry (the actual Home renderer does).
Source confirms the corner illustration uses a generic rounded card. Also, the
singular rule summary says `1 filters and sorting rules active`. Neither is fixed
in the installed APK; neither is a failure of the real filter/width/corner consumer.

The device checklist still marks unvisited bindings NOT TESTED. No all-settings
functional certification is claimed. Playback, subtitle loading with real media,
trailer timing, hardware, permissions/consent, entitlement and backend mutations
remain outstanding or outside the current approval. An unintended details page
was opened during navigation recovery; it was closed without starting playback
or changing its actions. No credentials/account data were inspected.

## Playback available after user source setup — third round

The user configured an additional source addon and requested a retry. Scrubs from
Continue Watching now opens and plays. **Full English embedded dialogue subtitles
loaded automatically before any manual subtitle selection.** This supersedes the
previous no-source block for basic playback; provider resolution/mutations remain
outside this test.

Individually exercised player controls:

- Main subtitles: Off removed the real dialogue text; selecting the English
  embedded track restored it.
- Size 120% → 170% → 120%, font Inter → JetBrains Sans → Inter, colour
  White → Yellow → White, opacity 100% → 50% → 100%, position 4/8 → 5/8 → 4/8,
  bold Off → On → Off and outline Black → White → Black visibly changed the
  rendered sample. Yellow also changed **actual embedded dialogue during video**.
- Background None → Dark 25% → cycle back to None and edge Outline → Shadow →
  None → Outline changed selected values. Black paused background limited visual
  assessment of dark backing/shadow; no complete effect certification.
- Delay 0 → +0.10 s → 0 saved/restored. Actual cue timing not measured.
- Volume 100 → 90 → 100 and speed 1 → 1.25 → 1 changed their UI values. No audio
  measurement or objective speed verification. A reprepare and E-AC-3 → AAC info
  change occurred around speed testing; track/subtitle retention through this
  transition needs controlled reproduction. No speculative playback fix made.

The player style controls are equivalent consumers of primary subtitle fields,
but this does **not** certify every global subtitle setting's persistence, every
font/colour, secondary subtitles, image subtitles or ASS handling on Android.

Additional preferences individually exercised and restored:

| Setting | Observed result | Remaining limit |
|---|---|---|
| Next episode automatically | Off → On → Off; children enable/disable | Actual unattended advancement pending |
| Countdown | 5 → 6 → 5 seconds; summary/description update | Actual ending countdown pending |
| Post-play recommendations | On → Off → On; percentage gated | Movie end behaviour pending |
| Recommendation percentage | Automatic/0 → 1% → 0 | Episode sample cannot certify movie recommendations |
| Still watching | Off/0 → 1 → 0 | Consecutive automatic episodes/prompt pending |
| Automatic full subtitles | On → Off → On; automatic On consumer observed above | Clean Off-policy comparison needs a title without saved manual preference |
| Show subtitles in player | Only my languages → All → original | Actual candidate-list comparison pending |
| Interface font | Inter → JetBrains Sans → Inter changes actual sidebar/settings text | Other faces/restart pending |
| Text/component scale | 85% → 90% → 85% changes actual geometry | Other values/Home/restart pending |
| Settings layout | Panel → List → Panel changes actual structure | Wider navigation/restart pending |
| Clock visibility | On → Off → On removes/restores actual clock | Other screens pending |
| Clock format | 24-hour → 12-hour → 24-hour shows/removes AM on actual clock | Guide/screensaver pending |
| App Icon & Banner | Existing supporter gate opens; no entitlement bypass | Actual alias/banner change blocked by entitlement |

### Corrections made locally after this round

These are **not present in the installed APK** and need an approved updated APK
for BlueStacks confirmation:

- `src/faixas.c`: the font selector incorrectly said “OpenSubtitles source”. It
  now uses the existing translated **Subtitle font** label. Font choice itself
  was working.
- `src/ajustes_ux_cenas.inc` / `src/ajcena.inc`: selected poster width and radius
  now drive the actual illustration geometry. Sharp radius 0 is respected by
  poster fill, light and focus; elapsed animation time cannot alter the chosen
  shape. Existing generic illustrations retain their previous default radius.
- Filters summary and one-episode interval use correct singular/plural English.
- Older subtitle selector harnesses did not link the newly used existing
  `legenda_idioma` implementation. Both scripts now link production `legenda.c`,
  rather than masking the missing dependency with a stub.

### Playback progress and restoration limits

BlueStacks denied `run-as` and direct access to private progress files. No root,
permissions or device security changes were attempted. The user was informed
before playback that exact progress backup/restoration was unavailable.
Playback advanced local progress, Restart was activated during control navigation,
and a subsequent seek occurred; last observed position was about 1:11. Do **not**
claim byte-for-byte progress or title-choice restoration. General preferences and
player styles above were restored. Selecting English manually may leave a
per-title choice. Settings/progress uploads and sync arming remain blocked;
authentication refresh is permitted by the approved build. No provider creation,
uncached download, account mutation or main-account inspection was performed.

### Local regressions

PASS: `faixas_estilo_ctl.sh`, `settings_expansion_subtitles.sh`, `legendasui.sh`,
`legendasui_425.sh`, `settings_expansion_native.sh`, `settings_stage_a.sh`,
`cw_parity.sh`, `settings_preservation.py` (413 bindings).
`settings_card_preview.sh` PASS with an actual hidden Mac GL framebuffer:
selected-width/sharp-versus-rounded pixels differ, identical selections at
0/1.5 seconds are identical. Initial sandbox graphics-service denial was an
execution limitation; rerun with graphics access passed.
PASS also: `validation_readonly.sh` (ordinary/strict/login boundaries),
`velocidade.sh` with the existing JDK via `NUVIO_TEST_JAVA` (actual Android
bridge and PCM restoration replay, not device audio certification).
Logs: ignored `build/settings-expansion/playback-*.log`.

**51 of 434 binding entries now have some BlueStacks observation**, including
UI-only and blocked entries. This is not 51 full functional passes, and does not
certify all settings. Remaining controls stay explicitly NOT TESTED in the JSON.

## 8 November — approved 197cc update and continuing individual tests

User approved the new APK installation and continued local testing. Installed
`build/settings-expansion/Better-Nuvio-settings-remaining-validation.apk`, retaining
app data. Public installed APK readback SHA-256 matches
`197cc1d01066c3712a67283cef95203e81ac5e569a286bd7f6e5eb11a776b048`.
Both ABI build flags include read-only validation and the existing login gate.
Settings/progress/provider writes and sync arming remain blocked; authentication
refresh is permitted. No Fire Stick actions, credentials inspection or commits.

New actual consumer checks:

- Editorial Home headings now read **Continue Watching / Family Watching** in
  English. All four title modes visited; original Classic restored.
- All five profile-picker backgrounds visited on the real picker. Movies gives
  poster wall, Stripes stripes, Projector cinema/film border. Profile art without
  remote art falls back to profile-colour glow; Light shows colour glow. Remote
  custom-art branch not exercised. Original Movies restored.
- Continue on profile picker Off removes programme preview; On restores it.
  Original On restored; same test profile selected throughout.
- Continue Watching visibility Off removes only that row, On restores it.
- Episode thumbnail Off uses Scrubs series artwork, On restores episode scene.
  Blur-next-episode saved/previewed but no qualifying unwatched aired card was
  established; do not count a consumer pass. Original On/Off values restored.
- Continue Watching Card/Wide both landscape; Poster visibly upright. Initial
  Right outside editor did not select Poster: repeated through actual editor,
  confirmed saved choice before Home. Original Card restored. Card-versus-Wide
  distinction remains a limitation, not three distinct-shape passes.
- Corrected poster-width preview narrows at 104 vs 134 dp; original 134 restored.
  Corrected corner preview responds at 12, 6 and sharp 0; original 12 restored.
- Global main subtitle size 120 -> 130 reaches actual player style state and
  rendered dialogue. Player size restored to 120; global recheck pending.
- Corrected **Subtitle font** label verified in real player. External English
  OpenSubtitles v3 SRT loaded automatically without a manual primary selection.
  Addon returned mixed S01E07/S02E07 names; episode-match correctness not certified.
- Selected a secondary S02E07 English external SRT: simultaneous top/bottom
  dialogue rendering confirmed. Secondary track restoration remains pending.
- Maximum quality 720p saves/previews, but existing Scrubs resumed 1080p AAC.
  Source explicitly implements a preference penalty plus only-source fallback,
  not strict exclusion. Original Automatic restored. Resume-path certification
  remains partial; no speculative source-policy change applied.

Additional local regression work: preservation guard PASS (413); Continue
Watching parity PASS; poster-provider and actual addon-root metadata parser PASS.
The poster harness now dead-strips unexercised discovery functions instead of
requiring unrelated progress/date/account link stubs. Top-menu regression PASS
with macOS graphics access: corrected Search fixture now identifies itself as a
Nuvio source, matching the existing Nuvio-only search policy. Settings Up routing
fix is covered; Android device menu-top layout check still pending.
Source-quality regression previously expected unsafe Profile-5 promotion,
contrary to current production safety policy. Updated those expectations and
added actual automatic-selection checks for all four persisted resolution
choices plus only-source-above-ceiling fallback. Suite PASS after explicit fixture
quality reset (missing keys intentionally retain previous in-memory preferences).
Subtitle automatic-selection mocks PASS. Logs: `remaining-*.log` in ignored build
folder. These are scoped passes, not certification of all 434 settings.

Still in progress: screensaver temporarily Off (restore original 2 min), secondary
subtitle temporarily selected (restore none), remaining unvisited controls,
paused Back/focus timing investigation and device-specific/provider limitations.

### Further navigation and subtitle checks

Each Explore, TV Guide, Schedule and Profile & Stats visibility switch was tested
individually against actual Dynamic Home menu membership. All four had the expected
addition/removal effect. Original Explore/Schedule/Profile & Stats On and TV Guide
Off restored. This tests visibility, not programme-guide data.

Modern layout: menu position Side -> Top produced the real horizontal menu; Up
from the Settings category-list top reached it, verifying the installed navigation
fix. Down returns underlying Home because Settings is modal; Return Settings
reopens Settings. With Modern sidebar Off, Sidebar Pinned -> Collapsed removed the
persistent rail and widened content; Pinned restored it. Restored menu Side,
Modern sidebar On, Sidebar Pinned and Home layout Dynamic.

Global primary subtitle 120% restoration confirmed in Settings. Secondary language
From account -> English -> Spanish and secondary size Automatic -> 150% saved and
reloaded, then original values visibly restored. These are partial checks: current
code uses secondary language in addon requests, candidate grouping/recommendations;
no automatic secondary activation was established. Manual secondary SRT rendering
worked; a later playback's actual Second slot showed **No second subtitle** checked,
confirming session reset and restoration. Do not claim all secondary style consumers
passed merely because their previews/save paths work.

Pause panel On -> Off disabled its subordinate settings. With Off, paused controls
remained stable without the idle overlay; captions activation and paused Back
hide-then-exit succeeded. Original On restored. Earlier failures involved inactivity
and pointer hovering transport/seek targets; no speculative production player fix.
Added paused-state regression to the existing Back harness. Native, webOS, sheet,
timeout and both Tizen-shell cases PASS using the existing ChatGPT bundled Node
runtime. The first run lacked Node, so its Tizen failures were environment-induced.
No runtime installed.


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

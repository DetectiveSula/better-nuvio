# Focused pre-release validation — 8 November 2026

The user approved replacing exhaustive one-by-one device certification with a
focused critical-flow check and automated regression coverage. This does not
certify all 434 controls. The individual device ledger still has 98 observations
(including partial/blocked cases) and 336 controls without an individual device
check. Keep that distinction in any release notes.

## Device observations

BlueStacks, dedicated test account, profile 1, approved read-only validation APK:

- Continue Watching opened Scrubs S2E7 and resumed video. English dialogue
  subtitles appeared automatically without opening a subtitle selector.
- 1080p/AAC information and advancing playback were visible. Pause changed the
  button to Continue at 9:29 / 21:35.
- Keyboard Escape was timing-sensitive around the paused overlay. Resuming and
  then dismissing controls/Back returned Home. Android toolbar Back exited cleanly.
  Do not infer complete remote-key or paused-overlay certification from this.
- Reopening retained authentication and presented existing profiles 1 and 2.
  Profile 1 loaded with Scrubs progress and the Native Family Watching row intact.
  This is a retained-session check, not a new login/account-switch test.
- Settings reopened correctly and retained Petrol accent, Frost background,
  Logo colour On and Automatic glass. No preferences changed in this focused run.
- Device left at profile 1 Home, playback stopped by app exit/reopen, no temporary
  preference restoration pending. Local progress advanced during approved testing;
  exact pre-test progress restoration is not claimed.

No fresh APK install, sync arming, live provider/account writes, commit or push.
Provider availability, cached additions and resolver tests use mocked transport.

## Artifact and existing build evidence

- APK: `build/settings-expansion/Better-Nuvio-addon-heading-validation.apk`
- SHA-256: `24875b3ee997d220aa26e9c8fc9e440e3834e2d4d954d762b3216895b7c3af6f`
- Size: 74,670,880 bytes.
- Existing package/signature, ARM64/ARMv7, TV launcher, assets, ZIP and 16 KiB
  alignment checks passed in the addon-heading build validation.
- Android unit result XML: 64 tests, six suites, zero failures/errors. Existing
  successful build/unit evidence reused because production code has not changed.

This is a **read-only validation APK**, not the normal public-release artifact.
It deliberately blocks settings/progress/provider writes and sync arming.

## Automated regressions

Current run: `build/settings-expansion/focused-release/`, with exit codes and log
paths in `results.json`. Fourteen suites passed initially: preservation, settings
A/B, resolver C and stream integration, TorBox D and cached addition/autoplay,
verified-uncached filtering, metadata/filter conversion, Continue Watching,
settings persistence, automatic subtitle policy, read-only transport guard and
Stage G profiles/rows. Player-persistence hit macOS sandbox display-service
initialisation before assertions; rerunning with `SDL_VIDEODRIVER=dummy` passed
separate-process persistence and one-time delay migration. All **15 suites PASS**.
The original environment-failure log is retained; no production fix was needed.

## Remaining limitations and release preparation

- No exhaustive individual certification; partial/blocked settings retain their
  existing ledger status. No claim that every control/sub-option works on device.
- Live TorBox account additions, provider playback variants and real cross-device
  settings/progress uploads are outside this read-only check.
- Fire TV/Android TV hardware, HDR/Dolby Vision, codec/audio passthrough, platform
  graphics and remote behaviour need relevant physical-device testing. BlueStacks
  does not certify these, LG webOS or Samsung Tizen.
- Objective speed/audio verification, certain paused-overlay interactions and
  unavailable/provider-gated settings remain unverified.
- Before a GitHub release: review the full uncommitted diff and secret/artifact
  exclusions, select intended changes, prepare release notes with limitations,
  build a normal intended release artifact and verify its identity/signature and
  safety gates. Committing, pushing or publishing still requires user approval.

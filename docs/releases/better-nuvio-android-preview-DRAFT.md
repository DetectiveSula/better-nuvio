# Better Nuvio Android TV — preview release notes (draft)

Unpublished draft. Release version, target repository, asset names and final
source commit are not selected yet.

## Included work to review

- Restored Nuvio-style settings navigation and English settings text, with
  additional Native controls retained and verified additional controls marked Legacy.
- Resolve Playable Links controls, provider preferences and TorBox cache status,
  cached autoplay and verified-uncached filtering improvements.
- Optional verified-cached TorBox account additions remain an explicit opt-in.
- Profile-specific Home row ordering and stable catalogue identities, preserving
  Native-exclusive rows and configurations beyond 30 rows.
- Compatible settings-sync infrastructure and portable Home presentation options.
- Settings additions, automatic subtitle selection and presentation fixes,
  including functional catalogue addon captions and poster previews.

This is a candidate summary, not an exhaustive changelog or approval of every
uncommitted change. The local tree also contains earlier Home/details, search,
artwork and profile work that needs scope review before a source checkpoint.

## Validation

- Fifteen focused native regression suites pass, covering settings, preservation,
  resolver, mocked TorBox/autoplay/filtering, Continue Watching, persistence,
  subtitles and profile-specific Home rows.
- Additional J7 mock two-device sync/privacy and release-tooling tests pass.
- Android unit tests: 64 tests, six suites, zero failures/errors.
- BlueStacks smoke checks: Continue Watching resume, playback, automatic English
  subtitles, pause, exit/reopen, retained login/profiles and saved appearance.
- APK signature, both ARM architectures, assets, TV launcher, ZIP and 16 KiB native
  alignment verified.

## Important preview limitations

- This candidate is a normal debug APK using
  `space.nuvio.nativelegacy.strictdebug`, version `2.0.8-strictdebug`. It is not a
  production-package release or a validated auto-update asset. Versioning remains
  unchanged during preparation.
- Validation write-blocking flags are OFF. Unlike the earlier read-only test APK,
  ordinary account/progress/provider policies apply if this APK is installed and
  launched. It has not been installed or launched as part of preparation.
- Android J4/J5 and J6 upload gates remain unchanged: debug builds require explicit
  account/profile arming; Android production builds currently refuse those writes.
  Existing Stage I automatic shared-setting sync is a separate mechanism.
  Do not advertise unrestricted production bidirectional sync.
- All 434 settings have not been individually certified. The device ledger has
  98 observations including partial/blocked cases and 336 unvisited controls.
- Provider tests use mocks. Live account additions and hardware-specific codec,
  HDR/Dolby Vision, audio and physical TV behaviour are not certified by BlueStacks.
- Keyboard Escape around paused overlays was timing-sensitive during the smoke
  check; Android toolbar Back exited cleanly.
- LG webOS and Samsung Tizen are not newly device-validated by this Android build.

No public release or source push is approved by this draft.

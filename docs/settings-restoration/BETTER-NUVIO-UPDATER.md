# Better Nuvio Android updater — 8 November 2026

Implemented locally after the user requested pointing the in-app updater at
Better Nuvio. Branch remains codex/settings-restoration; previous work preserved.
No commit, push, merge, publication, install or application-account mutation.

## Behaviour

- Android strictdebug uses the public
  `DetectiveSula/better-nuvio/releases?per_page=30` feed, because GitHub's latest
  endpoint excludes prereleases. Release-page/QR fallback points to Better Nuvio.
- Selects a non-draft Better Nuvio preview with a compatible asset. Canonical tag:
  `v<major>.<minor>.<patch>-better-nuvio-preview.<YYYYMMDD>.<serial>`.
- Full asset: `Better-Nuvio-<version>-strictdebug-preview-<YYYYMMDD>.apk`.
  Lite variants require their own strictdebug-lite-arm64/armv7 name; never fall
  back to a full/production/other-ABI APK.
- Requires the exact HTTPS repository, tag directory and asset name, plus a valid
  64-character SHA-256 from that same asset object. No neighbouring digest reuse.
- Selects highest base version, then date/serial within that base, rather than
  depending on GitHub creation order. Preserves complete long preview tags.
- Native byte-level SHA-256 verification remains before installation. Android
  now explicitly validates the downloaded APK's package ID, exact current signer
  set and strictly newer version code before opening permission/system installer.
  Android's own signing checks remain in force; no silent installer introduced.
- Android production uses the Better Nuvio stable latest endpoint and retains
  its existing production/Lite filename selection. This does not create a
  production release or enable cross-channel installation.
- Other platforms retain their existing Native Legacy update feeds unchanged.
- No automatic APK installation or provider/account operation during validation.

## Versioning and bootstrap limits

The already-published preview still contains the old Native Legacy updater and
has not been replaced. A one-time manual sideload of this updater-enabled build
is required. Future in-app updates must increase the numeric app version and
Android version code. Equal-version recuts, including date/serial-only changes
at 2.0.8/20008, are intentionally not offered as upgrades. The selected preview
tag/filename convention must be preserved when publishing future assets.

This local APK remains 2.0.8-strictdebug/20008 and has not been installed, launched
or published. There is no assertion that changing repository source modifies an
already-installed/published binary. End-to-end installer behaviour on a physical
TV still needs validation with two legitimately versioned, same-signer builds.

## Files and tests

- Native feed/query and bounded policy: src/atualizacao.c and
  src/atualizacao_better.inc.
- Android metadata extraction and policy: NuvioActivity.kt / ApkUpdatePolicy.kt.
- New isolated native test: tests/atualizacao_better_android.sh — PASS for full,
  Lite ARM64 and Lite ARMv7, including actual fioConsulta with mocked HTTP.
  Covers empty/malformed/failed responses, draft/foreign/incorrect-digest assets,
  nested-field isolation, version ordering, long tags, same-version refusal and
  known SHA-256 vector. Tests never call Android installer or live GitHub.
- Existing production variant selection — PASS all three variants. Fixed an
  existing macOS Bash empty-array/nounset harness failure (no production effect).
- Existing general updater and scheduling regressions — PASS.
- Android unit tests — 69 tests / seven suites / zero failures/errors, including
  five new package/version/signer-policy tests. Actual PackageManager extraction
  compiles; no real install/permission prompt exercised.
- Original 413 preservation, Continue Watching parity, read-only transport guard
  and git diff --check — PASS.

## Local artifact

`build/updater-better-nuvio/Better-Nuvio-2.0.8-strictdebug-updater.apk`

74,671,919 bytes. SHA-256:
`dcfe666a8f04960a236907e0c199478c1b9f41f7b6182f0d7e18c38c94c1b404`

Unchanged package: space.nuvio.nativelegacy.strictdebug.
Unchanged certificate:
`c0a433a85a1e3ca1fa9e1ef4b5998b36444b0cc8efc88c9a3ccb97d2f7c59bba`.
Normal build with validation flags OFF. Both packaged native libraries match the
current stripped output and contain the new feed and strictdebug filename policy.
Signature, ARM64/ARMv7, TV launcher and ZIP/16 KiB native alignment PASS.
Logs and verification.json are under build/updater-better-nuvio/.
Published b98b… APK and local release-preparation bundles remain unchanged.


## 8 November — updater preview 2 published with explicit approval

Published v2.0.8-better-nuvio-preview.20261108.2 at 15:31:35 UTC:
https://github.com/DetectiveSula/better-nuvio/releases/tag/v2.0.8-better-nuvio-preview.20261108.2

GitHub API confirms prerelease=true, draft=false, exact approved existing anchor
7cac4f25d11931b56c41cfefb952e4e183c45c1a and unchanged remote master. Preview 1 remains
available with its original APK digest. Downloaded APK, SHA256SUMS and INSTALL.txt
from the public release; all byte counts and hashes match the local approved
files and GitHub digests. Body matches the prepared notes.

APK SHA-256: dcfe666a8f04960a236907e0c199478c1b9f41f7b6182f0d7e18c38c94c1b404.
Size 74,671,919 bytes, unchanged strictdebug package/certificate and version 20008.
One manual install is needed to bootstrap the Better Nuvio updater; later in-app
updates must increase numeric version/versionCode. Physical installer validation
remains outstanding. No automatic installation, local source/history upload,
commit, push, merge, master modification, sync arming or provider/account mutation.
Evidence: build/updater-better-nuvio/published-verification-2.json and
published-prerelease-2.jpg. Previous local work and preview 1 preserved.

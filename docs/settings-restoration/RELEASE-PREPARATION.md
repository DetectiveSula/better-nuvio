# Local release preparation — 8 November 2026

Branch remains `codex/settings-restoration`. No staging, commit, push, merge,
version bump, device install, live account access or publication performed.
All pre-existing work remains in place.

## Normal preview APK — built and verified

`build/release-preparation/Better-Nuvio-2.0.8-strictdebug-normal-preview.apk`

- SHA-256: `b98b067ea1a74b9ba1040a42088ad6e48ddd32c85ab4feca13e3421bbf9b2784`
- Size: 74,670,667 bytes.
- Package: `space.nuvio.nativelegacy.strictdebug`; unchanged version 2.0.8-strictdebug.
- Certificate: `c0a433a85a1e3ca1fa9e1ef4b5998b36444b0cc8efc88c9a3ccb97d2f7c59bba`.
- Read-only and validation-login flags OFF in both current ABI CMake/Ninja builds.
  Both packaged libmain libraries match the current Gradle stripped output.
- Both libraries contain the new addon-heading caption and stable-name-copy code.
- Both ARM64 and ARMv7 libraries, 1,129 assets, TV launcher, signature, ZIP
  alignment and 16 KiB ARM64 load/RELRO alignment pass.
- No private settings, session, addon credential files or signing keys found in
  APK entry names. This filename scan is not a guarantee about every binary string.
- Existing approved read-only validation APK preserved separately and remains the
  installed artifact on BlueStacks. This normal APK was not installed/launched.

Build: existing `tools/android.sh --debug-only`, offline caches, original package
and debug signing. Initial sandbox attempt lacked cache write access; repeated
with approved local SDK/cache access succeeded. Release signing files were not read.

## Tests

- Previous focused regression manifest: 15/15 PASS, unchanged production sources.
- Fresh `tests/release.py`: PASS; child failure lines are intentional test fixtures.
- Fresh `tests/settings_stage_j7.sh`: PASS, isolated file-backed two-device backend;
  no live backend access. Includes profile/account/offline/conflict/privacy checks.
- Normal-configuration Android unit task PASS: 64 tests, six suites, zero errors or
  failures. Gradle correctly reused unchanged up-to-date test outputs.
- `git diff --check`: PASS.

## File review and proposed exclusions

`build/release-preparation/candidate-files.json` records 403 expanded changed or
untracked paths at the initial scan: 325 candidate source/test/documentation paths,
76 visual-reference files requiring separate review, and two excluded logs/cache
files. Counts are a snapshot and increase with these preparation documents.
No candidates were staged; this inventory is not a full review of every diff.

Exclude `prev.log` and `android/.kotlin/errors/errors-1794092765331.log` from a
checkpoint. Keep build outputs, APKs, local.properties, signing files and personal
configuration excluded. Existing ignored credentials were not added or printed.
Visual screenshots and internal audit/device documents require a privacy/scope
review before any public source commit. Do not blanket `git add .`.

Current-text heuristic privacy scan: 333 text files, one match, confirmed synthetic
`fixture.invalid` userinfo in a negative URL test; no actual credential finding by
these patterns. Seventy binary/reference files were not text-scanned. Findings
contain path/line/type only, never matched values. This is not a full secret audit.

## Confirmed source-publication blocker

Reachable history includes non-empty `deploy/app/art/tmdb.txt`, `trakt.txt` and
`addons.txt` blobs in commit `4ea949d33baa`. They are no longer tracked at HEAD,
but ignoring/deleting files does not remove historical blobs. The repository's
existing .gitignore also explicitly identifies these paths as personal secrets.
No values were printed; their validity or prior exposure was not tested.

Do not push this ancestry publicly without reviewing/rotating affected credentials
and agreeing a safe history/publication approach. Options include a reviewed clean
export to a separate publication repository, or an explicitly approved history
cleanup with credential rotation. This task did neither; no existing history,
branches or credentials were modified. An APK-only publication and a source push
are separate decisions and require review/approval of destination and artifact.

## Other release decisions

- Existing release scripts require a clean immutable source tree and coordinated
  versioning. This dirty development tree does not satisfy that release gate.
- The normal APK above is a debug preview, not a signed production package. A
  production package would use `space.nuvio.nativelegacy` and its separate release
  certificate. No identity/signing change is made implicitly.
- Android production J4/J5/J6 writes are currently blocked by `src/j7arme.h`;
  debug needs explicit account/profile arming. Preserve these safety gates and
  disclose this limitation; Stage I ordinary automatic uploads remain separate.
- No exhaustive settings or physical-device certification. Preserve the limits
  in FOCUSED-RELEASE-CHECK.md. Existing broader test-suite limitations are not
  superseded by passing this focused set.

Unpublished notes: `docs/releases/better-nuvio-android-preview-DRAFT.md`.
Next: choose preview versus production release scope, resolve historical-secret
publication strategy, review the exact intended source checkpoint, and obtain
separate approval before committing, pushing, installing or publishing.

## APK-only prerelease chosen — prepared locally

User selected the APK-only prerelease with existing strictdebug identity.
The destination `DetectiveSula/better-nuvio` was confirmed PUBLIC through read-only
GitHub metadata. Existing remote commit
`7cac4f25d11931b56c41cfefb952e4e183c45c1a` was verified on GitHub and is proposed
only as an existing release anchor; it is not this dirty-tree APK's source commit.
No branch has moved. GitHub-generated source archives of that old anchor are
unavoidable for a normal repository release and must not be described as the
latest APK source. No local history/source upload is proposed.

Local bundle: `build/release-preparation/Better-Nuvio-APK-only-prerelease-20261108.zip`.
Folder: `build/release-preparation/apk-only-prerelease-2026-11-08/`.
Five files: exact verified APK, SHA256SUMS, INSTALL.txt, RELEASE-NOTES.md and local
manifest.json. Proposed GitHub assets are APK, SHA256SUMS and INSTALL.txt, with
RELEASE-NOTES.md as the release body; manifest stays local. No source/logs, private
configuration, credentials or signing files included. ZIP contents and APK hash
verified. Existing validation and normal APKs unchanged.

Proposed title: Better Nuvio Android TV — experimental preview 2026-11-08.
Proposed tag: `v2.0.8-better-nuvio-preview.20261108.1`.
Mark prerelease and not latest. APK version code/name remain unchanged; full
checksum identifies this binary. Notes disclose write-capable behaviour, gates,
untested controls, limited hardware coverage and uncommitted build provenance.

Ready for review, not published. Approval must explicitly cover this public
repository, the assets/body, creation of the proposed tag at the existing remote
anchor, and prerelease publication. No commits, pushes, source uploads or installs
are implied. Historical credential risk still requires a separate resolution.


## 8 November — approved APK-only prerelease published

User explicitly approved public publication of the three reviewed assets, release
notes and tag at the existing remote anchor. Release published at 14:42:29 UTC:
https://github.com/DetectiveSula/better-nuvio/releases/tag/v2.0.8-better-nuvio-preview.20261108.1

GitHub API confirms prerelease=true, draft=false, exact approved tag anchor
`7cac4f25d11931b56c41cfefb952e4e183c45c1a`, and unchanged remote master. Release body
matches approved local notes. Downloaded all three public assets after publishing:
APK, SHA256SUMS and INSTALL.txt; every byte count and SHA-256 matches local approved
files and GitHub asset digests. APK: 74,670,667 bytes; SHA-256
`b98b067ea1a74b9ba1040a42088ad6e48ddd32c85ab4feca13e3421bbf9b2784`.

Evidence: build/release-preparation/published-verification.json and
published-prerelease.jpg. The APK-only local ZIP remains the original prepublication
preparation snapshot. No local source/history uploaded, local commits/pushes/merges,
branch movement, device install, live provider operation or application-account
mutation. GitHub's automatic old-anchor source archives are explicitly disclosed
as not matching the APK's uncommitted working-tree source. Historical credential
cleanup remains separate and unresolved.

Upload initially required manual user-enabled ChatGPT extension file-URL access;
browser policy blocked automating chrome://extensions and no bypass was attempted.
After user confirmation, uploaded only the approved checksum/install files and
published. Existing local work on codex/settings-restoration is preserved.

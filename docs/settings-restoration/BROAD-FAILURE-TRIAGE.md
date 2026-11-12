# Broad regression failure triage — 5 November 2026

Branch: `codex/settings-restoration`. Comparison checkpoint:
`2bb366167fb9359b854303d6f096b85c80a1abf5` (Stages B–D).
No branch switch, reset, commit, push, merge or production-source change.

## Original sweep recovered

Recovered the original runner and its per-test logs from Claude's task scratchpad.
The original summary contains **1,141 runs: 424 passed, 717 failed**. It ran eight
scripts concurrently, including visual captures and platform-dependent harnesses;
it is not a suitable unattended Android APK gate as currently configured.

The original summary, classifications, log hashes and reproductions are retained
under ignored `build/stage-j-cleanup/broad-sweep/`. The durable per-failure ledger
is `BROAD-FAILURE-TRIAGE.json` (all 717 original failures). Original logs were not
modified. Raw account/credential data was not copied into this report.

| First blocking failure in original log | Count | Findings |
|---|---:|---|
| Interrupted `sessao.c` edit did not compile | 399 | Same orphaned function body, now repaired. Nineteen affected suites already passed the focused gate; additional settings-default comparison passed. Removing this blocker does **not** establish that all 399 now pass: e.g. subsequent old scope/sync assertions are exposed. |
| Missing link dependencies/stubs | 111 | Minimal legacy harnesses omit dependencies such as CW days, metadata episode IDs, Home projection and the new sync bridge. Representative failures also occur at the checkpoint. One previously passing profile-selector fixture needed the new Stage G query; fixed below. |
| Other compiler/tool failures | 38 | 11 missing FFmpeg development-header cases; 3 Rust target/path cases; 8 stale/duplicate fixture declarations; 10 stale extracted function/signature cases; 1 platform compiler flag; 1 mixed Kotlin cache; 1 SDL include path; 3 other link/tool diagnostics. |
| Assertion failures | 15 | Stage B now passes after the exact privacy-log allowance. Discord, Guide cache and Guide filtering pass in controlled reruns. The other 11 reproduce at the unchanged checkpoint as well as the current tree. They remain maintenance failures, not passes. |
| Runtime/harness/prerequisite failures | 154 | 72 unavailable external-volume paths; 6 additional visual prerequisites; 7 missing `rg`; 11 missing tools/files; 2 JDK discovery; 2 pkg-config prerequisites; 1 WebAssembly SDK; 4 silent preconditions; 49 remaining runtime/contract cases. These counts are based on the original first-failure signatures, not a claim every underlying issue is resolved. |

There are 169 named visual/capture tests among the failed runs overall; this
cross-cuts the table (many failed compilation before reaching their GL capture).
Other prerequisites include FFmpeg/mkvmerge, libass, Emscripten, a .NET SDK,
Apple Container and historical Git references. No platform test is marked passed
merely because it cannot run on this Mac.

## Reproduction method and outcomes

Ran **41 representative tests on both trees: 82 current/checkpoint runs**. The
checkpoint was extracted to a new temporary directory using `git archive`, with
no `.git` checkout, branch switch or changes to the user's working tree.

Every reproduction ran inside an OS sandbox denying network access, with a
separate temporary directory and `NUVIO_DADOS`. Tests used mocks/fixtures. No
account/provider backend could be contacted, no device was used, and no live
settings write or APK installation occurred.

Only two representative tests had different pass/fail results between trees:

1. **`perfilsel`: checkpoint PASS, current missing `fil_tem_projecao`.** Stage G
   added a valid production dependency while the small fixture intentionally
   omits `fileiras.c`. Added a controllable test stub and assertions that a valid
   synced projection uses its own profile's addon scope while preserving the
   original primary-profile and local-fallback behaviour. Current test now
   **PASS**, including **ASan/UBSan**. No production fallback was weakened.
2. **`ajustes_sync`: checkpoint PASS, current FAIL.** The old test demands
   `c_settings_v1`/device snapshots and same-device settings restoration through
   the account document. Stage I deliberately removed that protocol behaviour.
   Do not restore device-snapshot uploads to make the obsolete test pass. The
   current Stage I/J4/J5/J6/J7 mocks test the replacement ownership, safe login,
   isolation and no-default-upload behaviour. This old harness remains a clearly
   identified obsolete contract requiring rescoping, not a hidden green result.

Current reruns of `discord_runtime`, `guiacache`, `guiafiltro` and
`ajustes_padroes` pass. For the first three, a controlled rerun alone does not
prove whether the original cause was PATH, concurrent load or shared temporary
artifacts; the original eight-way runner should not be used to infer a new app
regression without reproducing it in isolation.

The other representatives retain the checkpoint's outcome, including legacy
Continue Watching assertions, subtitle/source harness links, stale Home function
extractors, duplicate fixtures, FFmpeg-header prerequisites, four small renderer
crashes, the old context-menu source contract and localisation checks. These are
not declared harmless or fixed; checkpoint parity establishes they were already
present before the current Stage G/I/J work. Some failing aggregate checks also
contain new debug-string warnings (see backlog below).

## Additional test-harness repair

`android_live_ts.py` assumed `~/.local/jdks` and selected arbitrary cached jars.
It now honours `JAVA_HOME` and `GRADLE_USER_HOME`, falls back to the installed
system JDK 17, and excludes source/javadoc jars from executable dependencies.
The discovered runtime failure was Hamcrest's **javadoc jar** being chosen instead
of its class jar. The real Media3/JVM fixture now passes **3/3 tests offline with
network denied**. No dependency version or application implementation changed.

## What remains

The 717 original failures are now classified with evidence, but the whole sweep
is **not green** and was not blindly repeated. The following are separate
maintenance tasks before claiming an all-platform clean suite:

- Split host logic, visual/manual capture, Android, Linux/webOS/Tizen, media-tool
  and historical-contrast suites; document prerequisites and keep backend/network
  access fail-closed. Use unique output paths and bounded concurrency.
- Repair incomplete unit link fixtures; retain real behaviour assertions rather
  than adding production weak/no-op fallbacks to hide linker errors.
- Rescope the old `ajustes_sync` device-snapshot test to the current protocol.
- Investigate the 11 pre-existing assertion failures and reproduced small
  renderer crashes independently. Preserve the working Continue Watching policy.
- Repair stale source-text extractors and contracts. `ctxmenu_contract` is still
  failing after `rg` is available; a missing-tool fix alone does not close it.
- Resolve localisation debt: the broad scanner reports existing missing keys
  plus J0/J7 debug-notice formats. The 29-table consistency and current settings
  A/B English gates pass, but that does not make the whole-app scanner green.

The focused Stage J/APK gate remains **41/41**, plus **61 Android JVM tests**;
Home/profile/preservation checks also pass after the test-only repairs. Production
source hashes and APK hash are unchanged, so no rebuild is needed for these
harness changes. This supports **dedicated-test-device validation**, not a release
or a claim of complete all-platform test coverage.

## Candidate and approval boundary

Use the verified APK SHA-256
`a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`.
Follow `DEDICATED-DEVICE-VALIDATION.md`. Confirm dedicated test accounts first;
installation and any live sync writes require the user's explicit approval.
BlueStacks may still hold the main account: do not open or inspect its backend.
Stages E and H remain deferred. No application code or APK changed during triage.

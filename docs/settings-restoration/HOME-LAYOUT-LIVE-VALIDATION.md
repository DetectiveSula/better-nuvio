# Final Stage J Home-layout live validation — preparation

5 November 2026. Candidate SHA-256:
`a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`.
**No J4/J5 arming or setting changes authorised/executed yet. J6 stays disabled.**

## Verified read-only baseline

The known dedicated test-account fingerprint is `d21b69ba`. Fire's local JWT
subject matches it, selected local profile is 1, and its Stage I/J6 journals have
zero pending edits. Its arming file has no armed entries. No token/header/raw
account ID is stored in this report.

BlueStacks was already running, rather than started by this test. Current-process
logs contained **two account sessions**: earlier sync lines belonged to the prior
account. After correlating to the latest test-account login, there are no subsequent
settings-cycle lines. Latest identity matches the dedicated test account; its
**current selected profile is not independently verified**. Earlier profile-1 /
40-J6 / 31-Stage-I / empty-Better lines must not be used as current test-account
snapshots. BlueStacks was force-stopped to avoid unintended automatic writes.

Automatic approval review rejected online app launch/app-triggered snapshots
because existing Stage I uploads are independent of the J4/J5/J6 debug gates.
Neither rejected action was performed. A safer independent probe used only the
source-verified `sync_pull_profiles` and `sync_pull_profile_settings_blob` read
RPCs with the existing, privately verified Fire test-account access token. It
never started/resumed an app, refreshed authentication, or invoked a write RPC.
No raw backend documents or credentials were saved.

Dedicated account profiles **1 and 2 exist** (read HTTP 200). Profile 1 baseline:

| Namespace | Bytes | Fingerprint | Read |
|---|---:|---|---|
| `tv` | 260 | `f25096afddf566a6` | HTTP 200 |
| `better_nuvio` | 851 | `fdb3931afeb0a1c9` | HTTP 200 |
| `home_catalog_shared` | 2 | `9b8a3e00c63ea86f` | HTTP 200 |
| `mobile` | 2 | `9b8a3e00c63ea86f` | HTTP 200 |

Better has six existing fields; **`homeLayoutLocal` is absent**. This is a server
baseline obtained via Fire's verified credentials, not two independently obtained
on-device snapshots. No main-account backend document was accessed.

## Privacy and migration observations

- Existing Fire session/profile logs: ten lines, no raw account UUID or token.
- Blue logs **after the latest test-account login**: no raw account UUID/token.
- Fire profile-1 J6 journal: v2, hash-only owner, matches test account, no pending
  edits. Profile 2: **legacy v1 with raw identity in private storage**, same test
  account, no pending edits. It is unopened/unmigrated; do not call migration
  fully live-verified. Host migration tests passed previously.
- Blue's normal `run-as` access is unavailable. No root/permission changes or
  activation bypass were attempted.
- Both apps are stopped; no activation links/settings changes/writes occurred.
  New foreground/online profile tests are pending, not passes.

Authoritative evidence: `build/stage-j-final-live/server-read-only-snapshot.json`,
`blue-latest-account-only.json`, `blue-latest-account-filtered.txt`,
`fire-privacy-verified.json`, `fire-j6-format-check.json`,
`preflight-stopped-state.json`. Earlier uncorrelated Blue summaries are superseded.

## Approval needed to finish preflight

Choose a strictly read-only validation build that blocks backend mutations before
startup, or separately approve the existing Stage I automatic settings sync on
this dedicated test account during online preflight. Merely leaving J4/J5 and J6
off does not guarantee an online launch is read-only. No such write approval has
been inferred from permission to start validation.

A new build would require installation approval again. Whichever route is chosen,
verify the same account and actual selected profile on both devices, initial
backend fingerprints and unexpected pending fields before any J4/J5 activation.
Do not arm if pending fields other than the agreed test edits are present.

## Prepared controlled tests — not yet executed

| Test | Expected result |
|---|---|
| First open, all gates off | New Home layout binding causes no default upload; confirm actual UI values without editing them |
| Fire → Blue, profile 1 | Separately approve J4/J5; deliberately select agreed Modern/Default/Dynamic values; receiver applies the existing enum 0/1/2 with one update per actual change and no echo |
| Blue → Fire | Repeat opposite direction; values/UI survive restart; no duplicate uploads |
| Document isolation | Only approved `better_nuvio.fields.homeLayoutLocal` changes; retain the other six fields; `tv`, `home_catalog_shared`, `mobile` unchanged unless a separately identified Stage I update was explicitly approved |
| Profile 2/migration | With further agreed profile scope, open existing profile 2, verify v1 → v2 locally with pending state retained; no J6 arming or synthetic legacy-file injection |
| Foreground refresh | Record fresh current-account log timestamps/PIDs before HOME/resume; distinguish immediate HTTP pull from cached log lines and normal timer refresh; do not assume every foreground return starts a cycle |
| Safety/privacy | Disarm after each controlled test; verify both scopes off, private headers not printed, no raw account IDs/tokens/code in saved diagnostics |

No provider/TorBox operations, account-schema changes, production-account reads,
progress edits, Stage E/H work, commit/push/merge or master modification.

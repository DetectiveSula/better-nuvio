# Stage J — Better Nuvio cross-device synchronisation (audit and design)

Status: **Complete (6 November 2026).** J0–J7 implemented and verified on dedicated test devices, including bidirectional Home layout sync; see 17.38 and STATUS.md "Stage J — complete". (Earlier status line, kept for history: J0 verified; J1 finalised; J2–J5 implemented and mock-tested; Better-tier writes default Off.)
Scope: the Android TV build only. Samsung Tizen and LG webOS are out of scope for
now; the design keeps platform-neutral data so they can join later.
Reference: official NuvioTV `99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`
(`~/Documents/Nuvio-Official`) and the current `codex/settings-restoration` tree.
No live account was read or written for this audit.

Machine-readable inventory: [stage-j-inventory.json](stage-j-inventory.json)
(418 controls: the 413 Stage B controls plus the five Stage C/D controls).

## 1. Method

Every surface below was traced from the account call to the Native consumer:
RPC/table name, request scope (`p_profile_id`, `p_platform`), worker function,
main-thread apply, persistence, and the write path back to the account.
A setting is "synchronised" only when both directions exist in code. A Stage I
bridge entry alone is not taken as proof; the read (`puxarAjustesPerfil` →
`ajustes_compartilhados_aplicar`) and write (`empurrarAjustes` →
`ajustes_compartilhados_mesclar` → `sync_push_profile_settings_blob`) were
traced for the 32 fields.

## 2. Current synchronisation audit

### 2.1 Account data surfaces

Better Nuvio device A and device B use the same calls; "A → B" means a change
made in Better Nuvio on A reaches Better Nuvio on B through the account.

| Surface | Account call (scope) | Better Nuvio reads | Better Nuvio writes | A → B | Notes |
|---|---|---|---|---|---|
| Profiles (names, avatars, `uses_primary_addons`, primary) | `sync_pull_profiles` (account) | Yes | **No** (no `sync_push_profiles`) | No | Created/edited in regular Nuvio; both B devices read the same list |
| Profile PIN locks | `sync_pull_profile_locks`, `verify_profile_pin` | Yes | No (no set/clear) | n/a | PIN set in regular Nuvio |
| Addons (membership, order) | `addons` table read (profile, or primary when inheriting) | Yes | Enable/disable only (`PATCH addons`) | Toggles only | Installs/removals in Better Nuvio stay local (`addonsEnviarToggles`: "never change membership"); `sync_push_addons` unused |
| Plugins (repositories) | `sync_push_plugins` | Yes | Yes (whole list) | Yes | |
| Provider credentials (TorBox, RD, …) | `sync_pull/push_provider_credentials` | Yes | Yes | Yes | Separate credential RPC; never in settings blobs |
| Profile settings `p_platform:"tv"` | `sync_pull/push_profile_settings_blob` | Yes | Yes: 32 fields, merged into a fresh pull | **Yes** (32 fields) | Regular NuvioTV pushes the whole document; see 2.3 |
| Home order / disabled rows / titles | `sync_pull_home_catalog_settings` (`home_catalog_shared`, profile or primary) | Yes | **No** (read-only by design) | Only what regular Nuvio wrote | Both B devices show the same regular-Nuvio layout |
| Native row presentation (shape, size, Friends Watching / Last Session placement, TV-only hides, hero source) | none (`fileirasui-p<N>.txt`) | — | — | **No** | Local per profile per device |
| Collections (+ pinToTop) | `sync_pull_collections` (profile) | Yes | **No** | No | Edited in regular Nuvio |
| Watch progress / Continue Watching | `sync_pull/push/delete_watch_progress` | Yes | Yes | Yes | Unless Trakt/Simkl owns progress (provider sync) |
| Watched items | `sync_pull/push/delete_watched_items` | Yes | Yes | Yes | |
| Library / watchlist / favourites | `sync_pull_library`, `sync_push_library` (read-merge-write) | Yes | Yes | Yes | No server delete; whole list rewritten after a fresh read |
| Trakt / Simkl | Provider APIs + credential path | Yes | Yes | Via provider | Outside settings sync |
| Better Nuvio-only preferences | none | — | — | **No** | The old `c_settings_v1`/`c_devices_v1` extension is unwired since Stage I |

### 2.2 Settings inventory summary

| Class (stage-j-inventory.json) | Controls | Syncs today |
|---|---:|---|
| Official shared, verified (Stage I) | 32 | **Yes**, both directions, per profile |
| Candidate: official feature key, value conversion unverified | 64 | No (local per profile) |
| Candidate: Better Nuvio namespace (portable, no official key) | 103 | No (local per profile) |
| Device-local (scope "this TV only") | 60 | No, by design |
| Device-local hardware/display/network (keyword rule, mirrors official exclusions) | 16 | No, by design |
| Device-local or credential-adjacent (endpoints, IPTV, servers, keys): verify | 16 | No |
| Keep local: TorBox cached-add (download-capable safety opt-in) | 1 | No |
| Not a stored preference (actions, read-only rows, sections) | 126 | n/a |

Additional read-only imports (not bindings): account subtitle/audio languages,
TMDB language, stream-source regex/selected addons/plugins and badge rules are
read from the `"tv"` blob and never written back.

The 32 synchronised fields: 17 `layout_settings`, 2 `player_settings`, 12
`tmdb_settings`, and `debrid_settings.debrid_enabled` (Resolve Playable Links).
Stage I's mocked transport test proves the full A → account → B path; it has not
yet been observed on two physical devices.

### 2.3 Can existing account storage hold Better Nuvio-only preferences?

| Storage | Verdict | Evidence |
|---|---|---|
| Extra keys inside the `"tv"` profile blob (old `c_settings_v1`) | **Unsafe** | Regular NuvioTV's `ProfileSettingsSyncService.exportSettingsBlob` builds a fresh `{version, features}` document and pushes it whole after every local change; any extra key is erased. Issue #205 already recorded that risk. |
| Extra keys in the `home_catalog_shared` blob | **Unsafe** | `mergeHomeCatalogSettingsJson` keeps unknown top-level keys only when an in-memory cache from a prior pull exists; a push before the first pull of a process drops them. The blob is also shared with regular Nuvio's Home. |
| Same profile-settings RPCs under a **separate `p_platform`** (e.g. `"better_nuvio"`) | **Recommended, unverified** | Blobs are keyed by user, profile and `p_platform`; regular Nuvio only touches `"tv"` and `"home_catalog_shared"`. `home_catalog_shared` shows the column is not restricted to device kinds, but whether the server accepts a new value (constraint, size limit, RLS) is **not proven**. Needs one approved verification (J0). |
| New table / server / function | Not proposed | Would need backend work and your approval. Only a fallback if J0 fails. |

## 3. Proposed architecture

Three tiers, each with exactly one owner per setting:

1. **Official shared tier** — regular Nuvio's own formats, interoperable with
   regular Nuvio: the `"tv"` blob (Stage I bridge, extended field by field),
   `home_catalog_shared` (read), profiles, addons, plugins, collections,
   progress, watched items, library, provider credentials. Unchanged transport.
2. **Better Nuvio tier** — one per-profile document in the account under
   `p_platform:"better_nuvio"` via the existing `sync_pull/push_profile_settings_blob`
   (same auth, session, profile guards and retry). Regular Nuvio never reads it.
3. **Device tier** — hardware, display, decoder, audio output, buffers, caches,
   memory, network, IPTV/local servers, P2P, diagnostics, install-specific
   packages, and anything credential-adjacent. Never uploaded.

### 3.1 Better Nuvio document (schema `better-nuvio-profile`, version 1)

```json
{
  "schema": "better-nuvio-profile", "version": 1,
  "fields": {
    "<native storage key>": { "type": "int|bool|enum|string", "value": 2,
                               "rev": 7, "device": "<client id hash>", "at": 1793900000000 }
  },
  "home": {
    "rows": { "<official row key>": { "shape": 3, "size": 2, "tv_hidden": false, "rev": 4, "device": "…" } },
    "exclusive": { "social_activity": { "after": "<official row key>", "hidden": false, "rev": 2, "device": "…" } }
  },
  "unknown": {}
}
```

* Field keys are Native storage keys; row keys are the **official** row keys
  (Stage G identity, `chaveOficial`), never URLs or URL fingerprints.
* An allowlist decides what may enter `fields`; a denylist (keys, tokens, URLs,
  endpoints, credential stores) is enforced again at serialisation. Size cap
  (e.g. 64 KB) and strict validation as in Stage I.
* Unknown fields and future schema sections are preserved on merge.

### 3.2 Merge, conflicts and loops

* **Per-field revisions.** The server has no compare-and-set, so each field
  carries `rev` (monotonic per field) and the writing device id. On merge the
  higher `rev` wins; equal `rev` breaks ties by device id, so every device
  converges to the same result. Applying a remote value never creates a local edit.
* **Pull–merge–push.** A push always follows a fresh pull in the same cycle,
  merging only locally edited fields (generalised Stage I journal: per
  account/profile `observed/base/pending/rev`). A race between two Better
  devices inside one pull–push window is the only residual loss; the next cycle
  converges by revision.
* **No stale defaults.** A fresh installation applies the account before any
  upload; only fields the person edits on that device become pending.
  Defaults are never uploaded.
* **Loops.** Acknowledge by value and revision (as in Stage I); imported values
  are rebaselined, never journalled as edits.
* **Offline.** Edits are journalled immediately and pushed on the next cycle;
  resume/player-close refresh (Stage I) also applies.
* **Profile safety.** Owner + profile header on journals and caches, existing
  `nv_perfil_sessao` guards, and responses from a previous context discarded.
  Inheriting profiles keep their own Better document (official inheritance
  covers addons and Home order only).

### 3.3 Precedence

Official shared tier (verified fields) → Better Nuvio tier → device tier.
A setting lives in exactly one tier. Stage G stays authoritative for shared
row order and visibility; the Better tier only adds Native-only presentation.

### 3.4 Not proposed without separate approval

* Writing `home_catalog_shared` from Better Nuvio. This changes regular Nuvio's
  Home and needs a complete catalogue list guard, as official `sync_push_home_catalog_settings`
  does.
* Writing profiles, collections or addon membership.
* Any new server, table, function or authentication.

## 4. Backend limitations and risks

| Risk | Impact | Mitigation |
|---|---|---|
| New `p_platform` value may be rejected or limited | Better tier unavailable | J0 verification with approval (preferably a test account); fallback needs your decision |
| No server revision/CAS on whole-blob writes | Concurrent writers can lose an edit | Per-field revisions + pull–merge–push; residual window documented |
| Regular Nuvio replaces the `"tv"` document | Extensions lost | Never store Better data in `"tv"` |
| Unknown size/rate limits | Rejected writes | Size cap, existing backoff, one push per cycle |
| Regular Nuvio pulls profile settings only on startup | Better → regular changes appear after regular Nuvio restarts | Document; nothing to change on our side |
| Addon membership is not written by Better Nuvio | An addon installed on Better device A is absent on B | Keep as is, or approve a later stage using official `sync_push_addons` semantics |
| Official-counterpart values need conversion | Wrong value applied | Per-key conversion tables and tests (J5), only verified keys |
| Credential leakage | Security | Credential RPC only; denylist + allowlist; tests scan documents |

## 5. Staged implementation plan

| Stage | Work | Exit criteria |
|---|---|---|
| J0 | Verify `p_platform:"better_nuvio"` read/write on a **test account** (with your approval) | Pull of an empty namespace returns `[]`; a write round-trips; `"tv"` unchanged |
| J1 | Finalise the inventory: verify every candidate row (portable vs device), consumers and safe value ranges | Reviewed `stage-j-inventory.json`, no unverified row in the allowlist |
| J2 | Generic typed sync engine (bool/int/enum/string) generalising the Stage I journal: revisions, device id, merge, ack, offline | Mock-server tests: restore, both directions, conflicts, loops, offline, profile/account isolation |
| J3 | Better tier transport (pull/push under `better_nuvio`) behind a setting, default off until J7 | Two-process mock tests green; no change to `"tv"` traffic |
| J4 | Portable Better-only settings (Layout, Appearance, Content & Discovery first) | Allowlist migrated; device rows untouched |
| J5 | Native Home presentation (shape, size, Friends Watching/Last Session placement, TV-only hides) keyed by official row keys | Stage G order/visibility unchanged; per-profile tests |
| J6 | Official-counterpart expansion on `"tv"` for keys with verified conversions | Each key has a conversion test against official encoders |
| J7 | Two-device verification on Android TV with a test account, then enable by default | Section 6 manual plan passes |

## 6. Two-device test plan

### 6.1 Automated (mocks only)

A mock account server holds blobs keyed by `(user, profile, platform)` and the
other RPCs; two Better Nuvio processes with separate data directories share it
(extending `tests/syncaddons.c` / `settings_stage_i_transport.c`).

1. Fresh install B signs in: B shows A's synced values before any local default is uploaded.
2. Edit on A → B after its next cycle, resume or player close; and B → A.
3. Same field edited on A and B before either syncs: both converge to the higher revision.
4. Different fields edited on A and B offline: both survive after reconnecting.
5. Profile switching on B while A edits profile 2: profile 1 never receives profile 2 values.
6. Inheriting profile: Home order from primary; Better presentation stays per profile.
7. Hidden catalogue rows (Stage G second-instance keys) stay per profile on both devices.
8. Restart mid-push, ambiguous timeout, server 5xx: no duplicate upload, no lost edit.
9. Account switch: no value crosses accounts; logout leaves the server untouched.
10. Regular Nuvio writes `"tv"` concurrently: Better document untouched; the 32 fields still converge.
11. Documents never contain keys, tokens, URLs or credential-store values (scanner test).
12. Device-local controls never leave the device.

### 6.2 Manual (two Android TV installs, **test account**)

1. Install the debug APK on TV A and TV B; sign in to the same test account on both; pick the same profile.
2. On A: change a 32-field setting (e.g. Resolve Playable Links), a Better-only layout setting (after J4) and a row shape (after J5). On B: resume the app, wait about 3 s, and compare. Check `[ajustes-conta]` lines.
3. Reverse direction from B to A.
4. Switch both to another profile; confirm each profile's own values and India-row visibility.
5. Put A offline, edit two settings, edit one different setting on B, reconnect A, and compare after one cycle.
6. Edit the same setting on both while offline, reconnect both, and confirm the predictable winner.
7. Restart both apps; values persist; no `[ajustes-conta] enviando` line without an edit.
8. Sign A out and into a second test account; confirm nothing carries over.
9. Open regular Nuvio on the same account; change a 32-field setting; confirm both Better devices follow and regular Nuvio's Home is unaffected by Better-only data.

## 7. Decisions needed before implementation

1. Approve J0: a read/write check of `p_platform:"better_nuvio"` on a test account (or name another account to use).
2. Fallback if the namespace is rejected: keep Better-only settings local, or propose backend work?
3. Should addon installs/removals made in Better Nuvio sync to the account (affects regular Nuvio)?
4. Should Better Nuvio ever write the shared Home order (affects regular Nuvio)?

## 8. Stage J0 — `better_nuvio` namespace investigation (2026-11-04)

Source inspection and documentation only. No live account or backend request was
made; the account session exists only on the TV.

### 8.1 Evidence

| Evidence | Source | Finding |
|---|---|---|
| Profile settings are stored per `(user, profile, p_platform)` | NuvioTV `ProfileSettingsSyncService` (`p_platform:"tv"`), Nuvio mobile `ProfileSettingsSync.kt` (`p_platform:"mobile"`, 0.6.6-beta, `~/Documents/Nuvio-`) | Both call the same `sync_pull/push_profile_settings_blob`; each reads back only its own value (`decodeList(...).firstOrNull()` of `{profile_id, settings_json, updated_at}`) |
| The server does not enforce one settings schema | Mobile pushes `MobileProfileSettingsBlob` (different features and fields from TV's typed `{version, features}`) | `settings_json` is an opaque JSON object per namespace |
| Namespaces are not tied to device kinds | `home_catalog_shared` used by TV, mobile and Better Nuvio | A non-device value already exists |
| New values were introduced client-side | Commit `5249614b` (mobile) and `ebd059189` (TV), both 2026-06-11 "catalog order to be cross-platform": only client files changed | No server change in either repository (the backend is a separate project, so a private migration cannot be excluded) |
| No client-side allow-list | All four local client trees | Only `tv`, `mobile`, `home_catalog_shared` constants; no validation |
| No recorded rejection | This repository's docs, tests, logs | No platform-related 4xx, constraint or enum error recorded |
| Isolated tests | `settings_stage_i` transport/official tests | Cover the request/response envelope of this RPC pair; they cannot prove server acceptance of a new value |

**Conclusion: very likely supported, not proven.** Whether the column is free text
or an enum/allow-list, and any size or rate limit, can only be confirmed against
the server.

### 8.2 Proposed verification (awaiting approval; not run)

Run from the Better Nuvio debug build (where the session already is; no token
leaves the TV). Preferred target: a **dedicated test account**. If you prefer
your own account, use a profile you nominate.

1. **Read probe (no mutation).** `sync_pull_profile_settings_blob(p_profile_id=P,
   p_platform="better_nuvio")`. Expected `200 []` if the value is accepted for
   reads; a 4xx naming an enum/constraint/function error means unsupported (stop).
   This alone does not prove writes.
2. **Baseline.** Pull `"tv"`, `"home_catalog_shared"` and (read-only)
   `"mobile"` for P; record SHA-256 of each `settings_json` and `updated_at`.
   Contents are hashed, never logged.
3. **Write probe.** `sync_push_profile_settings_blob(P, "better_nuvio",
   {"schema":"better-nuvio-probe","version":0,"probe":"<random nonce>"},
   p_origin_client_id=<this install's client id>)` — about 90 bytes.
4. **Read back.** Pull `"better_nuvio"`: the nonce must round-trip unchanged;
   record `updated_at`.
5. **Isolation check.** Pull the three other namespaces again; hashes and
   `updated_at` must be identical to step 2.
6. **Second-device check (optional).** Pull `"better_nuvio"` from a second install
   signed in to the same account: the nonce is visible.
7. **Size check (optional).** Push a 64 KB document of the same shape; record
   acceptance. Re-push the small probe afterwards.

Risks:

* Creates one row for `(user, P, "better_nuvio")`. No known RPC deletes a single
  namespace; `sync_delete_profile_data` deletes the whole profile's data and must
  never be used for cleanup.
* Regular Nuvio clients never read this namespace (they request only their own
  values), so the row is inert. Unknown server triggers on settings writes (e.g.
  realtime notifications) would at most make a regular client re-read its own
  namespace.
* If the server rejects the write, nothing is stored.

Cleanup: overwrite the namespace with `{}` (push), or keep the probe document as
the J3 initial document. The other namespaces are verified unchanged in step 5.

Execution needs a small debug-only probe action (hidden diagnostics, logs status
codes, hashes and byte counts only). Building it is part of the approval request;
it is not implemented.

### 8.3 How Better Nuvio would use the namespace (if confirmed)

* One document per profile under `"better_nuvio"` (schema in 3.1), pulled and
  pushed in the existing sync cycle after the `"tv"` blob, with the same session,
  owner and profile guards, resume/player-close refresh and backoff.
* Pull–merge–push with per-field revisions; never upload defaults; only
  allowlisted portable fields; denylist for keys, tokens, URLs and credential
  stores enforced at serialisation; 64 KB cap; unknown sections preserved.
* Home: only Native presentation (shape, size, exclusive-row anchors, TV-only
  hides) keyed by official row keys; Stage G remains authoritative for shared
  order and visibility; inheriting profiles keep their own document.
* Never written: `"tv"` extensions, `"mobile"`, `home_catalog_shared` (unless
  separately approved).

### 8.4 Alternatives if the namespace is unsupported (not implemented)

| Option | Interference with regular Nuvio | Notes |
|---|---|---|
| Ask the Nuvio maintainers to allow `better_nuvio` | None | Cleanest; outside our control |
| Local export/import (file or QR) between Better installs | None | Manual; no backend |
| LAN device-to-device sync between Better installs | None | Both devices on the same network; no cloud restore after reinstall |
| Separate backend you own (e.g. your own Supabase project) | None | Needs your approval: new service, auth linkage and security review |
| Reuse `"tv"`, `"mobile"` or `home_catalog_shared` | **Yes** | Rejected: regular clients replace or share these documents |

### 8.5 Future-stage notes from your direction

* Better Nuvio settings across Better installs: the namespace above.
* Addon installations "where safe": the `addons` table already accepts Better
  Nuvio's per-row PATCH (toggles). A later stage could add installs/removals by
  read-merge-write (as the library does) or row inserts, never a partial list,
  and read-only for inheriting profiles (official `AddonManagementAccess.isReadOnly`).
  This changes regular Nuvio's addon list, so it remains a separate approval.
* Device-specific settings stay local; Stage G Home behaviour is unchanged.

### 8.6 J0 probe implementation (test account chosen; not yet run)

You chose a **dedicated test account**. The probe is implemented but has not run,
and no APK containing it has been built.

* `src/j0sonda.c/.h`: compiled only into Android **debug** builds
  (`NV_ANDROID` without `NDEBUG`; the release variant is `RelWithDebInfo` with
  `-DNDEBUG`). Every other build has empty stubs; `tests/j0sonda.sh` checks that
  a release object has no account calls.
* `src/deeplink.c/.h`, `src/app.c`: debug-only deep links, dispatched before the
  title-opening path (a probe link never opens a title or touches playback).
* Phases (STAGE-J 8.2): **read** issues an 8-hex token after a baseline of
  `tv`, `home_catalog_shared` and `mobile`; **write** needs that token, the same
  account and profile, and runs within 30 minutes; **clean** overwrites the
  probe with `{}` within 24 hours. A probe on a namespace the server refuses
  stops after the read.
* Output: one on-screen notice per phase plus `[j0]` log lines: HTTP status,
  byte counts, 64-bit FNV fingerprints and the token. Never document contents,
  owner or client ids, or credentials.
* Tests: `tests/j0sonda.sh` (mock server: free-text, enum, read-only and leaky
  servers; wrong token/account/profile; signed out; log privacy; release build
  free of probe code) and `tests/deeplink.sh` (release and debug variants;
  malformed links rejected; probe links never open titles). Both pass.

Procedure once a debug APK with the probe is approved, built and installed:

1. Create a test account in regular Nuvio and give it at least one profile. Close
   regular Nuvio on every device signed in to that account during the probe.
2. In Better Nuvio, sign in to the **test account** and select the test profile.
3. Read phase:
   `adb shell am start -a android.intent.action.VIEW -d "nuvio-legacy://debug/j0-probe" space.nuvio.nativelegacy.strictdebug`
   Expected notice: `leitura aceita (HTTP 200, vazio). … token XXXXXXXX`.
   `leitura recusada` means unsupported: stop (nothing was written).
4. Write phase (within 30 minutes, same profile):
   `adb shell am start -a android.intent.action.VIEW -d "nuvio-legacy://debug/j0-probe/write?token=XXXXXXXX" space.nuvio.nativelegacy.strictdebug`
   Expected: `PASSOU - escrita HTTP 200, leitura de volta OK, tv/home/mobile inalterados`.
5. Cleanup (optional, within 24 hours):
   `…/j0-probe/clean?token=XXXXXXXX` → `limpeza OK`.
6. Share the `[j0]` lines:
   `adb shell "run-as space.nuvio.nativelegacy.strictdebug cat files/dados/nuvio.log" | grep '\[j0\]'`

Removal: the probe is temporary. It will be deleted (or kept debug-only) when J3
adds the real transport.

### 8.7 J0 result — namespace verified live (2026-11-04)

Run by you with the J0 probe debug APK (`f12d763f…c562a5609`) on **BlueStacks**,
signed in to your **dedicated test account**:

| Check | Result |
|---|---|
| `better_nuvio` read (`sync_pull_profile_settings_blob`) | HTTP 200 |
| `better_nuvio` write (`sync_push_profile_settings_blob`) | HTTP 204 |
| Read-back of the probe document | Passed (nonce returned) |
| `tv`, `home_catalog_shared`, `mobile` fingerprints before/after | Unchanged |

**Conclusion: the account backend supports a separate per-profile `better_nuvio`
namespace on the existing RPCs, isolated from regular Nuvio's documents.** The
architecture in section 3 is viable without any backend change. Notes:

* Verified for one profile of one test account; per-profile separation follows
  from the `p_profile_id` key used by every namespace (checked again in J7 on two devices).
* Size and rate limits were not measured (optional step 7 not run); J2 keeps the
  64 KB cap until measured.
* Unless the cleanup step was run, the test account keeps the ~90-byte probe
  document under `better_nuvio`; it is inert and J3 will overwrite it.
* Run on an emulator, not the TCL; the server behaviour is device-independent.

## 9. Stage J1 — final settings inventory (2026-11-04)

Inventory: [stage-j-inventory.json](stage-j-inventory.json), regenerated by
`tools/settings-restoration/j1_finalise.py tools/settings-restoration/j1-data`
(official key resolution: `tools/settings-restoration/j1_official_keys.py`).
No sync code was written; Stage G and Stage I behaviour is unchanged
(`tests/settings_stage_i.py` still passes with 32 fields).

### 9.1 Method

1. Started from all 418 Native controls (413 Stage B + 5 Stage C/D).
2. Resolved each regular-Nuvio counterpart to its **stored preference key**,
   feature and type: (a) identical key strings in the official DataStores
   (59 controls, e.g. `mdblist_show_*`, `card_depth_*`, `selected_theme`);
   (b) the official ViewModel write or event handler down to the DataStore
   setter; (c) UI property names. Path (b)/(c) mappings were **reviewed by hand**:
   28 accepted, the rest rejected (wrong or doubtful key, key owned by another
   control, or a value inside the composite `stream_preferences` JSON).
3. Applied regular Nuvio's own export rules (`ProfileSettingsSyncService`:
   synced features, `localOnly*` exclusions, credential keys).
4. Classified Better Nuvio-exclusive rows by hand (portable vs device), with the
   reason recorded per row.

### 9.2 Policy

* **One owner per setting.** If regular Nuvio exports an equivalent, it syncs only
  through the official `"tv"` document (Stage I bridge, extended in J6), never also
  through `better_nuvio`.
* **Device-local wins.** Native "this TV only" scope and regular Nuvio's own
  device-local exclusions are respected; such rows are never uploaded.
* **Profile scope.** Every synced row is per profile; nothing is account-wide.
* **No secrets.** Credentials, API keys, tokens, URLs and endpoints are not
  settings rows; they stay in their stores or the provider-credential RPC.

### 9.3 Summary

| Tier | Controls | Meaning |
|---|---:|---|
| official: synced (Stage I) | 32 | Already bidirectional via "tv" |
| official tier: J6 candidate | 57 | Regular Nuvio exports it; sync through "tv" after a tested value conversion (never also in better_nuvio) |
| official tier: key unresolved | 26 | Counterpart exists; stored key composite/unresolved; local until J6 |
| better_nuvio tier: portable | 90 | Synced per profile through p_platform "better_nuvio" (J4) |
| device-local | 87 | Never uploaded |
| not a preference | 126 | Actions, read-only rows, sections |

**By category** (preferences only):

| Category | Synced | J6 | Unresolved | better_nuvio | Device |
|---|---:|---:|---:|---:|---:|
| Advanced | 1 | 0 | 0 | 0 | 11 |
| Appearance | 1 | 3 | 0 | 8 | 19 |
| Content & Discovery | 0 | 0 | 0 | 2 | 13 |
| Integrations | 12 | 2 | 9 | 4 | 0 |
| Layout | 14 | 15 | 7 | 38 | 5 |
| Notifications | 0 | 0 | 0 | 0 | 1 |
| Other | 0 | 16 | 0 | 2 | 1 |
| Playback | 2 | 18 | 10 | 34 | 33 |
| Switch Profile | 0 | 0 | 0 | 0 | 2 |
| Tracking | 2 | 3 | 0 | 2 | 2 |

#### Portable Better Nuvio-exclusive settings → `better_nuvio` (90)

* **Appearance** (8): Logo color; Glass outline; Glass opacity; Row titles; Row spacing; Accent-colour focus; Art tone; Stage background
* **Content & Discovery** (2): Nuvio catalog results in search; Show the source on search results
* **Integrations** (4): Release dates; Metacritic (users); Roger Ebert (critic); MDBList score
* **Layout** (38): Hero background; Hero with different art; Trailer on the hero; OK on the card; Backdrop dimming; Trailer aspect; Trailer source; Border on the focused card; TV Guide in the sidebar; Schedule in the sidebar; Profile & Stats in the sidebar; Hero transition; Addon posters; Addon hero background; Addon logo; Account folder artwork; Watched badge on posters; Hide logo during trailer; Space between rows; Space between titles; Menu position; Tab style; Hide watched movies; Show genre on hero; Show synopsis on hero; Show year on hero; Show runtime on hero; Synopsis size; Episode card size; Titles in Mural; Friends row style; Default collection view; Trailer audio; Dynamic layout background; Background color; Library row; Hero size; Show folder details on hero
* **Other** (2): Title page trailer sound; Clock position
* **Playback** (34): Colored badges; Seek bar thumbnails; Thumbnail strip; Thumbnail sync; When leaving the player; Ask what you thought at the credits; Second subtitle language; Second subtitle position; Size; Color; Background; Edge; Automatic pick prioritizes; Use the others if there are none; Regex match; Use the order; Minimum to enter Continue; Next episode countdown; Live statistics size; Live statistics side; Show add-on on opening; Opening screen backdrop; Prefer audio language; Subtitle font; Opacity; Position; Delay; Bold; Outline color; Opening screen style; Ratings when paused; Cast when paused; Format position in the player; Show the audio codec
* **Tracking** (2): Also in Continue watching; Next episode in Continue

#### Official tier: J6 candidates (57) — exported by regular Nuvio

* **debrid_settings** (3): Resolve with → `preferred_resolver_provider_id` (string); Prepare instant playback → `instant_playback_preparation_limit` (int); Preparation limit → `instant_playback_preparation_limit` (int)
* **layout_settings** (20): Discover location → `discover_location` (string); Addon name in catalog → `catalog_addon_name_enabled` (boolean); Hide unreleased → `hide_unreleased_content` (boolean); Overall ratings → `home_imdb_ratings_visibility` (string); Classic focus gradient → `classic_focus_gradient_enabled` (boolean); "Continue Watching" style → `continue_watching_card_style` (string); Sort order → `continue_watching_sort_mode` (string); Trailer button → `detail_page_trailer_button_enabled` (boolean); Expansion delay → `focused_poster_backdrop_expand_delay_seconds` (int); Depth effect → `card_depth_enabled` (boolean); Edge glow → `card_depth_edge_strength` (int); Sheen → `card_depth_sheen_strength` (int); Edge coverage → `card_depth_edge_coverage` (int); Depth on posters → `card_depth_posters_enabled` (boolean); Depth on "Continue" → `card_depth_continue_watching_enabled` (boolean); Depth on episodes → `card_depth_episode_cards_enabled` (boolean); Depth on cast → `card_depth_cast_enabled` (boolean); Depth on trailers → `card_depth_trailers_enabled` (boolean); Item width → `poster_card_width_dp` (int); Corner radius → `poster_card_corner_radius_dp` (int)
* **mdblist_settings** (10): MDBList → `mdblist_enabled` (boolean); Trakt ratings → `mdblist_show_trakt` (boolean); IMDb ratings → `mdblist_show_imdb` (boolean); TMDB ratings → `mdblist_show_tmdb` (boolean); Letterboxd ratings → `mdblist_show_letterboxd` (boolean); Rotten Tomatoes ratings → `mdblist_show_tomatoes` (boolean); Audience score → `mdblist_show_audience` (boolean); Metacritic ratings → `mdblist_show_metacritic` (boolean); MyAnimeList ratings → `mdblist_show_mal` (boolean); Show ratings on hero → `mdblist_show_on_hero` (boolean)
* **player_settings** (13): Audio language → `preferred_audio_language` (string); Watched percentage → `next_episode_threshold_percent_v2` (float); Forced subtitles when the audio is in your language → `subtitle_use_forced_subtitles` (boolean); Content rating in the player → `parental_guide_enabled` (boolean); Auto-play sources → `stream_auto_play_source` (string); Video opening screen → `loading_overlay_enabled` (boolean); Skip the intro automatically → `skip_intro_enabled` (boolean); Play the next episode automatically → `stream_auto_play_next_episode_enabled` (boolean); Second audio language → `secondary_preferred_audio_language` (string); Prefer binge group → `stream_auto_play_prefer_bingegroup_next_episode` (boolean); Reuse binge group across sessions → `stream_auto_play_reuse_binge_group` (boolean); Reuse last link → `stream_reuse_last_link_enabled` (boolean); Last link cache duration → `stream_reuse_last_link_cache_hours` (int)
* **stream_badge_settings** (1): Format in the player → `show_badges_in_player` (boolean)
* **theme_settings** (3): Accent color → `selected_theme` (string); Background → `amoled_mode` (boolean); Settings layout → `settings_ui_style` (string)
* **tmdb_settings** (1): Metadata language → `tmdb_language` (string)
* **trailer_settings** (2): Trailer quality → `trailer_allow_4k` (boolean); Hero trailer delay → `trailer_delay_seconds` (int)
* **trakt_settings** (4): Source for "Continue Watching" → `watch_progress_source` (string); Library source → `library_source_mode` (string); Trakt comments when paused → `show_meta_comments` (boolean); Days in Trakt Continue Watching (0 = all) → `continue_watching_days_cap` (int)

#### Official counterpart, key unresolved (26) — local until J6

Subtitle language; Modern sidebar blur; Auto-play trailer; Explore in the sidebar; Trailer on the focused poster; Home layout; IMDb; Rotten Tomatoes (critics); Popcornmeter (audience); Metacritic (critics); Trakt; TMDB; Letterboxd; MyAnimeList; Trailer sound; Maximum size; Minimum size; Prepare the source when a title opens; Source order; Show in player; Skip the recap automatically; Skip the credits automatically; Subtitle outline color; Preload video; Enriched context menu; Episode rating

#### Device-local (87) — never uploaded

* **Native TV-only scope (hardware, display, input, local service or install)** (57): Automatic source; Another source on failure; Source text; Image quality; App language; Animations; Interface resolution; TV guide source; Send logs automatically; Memory for images; Items per row; Visual effects; P2P server (experimental); Custom posters; Always use the Nuvio catalog; Use the primary profile's add-ons; Main resolution; Xtream format; Wait for a channel to open; Live TV player mode; Live TV TS proxy; Performance meter; App icon; Sync by audio; Seek cache on disk; Trailer and player zoom (experimental); Personal servers (experimental); Show advanced options; App logo; App opening; Search Cinemeta; Screensaver; Player interface brightness; Keep the video ready on exit; Screensaver style; Showcase titles; Clock format; Default aspect ratio; DTS converted to; Dolby Vision in MKV (experimental); P2P space limit; P2P cache location; Automatic subtitle sync; Interface brightness in HDR; Buffer size; Interface mode; Mic button in search; Keyboard; Quick resume on return; Time to keep video ready; Player; Anime player; Glass quality; Audio offset (ms); HDR UI (test); Tunneled Playback; Dolby Vision P7 to P8.1 (experimental)
* **hardware/display/GPU/audio capability** (14): Maximum quality; Dolby Vision; Dolby Atmos; Glass UI; Interface size; Frosted glass; Settings size; HDR and Dolby Vision; Text and component scale; Scale interface; Guide memory; Upscaling algorithm; Sharpness; Smooth gradients
* **consent or diagnostics (given per device)** (4): Receive polls; Send history to the Nuvio account; Social features; Log on the local network
* **Native TV-only scope** (3): Choose the source on Play; Interface font; Wait for add-ons
* **network/connection behaviour** (3): Play while checking the source; Warm up connections when a title opens; Check several sources at once
* **depends on device-only IPTV/guide sources** (3): Suggest channel categories; TV guide sources; Sort order
* **profile picker (shown before a profile is selected)** (2): Profile picker background; Continue on profile picker
* **download-capable safety opt-in (Stage D)** (1): Add cached TorBox torrents

### 9.4 Better Nuvio-exclusive state outside the settings table

| State | Store | Tier | Notes |
|---|---|---|---|
| Row shape, size, item order per row | `fileirasui-p<N>.txt` | `better_nuvio` (J5) | Keyed by official row key (Stage G `chaveOficial`), never URL fingerprints |
| Friends Watching / Last Session placement and visibility | `fileirasui-p<N>.txt` | `better_nuvio` (J5) | Anchored to the official row it follows |
| TV-only hides of shared rows | `fileirasui-p<N>.txt` | `better_nuvio` (J5) | Synced order/visibility from regular Nuvio stays authoritative |
| Hero source row | `fileirasui-p<N>.txt` | `better_nuvio` (J5) | Official row key |
| Row-count quota, emergency caps, registry/size budgets | `fileirasui-p<N>.txt` | Device-local | Hardware-dependent |
| Source fallback order/limits (`fontefalha`) | own file | Unresolved (J4/J6) | Contains addon instance hashes only; check overlap with official `stream_preferences` |
| Imported badge packages, poster provider URLs, IPTV/personal servers, P2P, debrid keys | own stores | Device-local / credential | May contain credentials |

### 9.5 Decisions before J2

1. **Unresolved rows (26)** — keep local until J6 (recommended), or carry them
   provisionally in `better_nuvio` and migrate to `"tv"` in J6.
2. **Consent rows** (Social, polls, account history, network log) — device-local
   as classified, or per profile?
3. **Six TV-only rows that could be portable** (12-hour clock, automatic source,
   source text, Nuvio-catalogue-only, Cinemeta search, screensaver style) — keep
   device-local for now (recommended) and revisit after J4?


## 10. Stage J2 — isolated generic engine (5 November 2026)

Implemented `src/bnsettings.c` / `src/bnsettings.h`, without changing any existing
application source, Stage G/I code, settings binding or inventory classification.
The 26 unresolved settings remain local until J6. There is deliberately **no
production settings registry**, observer, transport call, account-write toggle or
Home adapter in this stage. Synthetic portable fields are injected by tests only.
J3 transport and J4–J7 work have not begun.

### Contract and behaviour

* Version 1 `better-nuvio-profile` documents carry typed bool/int/enum/string
  values and per-field logical revision/device stamps. Owner/profile are checked
  in the local request envelope and never inserted into the JSON body as new
  server fields. Device IDs are opaque caller-supplied identities; no hardware
  identifier is read. Wall-clock timestamps are not used for conflict ordering.
* A fresh engine contains local defaults with revision zero, not dirty values.
  Defaults never enter an outbound document. A successful validated pull is
  required before preparing an update; edits/failures invalidate that permission.
  Missing fields retain local values and never act as deletion/reset commands.
* Local edits increment the field revision. Higher revisions win; equal revisions
  break ties lexicographically by device identity. Equal revision/identity with
  different values is rejected as inconsistent data. Merging does not turn an
  imported value into a local edit. Cached higher-revision values and previously
  known missing values can repair a lower/incomplete server snapshot on a later
  fresh pull; no default is involved in that repair.
* Prepared uploads are snapshots. Acknowledgement must match the prepared scope,
  values and stamps, independent of C structure padding or field ordering. It
  clears only unchanged sent values. A later edit survives an older acknowledgement.
  Failed/ambiguous writes retain pending work; a fresh matching pull confirms an
  already accepted write without another upload. Repeated matching pulls produce
  no outgoing update.
* `bn_checkpoint` / `bn_restore` export and validate the complete offline engine
  state, including pending values, revisions and scope. A new engine can resume
  it without granting permission to upload a stale base. This is a **typed state
  interface**, not a new application file format: J3's persistence adapter must
  encode its members explicitly and save atomically. Never persist raw C ABI
  bytes or pointers. No existing Native preferences file is read/replaced here.
* Definitions explicitly approve portability, type and range; unknown edit keys,
  non-portable definitions and credential/device-related keys are rejected.
  Strings are bounded valid single-line UTF-8; URL-shaped strings are rejected.
  This defence-in-depth is not a substitute for J4 binding only reviewed J1
  settings: arbitrary secrets cannot be identified reliably from text content.
* Malformed typed documents, duplicate keys, wrong scopes, unsupported versions,
  wrong types/ranges, revision overflow and oversized documents fail closed before
  applying values. No partial mutation is performed on failed validation. The
  JSON encoder caps output at 64 KiB. There is a 128-field safety budget, enough
  for the 90 J1 portable settings; this is **not a Home-row limit**.

### Deliberately deferred boundaries

J2 accepts validated typed document objects and emits JSON bodies; it does not
parse raw HTTP/JSON envelopes, perform RPCs, bind Native settings, or write disk
journals. Those adapters belong to J3/J4. J3 must validate raw JSON and preserve
unknown raw sections before any replacement write. In this core, unknown fields
or future versions are rejected, leaving current state intact and disabling
preparation, rather than silently dropping data. Transparent merging of future
opaque sections is not claimed. Home presentation/anchors are J5, not implemented.

The whole-blob server has no CAS. Deterministic stamps and retained state repair
observed lower-revision snapshots, but **cannot guarantee recovery if all copies
of a winning concurrent write are lost**. A later observation can converge only
when a client or server still retains that field. No timestamp/CAS/server schema
was invented, and guaranteed conflict-free multi-device writes are not claimed.

### Validation

* `bash tests/bnsettings.sh` — PASS with `-Wall -Wextra -Werror`, AddressSanitizer
  and UndefinedBehaviourSanitizer. In-memory mock account documents test first
  login, all four types, A→account→B and reverse, profile/account isolation,
  offline checkpoint restore into a fresh engine, reconnect, retries/ambiguous
  acknowledgement, edits during upload, deterministic conflicts, stale snapshot
  repair, loop prevention, partial/invalid/future data, duplicate/unknown keys,
  secret/device gates, UTF-8, revision limits and 64 KiB rejection. The emitted
  JSON was also decoded with Python's standard JSON parser.
* `tests/home_rows_stage_g.sh`, `tests/settings_stage_i.sh`,
  `tests/settings_preservation.py`, `tests/settings_stage_a.py` and
  `tests/settings_stage_b.py` — PASS. Existing Stage G/I production sources and
  the J1 inventory remain byte-for-byte unchanged; all 418 classifications remain
  90 portable, 57 official candidates, 26 unresolved, 87 device-local, 32 Stage I
  and 126 non-settings. No real settings are enrolled by the tests.
* Logs and the pre-change file-hash snapshot are in ignored `build/stage-j2/`.

No live account reads/writes or provider operations, APK build/install, commits,
pushes, merges, master changes or J3–J7 implementation occurred during J2.


## 11. Stage J3 — authenticated transport and local checkpoints (5 November 2026)

Implemented `src/bnaccount.c` / `.h` and `src/bncodec.c` / `.h`. The adapter uses
existing Native account services; it does not change Stage G, Stage I or J2 source,
login, provider credentials, progress, regular Nuvio documents or settings bindings.

### Transport and permission boundary

* Worker-facing `bna_open`, `bna_cycle`, `bna_edit` and `bna_get` wrap the J2 engine.
  Every RPC hardcodes `p_platform: "better_nuvio"`, uses the captured selected
  `p_profile_id`, and calls the existing authenticated `nuvem_rpc_com` service.
  Pushes include the existing `p_origin_client_id` from `dados_cliente_id()`.
  Neither `tv`, `mobile` nor `home_catalog_shared` is read or written by this adapter.
* Authentication reuses `sessao_token_copiar` and a single
  `sessao_token_recusado`/401 retry. JWTs exist only in a transient buffer, wiped
  after use. No body, token or credentials are logged or stored in checkpoints.
  Transient failures use the existing `nuvem_falhou`/`nuvem_freio_ativo` backoff;
  successful transport resets it with `nuvem_ok`. There is no retry spin loop.
* Each object captures the account, profile and `nv_perfil_sessao` generation.
  Identity is rechecked around token acquisition, retries, response handling,
  checkpoint commits and acknowledgement. Late responses are discarded after
  switching account/profile, even if the numeric profile index is reused.
  A request already dispatched under the old identity cannot be recalled, but
  it cannot be acknowledged into a new context or redirected with a new token.
* **The Better-tier write permission defaults Off** for a new account/profile on
  each device. `bna_writes` is an explicit durable local permission API. Restart
  retains a previously chosen permission; it is never imported from the remote
  document or another device. Disabling prevents subsequent dispatches and leaves
  pending edits available for a later opt-in. Existing Stage I/progress sync is
  not globally disabled or changed by this new permission.
* This is a callable transport integration, **not app startup/scheduler activation
  or a new visible settings control**. There is no production registry, Native
  observer or automatic cycle invocation yet: J4 will provide reviewed bindings
  and a controller. Tests inject only three synthetic fields. The 90 portable
  settings are not enrolled, and all 26 unresolved settings remain local until J6.

### Persistence, restoration and conflicts

* Checkpoints use explicit versioned JSON in the existing private data directory:
  `bn-checkpoint-v1-<owner>-p<N>.json`. Owner identifiers are validated safe IDs,
  not tokens. The header contains the exact owner/profile/client identity; slots
  store typed values, revision/device stamps and known/pending flags. Raw C ABI
  structures and request/response bodies are never persisted.
* `dados_gravar` supplies the existing atomic persistence path. Edits/permission
  changes become visible in the object only after a successful checkpoint save.
  Incoming merges likewise commit after persistence. Disk failures preserve the
  prior state. A failed durable acknowledgement retains pending work for recovery;
  a later fresh pull confirms an accepted write without uploading it again.
* Initial local values have revision zero and remain unobserved defaults. A fresh
  valid pull is mandatory before every prepared upload. Even enabling permission
  before the first pull, saving, restarting and reconnecting restores existing
  remote settings without uploading defaults. Missing data never resets settings.
* Conflict resolution remains J2's deterministic per-field revision/device rule.
  Edits made during HTTP survive older acknowledgements. Separate process tests
  exercise disk recovery, a second device's updates and a higher-revision peer
  winning over an offline local change. Whole-document server races still have no
  CAS; recovery/convergence requires a retained copy of the winning field.
* One caller-owned object serves a captured session; cycles are serialized and
  concurrent duplicate cycles are refused. Edits and permission changes can occur
  during network I/O under the object's mutex. Closing an in-flight object is
  refused. Callers must coordinate handle lifetime and not create competing local
  checkpoint writers for the same session/profile.

### Strict codec and fail-closed behaviour

The codec validates complete bounded RPC arrays, a single settings row, optional
matching `profile_id`, schema/version, exact field types, bounded values and
revision/identity data. The JSON reader rejects duplicate keys (including escaped
aliases), trailing/truncated data, unsupported versions/sections and malformed
Unicode. String decoding is lossless, including empty strings, quotes, backslashes
and Unicode surrogate pairs; truncation and embedded NUL are rejected. Checkpoint
loads also verify the exact account/profile/client and allowlisted field set.
Bodies/checkpoints are bounded to 64 KiB, with bounded token count and nesting.

Unknown fields/sections and corrupt or incompatible checkpoints **stop the cycle
or opening** rather than silently dropping data, resetting preferences or pushing
an incomplete document. Files are retained for recovery. This does not yet provide
transparent future-schema merging; Home sections remain J5. Registry changes will
need an explicit J4 migration strategy rather than deleting an old checkpoint.
A leftover J0 `better-nuvio-probe` document is not production settings and will
also be refused; J3 does not automatically overwrite or clean it.

### Validation and results

`bash tests/bnaccount.sh` — PASS with `-Wall -Wextra -Werror`, ASan and UBSan.
It compiles the real adapter, codec, engine and JSON writer against an isolated,
file-backed fake account transport and atomic temporary data-store implementation.
No real account, HTTP service or provider is contacted. Coverage includes:

* authenticated original RPC parameters and `better_nuvio` namespace exclusivity;
* default-Off on both devices; opt-in before first pull/restart without default upload;
* anonymous refusal, 401 refresh/retry, transient backoff, failed pulls and retries;
* offline edits recovered after `exec` into a fresh process; second-device updates;
* higher-revision conflicts, edits during push, permission changes during pull;
* ambiguous timeout after server commit without duplicate upload;
* failed local persistence and restart after failed durable acknowledgement;
* account/profile changes during HTTP; private checkpoint isolation;
* strict response/checkpoint validation, escaped-key duplicates, Unicode, empty
  strings, future/corrupt documents, and size bounds;
* credential-free persistence/payloads and byte-identical regular-document sentinels.

Also PASS: `tests/bnsettings.sh`, `tests/home_rows_stage_g.sh`,
`tests/settings_stage_i.sh` (including the existing 32 shared fields),
`tests/settings_preservation.py`, `tests/settings_stage_a.py` and
`tests/settings_stage_b.py`. Logs are in ignored `build/stage-j3/`. A file-hash
snapshot proves every pre-existing implementation/inventory/uncommitted file was
preserved; only this document and STATUS.md were updated in addition to new J3 files.

No live backend request, account mutation, APK build/install, commit, push, merge,
publication or master change occurred. J4–J7 were not begun. Real-device/backend
transport validation remains deferred; passing mocks are not that validation.


## 12. Stage J4 — portable Native settings bindings (5 November 2026)

### Registry and boundaries

Exactly the 90 `better_nuvio tier: portable` controls in the unchanged J1 inventory
are enrolled, per account and profile, using J2/J3's existing `better_nuvio`
namespace and authenticated transport. See [stage-j4-bindings.json](stage-j4-bindings.json)
for every Native ID, unchanged storage key, category, wire type, tested range and
conversion. `src/ajustes_better_map.inc` is the explicit allowlist; regression tests
compare it in order against J1 and reject additions from any other tier.

The 32 Stage I controls retain their existing `tv` transport and ownership. The
57 official candidates, 26 unresolved controls, 87 device-local controls and 126
non-settings are not enrolled. No credentials, tokens, provider endpoints, consent
or hardware preferences enter these documents. J1 classification is unchanged;
J6 remains responsible for unresolved/official candidates. No setting IDs,
defaults, Native storage keys or user-visible controls are added or renumbered.

### Native observation, application and value conversion

The existing Native save path captures changes in one durable J3 batch, rather
than treating the entire settings file as a remote document. The existing sync
worker performs the account cycle; the SDL frame path takes a nonblocking snapshot
and applies validated values with an echo guard. No HTTP is performed on the UI
thread. Loading is gated until Native preferences are available, so startup
defaults and automatic profile snapshots do not become user edits. Explicit profile
copy/reset/preset actions and the inactive-profile editor journal their actual
changes locally; they do not send RPCs for an inactive profile.

Boolean On/Off indices become actual booleans; enum indices retain existing
values; numeric controls retain every integer within the Native range. UI steps
are increments, not validation restrictions (including secondary-subtitle delay
137 ms). Language and trailer-audio selections use canonical codes, not menu
positions. Secondary-font selection uses a bundled index or existing content ID,
never a local path, display filename or font bytes. Validators reject invalid
codes and noncanonical IDs. A missing imported font retains its ID and uses the
bundled fallback; font transfer/import remains outside J4. Android-only trailer
source choices are retained in the model but deferred on unsupported platforms,
without reporting the fallback as a new user edit.

Remote projection uses existing Native persistence and applicable subtitle,
font and Continue Watching refresh hooks. Focus, scroll and Back state are not
rebuilt. Stage G's catalogue configuration, hybrid merge, Native-exclusive rows
and 30+ row support are untouched; no Home sections are enrolled (J5).

### Restoration, isolation and durability

J3's write permission remains persistent and default **Off**. No visible opt-in
control or automatic permission grant was added. Restores/read cycles can run
without uploading, and opting in still requires a successful pull before push.
Missing remote fields do not reset local values. Batched edits retain field
revisions and J2 conflict handling; projection does not re-journal remote changes.
A registry-expansion migration preserves existing known/pending records and seeds
new reviewed fields as unobserved, not dirty. Removed/unknown/type-incompatible
records continue to fail closed instead of silently being discarded.

Local account/profile checkpoints preserve dirty edits and the last received
values. Additional local-only applied-value anchors distinguish a genuine unsaved
Native change from a remote checkpoint persisted before UI application. Startup
captures the loaded Native vector for recovery even if both private-checkpoint
and profile-fallback writes failed after the global Native save succeeded.
Existing fallback files are retained. Ownership markers prevent another account's
same-numbered Native profile file or global secondary-font selection being
uploaded as this account's changes. Returning to an account restores even its
unobserved private baseline without turning it into a default upload.

In-flight account objects remain referenced until their worker cycle completes;
profile/account changes retire them without blocking the UI. Captured session
checks reject late responses. Existing J3 backoff, retry and ambiguous-write
handling remain in use. The server has no compare-and-swap: J2/J3's documented
concurrent-write limitations still apply; J4 does not invent a new protocol.

### Validation and results

All required suites PASS with isolated account/provider mocks:

* `tests/settings_stage_j4.sh`: real Native settings/save/apply code, exactly 90
  bindings, every dropdown value and every in-range integer, invalid conversions,
  default-Off, first-login restore, local batched uploads, remote application,
  missing font fallback, no echo/Stage I duplicate writes, disk-write failures,
  fresh-process offline recovery, inactive-profile edits, account/profile switches,
  unobserved account-local baseline restoration and an in-flight late response.
* `tests/bnsettings.sh` and `tests/bnaccount.sh`: engine/transport, including ASan
  and UBSan, restart/conflict/permission/malformed-data tests and registry expansion.
* `tests/settings_stage_a.sh`, `settings_stage_b.sh`, `settings_stage_c.sh`,
  `settings_stage_d.sh`, `settings_stage_i.sh`, `home_rows_stage_g.sh`,
  `home_rows_profiles.sh`, `home_rows_official.sh`, `syncaddons.sh`, `torbox_cached_add.sh`, `stream_cache_filter.sh`, `cw_parity.sh`.
* `python3 tests/settings_preservation.py` and Stage A/B/J4 static checks: existing
  bindings/defaults/platform restrictions and the staged wiring overlays preserved.

Evidence is in ignored `build/stage-j4/`, including test manifests and the initial
file-hash snapshot. The optional sanitizer build of the full Native J4 harness
stalled before its binding checks alongside macOS LaunchServices sandbox errors;
those test processes were stopped. It is **not** claimed as a sanitizer pass.
The normal full Native harness passes; the isolated engine and transport sanitizer
suites pass. Existing `ATOMIC_VAR_INIT` deprecation warnings remain unrelated.

No live backend/provider request, account mutation, APK build/install, commit,
push, merge or master change occurred. Real-device/backend validation is deferred.
J5–J7 were not implemented. Existing uncommitted work was retained.


## 13. Stage J5 — portable Home presentation (5 November 2026)

### Approved scope and storage

Implements the Home state explicitly approved in J1 section 9.4, outside the 418
control inventory. The inventory itself and the 90 J4 bindings are unchanged.
The existing account/profile `better_nuvio` document, transport, local write
permission and worker are reused. No `home_catalog_shared`, `tv`, addon or account
schema change is made. Live writes still default **Off**; no visible permission
control or automatic opt-in was added. J6 and J7 remain unimplemented.

[stage-j5-bindings.json](stage-j5-bindings.json) records the closed field families:

| Field | Value and actual consumer |
|---|---|
| `home_<digest>_s` | Native `FilTipo`; Home shape, row summary and preview getters |
| `home_<digest>_z` | Native `FilTam`; card size and effective layout scale |
| `home_<digest>_o` | Native `FilOrdem`; collection-folder item ordering |
| `home_<digest>_v` | Boolean personal hide; effective Home/editor visibility |
| `home_<digest>_a` | Preceding row digest or `start`; Friends Watching / Last Session placement |
| `homeHeroSource` | `auto`, `random` or a row digest; actual Home hero-source getter |

These are **separate revisioned properties**, not a replacement row list or a
combined row snapshot. Changing shape does not publish default size, visibility
or item order. Fixed app rows keep their existing shape controls (including J4's
Friends row style); item-order overrides are consumed only by collection rows.
No quotas, emergency caps, registry budgets, hardware preferences, endpoints,
credentials, tokens, consent or unrelated controls are enrolled.

### Identity, duplicates and availability

`fileiras_home.inc` derives portable IDs from Stage G's exact official row key,
including content type, catalogue ID and the complete parameter suffix. App and
collection keys use a separate domain. Catalogue labels never establish identity.
A compact deterministic digest is transmitted, not the local key or a URL hash.

Distinct configured instances of the same manifest ID remain distinct. Their
rank is calculated privately from the complete configured account instance list,
sorted by the existing endpoint strings; the strings and their fingerprints are
**neither hashed into the portable ID nor transmitted**. Rank is independent of
account-list ordering and of which catalogues have loaded on this device. The
duplicate-family size is also included: a partial one-instance family cannot
masquerade as the first instance of a two-instance family. Existing URL-keyed
Native preferences remain unchanged. No new endpoint/configuration sync is added.

Unknown/unavailable row IDs are retained in the private checkpoint and in later
outbound documents. Receiving them never calls the catalogue registrar or inserts
rows. Presentation is projected only onto rows the device already knows. When a
hero source is unavailable it renders Automatic while retaining its selection;
missing placement targets keep the current relative position. Overrides take
effect when the corresponding available row returns.

### Stage G, visibility and navigation

Stage G remains authoritative for original shared row order and enabled states.
J5 adds a personal hide mask; clearing that mask cannot enable an officially
disabled source, restore a missing addon, or enable local social consent. Existing
stale Native hides of explicitly listed shared rows are not reintroduced during
baseline import. Friends Watching / Last Session are repositioned after the
shared-row merge, anchored to available stable row IDs. Continue Watching stays
first. Anchor cycles cannot duplicate/remove rows. Missing anchors fall back to
the existing Stage G/Native placement rather than moving unrelated rows.

Remote application changes a runtime overlay and bumps `fil_revisao`; existing
Home rebuilding/focus restoration handles it. It does not reorder the Native row
registry, change the catalogue list, reset focus/scroll variables, or rewrite the
Native `fileirasui-p<N>.txt` fallback. Existing type/size/visibility summaries and
preview scale read the same effective values used by Home. User edits continue
to save through the existing Native paths, then journal the specific portable
property. Row moves capture the two exclusive anchors; ordinary shared row order
is not uploaded. Refreshes retain the overlay; profile/logout changes clear its
scope and reload the appropriate private checkpoint.

### First login, offline use and conflicts

The private account/profile checkpoint also stores unobserved local Home baseline
values at revision zero, with neither known nor dirty set. They provide offline
recovery but **never upload**. A valid remote revision wins over these baselines.
Existing remote fields are not reset when missing from a partial response, and
unavailable catalogue preferences survive subsequent uploads from a smaller device.

Local ownership markers distinguish the legacy profile file's owner. First use
can preserve that owner's legacy presentation; a different account gets a private
portable baseline rather than inheriting another account's presentation. Returning
to an account restores its checkpoint, including unobserved local preferences.
Original Native files are retained. Profile inheritance of the original Home list
remains Stage G behaviour; Better presentation belongs to the selected profile.

Actual changes use the same durable J3 checkpoint and per-property J2 revision /
device conflict rules as J4. Offline edits survive restart and are merged after a
fresh pull. Projection never invokes the edit observer; repeated cycles do not
produce uploads. A failed local edit commit remains pending in the runtime overlay
for retry; total storage failure is not claimed to be durable across process death.
Known server CAS/whole-document concurrency limitations from J2/J3 remain unchanged.

The adapter uses nonblocking worker snapshots and baseline seeding. Baselines are
revisited on Home revisions, not copied every frame; HTTP remains on the existing
worker. Sparse Home records have a closed validator and are permitted only by the
explicit J5 registry marker. Other unknown fields/sections still fail closed.
The registry migration preserves J4 checkpoints and all dynamic Home records,
including when the inactive-profile Native settings editor rewrites a checkpoint.

A fresh-install test found the J4 loading gate was not released on the valid
no-settings-file branch. That branch now calls `ajustes_better_boot()` after all
existing default initialisation/side effects. A separate preservation overlay
records this one additive change; no defaults or original statements were removed.

### Bounds and remaining limitations

* The field budget is now 4,096, document/checkpoint size 1 MiB, and strict JSON
  parser budget 98,304 tokens. This supersedes the historical J2/J3 128-field /
  64 KiB budgets. It covers 90 J4 controls plus all five properties for Stage G's
  structural 768-row registry. These are explicit memory/security bounds, not a
  new low enabled-row cap. Exceeding them fails closed without truncating rows,
  deleting Native preferences or uploading a partial replacement document.
* On this ARM64 Mac `sizeof(BnEngine)` is 3,965,240 bytes, a document 1,376,384
  bytes and a checkpoint 1,409,192 bytes. Engine/codec work buffers are on the heap;
  worker/context overlap and strict JSON parsing add temporary allocations.
  Android TV memory/performance and real backend payload-size acceptance remain
  unverified. Mock tests cover 40+ configured rows; Stage G's 10/15/20/30-row,
  bounded-loading and navigation regressions still pass. Device-local budgets
  and emergency protections remain intact.
* Safe catalogue matching waits for complete account instance identities. A failed
  manifest/incomplete identity cannot justify guessing which duplicate is meant;
  local presentation remains available while matching is deferred. Changes made
  before a catalogue has a safe portable identity remain in the Native fallback;
  no guessed outbound binding is created. Duplicate
  identity follows rank in the complete configured family. Replacing configurations
  that changes that correspondence is not a proven per-configuration migration;
  no stable public server instance ID exists in the audited Native account list.
  Such changes need device-level validation; original preferences are retained.
* Missing font/art/badge resources are not transferred; that remains J7. J5 adds
  no official J6 candidates. Older J4-only clients reject unfamiliar Home fields
  rather than overwriting them; use J5-capable clients for this namespace.
* Real TV/backend and LG webOS/Samsung Tizen device validation were not performed.
  This stage does not claim those validations or an Android APK build.

### Validation

PASS with isolated file-backed account mocks and mocked writes:

* `settings_stage_j5.sh` (ASan/UBSan): two devices with different available addons/catalogues,
  distinct repeated addon instances, content types/parameters, all supported shape,
  size and collection-order values, personal hides/shows, both exclusive anchors,
  unavailable hero fallback, explicit defaults, offline restart/reconnect,
  conflicts, smaller-device uploads retaining unknown rows, account/profile
  switching, owner-local baselines, duplicate-family reorder/partial-family safety,
  40+ rows, invalid values, no list replacement and unchanged Native file/focus index.
* `settings_stage_j5_native.sh`: real Native bootstrap/observer, default-Off,
  explicit opt-in, existing worker, peer document restore through SDL projection,
  no echo, and inactive-profile editing preserving Home fields.
* J2/J3 engine/transport sanitizer suites; J4's complete 90-binding conversion and
  Native lifecycle suite; Stage I's existing 32-field tests; Stage G original,
  profile and hybrid-row tests; settings A–D; TorBox cached addition/autoplay;
  stream filtering; Continue Watching; syncaddons; Android row-limit and remote
  navigation tests; collection sync and Native row tests; preservation and static
  English/routing/allowlist guards.

One extra collection harness initially failed to link because J5 introduced a
new `dados_ler` dependency. The Home ownership reader now uses the row module's
existing bounded local-read boundary, so the fixture remains unchanged and passes.
All final results/logs are recorded in ignored `build/stage-j5/`.

No live backend/provider contact, account mutation, APK build/install, commit,
push, merge, publication, master change or J6/J7 implementation occurred.
Existing uncommitted work was preserved.

## 14. Stage J6 — official-tier settings (5 November 2026)

Started by Codex (bridge `src/ajustes_official.inc`, map `src/ajustes_official_map.inc`,
hooks in `src/ajustes_shared.inc`, test `tests/settings_stage_j6.*`) and completed
here. Audit record: [stage-j6-audit.json](stage-j6-audit.json) (one decision with
evidence for each of the 57 J1 candidates and 26 unresolved rows).

### Design (unchanged from Codex)

J6 fields travel only in regular Nuvio's `p_platform:"tv"` document, merged into
a fresh pull next to Stage I's 32 fields, with typed `{type,value}` entries and
explicit lossless codecs: boolean (Native index 0 = On), integer (Native range
inside the official range), choice (1:1 enum names), hour presets, and the
compound focused-poster trailer (enabled + playback target). Per account/profile
journals (`settings-official-j6-<owner>-p<N>.txt`) hold base/pending state. Absent,
unknown or out-of-range remote values never reset a preference; Native-only values
(e.g. "From the art" theme, "Automatic" progress source) are held local. **Account
writes for J6 fields are Off by default** (`ajustes_oficiais_permitir_escritas`),
reset on every context/restart, never inherited, and no UI grants them.

### Problems found in the handed-over state and fixed

1. **J6 test aborted** (`build/stage-j6/j6.log`, `!ajusteAparelho(m->op)`): the
   bridge gated values with the legacy #205 device-backup predicate. That predicate
   marks every profile-local option as device-only, contradicting J1/J4 (J4 binds
   81 such controls). It silently disabled 9 of the 41 mappings (Content rating,
   Auto-play next episode, Reuse last link, Last link cache duration, Continue
   Watching source, Days in Trakt Continue Watching, Show subtitles, Resolve with,
   Preparation limit). The map is now the reviewed allowlist; J1 device-local rows
   are excluded by the audit check instead.
2. **Second owner removed:** `hide_unreleased_content` (Hide unreleased) is already
   applied by Stage G from `home_catalog_shared` (also written by the mobile client).
   Mapping it in `"tv"` too could flip the value; it was removed from J6.
3. **Trailer target default:** an enabled focused-poster trailer without a stored
   `focused_poster_backdrop_trailer_playback_target` was rejected; it now reads as
   `HERO_MEDIA` (Full screen), the official default. Unknown targets stay local.
4. **Missing audit:** the map cited a `stage-j6-audit.json` that did not exist; it is
   now written, and `tests/settings_stage_j6.sh` checks map = audit, no official key
   mapped twice, and no device-local, J4 or Stage I owner mapped.
5. **Audio-language regression guard:** J6 maps none of audio, secondary audio,
   subtitle, secondary subtitle or TMDB language (Stage I owns them as read-only
   account imports). A new test case applies them plus `hide_unreleased_content` in
   a TV document and asserts the Native values and J6 pending state are untouched.

### Decisions

Of the 57 J1 candidates: **38 mapped, 13 rejected, 6 kept local**. Of the 26
unresolved rows: **2 mapped** (Show subtitles, Trailer on the focused poster), **13
rejected, 10 kept local, 1 device-local**. J1 correction: Trailer quality
(`trailer_allow_4k`) is device-local in regular Nuvio and was never a candidate.

**Mapped (40)** — official `"tv"` owner, bidirectional, writes Off by default:

* **debrid_settings** (2): Resolve with → `preferred_resolver_provider_id`; Preparation limit → `instant_playback_preparation_limit`
* **layout_settings** (20): Discover location → `discover_location`; Addon name in catalog → `catalog_addon_name_enabled`; Overall ratings → `home_imdb_ratings_visibility`; Classic focus gradient → `classic_focus_gradient_enabled`; "Continue Watching" style → `continue_watching_card_style`; Sort order → `continue_watching_sort_mode`; Trailer button → `detail_page_trailer_button_enabled`; Expansion delay → `focused_poster_backdrop_expand_delay_seconds`; Depth effect → `card_depth_enabled`; Edge glow → `card_depth_edge_strength`; Sheen → `card_depth_sheen_strength`; Edge coverage → `card_depth_edge_coverage`; Depth on posters → `card_depth_posters_enabled`; Depth on "Continue" → `card_depth_continue_watching_enabled`; Depth on episodes → `card_depth_episode_cards_enabled`; Depth on cast → `card_depth_cast_enabled`; Depth on trailers → `card_depth_trailers_enabled`; Item width → `poster_card_width_dp`; Corner radius → `poster_card_corner_radius_dp`; Trailer on the focused poster → `focused_poster_backdrop_trailer_enabled`
* **mdblist_settings** (9): MDBList → `mdblist_enabled`; Trakt ratings → `mdblist_show_trakt`; IMDb ratings → `mdblist_show_imdb`; TMDB ratings → `mdblist_show_tmdb`; Letterboxd ratings → `mdblist_show_letterboxd`; Rotten Tomatoes ratings → `mdblist_show_tomatoes`; Audience score → `mdblist_show_audience`; Metacritic ratings → `mdblist_show_metacritic`; MyAnimeList ratings → `mdblist_show_mal`
* **player_settings** (6): Content rating in the player → `parental_guide_enabled`; Auto-play sources → `stream_auto_play_source`; Show in player → `subtitle_show_only_preferred_languages`; Play the next episode automatically → `stream_auto_play_next_episode_enabled`; Reuse last link → `stream_reuse_last_link_enabled`; Last link cache duration → `stream_reuse_last_link_cache_hours`
* **theme_settings** (1): Accent color → `selected_theme`
* **trakt_settings** (2): Source for "Continue Watching" → `watch_progress_source`; Days in Trakt Continue Watching (0 = all) → `continue_watching_days_cap`

**Rejected (26)** — wrong meaning, second owner or device-local in regular Nuvio:

| Setting | Reason |
|---|---|
| Subtitle language | owned by Stage I read-only account import (subtitle_preferred_language) |
| Audio language | owned by Stage I read-only account import (preferred_audio_language -> account slot); explicit Native choice wins; mapping caused the earlier audio-language regression |
| Hide unreleased | second owner: Stage G already applies hide_unreleased_content from home_catalog_shared (also written by the mobile client) |
| Trailer quality | J1 correction: trailer_allow_4k is device-local in regular Nuvio (localOnlyTrailerProfileSettingsKeys) |
| Metadata language | owned by Stage I read-only account import (tmdb_language); Native index list vs locale code |
| Explore in the sidebar | legacy search_discover_enabled (official local-only, migrated); discover_location is owned by AJ_DESCOBRIR |
| IMDb | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| Rotten Tomatoes (critics) | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| Popcornmeter (audience) | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| Metacritic (critics) | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| Trakt | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| TMDB | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| Letterboxd | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| MyAnimeList | second owner: the matching mdblist_show_* key is already owned by the AJ_MDB_* mapping |
| Background | different meaning: Native Art / Blurred art / Frost vs amoled_mode boolean |
| Prepare the source when a title opens | different behaviour from instant_playback_preparation_limit (owned by AJ_RESOLVE_LIMIT) |
| Settings layout | different values: Panel / List vs CLASSIC / ZEN / HORIZON |
| Video opening screen | different meaning: Native Cinema / Detailed style vs loading_overlay_enabled |
| Skip the intro automatically | different behaviour: Native auto-skips; skip_intro_enabled only fetches intervals for the Skip button |
| Show ratings on hero | different data: Native shows the IMDb rating on the hero; mdblist_show_on_hero shows MDBList ratings |
| Preload video | different behaviour: video preload vs preload_next_episode_sources |
| Second audio language | owned by Stage I read-only account import (secondary_preferred_audio_language) |
| Enriched context menu | different meaning: On/Off vs episode_options_overlay_style enum |
| Format in the player | different meaning: 5 badge styles vs show_badges_in_player boolean |
| Trakt comments when paused | different surface: comments while paused vs show_meta_comments on the title page |
| Prepare instant playback | derived from the preparation limit; the limit is the single wire owner |

**Kept local (16) and device-local (1)** — uncertain, composite or no official key:

| Setting | Reason |
|---|---|
| Modern sidebar blur | modern_sidebar_blur_enabled exists, but Native treats the blur as GPU/device (explicit device list) |
| Auto-play trailer | trailer_enabled governs all trailers in regular Nuvio; Native switch is title-page auto-play |
| Library source | Native "Nuvio list" not proven equal to official LOCAL; official MDBLIST unrepresentable |
| Home layout | Modern = MODERN only; Native Default not proven = CLASSIC; Dynamic Native-only; GRID unrepresentable |
| Trailer sound | focused_poster_backdrop_trailer_muted is the focused-poster trailer, inverted; Native switch is the hero trailer |
| Hero trailer delay | unit unverified: Native default 22 vs official trailer_delay_seconds default 7 |
| Watched percentage | official value is a half-step float that depends on a separate threshold-mode key; not lossless |
| Maximum size | value lives inside the composite debrid stream_preferences JSON |
| Minimum size | value lives inside the composite debrid stream_preferences JSON |
| Forced subtitles when the audio is in your language | official effective value also turns on via subtitle language "forced" (Stage I import); raw key alone is not the same behaviour |
| Source order | value lives inside the composite debrid stream_preferences JSON |
| Skip the recap automatically | no official recap auto-skip key (animeskip_enabled is a provider toggle) |
| Skip the credits automatically | no official credits auto-skip key |
| Subtitle outline color | 7 named Native colours vs arbitrary ARGB; palette equality unverified |
| Prefer binge group | Native preference also drives general source memory and is gated by source priority; official key is next-episode only |
| Reuse binge group across sessions | depends on AJ_FONTE_GRUPO, which stays local |
| Episode rating | 5 Native choices vs DetailImdbRatingsVisibility (4 values, episode projection) |

### Validation (5 November 2026)

* `tests/settings_stage_j6.sh` — PASS, plain and under UBSan
  (`-fsanitize=undefined -fno-sanitize-recover`): J1/J4 boundary check; audit = map;
  40 mappings with 938 lossless round-trips through the real codec; real sync worker
  with mocked RPCs: default-Off, private restart recovery, original TV payload only
  (no `better_nuvio`, `c_settings_v1`/`c_devices_v1`), unrelated fields preserved,
  no default upload or echo; conflicts; missing/malformed/future values; Native-only
  theme held; acknowledgement during edits; offline/restart/reconnect; profile and
  account isolation; stale responses; preparation limit 0–5; language and Stage G
  ownership regressions; trailer target default. Logs: `build/stage-j6/j6.log`,
  `build/stage-j6/j6-ubsan.log`.
* ASan + UBSan (`SANITIZE=1`) could not run: before `main`, Homebrew's `sdl2-compat`
  `libSDL2` initializer opens a modal macOS error dialog under the ASan runtime and
  waits forever (stack: `dllinit` → `error_dialog` → `NSAlert runModal`). This
  affects every full-app test linking SDL with ASan; it is an environment limit,
  not a J6 failure. UBSan alone runs.
* Regressions (`build/stage-j6/regress/`), all PASS: `settings_stage_i` (32 fields,
  official-payload and transport tests), `settings_preservation.py` (all **413
  original bindings**), `settings_stage_j4` (90 bindings), `settings_stage_j5`,
  `settings_stage_j5_native`, `bnaccount`, `bnsettings`, settings A–D,
  `torbox_cached_add`, `stream_cache_filter`, `cw_parity`, `home_rows_stage_g`,
  `home_rows_official`, `home_rows_profiles`, `syncaddons`, `fileiras`,
  `fileiras_android`, `fileiras_removidas`, `catordem`, `catordemcache`,
  `colfileiras_sync`, `homepos`, `j0sonda`, `deeplink` (27 suites).

No live backend contact, account mutation, APK build/install, commit, push,
merge or master change. J7 not started.

## 15. Stage J7 — real-device validation plan (5 November 2026)

**Plan only.** No code change, live write, APK, install or backend contact in this
step. Every phase below needs your explicit approval.

### 15.1 Current state (inspected)

| Path | Namespace | Default | Enable on device? | Device log |
|---|---|---|---|---|
| Stage I — 32 fields | `"tv"` | **writes ON** (`empurrarAjustes`, no gate, since Stage I) | n/a | `[ajustes-conta]` |
| J6 — 40 fields | `"tv"` (merged with Stage I) | Off; resets on every context/restart | **No caller** of `ajustes_oficiais_permitir_escritas` | none |
| J4 90 + J5 Home | `"better_nuvio"` | Off; persisted per profile in `bn-checkpoint-v1-<owner>-p<N>.json` | **No caller** of `ajustes_better_writes` | none |
| All pulls / first-login restore | all three | On (read-only) | — | partial |

Consequences:

1. Device A → B for J4/J5/J6 **cannot be tested today**: the app has no way to turn
   their writes on. Only reads/restoration would be observable.
2. Stage I's 32 fields already upload automatically whenever edited (existing,
   approved behaviour). This differs from "all writes disabled by default".
3. Better-tier and J6 permissions behave differently across restarts.
4. Without log lines, results on a TV cannot be confirmed.

### 15.2 Proposed preparation (J7a, local, needs approval)

* **A1 Debug-only write arming**, compiled out of release like the J0 probe
  (`NV_ANDROID` without `NDEBUG`), via deep links:
  `nuvio-legacy://debug/j7/arm` shows the account fingerprint (hash prefix, never the
  id) and a one-time token; `…/j7/arm?token=…` enrols that owner as the **test
  owner** and arms Better + J6 writes for the active profile; `…/j7/disarm` turns
  them off. Writes stay refused for any owner other than the enrolled test owner,
  and arming expires after 4 hours. Normal users cannot reach it.
* **A2 Privacy-safe diagnostics:** one line per cycle per tier —
  `[bn-sync]`/`[ajustes-oficial]`: HTTP status, fields received/applied/pending/sent
  (setting names only), document revision, armed state. No values, ids or tokens.
* **A3 Snapshot probe:** extend the J0 read phase to fingerprint `"tv"`,
  `"better_nuvio"`, `"home_catalog_shared"` and `"mobile"` for before/after checks,
  and count (never print) forbidden keys found in `better_nuvio` (credentials,
  device-local and consent keys from the J1 inventory).
* **A4 Combined two-device mock test** (`tests/settings_stage_j7.sh`): two app data
  directories sharing one mock server keyed by `(owner, profile, platform)`, all
  tiers active (Stage I 32, J6 40, J4 90, J5 Home, Stage G rows), covering every
  scenario in 15.4 before any live run; plain and UBSan.
* **Decision D1 — Stage I writes:** keep automatic (current, interoperable with
  regular Nuvio), or put them behind the same default-Off gate? Recommended: keep,
  and record it; gating would stop Better Nuvio settings reaching regular Nuvio.
* **Decision D2 — permission persistence:** align J6 with the Better tier
  (persist per profile) or keep both session-only? Recommended: both persist
  per owner/profile, but only for the enrolled test owner during J7.

### 15.3 Live set-up (J7b, after J7a approval and an approved APK build)

* Devices: **A = TCL Android TV**, **B = BlueStacks**, both with the same debug APK
  (`.strictdebug`), signed in to the **dedicated test account only**, profiles 1
  and 2 present. Regular Nuvio closed on all devices unless a step says otherwise.
* Before each scenario: snapshot probe on both devices (A3), record fingerprints.
* Abort immediately if any production account appears, if a snapshot shows
  `mobile` or `home_catalog_shared` changing, or a forbidden-key count is non-zero.

### 15.4 Scenarios and expected results

| # | Scenario | Steps | Expected |
|---|---|---|---|
| 1 | Writes off by default | Fresh install, sign in, change one J4, one J5, one J6 setting | No `better_nuvio` push; `"tv"` push only for Stage I fields; fingerprints of `better_nuvio` unchanged |
| 2 | Arm writes | A1 on A (profile 1) | Armed line; disarmed on B |
| 3 | A → B | On A change: 1 J4 setting, 1 J5 row shape/size, 1 J6 mapping (e.g. MDBList IMDb ratings), 1 Stage I field. On B: resume, wait ~3 s | B shows all four; logs list the four names as applied; B uploads nothing (no echo) |
| 4 | B → A | Arm B; change different settings on B; resume A | A applies them; no echo; A's own earlier values intact |
| 5 | First-login restore | `adb shell pm clear …strictdebug` on B; sign in to test account | Before any edit B restores A's J4/J5/J6/Stage I values; **no push** from B (defaults never uploaded) |
| 6 | Offline edits | Disconnect A's network; change 2 settings; restart A; reconnect | Edits persist across restart; uploaded once on reconnect; B receives them |
| 7 | Conflict, same field | Both offline; set the same J4 setting to different values; reconnect A, then B | Better tier: higher per-field revision wins, tie by device id, both converge; J6/Stage I: three-way rule (remote wins a conflicting base); no ping-pong |
| 8 | Conflict, different fields | Both offline; edit different fields; reconnect both | Both edits survive on both devices |
| 9 | Restarts | Force-stop and relaunch A and B | Values and armed state as decided in D2; no uploads without edits |
| 10 | Profile isolation | On A switch to profile 2, change settings; check profile 1 on B | Profile 1 unchanged; profile 2 receives only its own; Stage G Home visibility per profile unchanged |
| 11 | Account switching | Sign A out, into a second test account (or signed-out state), back in | No values cross accounts; arming refused for the non-enrolled owner |
| 12 | Local-only | On A change a device-local (e.g. interface resolution), a consent (Social), a credential (TorBox key) | B unchanged; forbidden-key count 0; credential travels only via its own RPC |
| 13 | Regular Nuvio interop (optional) | Change a J6 mapping on A; restart regular Nuvio on the test account | Regular Nuvio shows it; its `"tv"` push afterwards does not erase Better values |
| 14 | Disarm | `…/j7/disarm` on both | Further edits stay pending locally; nothing uploaded |

Coverage of the three groups: scenario 3/4/5 include at least one setting per
category of the 90 J4 bindings over the run, J5 shape, size, collection order,
personal hide and Friends Watching anchor, and J6 boolean, integer, choice, hours
and the compound trailer mapping.

### 15.5 Clean-up

Test account only. After the run: disarm both devices; optionally reset
`better_nuvio` for the test profiles to `{}` through an armed debug action; leave
`"tv"` as regular Nuvio wrote it. Production accounts are never armed, so they are
never written by J4/J5/J6.

### 15.6 Order of approval

1. Approve J7a (A1–A4 implementation and local tests; decisions D1, D2).
2. Approve a debug APK build containing A1–A3.
3. Approve live runs on the test account (scenarios 1–14).

## 16. Stage J7a — debug arming, diagnostics and two-device mock test (5 November 2026)

Approved scope: J7a only, with decisions **D1** (keep Stage I's automatic uploads)
and **D2** (remember J4/J5 and J6 upload permission per profile, test account only,
at most 4 hours). No live backend contact, APK, commit, push or merge.

### 16.1 Decisions as implemented

* **D1 — explicit exception.** Stage I's 32 official `"tv"` fields keep uploading
  automatically when edited (`empurrarAjustes`, unchanged). This is the one
  documented exception to the J4/J5/J6 default-Off policy: those fields are the ones
  regular Nuvio already shares, and gating them would stop Better Nuvio changes
  reaching regular Nuvio. The J7 test asserts that a default-Off device uploads
  exactly these and nothing from J4/J5/J6.
* **D2 — remembered, bounded permission.** Arming is stored in the app's private
  `j7-arme.txt`: the enrolled owner as a 64-bit FNV-1a hash (never the account id),
  and one `armed <profile> <expiry>` line per armed profile. The expiry is wall-clock
  time, so it is enforced after a restart; an expiry more than 4 h (+60 s slack) in
  the future (clock moved back, edited file) is refused, and a malformed file means
  "not armed" (fails closed).

### 16.2 Gate design (`src/j7arme.c`, `src/j7arme.h`)

* `j7arme_permite(owner, profile)` sits in front of **every** J4/J5/J6 upload:
  `bnaccount.c` `rpc()` (any write) and `bna_cycle` (push decision), and
  `ajustes_official.inc` `aj6Allowed()` (pending and merge). The older permission
  flags (`bna_writes_enabled`, `aj6Permission`) are still required too, so a stale
  stored flag can never upload without the J7 gate agreeing.
* **Build variants.** Android debug (`NV_ANDROID` without `NDEBUG`) compiles the full
  arming code from `src/j7arme.c` (the Android CMake globs `src/*.c`). **Android
  release** gets only inline stubs from `j7arme.h`: `j7arme_permite` is a constant 0
  and the deep-link parser has no `debug/j7` route. Verified on NDK r27 objects:
  release `j7arme.o` exports no symbols, release `bnaccount.o` has no J7 reference,
  release `deeplink.o` has no J7 route strings; in a linked debug `.so` the gate
  resolves to the real definition. Desktop/other hosts keep the previous
  explicit-permission behaviour (inline 1), so the J4–J6 tests still exercise
  uploads; the J7 test builds the Android debug logic with `NV_J7_TESTE`.
* **Fail-closed fallback.** Units that see the debug declaration but are linked
  without `j7arme.c` (unit tests that set `NV_ANDROID` for a single file) get a weak
  definition that refuses every upload; the real definition overrides it.
* **Protected activation (debug only).** `nuvio-legacy://debug/j7/arm` requires a
  signed-in account and shows **on screen only** the account fingerprint, profile and
  a random 8-hex code from `/dev/urandom`, valid 10 minutes, bound to that owner and
  profile. `…/arm?token=XXXXXXXX` arms only if the code, owner, profile and time all
  match and no other owner is enrolled. The code is never written to the log (a
  redacted line is logged instead); it lives only in the private state file until
  used or replaced.
* **Identity enforcement, every frame** (`j7arme_passo`): if the signed-in owner is
  not the enrolled one (another account, or signed out) all enrolment and arming are
  erased at once and uploads are switched off; returning to the test account needs a
  new code. Expired profiles are removed and logged. Both upload permissions follow
  the result (retried until both tiers accept it).
* `…/j7/disarm` erases enrolment and arming. `…/j7/snapshot` runs a read-only probe.

### 16.3 Diagnostics (names, counts, HTTP codes and fingerprints only)

* `[bn-sync] perfil N …` — pull HTTP code and number of fields received; `aplicados`
  with the setting names; local pending count when uploads are off; `enviando` with
  names; push confirmed / kept pending.
* `[ajustes-oficial] perfil N …` — received `x/40`, `aplicados` and `invalidos
  (mantidos)` with names, pending while off, `enviando` with names.
* `[j7] …` — arming, disarming, expiry, account change and gate state.
* Name lists are capped at 12 names (`+N`). No values, account ids, tokens,
  credentials or activation codes are logged; the J7 test greps the combined log of
  both devices to check this.
* **Snapshot** (`…/j7/snapshot`): reads `tv`, `better_nuvio`, `home_catalog_shared`
  and `mobile` for the active profile and prints HTTP code, size and a 64-bit
  fingerprint per namespace for before/after comparison. Leak checks: field names in
  `better_nuvio` outside the J4/J5 allowlist, case-insensitive sensitive patterns
  (token, password, senha, api_key, apikey, secret, bearer, http://, https://), and
  J4 keys found in `tv`. The summary ends `limpo` or `VAZAMENTO` (shown as an error).

### 16.4 Combined two-device mock test (`tests/settings_stage_j7.sh`)

Two separate processes and data directories (device A, device B) share one
file-backed mock account keyed by `(owner, profile, platform)`; each step is a fresh
process, so every step is also a restart. All tiers are live: Stage I (32), J6 (40),
J4 (90), J5 Home row shape, and the J7 gate, with the real sync worker. Any push to
`mobile` or `home_catalog_shared` aborts the test.

| Step | Result |
|---|---|
| Default Off | 90 J4 + 40 J6 + J5 edits stay local; stored permission flags cannot bypass the gate; only Stage I's 32 upload (D1) |
| Arming | Wrong code refused; code then confirm arms profile 1; pending J4/J5/J6 uploaded once each, no loop |
| A → B, first login | Fresh B restores all 90 J4, 40 J6, Stage I and J5 Home values; B uploads nothing |
| B → A | B armed, edits 30 J4, 14 J6 and a Home shape; A applies after restart, arming remembered, no echo |
| Offline | A edits offline, restarts, reconnects: uploaded once; B receives |
| Same-field conflict | Both offline, same J4 and J6 fields set differently: both converge; J4 uses per-field revision/device ordering, J6 uses three-way conflict handling; no ping-pong over 3 further cycles |
| Different-field conflict | Each device's own field survives on both |
| Profiles | Profile 1's arming does not arm profile 2; arming 2 changes only profile 2's document |
| Account switch | Another account: uploads off at once, enrolment erased, its document untouched; a code from one account cannot arm another |
| Expiry | Enforced in the running app (clock advanced 4 h) and after a restart with a past expiry; a stored expiry beyond 4 h is refused |
| Local-only | Interface resolution (device), Social (consent) and a TorBox key (credential) never reach `better_nuvio`, `tv` or `home_catalog_shared`; snapshot `limpo`; an injected foreign field and URL are flagged `VAZAMENTO` |
| Disarm | Further edits stay local |
| Logs | No token, credential, activation code, account id or value in J7/J4/J6 log lines |

### 16.5 Results

Ran on 5 November 2026 on macOS (host), with the J7 suite also built with UBSan.

* **J7a two-device suite:** PASS plain and PASS with UBSan; 21 steps, 278
  diagnostic lines with no value, account id, token, credential or activation code.
* **Regressions (41 suites run, 40 pass):**
  * settings: J7, J6, J5, J5 Native, J4, Stage I, the 413-binding preservation
    check, Stages A–D with interaction and stream tests;
  * account and playback: `bnaccount`, `bnsettings`, `torbox_cached_add`,
    `stream_cache_filter`, `cw_parity`;
  * Home and rows: `home_rows_stage_g`, `home_rows_official`, `home_rows_profiles`,
    `syncaddons`, `fileiras`, `fileiras_android`, `fileiras_navegacao`,
    `fileiras_removidas`, `catordem`, `catordemcache`, `colfileiras_sync`, `homepos`;
  * probes and links: `j0sonda`, `deeplink` (host and debug variants);
  * Android-variant unit builds: `ajustes_p2p_motivo`, `extplayer_decisao`,
    `player_android_retomada`, `ajustes_envio_auto`, `ajustes_perfil`.
* **Expected failure:** `ajustes_sync` asserts the device-snapshot upload that
  Stage I removed (already documented). `colfileiras` and `colfileiras_home` still
  fail to link, as they did before J7 (documented).
* **Fixed during J7a:**
  * The non-arming builds' gate is now inline in `j7arme.h`, so older test scripts
    with explicit source lists need no new file.
  * Units that fake `NV_ANDROID` for one file get the weak fail-closed fallback.
  * `tests/settings_stage_j5.c` (built as Android debug) supplies an armed-gate stub.
* **Android NDK r27:** all J7-touched files compile as Android debug and release;
  release objects contain no J7 arming code.

### 16.6 Remaining limitations

* **No device run yet.** Everything above is mock-backed; the real backend, Android
  lifecycle (resume, network loss) and TCL/BlueStacks behaviour are J7b.
* **ASan** still cannot run here (Homebrew `sdl2-compat` opens a blocking dialog
  under ASan); UBSan was used.
* **The arming code is shown on screen and kept in the app's private storage** for up
  to 10 minutes. On a rooted device or through `adb run-as` on the debug build it can
  be read; this is acceptable only because the build is debug-only and the account is
  the dedicated test account.
* **Account identity is by `sub` hash.** Enrolment proves "the account that was
  signed in when the code was shown"; it cannot prove that this account is the
  dedicated test account. The on-screen confirmation step relies on you checking the
  fingerprint before entering the code.
* **J4/J5 same-field conflicts use per-field revision/device ordering**, not
  first-confirmed-write ordering: higher revision wins; ties use the device id.
  J6/Stage I use their three-way remote-conflict rule. The earlier mock run's
  observed winner did not establish a first-writer guarantee. The live Fire Stick
  conflict test exposed this documentation error; no engine change was needed.
* **Stage I (D1)** still uploads automatically on any account, including production
  accounts, as before J7.
* Snapshot leak patterns are heuristics: a legitimate future field containing
  `token` or a URL would also be flagged; review before treating it as a real leak.

### 16.7 Next approvals

1. Debug APK containing J7a: **built and verified on 5 November 2026** (STATUS.md
   "Stage J7a debug APK": SHA-256 `94f4c656…619fa04f`, real gate in both ABIs,
   release native build has no J7 code). Not installed.
2. Installation and live J7b runs on the dedicated test account (section 15.3–15.5).

## 17. Stage J7b — real-device testing (started 5 November 2026)

### 17.1 Device B (BlueStacks) set-up and read-only baseline

* BlueStacks (Android 13) over `adb 127.0.0.1:5555`. The J7a APK
  (`94f4c656…619fa04f`) was installed over the J0 build, keeping the sign-in. The
  installed copy was pulled back and matched the approved SHA-256, as
  `space.nuvio.nativelegacy.strictdebug` 2.0.8-strictdebug (20008).
* `run-as` is blocked on this BlueStacks image. Logs are read from logcat
  (tag `nuvio`), filtered to `[j7]`, `[bn-sync]`, `[ajustes-oficial]` and
  `[ajustes-conta]` lines by `build/stage-j7b/j7b.sh`.
  * **Finding:** the app's older `[sessao]` and `[perfis]` lines write the account
    id to logcat. This predates J7, is unchanged and is never read unredacted here.
* Session restored on profile 1. The test account has no server-side profile rows,
  so the app runs on its default profile 1.
* First cycle, uploads off (`[j7] uploads J4/J5/J6 desligados (perfil 1)`):
  * `tv`: 0/32 Stage I and 0/40 J6 fields received, nothing applied;
  * `better_nuvio`: refused, nothing applied, nothing sent.
* Baseline snapshot, read-only, profile 1 (`build/stage-j7b/baseline.txt`):

| Namespace | HTTP | Size | Fingerprint |
|---|---|---|---|
| `tv` | 200 | 2 B (empty) | `9b8a3e00c63ea86f` |
| `better_nuvio` | 200 | 150 B (J0 probe row) | `bb09e12d232c709a` |
| `home_catalog_shared` | 200 | 2 B (empty) | `9b8a3e00c63ea86f` |
| `mobile` | 200 | 2 B (empty) | `9b8a3e00c63ea86f` |

  Leak summary: 0 fields outside the allowlist, 0 sensitive patterns, 0 Better keys
  in `tv` (`limpo`).

* Device A (Android TV) is not connected yet. Nothing was armed or written.

### 17.2 Blocker found: the leftover J0 probe stopped the Better tier

The J0 write left `{"schema":"better-nuvio-probe","version":0,"probe":"<token>"}`
in `better_nuvio` for profile 1. The engine correctly refuses unknown documents,
but the refusal ends the whole cycle before any upload. So J4/J5 could never start
on that profile, and J0's note that J3 "will overwrite it" was wrong. J0's clean
step (`{}`) is refused too, and its 24-hour window had passed.

### 17.3 Fix: narrow J0 probe compatibility (approved option 1)

* `bncodec.c` `legacyJ0()` accepts **exactly** two documents as "Better Nuvio sync
  not initialised":
  * the J0 probe: exactly three members, `schema` = `"better-nuvio-probe"`,
    `version` = integer `0`, and `probe` = 8 uppercase hex characters;
  * the J0 clean result `{}`. This is explicitly supported because it holds no data
    and was the J0 tool's own output.
* Recognition sits after all existing response checks: one row, matching
  `profile_id`, size limit and well-formed JSON. Duplicate keys were already
  rejected by the parser.
* Everything else is still refused, including every near miss: missing or extra
  members, other versions, `0.0`, `"0"`, lowercase or 7/9-character or non-hex
  tokens, other schemas and malformed JSON.
* A recognised probe takes the existing first-sync path: zero fields, nothing
  applied, nothing pending, so nothing is uploaded. The **remote probe is never
  deleted, reset or written directly**. It stays until an armed device makes a
  real local change, and that normal upload replaces it.
* Diagnostic: `[bn-sync] perfil N documento J0 de teste: tratado como nao
  inicializado (mantido no servidor)`.
* J4/J5/J6 behaviour, the J7 gate and default-Off are unchanged. Stage I is
  untouched.

Tests:

* `tests/bn_j0probe.sh` (ASan/UBSan) covers:
  * the exact probe, reordered/spaced, and `{}`, all accepted with 0 fields;
  * 26 near-miss/unknown documents and 5 malformed or other-profile responses,
    all refused;
  * a real document unchanged and not flagged as legacy;
  * no upload until a real local edit.
* `tests/settings_stage_j7.sh`, new two-device steps on a mock account seeded with
  the probe on profiles 1 and 2:
  * **first sync**, unarmed and armed without edits: nothing applied or uploaded,
    and the probe stays byte-identical;
  * a **second device** over the probe: no upload;
  * **offline** armed edit, restart and reconnect: the probe is replaced once, with
    no loop;
  * the peer receives the result with no echo;
  * **profile isolation**: profile 2's probe is kept until its own armed edit, and
    replacing it leaves profile 1 unchanged;
  * an **unknown-schema** document is refused and never overwritten, even when
    armed with real edits.
* 41 regression suites pass, including J2–J7 (J7 plain and UBSan), Stage I,
  settings A–D, the 413-binding preservation check, Home rows, TorBox and stream
  filtering, Continue Watching, the J0 probe and deep links.

### 17.4 Replacement debug APK (built, not installed)

Built with `tools/android.sh --debug-only` (log `build/stage-j7b/apk-build.log`,
exit 0, 13:14). The working tree was identical before and after the build.

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,564,017 bytes (71 MB) |
| SHA-256 | `fae8d7fb8a6b17dc2597f44cc97fa1536f9b58febf243e29388ff2deca1dd6ab` |
| Replaces | the J7a build `94f4c656…619fa04f` (still installed on BlueStacks) |
| Package | `space.nuvio.nativelegacy.strictdebug`, 20008, `2.0.8-strictdebug`, debuggable, arm64-v8a + armeabi-v7a |
| Signing | Android Debug `c0a433a8…c59bba`; `zipalign -P 16` OK |

Both `libmain.so` files contain the real gate (`j7arme_permite`, GLOBAL, 304/296
bytes), the J7 links and the probe diagnostic. The probe fix changes only the
Better-tier codec and one log line. The release exclusion verified in 16 is
unaffected: no J7 file changed.

**Next, needing your approval:** install this APK on BlueStacks (and on the TV once
it is connected), then continue J7b from the read-only steps.

### 17.5 Replacement APK on BlueStacks — read-only check (5 November 2026)

* **Install.** Before installing, the local file matched the approved SHA-256
  (`fae8d7fb…1dd6ab`). It was installed with `adb install -r`, keeping app data
  and the test-account sign-in. The installed copy was pulled back and matched,
  as `space.nuvio.nativelegacy.strictdebug` 2.0.8-strictdebug (20008).
* **Session.** Restored on profile 1 (account id redacted). The J7 gate was not
  armed.
* **First cycle (uploads off).**
  * `[j7] uploads J4/J5/J6 desligados (perfil 1)`.
  * `[bn-sync] perfil 1 pull HTTP 200: 0 campo(s) recebidos`, then
    `documento J0 de teste: tratado como nao inicializado (mantido no servidor)`,
    then `aplicados 0`. The Better tier now completes its cycle instead of
    stopping at the probe.
  * `tv`: 0/40 J6 and 0/32 Stage I received.
* **Snapshot, read-only**, identical to the 17.1 baseline:
  * `tv`, `home_catalog_shared` and `mobile` are `9b8a3e00c63ea86f`;
  * `better_nuvio` is 150 B, `bb09e12d232c709a` (the probe is still on the server,
    unmodified);
  * leak check `limpo`.
* **No writes.** No upload, pending, arming or write line appeared in the session.
* **Not yet tested.** Replacement of the probe by the first armed upload of a real
  edit is not tested on a device yet; it is covered by the mock suite. The Android
  TV has not been touched.

### 17.6 Separate J4/J5 and J6 permissions (approved option 1)

Before the first armed J4 test, inspection showed two problems:

* one arming step enabled both J4/J5 (`better_nuvio`) and J6 (official `tv`)
  uploads;
* the "J6 pending while off" line could never print, because the merge step that
  contained it only runs when uploads are already allowed.

So "keep J6 off" could not be guaranteed or checked on BlueStacks, where `run-as`
is blocked.

**Design**

* **Two independent scopes per profile**: `J7_J4` (J4/J5 → `better_nuvio`) and
  `J7_J6` (J6 → `tv`). Each has its own expiry (at most 4 hours), its own
  activation code and its own gate: `j7arme_permite_j4` sits in front of every
  Better-tier write (`bnaccount.c`) and `j7arme_permite_j6` in front of J6
  (`ajustes_official.inc`).
* **Links**:
  * `…/j7/arm` and `…/j7/arm?token=` arm **J4/J5 only** (the default activation);
  * J6 needs its own code from `…/j7/arm-j6` and `…/j7/arm-j6?token=`;
  * a J4/J5 code cannot arm J6 and a J6 code cannot arm J4/J5;
  * nothing may follow the token, so `arm?token=…&j6=1` is rejected rather than
    arming J4 silently;
  * `…/j7/disarm` clears both scopes on all profiles.
* **State file**: lines are `armed <profile> <expiry> <scope>`, and the pending
  code records its scope. A J7a-format line with no scope means J4/J5 only. An
  unknown scope is ignored, so it fails closed. Account change, signing out and
  expiry work as in J7a, per scope.
* **Logs**: `[j7] uploads J4/J5 LIGADOS|desligados, J6 LIGADOS|desligados
  (perfil N)`. After every `tv` pull while J6 is off, the app logs
  `[ajustes-oficial] perfil N pendentes com uploads J6 desligados <count>
  [official names]`. Names and counts only; no values, ids or tokens.
* **`tv` write isolation**:
  * with J4/J5 armed and J6 off, the J6 merge adds nothing;
  * a `tv` write can still come from Stage I (D1, unchanged), and then it carries
    only Stage I's fields;
  * J4/J5 keys are never written to `tv`.
* **Release builds** are unchanged: both gates are constant refusals and no J7
  links exist. Host builds keep explicit permissions; both inline gates return 1.

**New two-device steps** (`tests/settings_stage_j7.sh`, fresh mock account):

| Step | Checked |
|---|---|
| `s-j4only` | Default activation arms J4/J5 only; a J4 edit and a J6 edit: one `better_nuvio` upload, **no `tv` write**, J6 key absent; the J6 pending line shows `1 [mdblist_show_imdb]` |
| `s-restart` | After a restart J4/J5 is still armed and J6 still off; no uploads without edits; a Stage I edit writes `tv` once with the Stage I field only (no J6 or J4 keys); no loop |
| `s-codes` | A J4/J5 code cannot arm J6 and vice versa; J6 armed alone sends the waiting J6 field to `tv` and never `better_nuvio` |
| `s-profile` | Profile 2 inherits neither scope; J4/J5 armed on profile 2 writes only profile 2's `better_nuvio`; profile 1's `tv` unchanged; disarm clears both scopes everywhere |
| `s-legacy` / `s-legacy-check` | A J7a-format state file (no scope) arms J4/J5 only after a restart, never J6 |

The earlier J7a scenarios now arm both scopes explicitly, each with its own code.
The deep-link test covers `arm-j6`, both token forms, and the rejected
`arm-j6?token=<lowercase>`, `arm-J6`, `arm-j7` and `arm?token=…&j6=1`.

**Results**

* 41 regression suites pass, including the J7 suite (plain and UBSan) with the
  new steps, `bn_j0probe`, `deeplink` (host and debug), J4/J5/J6, Stage I,
  settings A–D, the 413-binding preservation check, Home rows, TorBox and stream
  filtering, Continue Watching and `j0sonda`.
* NDK r27: all changed files compile for both ABIs, debug and release. The release
  objects have no `j7arme` symbol, no `arm-j6`/`disarm` routes and no arming text.

### 17.7 Replacement debug APK with separate scopes (built, not installed)

Built with `tools/android.sh --debug-only` (log
`build/stage-j7b-scope/apk-build.log`, exit 0, 13:35). The working tree was
identical before and after the build.

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,565,837 bytes (71 MB) |
| SHA-256 | `a5fed5e294f4e83e8f91bea14dc290a42335760658f52c60d9094be0e555288c` |
| Replaces | `fae8d7fb…1dd6ab` (installed on BlueStacks; not armed) |
| Package | `space.nuvio.nativelegacy.strictdebug`, 20008, `2.0.8-strictdebug`, arm64-v8a + armeabi-v7a |
| Signing | Android Debug `c0a433a8…c59bba`; `zipalign -P 16` OK |

**Gate verification in both `libmain.so` files**

* `j7arme_permite_j4` and `j7arme_permite_j6` are separate GLOBAL entry points.
  Each loads its own scope (1 or 2) and branches to the shared real body: arm64
  `mov w2,#1|#2; b`, armv7 Thumb `movs r2,#1|#2; b.w`.
* That body locks the J7 mutex, loads the state and unlocks it:
  `pthread_mutex_lock`, `time` and `pthread_mutex_unlock` were resolved through the
  armv7 PLT. It is **not** the weak `return 0` fallback, and the old single
  `j7arme_permite` is gone.
* Callers on arm64:
  * `j7arme_permite_j4`: only `bna_cycle` (2 calls);
  * `j7arme_permite_j6`: only `ajustes_compartilhados_aplicar` (the pending log),
    `ajustes_compartilhados_mesclar` (2) and `ajustes_compartilhados_pendentes`.
  * So no J6 path can use the J4 permission, and no J4 path the J6 one.
* Both ABIs contain the `arm-j6` route, the scope log, the J6 pending log and the
  J0-probe diagnostic.

**Planned J4 test with this APK (after approval)**

1. Install and verify.
2. Read-only cycle: confirm `uploads J4/J5 desligados, J6 desligados` and read any
   J6 pending names.
3. Arm J4/J5 only with `…/j7/arm`. Do **not** use `arm-j6`.
4. Toggle **Show genre on hero** (`heroGeneroLocal`).
5. Expect exactly one `better_nuvio` upload replacing the J0 probe. `tv`,
   `home_catalog_shared` and `mobile` fingerprints unchanged.
6. Disarm.

### 17.8 Separate-scope APK on BlueStacks — read-only check (5 November 2026)

* **Install.** `a5fed5e2…288c` matched before installing. It was installed with
  `-r`, keeping app data and the test-account sign-in, and the installed copy
  matched. Session restored on profile 1 (account id redacted).
* **First cycle:**
  * `[j7] uploads J4/J5 desligados, J6 desligados (perfil 1)`;
  * `better_nuvio`: pull 200, J0 probe treated as uninitialised, nothing applied;
  * `tv`: 0/40 J6 and 0/32 Stage I.
* **Nothing waiting.** No J4/J5 pending line and no J6 pending line. The J6 line is
  now printed after every pull whenever anything is waiting, so its absence
  confirms that **0 J6 changes are waiting** and arming J4/J5 has nothing that
  could reach `tv`.
* **Snapshot, identical to the baseline:**
  * `tv`, `home_catalog_shared` and `mobile` are `9b8a3e00c63ea86f`;
  * `better_nuvio` is 150 B, `bb09e12d232c709a` (probe intact);
  * leak check `limpo`.
* **No writes.** No upload, pending, arming or write line appeared. Nothing armed.

### 17.9 First J4 attempt stopped: activation code not visible; notice fixed

* **Before the attempt.** The pre-test read-only snapshot matched the baseline
  (`build/stage-j7b/pre-j4.txt`).
* **Code requested, nothing armed.** `…/j7/arm` created a J4/J5 code for account
  fingerprint `d21b69ba`, profile 1; requesting a code arms nothing.
* **The code was cut off.** The top-of-screen notice (island) shows one line and
  cuts the rest. On BlueStacks it showed about 68 characters, "J7 J4/J5
  (better_nuvio): conta d21b69ba perfil 1. Confirme que e a…", so the code at the
  end was never visible. It is not logged by design, and `run-as` is blocked.
  **The attempt stopped before arming.** The unused code expired on its own after
  10 minutes.
* **Cause.** A J7a test gap: the mock test read the code from the full message and
  never checked what the one-line notice shows.
* **Fix.** The notice now starts with the code:
  `J7 J4/J5 codigo XXXXXXXX - conta <fp> perfil N - so na CONTA DE TESTE (10 min)`,
  and `J7 J6 codigo …` for `arm-j6`. The code occupies characters 17–24. The log
  line is unchanged: it says the code is shown on screen and never contains it.
* **Tests.**
  * The J7 helper now requires the code within the first 30 characters, as 8
    uppercase hex characters.
  * The suite's log check now also rejects any `codigo` followed by 8 hex
    characters.
  * The J7 suite passes, and all 41 regression suites pass.
* **Replacement APK** (`tools/android.sh --debug-only`, log
  `build/stage-j7b-notice/apk-build.log`, exit 0, 13:50; working tree unchanged by
  the build):

| Item | Value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,566,099 bytes (71 MB) |
| SHA-256 | `b73a55636764a11a445bde833689fd3e31f9f8f2da59426adfd413042f77420b` |
| Replaces | `a5fed5e2…288c` (installed on BlueStacks; not armed) |
| Package | `space.nuvio.nativelegacy.strictdebug`, 20008, `2.0.8-strictdebug`, arm64-v8a + armeabi-v7a |
| Signing | Android Debug `c0a433a8…c59bba`; `zipalign -P 16` OK |

  Both `libmain.so` files contain:
  * the two scope gates (`j7arme_permite_j4`, `j7arme_permite_j6`, GLOBAL) and no
    old single gate;
  * the new notice text, and not the old one;
  * the `arm-j6` route.

  Only the notice text in `j7arme.c` changed since `a5fed5e2…`, so the gate wiring
  verified by disassembly in 17.7 is unchanged.

### 17.10 Second J4 attempt stopped before arming: a Stage I setting was changed

* **Set-up.** APK `b73a5563…7420b` was installed and verified on BlueStacks. The
  read-only cycle was clean, with both scopes off and nothing waiting. The
  pre-test snapshot matched the baseline (`build/stage-j7b/pre-j4b.txt`). The J4/J5
  code was readable on screen ("J7 J4/J5 codigo …"; never logged).
* **You confirmed** the test account and changed a setting. The logs then showed
  **no J4 change waiting** (no `heroGeneroLocal`). Instead:
  * `[ajustes-conta] enviando 1 campo(s): hero_section_enabled` appeared 3 times,
    with `[sync] ajustes desta TV guardados na conta` after the uploads;
  * that is the Stage I `hero_section_enabled` (the Hero section on/off switch),
    which uploads automatically under D1.
* **Stopped. Nothing armed** (0 `LIGADOS` lines). Read-only snapshot afterwards:

| Namespace | Before | After |
|---|---|---|
| `tv` | 2 B `9b8a3e00c63ea86f` | **193 B `36dd6d0e82efc08d`** (now contains Stage I `hero_section_enabled`) |
| `better_nuvio` | `bb09e12d232c709a` | `bb09e12d232c709a` (probe intact) |
| `home_catalog_shared` | `9b8a3e00c63ea86f` | unchanged |
| `mobile` | `9b8a3e00c63ea86f` | unchanged |

  Leak check `limpo`: 0 Better keys in `tv`.
* **Assessment.**
  * This is Stage I's existing, approved automatic upload (D1). It wrote the
    official field regular Nuvio shares, to the test account only. No J4/J5/J6 code
    path was involved: J4/J5 and J6 were off throughout.
  * It does change regular Nuvio's `tv` document for the test account, which was
    empty until then. It happened because the switched control was **Show hero
    section** (Stage I), not **Show genre on hero** (J4, `heroGeneroLocal`).

### 17.11 First real J4 synchronisation over the J0 probe — PASSED (5 November 2026)

BlueStacks, APK `b73a5563…7420b`, dedicated test account (fingerprint `d21b69ba`,
confirmed by you), profile 1. Logs in `build/stage-j7b/j4-*.txt`.

| Step | Result |
|---|---|
| Edit with uploads off | You switched **Show genre on hero** (J4). Next cycle: `[bn-sync] perfil 1: 1 campo(s) locais pendentes; uploads desligados`. No J6 pending line, no new Stage I upload, probe still recognised. |
| Arm J4/J5 only | Code `J7 J4/J5 codigo …` read from the screen (not logged). `[j7] uploads J4/J5 LIGADOS, J6 desligados (perfil 1)`. `arm-j6` was not used. |
| First real upload | On the next cycle (triggered by bringing the app back to the foreground): probe recognised, `enviando 1 [heroGeneroLocal]`, `push HTTP 204: confirmado`. **No `[ajustes-oficial]` or `[ajustes-conta]` upload.** |
| Snapshot after | `better_nuvio` 150 B `bb09e12d…` → **256 B `46d82a0dd8ba9954`** (probe replaced by a profile document). `tv` `36dd6d0e82efc08d` unchanged since 17.10. `home_catalog_shared` and `mobile` `9b8a3e00c63ea86f` unchanged. Leak check `limpo`: 0 fields outside the allowlist, 0 sensitive patterns, 0 Better keys in `tv`. |
| Restart (force-stop and relaunch) | `uploads J4/J5 LIGADOS, J6 desligados` kept across the restart. Pull: `1 campo(s) recebidos`, `aplicados 0` (already equal), no probe line, **no push (no echo)**. |
| Disarm | `uploads de teste DESLIGADOS e inscricao apagada`, then `uploads J4/J5 desligados, J6 desligados`. Final snapshot identical to the post-upload one. 0 uploads after disarming. |

**Conclusions**

* The J0 probe is read as uninitialised on a real device. The first armed edit
  replaced it with a normal `better-nuvio-profile` document holding only
  `heroGeneroLocal`.
* J4/J5 arming did not enable J6 or cause any `tv` write.
* Arming persists across a restart, and disarming clears it.

**Observations**

* No periodic cycle ran within about 4 minutes while the app sat on Home. Cycles
  ran at start-up and when the app returned to the foreground; a second return
  within a short time did not trigger one. This is existing sync scheduling, not
  J7.
* The refusal after disarming was shown by the gate state and by 0 uploads. It was
  not shown by an edit while disarmed, because that would need another change
  from you.

**Not yet done in J7b:** the Android TV (device A), A→B and B→A between two devices,
offline edits, conflicts, profile 2, account switching, local-only/credential
checks on a device, and J6 arming.


### 17.12 TV ADB connection and installation-only approval (5 November 2026)

Continued from Claude's completed 17.11 evidence. The connected device A is a
**Xiaomi MiTV-AFMU0, Android 14**, at `192.168.0.43:5555` (the earlier draft named
a TCL TV). The user authorised ADB on the TV. BlueStacks remains connected,
currently under `emulator-5554`; its latest filtered J7 log shows J4/J5 off and
J6 off. `run-as` remains unavailable on that BlueStacks image, as already noted.

The TV initially had regular Nuvio (`com.nuvio.tv`) and no strictdebug package.
After **explicit installation-only approval**, the approved debug APK was installed:
`space.nuvio.nativelegacy.strictdebug`, versionCode 20008, `2.0.8-strictdebug`.
Its independently pulled installed copy matches SHA-256
`b73a55636764a11a445bde833689fd3e31f9f8f2da59426adfd413042f77420b`
(74,566,099 bytes). Regular Nuvio was not replaced. No Better app process was
running after verification. A read-only private-file check finds no persisted J7
arming file on the TV: both scope gates default off.

**Not performed:** app launch, login/account inspection, setting edit, scope arming,
backend snapshot/write, network toggle or live two-device scenario. The TV's
dedicated test account is **not yet confirmed**. Launch/sign-in and authenticated
read-only testing are the next approval boundary; no credentials or account IDs
are to be shared or logged. Stage I's automatic uploads remain an existing D1
exception, so do not change Stage I controls during read-only preparation.

Prepared the remaining receive-only, A↔B, J5, offline/restart, conflict and
profile/account isolation scenarios in `build/stage-j7b/TV-READONLY-PLAN.md`.
Every mutation/arming/network-toggle scenario remains pending explicit approval;
J6 approval is separate from J4/J5. Claude's earlier evidence and existing source
changes were preserved. No commit, push, merge or master change.


### 17.13 First TV receive-only restore and snapshot — PASSED (5 November 2026)

After explicit launch/sign-in approval, the user confirmed completion of the
requested dedicated-test-account / profile-1 sign-in. Existing logs were read
only through the J7 allowlist and isolated to the current strictdebug process;
no account identifiers, credentials or activation codes were exposed.

* TV gate log: J4/J5 off, J6 off, profile 1.
* First Better pull: HTTP 200, one field received; applied one `heroGeneroLocal`.
* Second pull: one field received, zero applied (already equal).
* Stage I received its existing `hero_section_enabled`; the next pull was equal.
* Current-process send/upload log lines: **0 Better, 0 J6, 0 Stage I**.

The existing `j7/snapshot` action then performed read-only namespace queries:

| Namespace | HTTP | Bytes | Fingerprint |
|---|---|---|---|
| `tv` | 200 | 193 | `36dd6d0e82efc08d` |
| `better_nuvio` | 200 | 256 | `46d82a0dd8ba9954` |
| `home_catalog_shared` | 200 | 2 | `9b8a3e00c63ea86f` |
| `mobile` | 200 | 2 | `9b8a3e00c63ea86f` |

All four match BlueStacks' post-J4/disarm snapshot from 17.11. Leak counts: zero
unallowlisted fields, zero sensitive patterns, zero Better keys in `tv`; summary
clean. No uploads appeared during the snapshot. Filtered evidence:
`build/stage-j7b/tv-first-restore-filtered.txt`.

This passes the initial **B→A receive-only restore/no-echo** check using the field
already uploaded by BlueStacks. It does not yet prove a new TV-originated upload,
conflicts, offline recovery or profile-2 isolation. No setting was edited and no
scope was armed. A new A→B write test requires separate explicit approval;
BlueStacks and J6 must remain off for that proposed J4-only test.


### 17.14 First TV → BlueStacks J4 transfer — PASSED (5 November 2026)

The user explicitly approved one TV-originated J4/J5 transfer and confirmed
changing **Show genre on hero** once, leaving Stage I's Hero section untouched.

1. With uploads off, the TV reported one pending Better field; J6 and Stage I
   had no pending send. No setting was changed by automation.
2. Requested a TV J4/J5 code. Privately verified the dedicated test-account
   fingerprint, profile 1 and J4-only scope from the arming state. The activation
   code was neither printed nor saved in evidence. No J6 arming was requested.
3. Resumed the TV: `enviando 1 [heroGeneroLocal]`, then `push HTTP 204: confirmado`.
   No J6 or Stage I send occurred.
4. Immediately disarmed the TV; filtered gate status confirmed both scopes off.
   Verified BlueStacks remained unarmed, then resumed it: HTTP 200, one received
   field, `aplicados 1 [heroGeneroLocal]`; no receiver upload was logged.
5. Read-only snapshots on both devices agree:

| Namespace | Bytes | Fingerprint | Change |
|---|---|---|---|
| `better_nuvio` | 255 | `41160f64fdb0e802` | Approved single-field update from prior `46d82a0dd8ba9954` |
| `tv` | 193 | `36dd6d0e82efc08d` | Unchanged |
| `home_catalog_shared` | 2 | `9b8a3e00c63ea86f` | Unchanged |
| `mobile` | 2 | `9b8a3e00c63ea86f` | Unchanged |

Both devices report J4/J5 off and J6 off, profile 1. Both snapshot leak checks
are clean (all three counters zero; all four reads HTTP 200).

Filtered evidence: `build/stage-j7b/tv-a-to-b-upload-filtered.txt`,
`tv-a-to-b-disarmed.txt`, `bluestacks-receive-from-tv.txt`,
`tv-after-a-to-b.txt`, `bluestacks-after-a-to-b.txt`.

Initial single-field J4 transfers now pass in both directions (17.13/17.14).
This does not complete the wider J5/Home, offline recovery, same/different-field
conflict, profile/account isolation or J6 live tests; they still need separate
approval. No source changes, commit, push, merge or master change.


### 17.15 Offline edit, restart and reconnect recovery — PASSED, disarmed-edit path (5 November 2026)

The user approved the offline recovery test. BlueStacks has an internal Ethernet
route, so a Wi-Fi toggle there would not prove an outage; no emulator network
change was made. Instead, the user turned off the Xiaomi TV's Wi-Fi, changed
**Show genre on hero** once, force-stopped/reopened the debug app without clearing
data, and confirmed the edit survived while still offline. ADB reported the TV
host down/offline before the user restored Wi-Fi.

After reconnection, current-process filtered logs showed profile 1 with both
upload scopes off, a successful pull, zero fields applied and **one local pending
field**. The server's prior value did not replace the recovered offline edit.

Following private verification of the dedicated test-account fingerprint, profile 1
and J4-only activation scope, TV J4/J5 was explicitly armed for the approved test.
A foreground refresh sent only `heroGeneroLocal`, with exactly one HTTP 204
confirmation. No J6 or Stage I send was logged. The TV was immediately disarmed.
Unarmed BlueStacks then received/applied `heroGeneroLocal` with no upload logged.

Final read-only snapshots agree on both devices:

| Namespace | Bytes | Fingerprint | Result |
|---|---|---|---|
| `better_nuvio` | 256 | `2b1fd615437388a7` | Recovered edit stored |
| `tv` | 193 | `36dd6d0e82efc08d` | Unchanged |
| `home_catalog_shared` | 2 | `9b8a3e00c63ea86f` | Unchanged |
| `mobile` | 2 | `9b8a3e00c63ea86f` | Unchanged |

All reads returned HTTP 200; both devices report both scopes off; all leak
counters are zero. Evidence is in `build/stage-j7b/offline-*.txt`.

**Scope of pass:** user-observed persistence across a genuine offline app restart,
pending-journal recovery after reconnection, then a single approved upload and
peer receipt without an echo. The app was disarmed during the offline/reconnect
phase, so this is not evidence of an automatically armed reconnect upload or
armed-state persistence during an outage. That variant remains untested.
Conflicts, profile/account isolation, J5 Home and J6 live scenarios remain pending
separate approval. No source changes, commit, push, merge or master modification.


### 17.16 Fire Stick replacement device — installation verified (5 November 2026)

The user requested the Fire Stick instead of the Xiaomi TV for subsequent tests.
Connected and authorised `192.168.0.242:5555`: Amazon AFTSS / sheldon, API 28,
ARMv7. The strictdebug package was initially absent. The first approved install
failed with insufficient storage; no deletion or uninstall was performed by the
assistant. Following the user's retry instruction, installation succeeded.

The installed APK was independently pulled and exactly matches the approved
SHA-256 `b73a55636764a11a445bde833689fd3e31f9f8f2da59426adfd413042f77420b`:
`space.nuvio.nativelegacy.strictdebug`, 20008 / `2.0.8-strictdebug`.
The app process is not running; no persisted J7 arming file exists, so both scopes
default off. No launch, login, setting edit, arming or backend operation occurred
in this installation session. Fire Stick test-account verification and all live
Fire Stick tests remain pending. Prior Xiaomi/BlueStacks results remain evidence
for those devices only, not Fire Stick validation.

Evidence: `build/stage-j7b/firestick-install-verified.txt`. No source changes,
commit, push, merge or master modification.


### 17.17 Fire Stick armed offline restart and reconnect — PASSED (5 November 2026)

User explicitly approved J4/J5 arming on Fire Stick profile 1, one offline
**Show genre on hero** edit, restart/reconnect and one upload; J6 stayed off.
User confirmed dedicated test account/profile 1. Activation scope and account
fingerprint were checked privately; no activation code or credentials were saved.
BlueStacks remained unarmed.

A device-local recovery timer was scheduled before disabling Wi-Fi: app restart
at 80 seconds and Wi-Fi restoration at 150 seconds. ADB became unreachable.
The user initially said the window was missed, then corrected this to “wait i did
it”; the subsequent check was at 74 seconds with the device still offline, before
the scheduled restart. No second toggle was requested. The device's status file
confirmed Wi-Fi disable and app restart. After reconnect, without another app
resume, filtered current-process logs recorded one `heroGeneroLocal` send and
one HTTP 204 confirmation. Thus the approved edit survived an offline restart
and uploaded automatically while the permission persisted.

Both devices were then disarmed. Unarmed BlueStacks pulled/applied exactly one
`heroGeneroLocal` field in the post-disarm observation, with no echo upload.
Read-only final snapshots agreed on both devices:

| Namespace | Initial fingerprint | Final fingerprint | Result |
|---|---|---|---|
| `better_nuvio` | `2b1fd615437388a7` | `d0893180d0229522` | Approved edit, 255 bytes |
| `tv` | `36dd6d0e82efc08d` | `36dd6d0e82efc08d` | Unchanged, 193 bytes |
| `home_catalog_shared` | `9b8a3e00c63ea86f` | `9b8a3e00c63ea86f` | Unchanged, 2 bytes |
| `mobile` | `9b8a3e00c63ea86f` | `9b8a3e00c63ea86f` | Unchanged, 2 bytes |

All snapshot reads returned HTTP 200 and all leak counters were zero.
Evidence: `build/stage-j7-final/01-*.txt`, `01-timing.json` and
`01-offline-device.sh`. No application source or APK change was required.
Same-field conflicts, profile/account isolation, J5 Home and J6 live tests remain
pending their specific approvals; this pass does not cover those scenarios.


### 17.18 Fire Stick ↔ BlueStacks same-field conflict — PASSED (5 November 2026)

User explicitly approved different **Hero size** edits on the dedicated test
account/profile 1, J4/J5-only activation as required, convergence/no-echo checks
and disarming. Both activation challenges verified the dedicated account and
profile 1 privately; no activation code was printed or persisted.

Both devices began with Hero size Normal. With uploads off, Fire Stick selected
Large and BlueStacks selected Extra large using the actual settings editors.
The devices stayed online while these pending edits were constructed; this is a
pending-edit conflict test, not another simultaneous network-outage test.
Fire Stick then sent `heroSizeLocal` once (HTTP 204). BlueStacks pulled the
result, retained its winning pending value and sent `heroSizeLocal` once (204).
Fire Stick subsequently applied that value; both visible settings panels showed
**Extra large**. J6 and the existing Stage I settings were not edited or sent.

**Documentation correction:** J4/J5's implemented policy is per-field revision,
then device-id tie breaking (`bn_pull`/`stamp` in `src/bnsettings.c`). It does not
guarantee first-confirmed-write wins. The prior mock fixture's observed winner
was incorrectly generalised in section 16 and its shell output. Those descriptions
have been corrected; the sync engine and APK were not changed. J6/Stage I retain
their separate three-way merge policy.

After convergence, both devices were legitimately re-armed for the approved
no-echo checks. Three **fresh process restarts** per device each verified a new
HTTP 200 pull, persisted J4/J5-on/J6-off state and zero sends/uploads. This replaces
the weaker initial foreground-only observations, whose buffered log entries could
not establish a fresh pull per observation. Both devices were then disarmed and
both scopes were verified off.

Final snapshots agree on both devices: `better_nuvio` 362 bytes,
`a5db70c709fe8f12` (initial `d0893180d0229522`, 255 bytes). Unrelated `tv`
193 bytes/`36dd6d0e82efc08d`, `home_catalog_shared` and `mobile` each
2 bytes/`9b8a3e00c63ea86f` remain unchanged. All reads HTTP 200, leak counters zero.

Regressions: `tests/bnsettings.sh` PASS (ASan/UBSan),
`tests/settings_stage_j7.sh` PASS (isolated two-device backend),
`python3 tests/settings_preservation.py` PASS (413 bindings/defaults/progress).
Evidence: `build/stage-j7-final/02-*`; fresh-cycle proof is in
`02-{fire,blue}-restart-{1,2,3}.txt`, not the earlier `settle` files.

Next prerequisite: the Fire Stick profile cache could not supply a list of
available profiles. This is not proof that remote profile 2 is absent. No profile
switch/creation or account change has been performed. Profile/account isolation,
J5 Home and J6 live tests remain pending their approvals/prerequisites. All
pre-existing project work is preserved; no build/install/commit/push/merge.

### 17.19 Handover after Codex's interruption — state recovered, Fire Stick made safe (5 November 2026, 17:08–17:13)

Claude took over from Codex. Inspected `build/stage-j7-final/` (01–03 evidence,
`session-plan.json`, `03-p2-send.py`, `03-p2-send-state.json`), the app's private
state on the Fire Stick (`run-as` works there), filtered logs and screens on both
devices, and fresh read-only snapshots. Evidence:
`build/stage-j7-final/claude/04-*`.

**What Codex completed in step 03 (profile 2 isolation, Fire Stick = A,
BlueStacks = B):**

* Profile-1 snapshots on both devices matched the post-conflict state (17:01–17:02):
  `better_nuvio` 362 B `a5db70c709fe8f12`, `tv` 193 B `36dd6d0e82efc08d`, and
  `home_catalog_shared`/`mobile` `9b8a3e00c63ea86f`.
* Switched both devices to an existing profile 2. The account has 2 profiles.
* Profile-2 snapshots on both devices: all four namespaces empty
  (`9b8a3e00c63ea86f`), leak check clean.
* **On the Fire Stick only**, `03-p2-send.py`:
  * changed profile 2's **Hero size to Large** through the settings editor. The
    profile-2 checkpoint now has exactly one pending field, `heroSizeLocal`, at
    rev 1. **The Hero size change did happen locally.**
  * **armed J4/J5 for profile 2.** The state file has `armed 2 <expiry> 1`
    (scope 1 = J4/J5), expiring 4 h after 17:07. J6 was not armed.
  * force-stopped and relaunched the app to get a fresh cycle, then wrote
    `03-p2-send-state.json` = `armed-awaiting-profile-selector`.
* **The interruption came here.** The relaunched app stopped at the profile
  chooser (`[sync] ciclo interrompido: 2 perfis e nenhum escolhido nesta TV`). No
  profile was chosen, so no sync cycle ran: **0 sends** in either Fire log file.
* BlueStacks was not armed in step 03. Its gate has been off since 16:52:51, it is
  on profile 2 with nothing pending, and it has made no sends since the conflict
  test.

**Server state confirmed:** a fresh BlueStacks profile-2 snapshot at 17:12:48 shows
all four profile-2 namespaces still empty (`9b8a3e00c63ea86f`), leak check clean.
**The Hero size change was never uploaded.** No sends were logged on either device
after the profile-1 snapshots, so profile 1 is unchanged from the 17:01 values
above.

**Made safe (no backend write):**

* A disarm link sent to the Fire Stick was **not processed**, because the app does
  not handle debug links while the profile chooser is open. Choosing a profile
  blind could have triggered the armed upload.
* The **Fire Stick app was force-stopped**. Nothing can upload while it is not
  running.
* **Still pending:**
  * the Fire Stick's J4/J5 arming for profile 2 remains in its private state and
    expires on its own about 3.9 h after 17:12;
  * the Hero size Large edit remains pending locally on Fire profile 2.
* Opening the app and choosing **profile 2** within that window would upload it.
  Choosing **profile 1** first and then disarming would not.

**Finding:** debug links, including `disarm`, wait until a profile is chosen. A
pending disarm cannot be guaranteed to run before an armed profile's first sync
cycle. Recorded for J7c, with no code change now.

Working tree: the same 108 modified/untracked paths as before the handover; no
source edited by Claude in this step.

### 17.20 Profile 2 isolation — PASSED (5 November 2026, 17:20)

You approved finishing Codex's profile-2 test. Fire Stick (A) profile 2 held one
pending `heroSizeLocal` = Large and was armed for J4/J5 only (17.19). BlueStacks
(B) was unarmed. Evidence: `build/stage-j7-final/claude/05-*`.

| Step | Result |
|---|---|
| Fire relaunched; chooser shows Profile 1 / Profile 2, Profile 2 focused | Before selection: 0 sends, still armed for profile 2 |
| Profile 2 selected | `uploads J4/J5 LIGADOS, J6 desligados`; `perfil 2 pull HTTP 200: 0 campos`; **`perfil 2 enviando 1 [heroSizeLocal]`, `push HTTP 204: confirmado`**; J6 0/40, Stage I 0/32; no J6 or Stage I send |
| Fire disarmed straight away (on Home, so the link was processed) | `DESLIGADOS e inscricao apagada`, `J4/J5 desligados, J6 desligados (perfil 2)`; state file has 0 `armed` lines |
| BlueStacks profile 2 (unarmed) brought to the foreground | `perfil 2 pull HTTP 200: 1 campo`, **`aplicados 1 [heroSizeLocal]`**; no send (no echo) |
| Profile-2 snapshots, both devices | `better_nuvio` **250 B `8f4e1d5814a9bbb5`** on both (was empty). `tv`, `home_catalog_shared` and `mobile` still empty (`9b8a3e00c63ea86f`). Leak check `limpo` |
| BlueStacks restarted, **Profile 1** selected (unarmed) | `perfil 1 pull: 2 campos`, **`aplicados 0`**, 0 sends. Snapshot: `better_nuvio` **362 B `a5db70c709fe8f12` (unchanged)**, `tv` `36dd6d0e82efc08d` unchanged, `home_catalog_shared`/`mobile` unchanged, `limpo` |

**Conclusion:**

* An armed J4/J5 upload on profile 2 wrote only profile 2's `better_nuvio`.
* Profile 1's documents and settings were unaffected. Its Hero size stays at the
  value converged in 17.18; profile 2 is Large.
* Profile 2's `tv`, `home_catalog_shared` and `mobile` stayed empty.
* The other device received the change on profile 2 only, with no echo.

**Final state:** both devices disarmed (J4/J5 and J6 off). Fire Stick is on
profile 2, BlueStacks on profile 1.

### 17.21 J5 Home row shape, BlueStacks → Fire Stick — PASSED (5 November 2026, 17:18–17:27)

You approved one J5 Home test. Profile 1 on both devices (Fire Stick moved from
profile 2 to profile 1 through the chooser, unarmed). Baseline on both:
`better_nuvio` 362 B `a5db70c709fe8f12`, `tv` `36dd6d0e82efc08d`,
`home_catalog_shared`/`mobile` `9b8a3e00c63ea86f`; both disarmed. Evidence:
`build/stage-j7-final/claude/06-*`.

| Step | Result |
|---|---|
| Edit with uploads off (BlueStacks) | Home → Trending Movies card → touch long-press (the remote's long press and Menu key do not open the menu through adb on BlueStacks) → **Row style** → **Automatic → Posters** → OK. Save and Mark as watched were not touched. Next cycle: `1 campo(s) locais pendentes; uploads desligados`; no J6 pending line; Stage I unchanged; 0 sends |
| Arm J4/J5 only | Code read from the notice; account `d21b69ba`, profile 1. `J4/J5 LIGADOS, J6 desligados` |
| Upload (a fresh cycle via restart and the profile chooser; a quick return to the foreground did not start one) | `perfil 1 pull 2 campos`, **`enviando 1 [home_7fe974408a40bdaf2fff23170a97d3a2_s]`**, `push HTTP 204: confirmado`. J6 0/40, Stage I unchanged, no other send |
| Disarm BlueStacks | `DESLIGADOS e inscricao apagada`, `J4/J5 desligados, J6 desligados` |
| Fire Stick receives (unarmed) | In a cycle before its restart: `pull 3 campos`, **`aplicados 1 [home_7fe97440…_s]`**. After the restart: `pull 3 campos`, `aplicados 0` (already equal). 0 sends. Checkpoint slot `home_7fe97440…_s` = 1 (`FIL_TIPO_CARTAZ`) at rev 1. **Its "Trending Movies - Movie" row displays as Posters.** The same row matched across devices even though the two show slightly different row titles |
| Final snapshots, profile 1, both devices | `better_nuvio` **494 B `df8695e6d89776a0`**. `tv` `36dd6d0e82efc08d`, `home_catalog_shared` and `mobile` `9b8a3e00c63ea86f` unchanged. Leak check `limpo`. Both disarmed |

**Conclusions:**

* One J5 Home shape property travelled as a single revisioned field. It did not
  publish the row's default size or visibility (`_z`/`_v` still rev 0, not known)
  and did not replace a row list.
* `home_catalog_shared` (Stage G) was not touched.
* The other device applied the shape to its own copy of the same row, with no
  echo.
* No J6 or Stage I write.

### 17.22 J6 official `tv` setting, BlueStacks → Fire Stick — PASSED with one limitation (5 November 2026, 17:29–17:37)

You approved the J6 test. Profile 1 on both devices; both disarmed. Baseline as at
the end of 17.21 (`tv` 193 B `36dd6d0e82efc08d`). Setting: **Layout → More options
→ Rows → "Add-on name in the row"** (`AJ_NOME_ADDON` ↔
`layout_settings.catalog_addon_name_enabled`, J6 boolean), found through Search
settings. Evidence: `build/stage-j7-final/claude/07-*`.

| Step | Result |
|---|---|
| Edit with uploads off (BlueStacks) | **Off → On** ("Setting saved on this TV"); 0 sends. Next cycle: **`[ajustes-oficial] perfil 1 pendentes com uploads J6 desligados 1 [catalog_addon_name_enabled]`**, the 17.6 diagnostic working on a real device. No J4/J5 or Stage I change waiting |
| Arm **J6 only** (`arm-j6`) | Notice `J7 J6 codigo …`, account `d21b69ba`, profile 1. `uploads J4/J5 desligados, J6 LIGADOS` |
| Upload (fresh cycle through restart and the chooser; arming persisted) | **`[ajustes-oficial] perfil 1 enviando 1 [catalog_addon_name_enabled]`**, then `[sync] ajustes desta TV guardados na conta`. **No `[ajustes-conta] enviando` (Stage I) and no `[bn-sync] enviando` (J4/J5).** Next pull: J6 1/40, Stage I 1/32 (only the earlier `hero_section_enabled`), `better_nuvio` unchanged |
| Disarm BlueStacks | `DESLIGADOS e inscricao apagada`, both scopes off |
| Fire Stick receives (unarmed, profile 1) | J6 **1/40 received**, `aplicados 0`, 0 sends. Its J6 journal shows `catalogAddonNameEnabled` observed = base = desired = 0 (Native `V_LIGA` index 0 = **On**; the J6 codec inverts to the official boolean), known 1, pending 0. **The Fire Stick was already On** (the recommended default), so nothing needed applying. Its J6 state now records the server value |
| Final snapshots, both devices | **`tv` 193 B → 259 B `9dcfd1563588074c`** (one added `layout_settings` entry). `better_nuvio` `df8695e6d89776a0`, `home_catalog_shared` and `mobile` `9b8a3e00c63ea86f` unchanged. Leak check `limpo`: 0 Better keys in `tv` |

**Conclusions:**

* J6 permission is independent. Arming J6 alone uploaded exactly one official
  field to `tv`, merged into the existing document with Stage I's field kept (still
  1/32).
* No J4/J5 write; `home_catalog_shared` and `mobile` untouched.
* The J6 pending line is visible while uploads are off.

**Limitation:** the receiving device's value was already equal, so this run proves
correct reception and no echo, but **not a visible change being applied** on the
second device. Proving that needs the reverse direction (Fire Stick Off → upload →
BlueStacks applies Off), which is another upload and needs separate approval.

**Handling note:** while interpreting the result, Claude printed the Fire Stick's
J6 journal header to the terminal. The header contains the account identifier.
It was not written to any evidence file or document (checked: 0 identifier-like
strings in `claude/*.txt`). Later reads skip the header line.

### 17.23 J6 reverse leg, Fire Stick → BlueStacks — PASSED; 17.22 limitation closed (5 November 2026, 17:35–17:45)

You approved the reverse J6 leg. Profile 1 on both devices; both disarmed at the
start. Evidence: `build/stage-j7-final/claude/08-*`.

| Step | Result |
|---|---|
| Edit with uploads off (Fire Stick) | Menu (Down, Left) → Settings → Search settings "addon name" → **"Add-on name in the row" · On → Off**. 0 sends |
| Unarmed cycle | `J6 1/40 recebidos` (server On), `aplicados 0`, **`pendentes com uploads J6 desligados 1 [catalog_addon_name_enabled]`**. The local edit was kept against the server value (J6 three-way rule: base On, desired Off). No J4/J5 or Stage I change waiting |
| Arm **J6 only** on the Fire Stick | Notice `J7 J6 codigo …`, account `d21b69ba`, profile 1. State file: `armed 1 … 2` (scope 2 = J6). `J4/J5 desligados, J6 LIGADOS` |
| Upload (restart and the chooser; arming persisted) | **`[ajustes-oficial] perfil 1 enviando 1 [catalog_addon_name_enabled]`**, `[sync] ajustes desta TV guardados na conta`. No Stage I or J4/J5 send |
| Disarm the Fire Stick | `DESLIGADOS e inscricao apagada`; 0 `armed` lines |
| BlueStacks receives (unarmed, was On) | `J6 1/40 recebidos`, **`aplicados 1 [catalog_addon_name_enabled]`** (On → Off). 0 sends (no echo) |
| Final snapshots, both devices | **`tv` 259 B → 260 B `f25096afddf566a6`** (`true` → `false`). `better_nuvio` `df8695e6d89776a0`, `home_catalog_shared`, `mobile` unchanged. Leak check `limpo`. Both disarmed |

**Conclusion:** a J6 official setting now travels in both directions. A real value
change was applied on the other device. J6 arming stayed separate from J4/J5, there
was no echo, and the only `tv` change was the one field. The 17.22 limitation is
closed. No account identifier appears in any `claude/*.txt` evidence file.

### 17.24 Standing approval for the remaining tests (5 November 2026, 17:48)

You asked Claude to run the remaining J7 tests without further approvals. Claude
used that as approval to arm J4/J5 or J6 on the dedicated test account (profile 1),
upload, and disarm after each test. It kept the stop-on-surprise rule and two
limits:

* **Account isolation:** not run. It needs signing in to a second account, which
  Claude cannot do (no account creation or password entry).
* **Credentials:** no credential was typed into the app, because that would enter
  a key into a live third-party service. Credential isolation rests on the J7
  automated suite (which uses a fake TorBox key against a mock) and the live leak
  scans, which counted 0 sensitive patterns in every snapshot.

Helper: `build/stage-j7-final/claude/j7c.py`. It reads only filtered, name-only log
lines. It reads activation codes with Codex's on-device OCR tool
(`/tmp/j7-screen-ocr`, macOS Vision) and never prints or saves them. Every arming
step checks that the challenge is for account `d21b69ba`, profile 1, and the
intended scope. Settings changes are made only after the screen shows the intended
row's own description; where OCR could not read a switch, the value was confirmed
from a screenshot.

### 17.25 J5 Home size, Fire Stick → BlueStacks — PASSED (J5 now both directions)

| Step | Result |
|---|---|
| Fire Stick edit (uploads off) | Settings → "Reorder and enable rows" → Trending Movies - Movie → Size column → **Default → Large** (Status/Remove and the move action not touched). `1 campo(s) locais pendentes`, no J6 pending, 0 sends |
| Arm J4/J5 only | `J4/J5 LIGADOS, J6 desligados` |
| Upload | **`enviando 1 [home_7fe974408a40bdaf2fff23170a97d3a2_z]`**, `push HTTP 204` |
| Disarm, BlueStacks receives | **`aplicados 1 […_z]`**, 0 sends |
| Snapshots | `better_nuvio` 494 → **628 B `4bea8937b321df31`** on both; `tv`, `home_catalog_shared` and `mobile` unchanged; `limpo` |

Notes:

* On both devices an `adb` long press arrives as a short press, so the card menu's
  Row style is reachable through `adb` only on BlueStacks (touch). The row editor
  works with the arrow keys on both.
* **UI text finding:** the Home rows editor still says "Applies to this TV and this
  profile only · does not change Home on other devices". Since J5 that is no longer
  true when J4/J5 uploads are on. Recorded for J7c; no code change now.

### 17.26 Different-field conflict (J4) — PASSED

With uploads off, the Fire Stick set **Show synopsis on hero On → Off** and
BlueStacks set **Show year on hero On → Off**; each showed `1 campo(s) locais
pendentes`. Both were armed for J4/J5 only.

| Cycle | Result |
|---|---|
| Fire Stick armed | `enviando 1 [heroSinopseLocal]`, 204 |
| BlueStacks armed | `aplicados 1 [heroSinopseLocal]`, then `enviando 1 [heroAnoLocal]`, 204 |
| Fire Stick armed | `aplicados 1 [heroAnoLocal]`, 0 sends |
| Settle, BlueStacks then Fire Stick | `aplicados 0`, 0 sends each (no ping-pong) |

Both devices were disarmed. Fire Stick checkpoint: `heroSinopseLocal=false`,
`heroAnoLocal=false`, both rev 1, nothing dirty. The BlueStacks Layout screen
shows both Off. `better_nuvio` is **852 B `975ad35e869249f6`** on both; other
documents unchanged; `limpo`. Evidence: `claude/10-*`.

### 17.27 Device-only and consent settings stay local — PASSED (credentials: see 17.24)

* **Edits on BlueStacks:** **Items per row 12 → 18** (THIS TV, device-only; the
  memory warning was confirmed) and **Receive polls On → Off** (consent).
* **Unarmed cycle:** nothing waiting and 0 sends.
* **Then armed both J4/J5 and J6** (`J4/J5 LIGADOS, J6 LIGADOS`) for two fresh
  cycles: **0 sends** in each, J6 0/40 waiting, Stage I unchanged.
* **Disarmed.** The snapshot after is identical to the before snapshot in all four
  documents; `limpo`.
* **Restored** both settings (18 → 12, Off → On) while unarmed.

**Observation:** the Receive polls description says the choice "is kept on your
account". The J1 inventory classes it as per-device consent, and it is in none of
the four settings documents. Any account storage would be through a separate
existing mechanism outside J7 scope; nothing reached `tv`, `better_nuvio`,
`home_catalog_shared` or `mobile`.

**Side event:** one search attempt on BlueStacks opened the **Interface mode**
picker by mistake. It was closed with Back without confirming; the value stayed
Automatic (confirmed on screen).

### 17.28 Final regressions and state — J7b live testing complete except account isolation

* **Regressions:** all 41 suites pass, including J7 (plain and UBSan, with the probe
  and scope steps), `bn_j0probe`, J4/J5/J6, Stage I, settings A–D, the 413-binding
  preservation check, Home rows, TorBox and stream filtering, Continue Watching,
  `j0sonda` and `deeplink` (`claude/12-regressions.txt`).
* **Final profile-1 documents (both devices identical):** `tv` 260 B
  `f25096afddf566a6`; `better_nuvio` 852 B `975ad35e869249f6`;
  `home_catalog_shared` and `mobile` empty `9b8a3e00c63ea86f`. Leak check `limpo`.
  Profile 2 is unchanged since 17.20 (`better_nuvio` `8f4e1d5814a9bbb5`); no
  profile-2 send has occurred since.
* **Both devices disarmed.** Fire Stick state file has 0 `armed` lines; BlueStacks'
  last gate line reads `J4/J5 desligados, J6 desligados`. Both still run the approved
  APK `b73a5563…7420b` (pulled and hashed).
* **Account IDs:** none in any `claude/*.txt` evidence file.
* **Working tree:** the same 108 paths on `codex/settings-restoration`; no source
  change in J7b after the APK build. No commit, push or merge.

| J7b scenario | Result |
|---|---|
| Read-only restore, snapshots, default off | Passed (17.1, 17.5, 17.8, 17.13) |
| A → B and B → A, J4 | Passed (17.11, 17.13–17.14) |
| Offline edit, restart, reconnect (unarmed and armed) | Passed (17.15, 17.17) |
| Same-field conflict | Passed (17.18) |
| Different-field conflict | Passed (17.26) |
| Profile isolation | Passed (17.20) |
| J5 Home, both directions | Passed (17.21, 17.25) |
| J6, both directions, separate permission | Passed (17.22–17.23) |
| Device-only and consent stay local | Passed (17.27) |
| Credentials stay local | Automated suite and live leak scans only (17.24) |
| Account isolation | **Not run**: needs a second test account signed in by you |

**Findings for J7c (no code changed):**

1. Debug links, including `disarm`, wait until a profile is chosen (17.19).
2. The Home rows editor text still says changes do not reach other devices (17.25).
3. A quick return to the foreground does not always start a sync cycle; a restart
   does.
4. Older `[sessao]`/`[perfis]` log lines print the account ID (pre-existing).
5. The J6 journal header holds the account ID, which tools must skip (17.22).

### 17.29 Account isolation — stopped before any Account B access (5 November 2026)

* **Set-up.** You signed BlueStacks in to "Account B" and kept Account A (the
  dedicated test account, `d21b69ba`) on the Fire Stick.
* **Stopped before any Account B access.** BlueStacks was waiting at Account B's
  profile chooser, which lists six profiles with personal names. That suggested a
  real household account, so Claude stopped before choosing a profile, taking
  snapshots, syncing or arming, and asked. **You confirmed Account B is not a test
  account and said stop.**
* **No Account B document was read or written by J7**, and no profile was chosen,
  so no sync cycle ran for B. BlueStacks' log shows Account A's session was ended
  and deleted from the device before B signed in. The J7 enrolment had already been
  cleared and both scopes were off.
* **Fire Stick (Account A)** is unchanged: profile 1, disarmed, last snapshot
  `better_nuvio` `975ad35e869249f6`, `tv` `f25096afddf566a6`.
* **Handling note.** While checking session state, Claude printed one app log line
  (`[sessao] logado por e-mail como …`) that contains Account B's account ID; the
  redaction filter only covered IDs in brackets. It appeared in Claude's terminal
  output only, in no evidence file or document (0 ID-like strings in `claude/`).
  The helper now also filters bare IDs. Profile names seen on screen are not
  recorded.
* **Account isolation remains untested live.** It needs BlueStacks (or another
  device) signed in to a second **dedicated test account**. The mock suite
  (`settings_stage_j7` `a-account` step) still covers account switching. Signing
  BlueStacks out of Account B is left to you.
* Re-check after the request was sent again: BlueStacks still has the **same**
  Account B session (only one sign-in, 18:16:47; fingerprint `db5b47a6`, ≠ Account
  A `d21b69ba`). No profile has been chosen and no sync has run for it
  (`ciclo interrompido: 6 perfis e nenhum escolhido`). Not run again, for the reason
  above.

### 17.30 Account isolation — read-only part done; stopped before arming (5 November 2026)

You then confirmed in chat that Account B is a dedicated test account. Claude ran
only the read-only steps.

* **Identities** (hashed on the Mac, never printed): Fire Stick = **`d21b69ba`**
  (Account A, profile 1, 0 `armed` lines). BlueStacks = **`db5b47a6`** (Account B,
  one sign-in, 6 profiles).
* **First Account B cycle** (profile 1 of 6, unarmed):
  * `better_nuvio` pull 200, **0 fields**, so none of Account A's settings are
    present;
  * `tv` 40/40 J6 and 31/32 Stage I received; the device applied 4 of **B's own**
    J6 values locally;
  * **no J4/J5 or J6 pending line and 0 sends**, so B did not inherit Account A's
    pending changes.
* **Snapshots:**

| Namespace | Account A (Fire Stick, p1) | Account B (BlueStacks, p1) |
|---|---|---|
| `tv` | 260 B `f25096afddf566a6` (unchanged) | 60,789 B `4eb9b2da97a72c43` |
| `better_nuvio` | 852 B `975ad35e869249f6` (unchanged) | empty `9b8a3e00c63ea86f` |
| `home_catalog_shared` | empty | empty |
| `mobile` | empty | 23,029 B `f194cba4aacdf77b` |
| Leak summary | `limpo` | **`chaves Better no tv 1 - VAZAMENTO`** (0 foreign fields, 0 sensitive patterns) |

* **Stopped before arming**, for two reasons:
  1. **The leak flag.** One J4 key name appears in Account B's `tv` document. This
     build wrote nothing to B (0 sends), so the likely source is a historical
     pre-Stage-I Native device snapshot (`c_settings_v1`) written by an older build.
     This is not verified, because verifying it means reading B's document content.
  2. **Account B's documents look like a heavily used real account** (a 60 KB `tv`
     and 23 KB `mobile`, six named profiles), which contradicts the earlier
     answer that it is not a test account.
* **Isolation result so far:** across accounts, no settings were applied and
  nothing was inherited. The armed upload step was not run. No write to either
  account. Profile names are not recorded. 0 ID-like strings in `claude/*.txt`.
  Evidence: `claude/13-*`.

### 17.31 Account isolation — PASSED (upload on Account A, Account B as observer)

You chose "upload to A": the controlled J4/J5 upload was made on **Account A**
(dedicated test account, Fire Stick), and **Account B** (BlueStacks) stayed
unarmed as the observer. No write was made to Account B. Evidence:
`claude/14-*`, `claude/13-*`.

| Step | Result |
|---|---|
| A edit with uploads off | **Show synopsis on hero Off → On** (row focused by its own description; value confirmed on screen). Unarmed cycle: `1 campo(s) locais pendentes`, no J6 pending, 0 sends |
| Arm J4/J5 on A only | Challenge verified for `d21b69ba`, profile 1, J4/J5 scope. `J4/J5 LIGADOS, J6 desligados` |
| A upload | **`enviando 1 [heroSinopseLocal]`**, `push HTTP 204: confirmado`. No J6 or Stage I send |
| Disarm A | Confirmed; 0 `armed` lines |
| B (unarmed, profile 1, fresh process) | `uploads J4/J5 desligados, J6 desligados`; **`better_nuvio` pull 200: 0 fields**, `aplicados 0`; J6 40/40 and Stage I 31/32 are B's own, all already equal; **0 sends** |

| Namespace | Account A before → after | Account B before → after |
|---|---|---|
| `tv` | `f25096afddf566a6` → same | `4eb9b2da97a72c43` (60,789 B) → **same** |
| `better_nuvio` | `975ad35e869249f6` → **`fdb3931afeb0a1c9`** (851 B) | empty → **empty** |
| `home_catalog_shared` | empty → same | empty → same |
| `mobile` | empty → same | `f194cba4aacdf77b` (23,029 B) → **same** |
| Leak summary | `limpo` | `chaves Better no tv 1 - VAZAMENTO`, **already present before** (17.30) and unchanged |

**Conclusions:**

* Only Account A's `better_nuvio` changed.
* Account B's four documents are byte-identical, and B received none of A's
  Better settings, before or after A's upload.
* B inherited none of A's pending changes (17.30), and there was no
  cross-account upload. Account isolation is now verified live (A→B direction),
  alongside the mock suite's `a-account` step.

**Open item:** Account B's `tv` contains one Better Nuvio key name. Its probable
source is a pre-Stage-I Native `c_settings_v1` device snapshot written by an older
build, but that is not verified, because verifying it means reading B's document.
J7 did not cause it (B had 0 sends throughout). Whether to clean it up is your
decision; Claude has made no change.

Final state: both devices disarmed. Fire Stick on Account A profile 1; BlueStacks
on Account B profile 1, unarmed. 0 ID-like strings in `claude/*.txt`; profile
names not recorded.

### 17.32 Local cleanup after Claude handover — 5 November 2026

Inspected the current `codex/settings-restoration` tree and Claude's sections
17.20–17.31 and follow-up notes; preserved all prior work. Historical live J7b
passes were not repeated. Fire Stick's private local arming file had zero armed
entries. BlueStacks was not accessed because its current account may be the main
account. No backend snapshots, account reads/writes, live provider operations,
arming, installation, commit, push, merge or master change were performed here.

#### Privacy and journal migration

- Preserved Claude's fingerprint-only `[sessao]` and `[perfis]` logs and the
  header-only `sessao_impressao`. Removed an orphaned function-body fragment left
  in `sessao.c` by the interrupted edit; it prevented compilation.
- J6 writes `v2 <64-bit-owner-hash> <profile>` journals. Valid matching legacy v1
  journals retain pending edits and rewrite to v2 when their profile is opened.
  Unopened private legacy files are not globally scanned/deleted. Existing J4
  private checkpoint formats remain unchanged.
- Strengthened header validation: wrong owner/profile, invalid/overlong hash,
  unknown version and trailing header fields are rejected without changing the
  in-memory pending state. Isolated migration/rejection tests pass.
- Stage B's frozen `perfis.c` contract failed on the reviewed privacy-only printf
  change. Its guard now normalises only that exact line before checking the
  existing hash; the rest of Stage G's contract remains frozen. No blanket
  baseline replacement was made.
- Preserved Claude's translated Home-row editor message, conditional on actual
  J4/J5 write permission; the 29-language consistency check passes.

#### Home layout ownership and compatibility

Added only `AJ_HOME_LAYOUT` / `homeLayoutLocal` to the Better allowlist: now
**91 controls = the original 90 J4 controls plus one reviewed extension**.
Wire value is the existing enum: 0 Modern, 1 Default, 2 Dynamic. Storage ID, key,
default and Native rendering consumers are unchanged.

The owner is exclusively `better_nuvio`, selected account/profile. No mapping to
`tv`, `selected_layout`, MODERN/CLASSIC/GRID or a new server schema was added.
Dynamic has no lossless official equivalent. This synchronises Better Nuvio's
presentation between Better devices; it does not claim regular Nuvio can render
Dynamic. The approved J1 inventory is retained unchanged as historical evidence;
the explicit exception is recorded in `stage-j4-bindings.json` and the J6 audit.
Other existing J6 decisions/exclusions, 32 Stage I settings, J5 fields and all
413 original bindings remain preserved.

Mock evidence includes all three layout values, offline edit/restart/recovery,
remote apply, profile-2 isolation and return, no default/echo uploads, retained
unrelated official fields, and an unchanged official `tv` fingerprint. A 90-slot
checkpoint expands to 91 without changing prior slots/pending edits or marking
the new baseline dirty. Upload permission remains default Off.

#### Debug revocation before profile selection

Added a mutex-protected early dequeue for **disarm only**. The app processes it
before login/profile early returns, without waiting for a network/title job.
Arm, arm-j6, snapshots and normal deep links retain their existing gated route.
Host/debug parser-routing tests verify one-time revocation and retention of all
other queued requests. This does not bypass activation checks or enable writes.
New-device behaviour still requires installation approval and live verification.

### 17.33 Cleanup validation and APK — 5 November 2026

- **41/41 relevant regression suites pass**, including J4/J5/J6/J7 (J7 also UBSan),
  Stage I, settings A–D and interaction tests, Stage G, TorBox cached-addition,
  verified-uncached filtering, Continue Watching and 413-binding preservation.
  The initial Stage B guard failure and successful rerun are retained separately.
- Language tables: **6,372 entries × 29 languages**, consistency check PASS.
- Android JVM: **61 tests, five suites, zero failures/errors/skips**. The missing
  pinned JUnit dependency was fetched from configured Maven repositories; these
  were build-dependency requests, not account/provider operations.
- Debug APK build: PASS with existing local SDK/NDK/JDK/Rust and environment-only
  path corrections. The build itself ran offline. No local.properties, signing
  configuration or dependency versions changed. The optional P2P engine remains
  absent on this Mac, as in the preceding APK; it was not silently removed.

| Item | Verified value |
|---|---|
| APK | `build/android/Nuvio-2.0.8-android-debug.apk` |
| Size | 74,567,092 bytes |
| SHA-256 | `a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad` |
| Application ID | `space.nuvio.nativelegacy.strictdebug` |
| Signing certificate SHA-256 | `c0a433a85a1e3ca1fa9e1ef4b5998b36444b0cc8efc88c9a3ccb97d2f7c59bba` (unchanged) |
| Architectures | ARM64 and ARMv7; six libraries each, same set as previous APK |
| Assets | 1,129; all previous asset paths retained |
| TV launcher | LEANBACK_LAUNCHER present |
| Alignment | ZIP 16 KB check PASS; all six ARM64 ELF libraries support 16 KB LOAD alignment |

Both packaged `libmain.so` files match the allocated code/data sections of the
unstripped outputs. Their verified `jbMap` has 91 entries, ending in original
binding 137 (`AJ_HOME_LAYOUT`), and includes the early-disarm function. This is
stronger evidence than searching for a pre-existing Home-layout label string.
The prior approved `b73a5563…7420b` APK was preserved in
`build/stage-j-cleanup/pre-cleanup-b73a55636764.apk`.

Evidence: `build/stage-j-cleanup/regressions-final.json`, per-suite logs,
`android-unit-counts.json`, `apk-verification.json`, `packaged-source-manifest.json`.
Logs are filtered to omit configuration-sensitive build lines. Nothing installed.

#### Remaining limits and next action

Local Stage J cleanup and targeted build verification are complete. The new
Home-layout binding and privacy/early-disarm changes have **not** been validated
live on this APK. Recommended next step: obtain approval to install this exact
checksum on devices signed into confirmed dedicated test accounts, then separately
approve only the required sync scope. Do not use the main account's documents.

A quick foreground return can still wait for an existing worker cycle; restart
was the reliable live-test trigger. No protocol/timer redesign was introduced.
LG webOS/Tizen and this new APK's physical-device focus/rendering remain untested.

Claude's separate 1,141-script sweep finished with a summary of 717 failures;
that broad run is **not** a clean result. Its per-script result ledger was not
located in the project, so the failures cannot all be attributed to the interrupted
`sessao.c` edit or declared fixed. The current 41-suite focused gate above is the
verified result, not a claim that all 1,141 scripts pass. Triage/re-running that
broader sweep safely remains separate work; do not run unknown live-backend scripts.

Stage E and H requirements were reviewed in `NEXT-STAGES-REVIEW.md`; neither stage
was implemented. No code/date policy or visual redesign was started for either.

### 17.34 Broader failure triage completed — 5 November 2026

Recovered Claude's original scratchpad runner, summary and per-test logs. This
supersedes the earlier statement that the detailed ledger could not be located.
Original result confirmed: **1,141 runs, 424 passed, 717 failed**. Classified every
failure in `BROAD-FAILURE-TRIAGE.json`; analysis and backlog are in
`BROAD-FAILURE-TRIAGE.md`.

First blocking causes: 399 interrupted-session-source compiler failures, 111
missing-link-dependency cases, 38 other compiler/tool cases, 15 assertion cases
and 154 runtime/harness/prerequisite cases. The eight-way sweep also included
visual captures, external-volume paths and unavailable SDK/tool prerequisites;
it is not an unattended APK release gate. Removing the session syntax blocker
is not a claim that all 399 individual tests now pass.

**Controlled verification:** 41 representative tests × current/checkpoint trees
= **82 runs**, using an archive of unchanged `2bb366167` and no branch switch.
Every subprocess was OS-sandboxed with networking denied, with temporary test
storage. No account/provider backend or device was accessed.

Only two representatives changed from checkpoint PASS to current FAIL:

- `perfilsel`: the small fixture lacked Stage G's Home-projection dependency.
  Added a controllable test stub and meaningful own-profile/primary-fallback
  scope assertions. PASS plain and ASan/UBSan. Production code unchanged.
- `ajustes_sync`: deliberately obsolete device-snapshot protocol expectations;
  do not restore `c_settings_v1` account uploads. Current Stage I/J4–J7 mocks are
  the applicable protocol gate. This legacy test remains non-green and identified.

Also repaired the offline Android TS harness's JDK discovery and arbitrary cache
jar selection (Hamcrest javadoc was being treated as executable code). It now
honours existing environment paths and filters source/javadoc jars; **3/3 JVM
fixture tests PASS with networking denied**. No dependency version changed.

The 11 remaining original assertion failures reproduce on the checkpoint too.
Four sampled small renderer crashes also reproduce on both trees. Other sampled
link, stale-source-contract and toolchain failures are not hidden as passes.
Discord/Guide cache/Guide filtering pass in controlled reruns. Whole-app i18n
still has older missing keys plus debug-notice translation debt; the consistency
and settings English gates do not imply that broader scanner is green.

**Outcome:** triage complete; the whole 1,141-script sweep is **not green**.
Separate legacy-harness/platform/localisation maintenance remains. Focused
41-suite and 61-test Android results remain valid; production source hashes and
APK bytes did not change. The new test-only profile/TS fixes, Home profile
regression and 413-binding preservation checks pass. No APK rebuild is needed.

Dedicated-device validation is prepared in `DEDICATED-DEVICE-VALIDATION.md`.
First confirm dedicated accounts, then obtain installation-only approval for
`a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`.
App launch/arming/backend writes require a separate explicit scope. Existing
Stage I automatic uploads are disclosed; an online launch is not promised to
be unconditionally read-only. BlueStacks/main-account documents were not read.
No Stage E/H changes, installation, arming, backend writes or Git publication.

### 17.35 Approved installation completed — 5 November 2026

User confirmed both strictdebug apps use dedicated test accounts with profiles
1 and 2 available, and explicitly approved installation **only** of checksum
`a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`.
No live launch/synchronisation approval was included.

- Verified local APK checksum, connected device models and existing strictdebug
  package before replacement. BlueStacks was reconnected at its existing local
  ADB endpoint. The Xiaomi TV was not used.
- Force-stopped the target apps before updating; used data-preserving replacement
  installs. No uninstall, data/cache clearing, storage cleanup or setting changes.
- BlueStacks installation succeeded. Fire Stick initially returned
  `INSTALL_FAILED_INSUFFICIENT_STORAGE` (about 672 MB reported free); one retry
  using non-streamed replacement succeeded. Nothing was removed to obtain space.
- Both installed base APK hashes exactly match the approved checksum. Package is
  `space.nuvio.nativelegacy.strictdebug`, versionCode 20008,
  versionName `2.0.8-strictdebug`. Exact byte identity also confirms the previously
  verified signing certificate, assets and native implementations.
- Neither app is running after installation. No app launch, activation/snapshot
  link, sync arming, account/profile change or backend access/write was performed.
  This is installation evidence, **not** a live Home-layout/privacy/disarm pass.

Evidence: `build/stage-j-cleanup/install-only/results.json`, `fire-retry.json`,
`final-verification.json`. Existing app data was retained via replacement; no
private credential files were read. No Git commit/push/merge/master changes.

Next: separate approval for app launch/live sync validation. Confirm the devices
use the **same** dedicated test account for cross-device Home-layout tests;
installation confirmation alone did not establish that. Existing Stage I automatic
sync must be included in the agreed launch scope; do not promise an unconditional
read-only online launch. J6 need not be armed for these Better-only layout tests.

### 17.36 Final live-validation read-only preflight — partial, 5 November 2026

User authorised dedicated-test-account read-only validation, explicitly requiring
separate approval before J4/J5 activation **or any backend writes**, with J6 off.

- Fire local identity matches the known test account; profile 1, zero armed
  entries and zero pending Stage I/J6 fields. Blue was already running.
- Corrected Blue log attribution: it switched accounts **within the same process**.
  Earlier profile/sync lines predate its latest test-account login and were
  incorrectly included in initial summaries. They are now excluded/superseded.
  Latest session identity matches Fire's test account; Blue's current profile and
  on-device test-account snapshot are **not independently verified yet**.
- Force-stopped Blue for safety. Both apps are now stopped. No activation or
  setting-edit commands were sent, and no backend write was performed.
- **Automatic approval review rejected online launch/app-triggered snapshots**:
  existing Stage I can upload independently of both debug permission gates.
  The rejected command did not execute. It was not retried through another
  lifecycle route.
- A safer independent reader invoked only source-verified read RPCs with Fire's
  existing, privately checked test-account access token; no app worker, token
  refresh, write RPC or raw document persistence. It confirmed account profiles
  1/2 and profile-1 fingerprints: `tv` `f25096afddf566a6` (260 B),
  `better_nuvio` `fdb3931afeb0a1c9` (851 B), home/mobile empty
  `9b8a3e00c63ea86f`. All HTTP 200. Better has six fields, no Home-layout field.
- Existing logs show no raw identities/tokens in the checked current-account
  session/profile lines. Fire J6 p1 is hash-only v2; p2 remains legacy v1 in
  private storage, with no pending edits. **Live migration is incomplete** until
  that profile is safely opened; no private legacy file was injected/rewritten.

Prepared both-direction Modern/Default/Dynamic tests and acceptance criteria in
`HOME-LAYOUT-LIVE-VALIDATION.md`. Foreground lifecycle tests remain pending.
Next choice: prepare a strictly read-only test build, or explicitly authorise
existing Stage I automatic settings sync on the dedicated account for online
preflight. J4/J5 arming still requires its own agreed test scope; J6 remains off.
No main-account backend reads, provider operations, source/build changes,
installation, commit/push/merge/master changes occurred during this preflight.
Evidence: ignored `build/stage-j-final-live/`; use latest-account-only Blue files.

### 17.37 Debug-only read-only validation artifact — 5 November 2026

User explicitly chose a read-only validation build rather than approving Stage I
automatic uploads. Existing uncommitted work and the prior approved APK are preserved.

Implemented an opt-in `NUVIO_VALIDATION_READONLY=1` debug-only build variant.
The Nuvio account boundary permits exact source-verified read RPCs and addon/plugin
GETs; all other RPCs, auth/refresh, edge functions and addon PATCH are denied
before networking with local temporary status 503 and no success acknowledgement. Native HTTP mutation paths
also deny non-allowlisted POST/PUT/PATCH/DELETE ahead of curl/Android fallback.
J4/J5 and J6 activation is refused; retained permissions cannot become effective;
disarming remains available. Ordinary build policy is unchanged. See
`READ-ONLY-VALIDATION.md` for scope, limitations and reproduction.

- Guard tests: ordinary/read-only transport boundaries and release rejection PASS.
- Real native HTTP/permission mock harness PASS, including retained permission
  entries, activation refusal, disarming and POST/PUT/DELETE/PATCH/OAuth refusal.
- Android unit tests: **61/61 PASS**, no skips/failures/errors, offline.
- Android build PASS; ARM64 objects were rebuilt after the final guard changes;
  both architectures' guarded objects are newer than their source/header inputs.
- Verified signature/certificate unchanged, strictdebug identity unchanged, TV
  launcher, ZIP 16 KB alignment, ARM64 ELF 16 KB alignment, 12 native libraries,
  1,129 assets, 91 Better bindings including Home layout, early disarm symbol and
  exact allocated native sections matching the packaged libraries.
- An additional HTTP harness had a callback-name collision already present at
  HEAD; renamed only its test helper. Native HTTP/TLS and buffer checks PASS;
  the two following web JavaScript checks cannot run because the Node CLI is unavailable (the bundled REPL forbids their dynamic
  Function construction).
  Android HTTP fallback mock PASS. Focused regression matrix **41/41 PASS** (ordinary policy), covering Stage G/I,
  settings A–D, TorBox/autoplay/filtering, Continue Watching and preservation.

APK: `build/android/Nuvio-2.0.8-android-readonly-validation.apk`
Size: **74,565,640 bytes**. SHA-256:
`573b0192a787a6370e04b3ff0d4d604dc3640d74cf9f641a5c48197ac4e4d10e`

No installation, app launch, activation, backend request/write or main-account
access during this build session. Device-state evidence remains the preceding
partial preflight; do not count the host tests as live migration/lifecycle passes.
J6 remains OFF. Installation of this new checksum needs separate approval; later
Home-layout outbound tests need a writable build and separately approved J4/J5
activation/writes. Session expiry cannot refresh in this read-only build.

Evidence: ignored `build/stage-j-readonly/` (filtered build logs, source
fingerprints, APK/native verification and isolated tests). Existing approved
`a9cd562e…aad5ad` artifact retained there. No commit/push/merge/master changes.
Stage E/H remain deferred.

Final read-only artifact checks: **blocked refresh preserves the stored session
byte-for-byte and keeps the account logged in**. Local denials deliberately use
503 rather than an authentication rejection, so they cannot cause the existing
refresh code to erase credentials. Both devices were observed connected with
their apps stopped. No install or live account operations occurred. Final
evidence: `build/stage-j-readonly/FINAL.json`. The separate broad legacy-test
maintenance backlog is not declared resolved by these focused passes.

### 17.38 Read-only live checks completed on both devices — 5–6 November 2026

Claude took over from Codex. Codex had already installed the read-only APK
(`573b0192…4d10e`, data kept) on both devices at 23:13 and captured only startup
logs (both apps idle at the profile chooser). Claude did not reinstall; it
verified the installed bytes on both devices match the approved SHA-256.

All checks were read-only: no arming, no settings edits, no backend writes. The
only backend contact outside the app was Codex's allowlisted read-RPC probe
(`build/stage-j-readonly-live/backend.py`), which asserts the test-account
fingerprint before reading.

| Check | Result |
|---|---|
| BlueStacks account and profile | Test account `d21b69ba` (same as the Fire Stick); profiles 1 and 2 opened and pulled normally; both upload gates OFF |
| Stage I write blocked | BlueStacks profile 1 tried to upload one pending local edit `hero_section_enabled`; refused locally with HTTP 503. No other send on either device |
| Fire Stick profiles 1 and 2 | Both opened and pulled normally; 0 sends; gates OFF |
| Fire profile 2 J6 journal | Migrated **v1 → v2** on first open: still 40 rows, row hash unchanged (`e6fb407e…`), raw ID removed, 0 pending |
| Foreground refresh (both devices) | Return after 5 s: skipped by design ("conta lida ha 38/54 s, sem nova leitura", 60 s minimum). Return after 75 s: full re-read, 0 sends |
| Privacy | Full Fire log (484 lines): 0 UUIDs, JWTs, bearer headers, emails or activation codes. BlueStacks: the Nuvio lines captured contained none; the only ID-shaped strings in the full logcat belong to other apps' WorkManager/Glean logs |
| Backend before/after | All 8 profile×namespace documents byte-identical (same fingerprints); `homeLayoutLocal` still absent; 0 write RPCs, no token refresh |
| Close-down | Disarm link sent on both, apps force-stopped; Fire arming file has 0 armed lines |

Notes:

- `get_sync_owner` is not on the read-only allowlist, so the app falls back to the
  token subject (logged, harmless).
- The adb server dropped both devices mid-session; restarting it restored them.
- **Pending local edits that a writable build would upload:** BlueStacks profile 1
  `hero_section_enabled` (Stage I, from the earlier mistaken Hero toggle) and Fire
  profile 2 local settings logged as "tinha ajustes desta TV sem subir: mantidos".
  These must be resolved or explicitly accepted before any writable test.

Evidence: `build/stage-j-readonly-live/claude/` (filtered logs, journal
summaries, privacy scan) and `build/stage-j-readonly-live/backend-after.json`.
No commit/push/merge/master changes.

**Broad sweep (717 failures) — rerun and baseline comparison.** Claude reran all
717 originally failing scripts on the current tree (with the `sessao.c` compile
fix), then ran every script that still failed on a baseline copy with today's
privacy/UI edits reverted:

| Outcome | Count |
|---|---:|
| Now pass on the current tree (306 had been masked by the interrupted `sessao.c` edit; 5 others) | 311 |
| Still fail, and fail identically on the baseline (pre-existing legacy/environment failures) | 405 |
| Failed only under parallel load (`discord_runtime`, 250 ms timing assertion); passes 3/3 alone | 1 |
| **Regressions caused by today's changes** | **0** |

By Codex's categories, the 405 remaining are: 110 link-fixture, 38
compile/tool, 151 runtime/harness/prerequisite, 13 assertion, and 93 that had
been hidden by the compile error. Examples: tests asserting old Settings paths
renamed in Stage A, screenshot tests needing `/Volumes/ExternalSSD` or failing
image saves, and subset links missing newer symbols. These are legacy
maintenance work, not release blockers for Stage J. The overall sweep is still
**not green**. J7 suite including the `privacy` step: PASS, both plain and with
ASan/UBSan. Ledgers: the session scratchpad's `reg-rerun/` and `reg-base/`.

**Pending Hero edit discarded locally (6 November 2026).** At the user's request
("Discard it locally"), BlueStacks profile 1's pending Stage I edit
`hero_section_enabled` was discarded rather than uploaded. Show hero was On
locally while the account value is Off; one tap set it Off (Settings → Layout).
No other value on the page changed; Home layout on BlueStacks reads **Dynamic**.
A fresh profile 1 cycle then logged "ja iguais 1; edicao local pendente 0" with
0 sends. Backend documents are still byte-identical to the pre-check snapshot.
App disarmed and stopped. Fire Stick profile 2's local-only settings were not
touched. Evidence: `build/stage-j-readonly-live/claude/blue-p1-after-discard.txt`,
`build/stage-j-readonly-live/backend-after-discard.json`.

**Fire Stick profile 2 "local settings" identified (6 November 2026, read-only).**
The log line "perfil 2 tinha ajustes desta TV sem subir: mantidos" comes from
`sync_trocar_perfil` (`src/sync.c:2273`). It is a whole-profile protection flag
(`ajustes-locais.txt`) that stops the account's `tv` copy overwriting settings
changed on this TV. It is not a list of queued uploads. Uploads come only from
the per-field journals, read on the Fire Stick by name and flag only:

| Tier | Profile 1 pending | Profile 2 pending |
|---|---|---|
| Stage I (`settings-shared`, 32 fields) | 0 | 0 |
| J6 (`settings-official-j6`, 40 fields, v2) | 0 | 0 |
| J4/J5 (`bn-checkpoint`, 176 slots) | 0 dirty | 0 dirty |

Profile 2's local values differ from profile 1 in three settings (left over from
the earlier profile-isolation test):

- `heroSizeLocal` (J4): matches the account's profile 2 field, so already synced.
- `heroAnoLocal` (J4): no account value and not dirty, so it stays local.
- `catalogAddonNameEnabled` (J6 `layout_settings.catalog_addon_name_enabled`):
  no account value and not pending, so it stays local.

**Nothing on the Fire Stick would upload under a writable build.** The protection
flag has no practical effect because profile 2's account `tv` document is empty.
Fire Stick Home layout (`homeLayoutLocal`) is local value 1 on both profiles,
never synced (not known, not dirty). No files were changed. The checkpoint's
`device` field (a local install ID, not an account ID) was printed once to the
terminal during inspection and was not saved.

**Writable APK reinstalled, install only (6 November 2026).** The user approved
"Install only". The preserved approved APK `build/stage-j-readonly/approved-a9cd.apk`
(SHA-256 `a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`,
74,567,092 bytes) replaced the read-only build on both the Fire Stick and
BlueStacks. Models and the existing package were confirmed first, and app data
was kept (`install -r`, no uninstall). The installed bytes match on both devices,
version is 20008 / `2.0.8-strictdebug`, and both apps remain **stopped**. The
Fire Stick's 82 data files are intact, including the session and both v2 J6
journals with 0 pending. No launch, arming, settings change or backend
operation. Note: `build/android/Nuvio-2.0.8-android-debug.apk` currently holds
the read-only build's bytes (`573b…`), so do not use it as the writable
artifact. Evidence: `build/stage-j-writable-install/results.json`.

**Launching this build turns Stage I automatic settings sync back on** for the
test account. That isn't gated by J4/J5/J6. With 0 pending edits on both devices
it should upload nothing on its own. The Home-layout test still needs approval to
launch and for J4/J5 arming in each direction.

**Home layout Fire Stick → BlueStacks: PASSED (6 November 2026).** The user
approved "Start Fire → Blue". Both devices ran the writable `a9cd562e…` build on
test account `d21b69ba`, profile 1.

1. Both devices opened profile 1 with 0 pending in every tier and 0 sends.
2. J4/J5 armed on the **Fire Stick only** (profile 1, 4 h, code read on-device).
   J6 stayed off; BlueStacks was never armed.
3. Fire Stick Settings → Layout → Home layout changed **Default → Modern**
   (`homeLayoutLocal 1 -> 0`).
4. Fire Stick uploaded exactly one field: "perfil 1 enviando 1 [homeLayoutLocal]",
   "push HTTP 204: confirmado". No Stage I or J6 upload.
5. Fire Stick disarmed (0 armed lines).
6. BlueStacks, on returning to the foreground: "pull HTTP 200: 7 campo(s)
   recebidos", "aplicados 1 [homeLayoutLocal]", **0 sends (no echo)**. Its Home
   switched to the Modern layout, and Settings → Layout shows **Modern** (was
   Dynamic). Other values unchanged.
7. Backend: only profile 1 `better_nuvio` changed (851 → 960 bytes, 6 → 7 fields,
   `homeLayoutLocal` present). Profile 1 `tv`, `home_catalog_shared`, `mobile`
   and all of profile 2 are byte-identical.

Both apps were then stopped. Observations:

- After the reinstall, the Fire Stick was asleep (black screen, app paused at
  once). A wake key fixed it. `monkey` launching also failed there, so launches
  now use `am start -n …/.Icone_original`.
- The Home layout detail panel still says "Applies to: This profile only, on this
  TV", although the setting now syncs. That's a UI label to correct.

Evidence: `build/stage-j-writable-install/` (`fire-upload.txt`,
`blue-receive.txt`, `backend-fb-before.json`, `backend-fb-after.json`).

**Home layout BlueStacks → Fire Stick: PASSED (6 November 2026).** The user
approved "Run Blue → Fire". Same build, test account and profile 1.

1. Both devices opened profile 1 in step at 7 Better fields, 0 pending, 0 sends.
2. J4/J5 armed on **BlueStacks only** (profile 1, 4 h). J6 off; the Fire Stick
   was not armed.
3. BlueStacks Settings → Layout → Home layout changed **Modern → Dynamic**
   (`homeLayoutLocal 0 -> 2`).
4. The upload ran on the next sync cycle, triggered when Settings was closed:
   "enviando 1 [homeLayoutLocal]", "push HTTP 204: confirmado". Exactly one
   field; no Stage I or J6 upload. (While Settings stayed open for about 90 s,
   no cycle ran.)
5. BlueStacks disarmed ("J4/J5 desligados, J6 desligados").
6. Fire Stick, on returning to the foreground: "pull HTTP 200: 7 campo(s)
   recebidos", "aplicados 1 [homeLayoutLocal]", **0 sends (no echo)**. Settings →
   Layout shows **Dynamic**. Fire Stick arming file: 0 armed lines.
7. Backend: only profile 1 `better_nuvio` changed (value only; still 960 bytes,
   7 fields). Every other document is byte-identical.

Both apps were stopped afterwards. **The bidirectional Home layout sync test is
complete.** Final state: both devices on Dynamic, profile 1. Side note:
"Dynamic layout background" differs between the devices (Fire Stick Gray,
BlueStacks Dark gray). It's a separate setting, not part of this test.

Evidence: `build/stage-j-writable-install/` (`blue-upload.txt`,
`fire-receive.txt`, `backend-bf-before.json`, `backend-bf-after.json`).

**Home layout "Applies to" label fixed (6 November 2026, local only).** The
Settings detail panel took its scope label only from the static scope table, so
Home layout (and any other J4-bound setting) read "This profile only, on this TV"
even while it was syncing. Change:

- `src/ajustes_better.inc`: new `ajustes_better_op_vinculada(op)`, true when the
  option is one of J4's Better Nuvio bindings. It's declared in `src/ajustes.h`.
- `src/ajustes_ux_dados.inc` `uxEscopoValor`: a J4-bound option now reads
  "Account/profile" (`Conta/perfil`) **only while `ajustes_better_envia()` is
  true**, meaning this profile's Better Nuvio uploads are actually on. That's the
  same rule as the Home-rows notice fixed earlier. Otherwise the label is
  unchanged. Release builds have no J7 gate, so their labels don't change.
- No new strings and no language-table changes.

Tests: `ajustes_ux_dados` now asserts the binding check and the unchanged local
label without an upload scope. The J7 suite's `privacy` step asserts Home layout
is bound across the arm/disarm transitions. **41/41 focused suites PASS**
(including J7 plain and ASan/UBSan) and `ajustes_ux_dados` PASS. Logs are in
`build/stage-j-label/`. Not yet in any APK: seeing it on a device needs a rebuild
and separate install approval.

**Stage J marked complete by the user (6 November 2026).** Next: BlueStacks
side-rail touch polish, then Stage E.


### 8 November — settings expansion compatibility verification

Stage J transport and live-validation status are unchanged. The pre-existing
Family/Home extensions make the J4 allowlist 93, not the stale 91 in several
fixtures. Those extension records and checkpoint tests now match the actual
existing map; original J1 history is retained. The 14 newly appended settings
IDs are not enrolled in J4, J6 or Stage I. Eight normal preferences remain local;
six virtual editors use the existing source store. Native harness checks verify
all three sync exclusions and profile reloads. Relevant J4–J7/I/G regressions
pass with isolated mocks; no account/device access or live writes performed.
See `SETTINGS-EXPANSION.md` for the complete local verification report.

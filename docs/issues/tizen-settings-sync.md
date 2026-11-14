# Better Nuvio Android / Tizen settings sync

The Android and Tizen clients share the settings implementation. Better Nuvio
layouts (including Dynamic) use the `better_nuvio` profile blob, not the official
Nuvio `tv.layout_settings.selected_layout` enum. Both clients must use the same
account and profile.

Two reproducible gaps were found on 14 November 2026:

- The first 93 Better Nuvio bindings, including `homeLayoutLocal`, used existing
  saved preferences as an unobserved baseline. A preference customised before
  automatic sync started was never uploaded until it was changed again. The
  previous two-installation test only exercised changes made after startup.
- `sync_pull_provider_credentials` accepts `credential_json` as an object or a
  JSON-encoded string. The reader searched the raw string without decoding it,
  silently missing API keys in the latter format.

After a successful account pull, the settings bridge now journals non-default
legacy preferences whose fields are absent from the account. Existing account
fields take precedence over this migration. Failed pulls do not seed anything,
and a profile marked as belonging to another account cannot supply migration
values. Ordinary subsequent edits keep their existing merge rules. Credential
strings are decoded before extracting their fields.

Validation:

- `bash tests/settings_sync_auto.sh`: includes the original layout failure,
  offline migration, restoration by another installation, existing cloud values
  winning, and the existing settings/key-file/offline/account-isolation suite.
- `bash tests/settings_stage_i.sh --transport-only`: real sync worker with
  mocked HTTP; object and string provider credentials, later key replacement,
  settings pulls, failures, retries and stale-session rejection.
- `git diff --check`.

Both new functional regressions failed before the fixes. The full Stage I
wrapper's static inventory audit has pre-existing stale consumer snippets in
`stage-i-inventory.json`; `--transport-only` runs the production transport tests
independently without changing the default full audit.

No live account or physical TV was used. These are source fixes, not installed
TV updates. Normal account polling is every five minutes outside playback;
foreground and player-close events also request refreshes.

## Copy my settings to other profiles

The original copy action copied only scalar preferences into local profile
files. It explicitly excluded API keys and service configuration, and its
account updates waited until the target profile became active. It also omitted
the newer official-settings journals. Consequently its success notice did not
mean that another TV could restore the copy.

The revised action captures the source's portable preferences, visible Home
presentation, subtitle/source preferences, language fallbacks, API keys and
supported service files in a durable, account-scoped request. API-key files
include TorBox/other debrid providers, TMDB, MDBList, Fanart, Seekr and poster
configuration. IPTV bundle filenames are rebased to the destination profile.
Provider credentials use separate destination-profile files so unselected local
profiles are not overwritten through the legacy shared file.

The account worker pulls each destination's documents before merging the copy;
it can update inactive targets without changing the active profile. The merge
preserves unrelated account fields and assigns newer Better Nuvio field
revisions. Failed or ambiguous requests remain queued, while an already-applied
copy is detected on retry. Every request uses the cycle's captured session and
token; switching accounts cannot publish the previous account's queued copies.
If the target becomes active before upload, it is refreshed after the copy so
the pre-copy response cannot restore stale settings over it.

The copy screen and confirmation now state that API keys are included. History
and hardware compatibility settings remain separate. Copies are snapshots,
not a permanent link between profiles.

The two-installation mock suite covers key transfer, inactive targets, existing
target settings, offline copying, session/account changes, ambiguous writes,
Home presentation, subtitles, source rules and language fallbacks. The local
profile-scope suite and translation-table alignment checks are also exercised.
These changes have not been installed on the connected TV.

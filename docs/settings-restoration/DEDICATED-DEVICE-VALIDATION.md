# Dedicated-device validation proposal

Prepared 5 November 2026, after broad-failure triage. **Installation approved and completed on both devices. Live launch/sync validation remains unapproved and unexecuted.**

## Candidate

- File: `build/android/Nuvio-2.0.8-android-debug.apk`.
- SHA-256: `a9cd562e7a610aa79a87f29d91b70bd764c9b5547c11c7a12181a5ffb3aad5ad`.
- 74,567,092 bytes; unchanged `space.nuvio.nativelegacy.strictdebug` and debug
  certificate; ARM64/ARMv7, TV launcher/assets/alignment verified.
- Test-only triage changes do not alter this candidate. Previous approved APK
  remains preserved in `build/stage-j-cleanup/`.

## Preconditions — user confirmation needed

Fire Stick is Device A and BlueStacks is Device B. Confirm that **Better Nuvio's
strictdebug app**, not just regular Nuvio, is signed into the dedicated test
account on each. Confirm profiles 1 and 2 exist. Do not send credentials.

BlueStacks may currently hold the main account. Do not launch it, choose its
profile, inspect its backend or send activation/snapshot links while that is
uncertain. The user must put it on the intended test account first. No sign-out,
account switch, profile creation or main-account cleanup is authorised by this plan.

## Approval 1: installation only — completed

After confirmation, verify the exact local checksum again, then install with
replacement on the two confirmed test devices. Preserve app data and package
identity; never uninstall/clear data. Do not automatically launch, sign in,
change settings, arm either scope or make backend writes. Read-only package,
signature and installed-byte verification may follow. Disarm state must be
verified before any separately approved live test; installation does not itself
clear old permission files.

## Approval 2: bounded live validation — ask separately

Obtain explicit approval before app launch with live sync, changing test values,
arming or backend writes. Existing Stage I's 32 shared settings can still upload
independently of the J4/J5 and J6 gates; do not describe an online launch as
unconditionally read-only. Check/report any Stage I pending work and agree its
scope before proceeding. Never use production accounts.

Proposed order, with filtered logs and account/profile verification at every step:

1. Confirm both J4/J5 and J6 gates are off. Verify default-off behaviour and that
   first login does not send `homeLayoutLocal` merely because it is newly bound.
2. Under approved J4/J5-only permission, change Home layout between Modern,
   Default and Dynamic on profile 1. Check the receiving device's actual UI and
   persisted value, and one expected update per deliberate change.
3. Verify only `better_nuvio` changed; original `tv`, `home_catalog_shared` and
   `mobile` fingerprints must remain unchanged unless a separate known Stage I
   update was explicitly approved. Stop on unexpected writes; do not clean up a
   backend automatically.
4. Reverse direction; restart/offline recovery; then switch between existing
   profiles 1 and 2 and verify values and permissions stay isolated. Preserve
   Native-only rows, Friends Watching, catalogue availability and 30+ rows.
5. Disarm, return to the profile chooser and verify the new early disarm route
   clears enrolment without first selecting a profile. Activation/snapshots must
   still wait for the correct authenticated profile.
6. Confirm privacy-safe diagnostics and v2 journal migration using only filtered
   names/counts/fingerprints; never print raw journal headers or activation codes.
7. Disarm both scopes on both devices and record final fingerprints/no-echo/leak
   checks. Do not restore/change test values or backend documents without approval.

J6 does not need arming for Home layout validation. No TorBox/provider operations,
downloads, progress-format changes, Stage E/H implementation or publication.

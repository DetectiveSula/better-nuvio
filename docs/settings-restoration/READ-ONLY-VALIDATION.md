# Debug-only account read-only validation build — 5 November 2026

User chose a separate read-only build instead of authorising Stage I automatic
uploads during preflight. This is a temporary validation artifact, not a sync
release or a completion claim for live Stage J.

## Boundary and behaviour

`NUVIO_VALIDATION_READONLY=1 bash tools/android.sh --debug-only` explicitly passes
a CMake option, default OFF. Non-debug build requests fail closed. Existing package,
signing and preference storage are unchanged; ordinary builds retain their policy.

`nuvem.c` is the common Nuvio account transport for session/login, account RPCs,
Stage I, J4/J5, J6, progress, profiles and addon updates. The validation variant:

- Allows only exact named read RPCs: profiles, profile locks, profile settings
  blobs, Home catalogue settings, collections, library and watched items.
- Allows only GET table reads for `addons` and `plugins`.
- Rejects all other POST paths/RPCs, edge functions, authentication/refresh calls
  and the separate addon PATCH path locally with temporary status 503 and no network call.
- Does not load provider credentials through the account pull RPC during preflight.
- Adds native HTTP-method guards ahead of Android fallback/curl/structured
  requests: non-read POST/PUT/PATCH/DELETE calls from background workers are
  rejected before networking, including third-party mutation requests.
- Refuses code generation/activation and reports both J4/J5 and J6 permissions OFF,
  including permissions retained from another build. Disarming remains available.
- Emits only a fixed read-only marker, never bodies, credentials or raw identities.

No fake success response is returned for a denied mutation. Existing pending
changes remain governed by their normal failure/retry handling. The variant does
not change the remote protocol, documents, defaults or settings bindings.

## Deliberate limitations

This is a **Nuvio account transport guard**, not an OS firewall or general network
sandbox. Third-party integrations and playback are outside its validation scope;
do not initiate playback, provider actions, tracking actions or sign-ins during
preflight. It cannot be used to test outbound Home-layout sync: that requires a
separately approved writable artifact and separately approved J4/J5 activation.

The existing test-account session must be valid. Token refresh/login is blocked;
expiry stops authenticated live reads instead of silently modifying auth state.
No credentials are copied into fixtures or build logs. Current account/profile
must be checked locally before launching this artifact. Only dedicated test accounts
are authorised; main-account documents remain off limits.

## Planned controlled device checks

After separate installation approval, reconfirm test identities from local state,
then run read-only snapshot/profile/foreground checks with epoch-scoped filtered
logs and before/after server fingerprints. Open existing profile 2 only for the
J6 v1-to-v2 **local journal** migration check; J6 remains OFF. No settings edits,
sync activation or account writes are permitted. Stop/disarm after each check.
Modern / Default / Dynamic outbound tests remain separately approval-gated.

## Validation evidence

Build, regression, transport-boundary and permission-refusal evidence is under
ignored `build/stage-j-readonly/`. Installation and live checks are not included
in local build/test passes. Final artifact verification and results will be recorded
in STATUS.md and STAGE-J.md after completion.

Final local results: **41/41 focused ordinary-policy suites**, **61/61 Android
unit tests**, guarded transport/release-refusal tests and real native HTTP/arming
refusal tests PASS. Blocked refresh preserves the stored session byte-for-byte;
local denial uses temporary status 503, not a credential rejection. Native
HTTP/TLS, buffer and Android-fallback tests PASS. Two additional web-only mock
tests remain unexecuted because no Node CLI is available and the bundled REPL
forbids their dynamic Function construction. No live functional pass is claimed.
See `build/stage-j-readonly/FINAL.json` for the final checksum and evidence.

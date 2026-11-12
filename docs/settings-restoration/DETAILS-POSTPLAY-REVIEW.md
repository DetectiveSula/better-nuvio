# Details sidebar and post-play review — 8 November 2026

Owner confirmed the sidebar flickers only during opening/closing on details,
and the failed post-play case was a film. Work remains local on
`codex/settings-restoration`; all existing working-tree changes are preserved.

## Details sidebar

The Modern sidebar over details used the same narrow-rail-to-wide-panel morph
as Home, although details has no collapsed rail. Its changing glass footprint
changes backdrop sampling regions and can cross trailer/video-hole boundaries.
This is a source-level risk consistent with the transition-only report; the
physical Android GPU symptom was not directly reproduced on the Mac.

The details overlay now uses a full-size panel with an opacity transition.
Home retains its existing expansion; the details background/video is not frozen,
blur preferences remain honoured, and no new rendering framework/cache is added.
Tests verify constant bounds through the complete expansion range, Home behaviour,
open/close endpoints, and native GL rendering with a real SDL Back event. The
frost probe passed (sharp-edge fraction 0.00170, broad contrast response 19.62).
Actual Android/BlueStacks confirmation remains necessary.

The existing screenshot harness had two unrelated test defects: Bash 3's empty
array under nounset, and a fixed pixel probe that could sample outside the now
scaled/docked menu. Corrected those fixture issues and retained initial logs.

## Film post-play recommendations

Confirmed wiring defects:

1. Direct Continue Watching playback bypassed details, the previous owner of the
   related-title request. Player opening now reuses the existing idempotent
   movie metadata request when recommendations are enabled. No new resolver,
   provider API, account setting or sync field is introduced.
2. The player could exit at EOF while a recommendations panel was visible,
   especially when configured for 100%. EOF now retains a visible post-play
   choice; dismissing it still releases playback normally. This also protects
   a visible presence prompt if the episode catalogue refreshes at EOF.
3. A backend finishing fractionally short of duration could never reach an exact
   100% threshold. Confirmed EOF within the existing 1.5-second end tolerance now
   supports that threshold, even when resuming immediately at the end. Early or
   truncated EOF does not qualify. Other percentages retain their exact policy;
   automatic/credits behaviour and the master Off preference remain intact.

Tests exercise actual recommendation-panel visibility at 10–100%, all 100
percentage-policy values, late/empty related lists, master Off, dismissal,
fractional real EOF, truncated EOF, and the player's actual EOF-retention helper.
Recommendations still require available related-title metadata from supported
configured services. An empty/failed metadata response does not invent titles.
Series keep their existing Next Episode behaviour.

## Are you still watching?

The existing episode counter works in the automated tests: every interval 1–20,
Off, autoplay Off, automatic player close/reopen transitions, prompt persistence,
and manual-stop reset. Added protection so a held/repeated OK cannot confirm a
new presence prompt; a fresh OK is required.

It requires **Play next episode automatically**. Zero is Off. Manual Next, seeking
and other explicit playback interactions reset an unattended sequence by design.
For a quick device check, use interval 1 and let the episode reach its next-episode
window; the prompt should stop automatic advancement until a fresh OK.

## Validation and delivery

Passed: settings A/B, original 413-binding preservation, playback preference
persistence, post-play/countdown/recuo, Continue Watching parity, TorBox cache and
cached-addition/autoplay, verified-uncached filtering, menu regression and native
details GL checks. Post-play fixture now disables dynamic HTTP transport; provider
regressions use isolated mocks. No live TorBox/account mutation or sync arming.

Built a fresh normal strictdebug Android TV APK and reran all 69 Android JVM tests
successfully. ARM64/ARMv7 libraries, signature/certificate, package, 16 KB ZIP/ELF
alignment, TV launcher and assets verified. Package and numeric version unchanged.
APK, SHA256SUMS, verification and logs: `build/details-postplay-review-20261108/`.

No installation, commit, push, merge or publication. GitHub preview 3 is unchanged
and does not contain these new fixes. Device behaviour remains unverified until
the new APK is installed with approval. This review does not certify all settings.


## BlueStacks follow-up — Ratings/Cast and Coco (8 November 2026)

Read-only package hashing confirmed the installed APK is preview 4
(`66b3967ba529aea2384607c8af7577efe1f3000a4c8c0ebd833676f32db26ea3`).
The owner reports flickering during opening/closing over Ratings and Cast.
Direct device screenshots confirmed foreground Ratings content visible in the
sidebar's blur. The earlier top-of-details fixture did not cover these sections.

The background blur used last frame's sampling regions. An overlay outside those
regions, or a foreground full-screen veil, could select a live framebuffer
capture containing text/cards instead. The sidebar now reserves its full-height
footprint before the page draws and uses an explicit background-only source.
This preserves blur settings and the existing fixed-size details transition;
ordinary Home navigation and other modal source policies remain unchanged.
The reservation is limited to the sidebar strip plus blur reach, not an extra
full-screen copy every frame. GL allocation failure uses the existing solid
fallback. Native fixtures now draw actual Ratings and Cast sections, without
painting the previous synthetic pattern over the page. Pixel tests cover a new
region, foreground cards, a full-screen veil and repeated frames.

The owner authorised Coco playback testing and positioned it at its credits.
At 1:36:24 / 1:45:02, filtered diagnostics confirmed the credits-marker condition
was already true, but no related-title panel opened. This is not a percentage
threshold failure. The per-frame empty-list log was flooding logcat and erasing
metadata diagnostics; it now reports only state changes with safe counts.

The approved APK has no TMDB key asset and its build configuration has no TMDB
app key. No personal key was read, printed, requested in chat or embedded.
The owner confirmed More like this is On, and explicitly requested a local key
field. The existing app offered no such field. This explains the missing
recommendation source when no usable account/Trakt credential is available;
account credentials were not inspected, so their absence is not asserted.

Added **Integrations → TMDB → TMDB API key on this device**, after the TMDB
master switch. This is a new append-only action, not a replacement binding.
The blank password keyboard is masked; list status never displays key bytes.
Only an empty value or exactly 32 hexadecimal characters (TMDB v3 key) is
accepted. A private 0600 atomic file is scoped to account and selected profile,
using the existing provider credential-file naming/writer contract. Empty removes
the local override while retaining an available account/app fallback. Profile
and account changes reload the corresponding key; incoming account refreshes
cannot overwrite the local override. The credential is excluded from normal
preferences, better_nuvio, official settings documents and release assets.
No new remote field or protocol is introduced. TMDB's master Off remains
respected. Saving invalidates previously empty metadata requests only; reopen the title to refresh. It does not reset the account session or discard MDBList/provider credentials.

A separate verified source defect omitted a movie metadata request when only
recommendations were wanted and other movie extras were Off. Movie post-play
now requests/parses recommendations independently of unrelated TMDB sub-options,
through the existing enabled TMDB transport. Same-title reuse includes that
request policy. No catalogue-based substitute recommendations or provider-master
bypass is introduced. Isolated HTTP tests exercise the actual threaded request
and JSON parsing with a synthetic key and canned response.

Device success after these changes still requires installing the new APK with
approval, entering a valid key locally (if no account source is available), and
repeating Coco and Ratings/Cast checks. No new installation, sync arming, backend
settings writes, provider mutations, publication or source commit is authorised
by the local build. Existing user-authorised playback may save ordinary progress.
Evidence and final test/build results: `build/details-device-followup-20261108/`.

# Profile editor — 11 November 2026

The owner approved items 1–9 of the profile-editor audit. Reference: official Nuvio TV
`ui/screens/profile/ProfileSelectionScreen.kt` (options dialog, `EditProfileOverlay`) and
official mobile `features/profiles/ProfileRepository.kt` (account calls).

## What changed

Holding OK on a profile (touch: long press) opens an Android panel
(`ProfileEditor.kt`). The plain Material dialog has been replaced.

* **Options first**, as in official Nuvio TV: Edit profile, Set/Change PIN, Remove PIN and
  Delete profile, then Close. Delete appears only for a profile that isn't profile 1,
  isn't the primary profile and isn't the active one. A one-line note explains why
  Delete is missing.
* **Editor**: a rounded glass panel. Left: a live avatar preview in the chosen colour,
  the name and the name box, then Cancel. Right: Avatar / Colour / Background tabs;
  the tab under focus is the one shown. Save is at the top right. On a phone-width
  screen the two columns stack.
* **Focus (bug 1)**: every control has a resting glass state and a focused state with an
  accent fill and a 2 dp accent outline. The accent is the user's `ajustes_acento()`.
  Back steps out one level: link → editor → options → close.
* **Name (bug 2)**: typing stops at 20 characters / 63 UTF-8 bytes
  (`ProfileEditPolicy.clampName`). The old box allowed 40 characters and then failed
  at Save.
* **Catalogues (bug 3)**: the Avatar tab shows "Loading avatars…" while the catalogue
  loads. If it fails, it shows "Avatars couldn't load." with Try again. Previously it
  showed "catalogue unavailable".
* **Hint (bug 4)**: the profile picker now shows "Hold OK on a profile to edit it,
  change its PIN or delete it" at the bottom (touch wording in the Mobile layout). It
  is hidden behind the PIN pad and while a profile is loading. Translated into all 29
  tables.
* **Avatar grid (6)**: the focused avatar's name is shown under the grid, and the chosen
  avatar has an accent check. Pressing OK on the chosen avatar again undoes the choice
  (official behaviour). Choosing an avatar also takes its catalogue colour
  (`bg_color`). Personal photo, Photo link and No photo remain below the grid.
* **Colour (7)**: official Nuvio TV's eight profile colours.
* **Background (8)**: Standard, the current link, supporter backgrounds from
  `get_member_profile_background_catalog` (members only; images are read through
  the authenticated storage path), or a new image link. Better Nuvio draws only
  link backgrounds (`profile_background_url`), and the panel says so when
  supporter backgrounds are listed.
* **PIN (9)**: a 4-digit on-screen keypad (number keys also work). A new PIN is
  entered twice. For a locked profile, the PIN typed to open the editor is passed
  in memory as `p_current_pin`. Remove PIN asks for confirmation, with focus on
  Cancel.
* **Delete (9)**: a confirmation that says it cannot be undone, with focus on Cancel.

## Account calls (all writes need Save, a PIN confirmation or Delete; read-only builds refuse them)

| Change | Call |
|---|---|
| Name / avatar / photo only | `sync_patch_profile` (unchanged, sparse) |
| Colour or background changed | `sync_pull_profiles` → `sync_push_profiles` with every profile, the others copied unchanged from the fresh pull → `sync_pull_profiles` to confirm |
| Set / change PIN | `set_profile_pin` (`p_current_pin` when known) |
| Remove PIN | `clear_profile_pin` (`p_current_pin`) |
| Delete | `sync_delete_profile_data`, then (official TV) push the list without the profile unless the server already dropped it, then confirm with a pull |

The push is refused if the pulled list is unexpected: more than 6 profiles, a
duplicate index or an index outside 1–8. None of these calls has been run against
a real account. They follow official Nuvio's code and are exercised only against
the JVM fake server.

## Native

`profile_editor.c` passes colour, background, PIN state, role, the active profile,
the accent and the client id. It applies:

* `saved` events: name/photo plus colour/background;
* `final:0` + `pin` events, without closing;
* `deleted` events.

`perfis.c` gained `perfis_aplicar_aparencia`, `perfis_aplicar_pin` and
`perfis_aplicar_remocao` (refuses profile 1, the primary profile and the active
profile).

Fixed: `js_texto` treats `""` as missing, so a save with no photo (or with the
standard background) was never applied to the native cache. This was already
true of "Remove photo" before this change.

## Known limits / follow-ups

* Deleting a profile does not remove this device's local per-profile files
  (`*-p<N>.txt`, catalogue cache, settings checkpoint). Official TV clears them. A
  profile created later with the same index would inherit them on this device.
* The editor panel is Android UI, so the host shot tests can't render it. Visual
  review needs an install on BlueStacks.

## Tests

* JVM: `ProfileAccountApiTest` (18) and `ProfileEditPolicyTest` (5), including the
  push/PIN/delete/read-only paths on the fake server.
* `tests/profile_editor_events.sh` (new): native open payload, PIN/save/delete events,
  `""` values, stale/closed panels, session change.
* `tests/perfilsel.sh`: apply colour/background/PIN, refusals, plus a new `remocao`
  process for deletion and the cache.
* `tests/profile_editor_hold.sh`: updated for `profile_editor_open(slot, pin)`.

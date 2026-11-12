# Dynamic sidebar opening flicker — 11 November 2026

The requested sidebar is Dynamic (Apple TV pill-to-panel), clarified during testing.

The Dynamic material previously used the ordinary overlay capture. During its growth, its read area could exceed the previous frame’s background region and switch to a live foreground capture, including page text/cards or the navigation scrim. The next frame could return to background-only sampling.

`desenharAtrasDoPainel` now prepares navigation background reservations before every page rather than only detail overlays. Dynamic reserves the union of the closed pill and open panel widths over the real screen height, even when the closed pill is hidden. Its material uses reserved background-only layer 3 for the entire morph. Existing shape, opacity, focus and streaming animations remain.

Validation:
- `tests/menu_tvos.sh`: desktop and touch pass; every Dynamic destination, normal/detail overlay, two scales, repeated opening/closing and variable frame intervals retain the same reservation.
- `tests/phone_menu.sh` and `tests/menu_geometry.sh`: pass.
- `NUVIO_SHOT_DYNAMIC_SIDEBAR=1 bash tests/phone_layout_shot.sh build/sidebar-flicker-20261111/native 2`: offline native renderer passes 1,080 frames across Home, empty Home, Search, Library and Settings. Each visible open frame retains a blur source.
- Guarded Android assembleDebug and JVM test task succeed. Both ABI production compile commands contain readonly/login validation guards, and stripped native binaries match the APK bytes.
- Updated dedicated-account BlueStacks installation with install -r; launched and exercised Dynamic navigation. Screenshot sampling is not a frame-by-frame device flicker measurement.

Artifact: `build/sidebar-flicker-20261111/Better-Nuvio-2.0.8-dynamic-sidebar-fix-readonly.apk`
SHA-256: `9ccc4e4bce9e58520b1ffc6a8044bde2ee0c9f985680678c94851b2cd1dd30f9`
The earlier Modern-named APK in the build directory is superseded.

## Follow-up: background bleed while moving between items

Opening/closing was confirmed improved by the user. The remaining report concerned focus navigation through the Dynamic bar. Its glass fallback (frost disabled, video overlap or unavailable source) intentionally leaves page content visible. Added a geometry-matched charcoal backing underneath the open panel, blended in with the existing morph. The closed pill remains translucent. Blurred background colour, rim, highlights and focus/streaming animations still draw above the backing.

A matched-frame regression uses production rendering with frost disabled: move focus through the destinations, then render the same state over alternating red and blue foreground rectangles. Before the backing, a panel pixel changed from RGB 25,25,90 to 90,25,25 on the first pair. After the change, all 240 pairs stay within one RGB level. Existing 1,080-frame opening/closing native checks and desktop/touch menu/phone routing checks pass. Build and JVM task succeed.

Updated guarded APK installed over the dedicated BlueStacks copy. Launched and moved focus from Home through Schedule to Settings; the panel retained its dark surface while the hero changed behind it. Live screenshots are spot checks, not frame-by-frame device measurement.

Latest artifact: `build/sidebar-scroll-20261111/Better-Nuvio-2.0.8-dynamic-sidebar-scroll-fix-readonly.apk`
SHA-256: `25b881e26de2de3ece80425018e2d7793882501d7f70163e4f0b49dad9eab683`
Native logs: `build/sidebar-scroll-20261111/{before,after}.log`.

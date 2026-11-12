# Phone layout test build — 11 November 2026

The Android test APK adds a phone layout using the existing native TV screens,
catalogue, settings, navigation destinations, artwork and playback paths.
It does not package or display the HTML design preview.

Automatic device detection already runs before SDL starts. Non-TV devices use
Mobile when Interface is Automatic; an explicitly saved TV or Mobile choice
continues to win. Portrait Mobile now has a touch-sized menu header and drawer
with the TV destinations and their translated labels. Portrait typography has
a density adjustment; the stored text-size choice remains unchanged. Returning
to landscape or TV removes that adjustment. Empty Home and Library layouts fit
portrait screens. Android phones continue to use the existing system keyboard.

A local first-run dialog explains the detected phone layout. Continue records
only a device-local acknowledgement. It does not change the saved interface
choice, sign in, enable sync, modify providers, or write account settings.
The new setup explanation and phone setup-guide text are currently English.
Existing menu labels retain their translations.

## Testing in BlueStacks

Use the APK under `build/phone-layout-20261111/apk/`. It uses the existing
`space.nuvio.nativelegacy.strictdebug` package, versionCode 20008 and the same
debug signing certificate as the previously installed test build. Install as
an update to retain its data; do not uninstall the old package first.

Rotate BlueStacks to portrait and use Interface **Automatic** or **Mobile**.
A saved **TV** override intentionally keeps the TV interface. The setup dialog
appears once on a detected mobile device with Mobile active. Use the menu button
or the existing edge gesture to open the drawer. Normal landscape TV behavior
and the separate post-play mockup remain unchanged.

This task built the APK; it did not install or launch it on BlueStacks or any
other device. Android device rendering and touch still need hands-on testing.

## Validation

- Offline Android build and all 70 JVM tests passed, including the setup policy.
- Native phone menu routing, target geometry, scrolling and rotation checks passed.
- Existing TV menu, interface mode, Android detection, Home and hero checks passed.
- Settings stages A/B/C/D passed; the original 413 provenance records remain wired.
- The isolated native renderer checked Home, drawer, empty Home, Search, Library
  and Settings. The final renderer replaces network transport with offline stubs
  and uses temporary settings and fixture artwork, with no account session.
- APK signature matches the previous test build; ZIP and all arm64 ELF load
  segments pass 16KB alignment. ARM64 and ARMv7 are included. Private-file scan passed.

Build logs, native screenshots and `apk/verification.json` are in
`build/phone-layout-20261111/`. The build reuses the existing cached Android Rust
libraries because the currently selected Rust toolchain lacks the Android target;
it retains the Rust media core rather than switching to the C fallback.

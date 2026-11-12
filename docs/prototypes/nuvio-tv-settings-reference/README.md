# Original Nuvio Android TV settings — visual review

Open `index.html` directly, or serve this directory. It is an isolated HTML/CSS/JS
mockup: no app implementation, preferences, backend or device connections.

The 1920×1080 screenshots show Content & Discovery, Playback, expanded subtitles
and Connected Services. The primary-profile sidebar is scrollable in the original
category order. Click its categories; arrows move between rows/panes, Enter opens
an illustrative action, and Escape/Back returns. No values are saved.

## Reference

Official local source `~/Documents/Nuvio-Official`, pinned commit
`99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f`. Default **Classic**, **Inter**, **Ocean**.
Layout derives from SettingsScreen.kt / SettingsDesignSystem.kt: 220 dp rail,
56 dp minimum rail rows, 62 dp minimum settings rows, 28 dp outer radius,
20 dp workspace padding, pill rows and grouped surfaces. The mock uses 2 pixels
per source dp. Copy is drawn from the pinned English strings where available.

Categories: Account, Switch Profile, Watch Party, Appearance, Layout,
Content & Discovery, Playback, Integrations, Tracking, Advanced, About.
Watch Party is conditional. Account/profile visibility depends on primary profile.
The source has **no Notifications category**. Internal Debug and hidden Experience
categories are not exposed here. Both panes scroll rather than squeezing all rows
into one screen. A partially visible boundary row indicates additional content;
there is no overlapping footer.

## Legacy examples

Verified against `stage-b-inventory.json` / `LEGACY-BADGES.md`: IPTV/EPG,
addon/provider fallback, extra second-subtitle appearance and delay, AllDebrid,
Discord presence. Shared category names, subtitles parent and secondary subtitle
language remain unbadged. Evidence is in `reference.json`.

This is a source-based visual reproduction, **not a pixel-certified Compose
capture**, nor a full settings audit or implementation. Icons are lightweight SVG
approximations of the source's Material icons. Some less relevant category panels
contain representative rows rather than every existing Native control. Nothing is
removed from Better Nuvio; all underlying settings/bindings remain untouched.
All shown account/provider values are synthetic; there are no credentials.

Generated screenshot dimensions have been checked as 1920×1080. The production
application, repositories' Git state, Android packages and devices are unchanged.

## Requested preview refinements — 7 November 2026

- Appearance includes App Icon & Banner, distinct AMOLED Mode and Pure Black
  Surfaces switches, plus the retained theme/style/font/language controls.
  The source only exposes Pure Black Surfaces when AMOLED is on. The preview
  starts with AMOLED on and surfaces off to show both; toggles change only
  this temporary browser illustration and are never saved.
- Layout includes Focused Poster and Poster Card Style, with existing original
  section ordering; both are unbadged original settings. The Layout screenshot
  scrolls to them; the live sidebar opens the section at the top normally.
- Content & Discovery has exactly two top-level rows, Add-ons and Plugins,
  as requested. Native source/fallback examples remain in the Add-ons
  subpage so the preview does not imply those extras are being deleted.
  This intentionally differs from the pinned official hub's Media Servers row.
- Discord presence is omitted from the Integrations preview. Its existing
  application implementation and stored preferences have not been removed.
- Advanced groups: Experience Mode, Performance & Navigation, Diagnostics,
  Cache and DV Diagnostics. Startup/memory/navigation and logging examples
  are represented inside these group descriptions.

Only this prototype directory was changed. No application settings, platform
gates, account data, APKs or device state were modified. The preview is still
representative rather than an exhaustive implementation of every control.

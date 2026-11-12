# Profile selector poster loop — 11 November 2026

The Films background wrapped its vertical offset after one row, while each fixed drawing slot kept the same artwork index. At each row boundary the visible posters jumped to the preceding/following image. The effect repeated at the different column speeds.

`psestilos.c` now wraps over the complete poster sequence and subtracts the crossed row count from each artwork index. The same poster therefore keeps moving through every row boundary and through a complete sequence wrap, in both directions. The animation clock uses double precision to preserve subpixel motion in long sessions. Speeds, spacing, rotation, fades, profile crossfades and reduced-motion behavior are preserved.

Validation: `tests/psestilos_loop.sh` runs actual production drawing against recorded geometry and texture IDs. It checks every supported count (1–12), all three speeds, row/full-sequence boundaries in both directions and long sessions. Previous implementation fails this test; updated implementation passes. Existing `tests/psestilos_luz_barra.sh` passes. An additional attempted `tests/psfundo.sh` check cannot link against the existing catalog dependencies (unrelated missing stubs: ajustes_fonte_biblioteca, desc_tmdb_idioma and prog_*).

Android assembleDebug and JVM tasks succeed. Guards verified in every production compile command for both ABIs, and stripped binaries byte-match the APK. Installed over the dedicated BlueStacks copy with install -r; no live profile-session interaction performed for this update.

APK: build/profile-loop-20261111/Better-Nuvio-2.0.8-profile-endless-loop-readonly.apk
SHA-256: c05cc373822a9afa3fd5c99637b5e886f524fc44712241f1786c82fe5c50024f

## Distinct titles across rows

The old overlapping column offsets could repeat a title in neighbouring columns as their independent loops moved. Each column now owns an interleaved subset of the available titles. With at least nine titles, subsets are disjoint across all nine columns, so no row repeats a title between columns at any time. Fewer than nine titles necessarily repeat across the nine-column wall. Titles still repeat vertically and eventually cycle; this does not expand the stored artwork/cache budget.

The production draw-call regression checks both seamless boundaries and no shared title between distinct columns, across every supported count and long sessions. Loop and existing light-background checks pass. Android build/JVM tasks pass, both native ABIs verified guarded and byte-matched.

Latest APK: build/profile-distinct-titles-20261111/Better-Nuvio-2.0.8-profile-distinct-titles-readonly.apk
SHA-256: e053939242d16afdd34d99d4ded1745f2d0d784280259d5d165c16aa9b2826b3

## Follow-up: repeated vertical stacks

The screenshot exposed the consequence of partitioning a pool capped at 9–12: several columns owned only one title. Expanded the catalog mural, per-profile saved wall and Films rendering capacity to 36 (four per column). Existing profiles refill their wall from their catalog when the selector opens, using the existing local registration path. No provider writes or added background source. With at least 27 available titles, all three visible rows in each column are distinct; fewer available titles still require reuse.

The older horizontal mural now derives its drawing range from the scroll offset and viewport instead of a fixed 16-card window, so a longer sequence cannot scroll into an empty region. The regression covers all 1–36 title counts, distinct column pools, no repeats within three adjacent rows when enough titles exist, and continuity at row/full-loop boundaries over long sessions. Existing light-background checks pass. Offline local-art native render inspected at build/profile-grid-20261111/profile-wall-36.png: visible rows/columns contain different posters. Android build/JVM task pass; both ABI guards verified and binaries match APK.

Latest APK: build/profile-grid-20261111/Better-Nuvio-2.0.8-profile-varied-grid-readonly.apk
SHA-256: f9f5cf92dd3b6f6a520b387eec930f62dfc700925a221796f99baed59b4f09a3

## Apply to existing and future profiles

`parMontar` no longer treats an existing 4–12 poster cache as a complete wall. It keeps that profile’s saved titles first, deduplicates them, then fills remaining spaces from the existing catalog mural fallback up to 36. New profiles use the same path. This is decorative rendering only: it does not write one profile’s history into another profile’s saved wall or change the active profile. PIN profiles still skip their private cached wall.

The selector now rebuilds/crossfades if the catalog pool grows while already open, or a saved wall/PIN source changes. It compares URL lists when a source changes and avoids per-frame wall copying/cache marking for stable sources.

Production regression passes for Profile 2’s legacy 12-poster cache, profiles 3–8 with no cache, deduplication, late catalog arrival, PIN cache-read exclusion, all 1–36 pool sizes and long-session seamless rendering. Existing light checks pass; Android build and JVM task pass. Both ABI validation guards and APK binaries verified.

Latest artifact: build/profile-all-20261111/Better-Nuvio-2.0.8-all-profile-backgrounds-readonly.apk
SHA-256: 85352bc58c1be5cd2d0838fa91e920b5180ee1dc4079e71c66bc8f704b4581cf

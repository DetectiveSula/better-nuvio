# Local checkpoint: Stages B–D

Validated 4 November 2026 on `codex/settings-restoration`.

Commit message: `Complete Legacy settings, resolver and TorBox autoplay restoration (Stages B-D)`.
Parent: `bd0b7edb120b042a8db2f2dd3980dd7b390ba403` (Stage A).
The checkpoint SHA is recorded by Git rather than embedded in its own contents.

## Scope and review

Includes Legacy integration and verified badge evidence, settings list footer clearance,
Resolve Playable Links controls and policy, TorBox availability and actual stream labels,
autoplay wiring, and the explicit cached-account-addition opt-in. Existing 413 bindings
are preserved; five new preferences are appended. Cached additions default to Off,
require the selected TorBox provider and fresh cached evidence, and use durable duplicate
suppression after ambiguous timeouts. The fixtures and all provider operations in these
tests are mocked; no live account requests or downloads were performed.

All modified and non-ignored untracked files were reviewed. All included files are UTF-8;
JSON fixtures and the evidence inventory parse successfully. The candidate scan found no
private keys, JWTs, literal credentials, authenticated URLs or personal machine paths.
Synthetic fixture keys remain test data. System Homebrew include/library paths in native
test runners are dependency lookup paths, not private local configuration.

`local.properties`, private/runtime configuration and generated build outputs remain
ignored and excluded. No APK, binary, signing key, account data or machine-specific
configuration is included. No production changes beyond the requested Stages B–D are
introduced by this checkpoint. Stage E and platform builds/device validation remain pending.
See `STATUS.md` and the resolver/TorBox reports for supported paths and limitations.

## Fresh regression results

Every command below completed with exit code 0 before staging the checkpoint:

| Command | Result |
| --- | --- |
| `python3 tests/settings_preservation.py` | PASS: 413 existing bindings, defaults and progress sources preserved |
| `bash tests/settings_stage_a.sh` | PASS |
| `bash tests/settings_stage_b.sh` | PASS, including evidence policy, persistence and remote interaction |
| `bash tests/settings_stage_c.sh` | PASS, including resolver policy, mocked debrid and preparation limits |
| `bash tests/settings_stage_d.sh` | PASS, including actual UI captions and autoplay URL |
| `bash tests/torbox_cached_add.sh` | PASS, including Off/provider gates, fresh negative/error evidence, durable timeout/concurrency suppression and readiness |
| `bash tests/cw_parity.sh` | PASS: Continue Watching parity regressions |
| `git diff --check` | PASS |

## Included files

### Implementation

- `src/ajustes.c`
- `src/ajustes.h`
- `src/ajustes_legacy.inc`
- `src/ajustes_scenes.inc`
- `src/ajustes_ux.h`
- `src/ajustes_ux_dados.inc`
- `src/ajustes_ux_desenho.inc`
- `src/ajustes_ux_ilha.inc`
- `src/ajustes_ux_interacao.inc`
- `src/ajustes_ux_padrao.inc`
- `src/ajustes_ux_tela.inc`
- `src/ajustes_ux_visual.inc`
- `src/app.c`
- `src/debrid.c`
- `src/debrid.h`
- `src/debrid_add_ledger.c`
- `src/debrid_add_ledger.h`
- `src/resolver_policy.c`
- `src/resolver_policy.h`
- `src/spotlight.c`
- `src/stream_parse.c`
- `src/streams.c`
- `src/streams.h`
- `src/streams_cache.inc`

### Tests and fixtures

- `tests/debrid.c`
- `tests/fixtures/settings-restoration/scopes.h`
- `tests/fixtures/settings-restoration/stage-b-routing.json`
- `tests/fixtures/settings-restoration/stage-b-wiring.json`
- `tests/fixtures/settings-restoration/stage-c-english.json`
- `tests/fixtures/settings-restoration/stage-c-wiring.json`
- `tests/fixtures/settings-restoration/stage-d-optin-english.json`
- `tests/fixtures/settings-restoration/stage-d-optin-wiring.json`
- `tests/fixtures/settings-restoration/stage-d-wiring.json`
- `tests/resolver_policy.c`
- `tests/settings_preservation.py`
- `tests/settings_stage_a.py`
- `tests/settings_stage_b.py`
- `tests/settings_stage_b.sh`
- `tests/settings_stage_b_interaction.c`
- `tests/settings_stage_b_interaction.sh`
- `tests/settings_stage_c.c`
- `tests/settings_stage_c.py`
- `tests/settings_stage_c.sh`
- `tests/settings_stage_c_interaction.sh`
- `tests/settings_stage_c_streams.c`
- `tests/settings_stage_c_streams.sh`
- `tests/settings_stage_d.c`
- `tests/settings_stage_d.sh`
- `tests/stage_d_offline_transport.c`
- `tests/torbox_cached_add.c`
- `tests/torbox_cached_add.sh`

### Policy tool

- `tools/settings_legacy_policy.py`

### Documentation and audit evidence

- `docs/settings-restoration/CHECKPOINT-B-D.md`
- `docs/settings-restoration/LEGACY-BADGES.md`
- `docs/settings-restoration/RESOLVE-PLAYABLE-LINKS.md`
- `docs/settings-restoration/STATUS.md`
- `docs/settings-restoration/TORBOX-CACHED-ADDITION.md`
- `docs/settings-restoration/TORBOX-STAGE-D.md`
- `docs/settings-restoration/stage-b-inventory.json`

Total: 59 files. No push, merge or publication is part of this checkpoint.

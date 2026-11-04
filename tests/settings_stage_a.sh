#!/usr/bin/env bash
# Stage A gates: native catalogue/routing, settings English and binding ABI.
# macOS native interaction harness also verifies remote input without account startup.
set -euo pipefail
cd "$(dirname "$0")/.."
python3 tests/settings_preservation.py
python3 tests/settings_stage_a.py
bash tests/ajustes_secoes.sh
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-settings-stage-a-english.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc tests/settings_english.c src/idioma.c -Isrc -I/opt/homebrew/include -pthread -o "$work/english"
"$work/english"
cc tests/idioma.c src/idioma.c -Isrc -I/opt/homebrew/include -pthread -o "$work/idioma"
"$work/idioma"
bash tests/settings_stage_a_interaction.sh
printf '%s\n' 'PASS: Stage A settings gates'

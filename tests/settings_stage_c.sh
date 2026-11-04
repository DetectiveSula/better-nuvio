#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 tests/settings_stage_c.py
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-resolver-policy.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc -Isrc -I/opt/homebrew/include src/resolver_policy.c tests/resolver_policy.c -o "$work/policy"
"$work/policy"
bash tests/debrid.sh
bash tests/settings_stage_c_interaction.sh
bash tests/settings_stage_c_streams.sh
printf '%s\n' 'PASS: Stage C resolver gates'

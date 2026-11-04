#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 tests/settings_preservation.py
python3 tools/settings_legacy_policy.py
python3 tests/settings_stage_b.py
bash tests/settings_stage_b_interaction.sh
printf '%s\n' 'PASS: Stage B settings gates'

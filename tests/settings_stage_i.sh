#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 tests/settings_stage_i.py
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-stage-i.XXXXXX")
trap 'rm -rf "$work"' EXIT
python3 - "$work" <<'PY'
from pathlib import Path
import sys
work=Path(sys.argv[1]);s=Path('src/ajustes.c').read_text()
work.joinpath('shared-test-enum.inc').write_text(s[s.index('typedef enum {'):s.index('} OpcaoId;')+len('} OpcaoId;')])
s=Path('src/ajustes_device.inc').read_text()
work.joinpath('shared-test-json.inc').write_text(s[s.index('// JSON member iterator'):s.index('#define AJ_DEVICE_FILES')])
PY
cc -O1 -g -pthread -Isrc -I"$work" tests/settings_stage_i.c src/js.c src/jsw.c -o "$work/unit"
mkdir "$work/unit-data" "$work/transport-data"
"$work/unit" "$work/unit-data"
cc -O1 -g -pthread -Isrc -I"$work" tests/settings_stage_i_official.c src/js.c src/jsw.c -o "$work/official"
mkdir "$work/official-data"
"$work/official" "$work/official-data"
sources=()
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/ajustes.c|src/nuvem.c) continue;; esac
  sources+=("$source")
done
cc "${sources[@]}" tests/settings_stage_i_transport.c -Isrc -o "$work/transport" -O0 -g -pthread \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
  -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
"$work/transport" "$work/transport-data"

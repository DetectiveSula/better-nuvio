#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 tests/settings_stage_j4.py
work=$(mktemp -d "${TMPDIR:-/tmp}/bn-j4.XXXXXX")
trap 'rm -rf "$work"' EXIT
python3 - "$work/j4-transport.inc" <<'PYGEN'
from pathlib import Path
import sys
s=Path('tests/settings_stage_i_transport.c').read_text().split('int main(int argc,char **argv)')[0]
s=s.replace('#include "../src/ajustes.c"','#include "ajustes.c"')
Path(sys.argv[1]).write_text(s)
Path(sys.argv[1]).with_name('j5-native-fixture.inc').write_text(Path('tests/settings_stage_j4.c').read_text().split('int main(int argc,char **argv)')[0])
PYGEN
sources=()
for source in src/*.c src/dts/*.c; do
 case "$source" in src/main.c|src/ajustes.c|src/nuvem.c)continue;;esac
 sources+=("$source")
done
# Engine suite: keeps the pre-automatic explicit opt-in (src/ajustes.h NV_SYNC_AUTO).
flags=(-fno-omit-frame-pointer -DNV_SYNC_AUTO=0)
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined); fi
cc "${flags[@]}" "${sources[@]}" tests/settings_stage_j5_native.c -Isrc -Itests -I"$work" -o "$work/test" -O0 -g -pthread \
 -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
 -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations -Wno-macro-redefined
mkdir "$work/data"
"$work/test" "$work/data"

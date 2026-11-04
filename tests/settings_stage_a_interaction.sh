#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# Stage A sidebar, scrolling, search and scene routes; no window or account startup.
#   bash tests/settings_stage_a_interaction.sh
set -eu
cd "$(dirname "$0")/.."
sources=()
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/ajustes.c) continue;; esac
  sources+=("$source")
done
binary="$(mktemp ${TMPDIR:-/tmp}/nuvio-settings-stage-a.XXXXXX)"
trap 'rm -f "$binary"' EXIT
cc "${sources[@]}" tests/settings_stage_a_interaction.c -Isrc -o "$binary" \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
"$binary"

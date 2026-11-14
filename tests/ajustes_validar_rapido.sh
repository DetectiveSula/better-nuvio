#!/bin/bash
# Settings sync froze the Samsung S90C for 5.3 s: see tests/ajustes_validar_rapido.c.
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
set -eu
cd "$(dirname "$0")/.."
sources=()
for source in src/*.c src/dts/*.c; do
  [ "$source" != src/main.c ] && [ "$source" != src/rede.c ] && sources+=("$source")
done
cc "${sources[@]}" tests/posplay_offline_transport.c tests/ajustes_validar_rapido.c -Isrc -o "$TMPDIR/nuvio-ajustes-validar" \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
"$TMPDIR/nuvio-ajustes-validar"

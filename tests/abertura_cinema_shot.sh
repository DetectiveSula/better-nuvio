#!/bin/bash
# Frames of the cinematic opening (tests/abertura_cinema_shot.c). Needs a GL window.
#   bash tests/abertura_cinema_shot.sh <dir>
set -eu
cd "$(dirname "$0")/.."
out=${1:?output dir}
mkdir -p "$out"
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-abertura.XXXXXX")
trap 'rm -rf "$work"' EXIT
sources=()
for f in src/*.c src/dts/*.c; do [ "$f" != src/main.c ] && sources+=("$f"); done
cc "${sources[@]}" tests/abertura_cinema_shot.c -Isrc -o "$work/shot" -O1 -g \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
  -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations -Wno-macro-redefined
mkdir -p "$work/data"
NUVIO_DADOS="$work/data" NUVIO_TESTE_DIR="$work/data" "$work/shot" "$out"

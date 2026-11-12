#!/bin/bash
# Relogio de 24 h do Perfil com gfx_setor: compara com o quadrado inteiro e
# salva uma captura. Janela GL (fora da suite, como os outros *_shot.sh).
#   bash tests/relogio_shot.sh docs/shots/profile-stats/step3
set -eu
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-relogio-shot.XXXXXX")
trap 'rm -rf "$work"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do [ "$source" != src/main.c ] && sources+=("$source"); done
cc "${sources[@]}" tests/relogio_shot.c -Isrc -o "$work/shot" \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined -w
mkdir -p "$work/dados"
NUVIO_DADOS="$work/dados" "$work/shot" "$@"

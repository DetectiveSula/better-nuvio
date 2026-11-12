#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# Requires a native GL window; compares framebuffer pixels without TV or network.
set -eu
cd "$(dirname "$0")/.."
tmp=$(mktemp -d ${TMPDIR:-/tmp}/nuvio-gfx-cor-painel.XXXXXX)
trap 'rm -rf "$tmp"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do
  if [ "$source" != src/main.c ]; then sources+=("$source"); fi
done
cc -DNV_SHOT_HOOKS "${sources[@]}" tests/settings_expansion_render.c -Isrc -o "$tmp/teste" \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
"$tmp/teste"

python3 - <<'PYWIRES'
from pathlib import Path
assert 'NV_CARD_EPISODES, tr, tex, GFX_CARD, selF' in Path('src/episodios.c').read_text()
s=Path('src/detail.c').read_text()
assert 'NV_CARD_EPISODES, th, t2, GFX_CARD, f' in s
assert 'NV_CARD_TRAILERS, v, tex, GFX_CARD, f' in s
assert 'NV_CARD_CAST, av, t2, GFX_AVATAR, f' in s
assert '?NV_CARD_CONTINUE:NV_CARD_POSTERS, card, t, GFX_CARD, f' in Path('src/home.c').read_text()
print('PASS production surfaces carry their actual focus state into the pixel-tested shaders')
PYWIRES

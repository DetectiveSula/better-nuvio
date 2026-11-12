#!/usr/bin/env bash
# Source autoplay with AIOStreams-style TorBox links (url + hash); offline mocks.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-tb-addon-link.XXXXXX")
trap 'rm -rf "$work"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do
 case "$source" in src/main.c|src/streams.c|src/rede.c) continue;; esac
 sources+=("$source")
done
cc "${sources[@]}" tests/autoplay_tb_addon_link_transport.c tests/autoplay_tb_addon_link.c -Isrc -o "$work/test" \
 -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
 -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations -Wno-macro-redefined
"$work/test" "$@"

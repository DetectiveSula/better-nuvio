#!/usr/bin/env bash
# TorBox Instant bare hashes must not use up autoplay attempts; offline mocks.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-instant.XXXXXX")
trap 'rm -rf "$work"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do
 case "$source" in src/main.c|src/streams.c|src/rede.c) continue;; esac
 sources+=("$source")
done
cc "${sources[@]}" tests/autoplay_tb_addon_link_transport.c tests/autoplay_instant_hashes.c -Isrc -o "$work/test" \
 -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
 -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations -Wno-macro-redefined
"$work/test" "$@"

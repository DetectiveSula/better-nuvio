#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="${1:?output directory required}"
mkdir -p "$out/obj"
scratch=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-phone-render.XXXXXX")
trap 'rm -rf "$scratch"' EXIT
objects=(); pids=()
for source in src/*.c src/dts/*.c; do
  [ "$source" != src/main.c ] || continue
  object="$out/obj/$(basename "$source" .c).o"; objects+=("$object")
  if [ "$source" = src/rede.c ]; then source=tests/stage_d_offline_transport.c; fi
  cc -DNV_TOUCH_UI -O1 -g -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
    -Wno-deprecated-declarations -Wno-macro-redefined -c "$source" -o "$object" & pids+=($!)
  if [ ${#pids[@]} -ge 4 ]; then wait "${pids[0]}"; pids=("${pids[@]:1}"); fi
done
for pid in "${pids[@]}"; do wait "$pid"; done
cc -DNV_TOUCH_UI tests/phone_layout_shot.c "${objects[@]}" -Isrc \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
  -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations -o "$out/render"
NUVIO_DADOS="$scratch" "$out/render" "$out" "${2:-0}"

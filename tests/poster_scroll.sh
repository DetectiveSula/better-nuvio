#!/bin/bash
# Poster fast-scroll harness for the Samsung .tpk cache policy (see tests/poster_scroll.c).
# Env: PS_RAM (MemTotal MB, default 2400), PS_MB (Settings "memory for images"), PS_PASSO (ms per row step),
# PS_HERO_MS (hero backdrop decode ms, enables hero + neighbour requests), PS_LIVRE (MemAvailable MB sample).
# TEX_SRC overrides the tex_cache.c under test (for before/after comparisons).
set -eu
cd "$(dirname "$0")/.."
TMPDIR="${TMPDIR:-/tmp}"; TMPDIR="${TMPDIR%/}"
obj=${PS_OBJ:-$TMPDIR/nuvio-poster-scroll-obj}
mkdir -p "$obj"
FLAGS="-O1 -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -Wno-deprecated-declarations -Wno-macro-redefined -DNV_TEX_HARNESS_TPK"
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/tex_cache.c) continue;; esac
  o="$obj/$(echo "$source" | tr / _).o"
  [ "$o" -nt "$source" ] || echo "$source"
done | xargs -P 8 -I{} sh -c 'cc '"$FLAGS"' -c "$1" -o "$2/$(echo "$1" | tr / _).o"' _ {} "$obj"
src="${TEX_SRC:-src/tex_cache.c}"
tmpc="$obj/harness.c"
abs_src="$(cd "$(dirname "$src")" && pwd)/$(basename "$src")"
sed -e "s#\.\./src/tex_cache\.c#$abs_src#" -e "s#\.\./src/#$PWD/src/#g" tests/poster_scroll.c > "$tmpc"
cc $FLAGS -Isrc -iquote src -c "$tmpc" -o "$obj/harness.o"
cc $(ls "$obj"/*.o | grep -v harness.o) "$obj/harness.o" -o "$obj/poster_scroll" -L/opt/homebrew/lib \
  -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL
poster="${PS_POSTER:-$TMPDIR/poster-500x750.jpg}"
[ -f "$poster" ] || sips -z 750 500 tests/amostra.jpg --out "$poster" >/dev/null
echo "== cold disk =="
"$obj/poster_scroll" "$poster" "$TMPDIR/nuvio-poster-scroll-cache" | grep -E '^\[(ps|tex-stats)\]'
echo "== warm disk (every poster already on disk, as after days of use) =="
PS_PREWARM=1 "$obj/poster_scroll" "$poster" "$TMPDIR/nuvio-poster-scroll-cache" | grep '^\[ps\] prewarm'
PS_WARM=1 "$obj/poster_scroll" "$poster" "$TMPDIR/nuvio-poster-scroll-cache" | grep -E '^\[(ps|tex-stats)\]'

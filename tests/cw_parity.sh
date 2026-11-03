#!/usr/bin/env bash
# Source-backed Nuvio Android CW parity gates. No account or network required.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-cw-parity.XXXXXX")
trap 'rm -rf "$work"' EXIT
bash tests/progresso.sh
bash tests/syncprog.sh
bash tests/contalib.sh
flags=(-O1 -g -Isrc -ffunction-sections -fdata-sections)
if [ "$(uname -s)" = Darwin ]; then flags+=(-Wl,-dead_strip); else flags+=(-Wl,--gc-sections); fi
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
cc "${flags[@]}" src/cwordem.c src/cwretido.c tests/cwordem.c -o "$work/order" -pthread -lm
"$work/order"
# SDL headers are used by discovery's public interface; the fixture stubs SDL
# and network I/O. It exercises the actual catalog and discovery publishers.
read -r -a sdl <<< "$(pkg-config --cflags sdl2)"
cc "${flags[@]}" "${sdl[@]}" src/catalogo.c src/progresso.c src/cwordem.c src/cwretido.c \
  tests/cwordem_desc.c src/cotacat.c src/js.c src/metaprov.c src/colecoes.c \
  src/redeurl.c src/catordem.c -o "$work/discovery" -pthread -lm
"$work/discovery"
bash tests/cwlocal.sh
bash tests/cwdesmarcado.sh
bash tests/simkl_cw.sh
printf '%s\n' 'PASS: Continue Watching Android parity gates'

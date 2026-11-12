#!/bin/bash
# Painel de 30 dias do perfil (Nuvio sync): conta pura, sem rede nem SDL.
set -eu
cd "$(dirname "$0")/.."
flags=(-O1 -g -std=gnu11 -Isrc -Wall -Wextra -Wno-misleading-indentation)
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
bin="$(mktemp "${TMPDIR:-/tmp}/nuvio-perfilnuvio.XXXXXXXX")"
trap 'rm -f "$bin"' EXIT
cc "${flags[@]}" src/perfilnuvio.c src/js.c tests/perfilnuvio.c -o "$bin"
"$bin"

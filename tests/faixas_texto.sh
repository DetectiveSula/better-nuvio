#!/bin/bash
# Cross-platform embedded text subtitle fixture used by mkvass_texto.sh.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-subtitle-text.XXXXXX")
trap 'rm -rf "$work"' EXIT
for target in LG NV_TPK NV_ANDROID; do
 cc -D"$target" tests/faixas_texto.c src/faixasmkv.c -Isrc -O1 -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -ffunction-sections -fdata-sections -Wl,-dead_strip -Wno-macro-redefined -o "$work/test"
 "$work/test"
done

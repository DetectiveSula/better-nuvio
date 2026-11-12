#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-expansion-subs.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc tests/settings_expansion_subtitles.c src/linguas.c src/legextras.c -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -std=gnu11 -pthread -ffunction-sections -fdata-sections -Wl,-dead_strip -Wno-macro-redefined -Wno-return-type -o "$work/test"
"$work/test"

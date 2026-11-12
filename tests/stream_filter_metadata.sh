#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-stream-filter-metadata.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc tests/stream_filter_metadata.c src/streams.c src/fontefiltro.c src/linguas.c \
  -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -O1 -g -ffunction-sections -fdata-sections -Wl,-dead_strip \
  -Wno-deprecated-declarations -Wno-macro-redefined -o "$work/test"
"$work/test"

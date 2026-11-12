#!/usr/bin/env bash
# Per-profile synced Home visibility across profile switches (real fileiras,
# catordem and catordem cache). No network, no account.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-home-profiles.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc -O1 -g -DNV_ANDROID -DFIL_TESTE -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  src/fileiras.c src/catordem.c src/catordemcache.c src/js.c tests/home_rows_profiles.c -pthread -o "$work/test"
"$work/test" | grep -E '^(  profile|PASS)'

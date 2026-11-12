#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc tests/profile_editor_hold.c -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -ffunction-sections -fdata-sections -Wl,-dead_strip -pthread -O1 -g \
  -Wno-deprecated-declarations -o "${TMPDIR:-/tmp}/nuvio-profile-editor-hold"
"${TMPDIR:-/tmp}/nuvio-profile-editor-hold"

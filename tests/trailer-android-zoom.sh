#!/bin/bash
# Stage H trailer polish (Android): zoom before the first visible frame, art
# held until the native layout confirms it, no full-frame step or re-apply.
set -eu
cd "$(dirname "$0")/.."
tmp="$(mktemp -d "${TMPDIR:-/tmp}/nuvio-trailer-android-zoom-XXXXXX")"
trap 'rm -rf "$tmp"' EXIT
cc -DNV_ANDROID -O1 -g -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -ffunction-sections -fdata-sections tests/trailer-android-zoom.c \
  -Wl,-dead_strip,-undefined,dynamic_lookup -Wno-deprecated-declarations \
  -o "$tmp/test"
"$tmp/test"

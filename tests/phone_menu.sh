#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-phone-menu.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc tests/phone_menu.c -DNV_TOUCH_UI -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -Wno-deprecated-declarations -Wno-macro-redefined -Wl,-undefined,dynamic_lookup -o "$work/phone-menu"
"$work/phone-menu"

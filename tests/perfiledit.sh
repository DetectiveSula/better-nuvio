#!/bin/bash
# Profile editor rules and request bodies (tests/perfiledit.c), no GL, no network.
set -euo pipefail
cd "$(dirname "$0")/.."
tmp="$(mktemp -d "${TMPDIR:-/tmp}/nuvio-perfiledit-XXXXXX")"
trap 'rm -rf "$tmp"' EXIT
cc -std=gnu11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined -DNV_PERFILEDIT_SO_REGRAS \
  -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  src/perfiledit.c src/js.c src/jsw.c tests/perfiledit.c -o "$tmp/teste"
"$tmp/teste"

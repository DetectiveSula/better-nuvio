#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-postplay-preview.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc tests/postplay_preview.c -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -Wall -Wextra -Werror -o "$work/test"
"$work/test"

#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/bn-account.XXXXXX")
trap 'rm -rf "$work"' EXIT
mkdir "$work/data"
cc -std=c11 -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -fsanitize=address,undefined -g -pthread -Isrc -I/opt/homebrew/include/SDL2 \
 src/bnaccount.c src/bncodec.c src/bnsettings.c src/js.c src/jsw.c tests/bnaccount.c -o "$work/test"
"$work/test" "$work/data"

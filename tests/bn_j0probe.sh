#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/bn-j0probe.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -fsanitize=address,undefined -g -Isrc src/bncodec.c src/bnsettings.c src/jsw.c tests/bn_j0probe.c -o "$work/test"
"$work/test"

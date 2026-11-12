#!/bin/bash
# Caixa do setor de anel do relogio de 24 h (src/setor.h). Sem GL.
set -eu
cd "$(dirname "$0")/.."
bin="$(mktemp "${TMPDIR:-/tmp}/nuvio-setor.XXXXXXXX")"
trap 'rm -f "$bin"' EXIT
cc -O1 -g -std=gnu11 -Wall -Wextra -Isrc ${SANITIZE:+-fsanitize=address,undefined} tests/setor.c -lm -o "$bin"
"$bin"

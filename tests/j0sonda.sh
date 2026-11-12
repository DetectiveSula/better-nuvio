#!/usr/bin/env bash
# Stage J0 namespace probe against a mock account server. No network, no account.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-j0.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc -O1 -g -Wall -DNV_J0_TESTE -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  src/j0sonda.c src/jsw.c tests/j0sonda.c -pthread -o "$work/test"
"$work/test" < /dev/null
# Release Android builds (NDEBUG) contain no probe code or account calls.
cc -c -DNV_ANDROID -DNDEBUG -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 src/j0sonda.c -o "$work/rel.o"
! nm "$work/rel.o" | grep -q -E 'sessao_rpc|dados_gravar'
echo "PASS: release build compiles the probe out"

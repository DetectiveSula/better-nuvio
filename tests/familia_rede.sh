#!/bin/bash
# Stage H Family Watching live transport against a fake account server.
#   bash tests/familia_rede.sh
set -eu
export TMPDIR="${TMPDIR:-/tmp}"
cd "$(dirname "$0")/.."
flags=()
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
cc ${flags[@]+"${flags[@]}"} -DNV_FAMILIA_SEM_FIO src/familia.c src/familia_app.c src/familia_rede.c src/js.c tests/familia_rede.c \
  -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -o "${TMPDIR%/}/nuvio-familia-rede-tests" -O1 -g \
  -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -Wno-macro-redefined -lpthread -lm
"${TMPDIR%/}/nuvio-familia-rede-tests"

#!/bin/bash
# Stage H Family Watching (src/familia.c, src/familia_app.c) with fixtures:
# no SDL window, no network, no account.
#   bash tests/familia.sh
set -eu
export TMPDIR="${TMPDIR:-/tmp}"
cd "$(dirname "$0")/.."
flags=()
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
cc ${flags[@]+"${flags[@]}"} -DNV_FAMILIA_SEM_FIO src/familia.c src/familia_app.c src/js.c tests/familia.c \
  -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -o "${TMPDIR%/}/nuvio-familia-tests" -O1 -g \
  -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -Wno-macro-redefined -lpthread -lm
"${TMPDIR%/}/nuvio-familia-tests"

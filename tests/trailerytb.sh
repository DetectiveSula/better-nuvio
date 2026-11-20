#!/bin/bash
# The YouTube trailer extractor (src/trailerytb.c), ported from NuvioTV.
#   bash tests/trailerytb.sh                        parsing only (offline)
#   NUVIO_YT_VIVO=Way9Dexny3w bash tests/trailerytb.sh   plus one real extraction
set -eu
cd "$(dirname "$0")/.."
bin=${TMPDIR:-/tmp}/nuvio-trailerytb
cc -O1 -g -Wall -Wextra -fsanitize=address,undefined -Isrc -I/opt/homebrew/include \
  tests/trailerytb.c src/trailerytb.c src/js.c src/rede.c src/redeurl.c -o "$bin" -lpthread 2>&1 | grep -v "^ld: warning" || true
"$bin"

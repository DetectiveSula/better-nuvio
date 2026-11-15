#!/bin/bash
# Continue Watching keyed by a TMDB number gets TMDB artwork. Fake network.
#   bash tests/cw_tmdb_arte.sh
set -euo pipefail
cd "$(dirname "$0")/.."
dir=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-cwtmdb.XXXXXX")
trap 'rm -rf "$dir"' EXIT
cc src/trakt.c tests/stub_fichameta.c src/js.c src/metaprov.c src/metaep.c tests/cw_tmdb_arte.c \
  -Isrc -Itests -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -o "$dir/teste" -O1 -g -Wall -Wextra -Wl,-dead_strip -lpthread
"$dir/teste"

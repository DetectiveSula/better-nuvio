#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc tests/psestilos_loop.c -Isrc -O1 -g -Wall -Wno-deprecated-declarations \
  -Wno-macro-redefined -o "${TMPDIR:-/tmp}/nuvio-psestilos-loop"
"${TMPDIR:-/tmp}/nuvio-psestilos-loop"

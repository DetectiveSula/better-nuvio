#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# Sem rede: o trailer da Apple COM SOM no .tpk (master reduzido no servidor local).
set -eu
cd "$(dirname "$0")/.."
cc -O1 -g -Wall -Wno-unused-function -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 tests/trailerapple-som.c src/js.c -L/opt/homebrew/lib -lSDL2 -lpthread -o ${TMPDIR:-/tmp}/nuvio-trailerapple-som
${TMPDIR:-/tmp}/nuvio-trailerapple-som

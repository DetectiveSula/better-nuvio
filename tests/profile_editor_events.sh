#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -DNV_PROFILE_EDITOR_TEST src/profile_editor.c src/perfis.c src/js.c src/jsw.c tests/profile_editor_events.c \
  -Isrc -Itests -I/opt/homebrew/include -O1 -g -Wall -Wno-deprecated-declarations -Wno-macro-redefined -Wno-unused-function \
  -o "${TMPDIR:-/tmp}/nuvio-profile-editor-events"
DIR=$(mktemp -d); trap 'rm -rf "$DIR"' EXIT
NUVIO_TESTE_DIR="$DIR" "${TMPDIR:-/tmp}/nuvio-profile-editor-events"

#!/usr/bin/env bash
# Automatic settings sync: two installations (separate processes and data dirs)
# against one file-backed mock account, with no debug arming. Never contacts the
# live backend. Reuses the Stage J7 harness (everything before its main()).
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/bn-sync-auto.XXXXXX")
trap '[ -n "${SYNC_KEEP:-}" ] && cp "$work/log.txt" "$SYNC_KEEP"; rm -rf "$work"' EXIT
python3 - "$work/j4-transport.inc" "$work/j7-harness.inc" <<'PYGEN'
from pathlib import Path
import sys
s=Path('tests/settings_stage_i_transport.c').read_text().split('int main(int argc,char **argv)')[0]
Path(sys.argv[1]).write_text(s.replace('#include "../src/ajustes.c"','#include "ajustes.c"'))
h=Path('tests/settings_stage_j7.c').read_text().split('int main(int argc,char **argv)')[0]
Path(sys.argv[2]).write_text(h)
PYGEN
sources=()
for source in src/*.c src/dts/*.c; do
 case "$source" in src/main.c|src/ajustes.c|src/nuvem.c)continue;;esac
 sources+=("$source")
done
build(){ local out=$1; shift
 cc -fno-omit-frame-pointer -DNV_J7_TESTE "$@" "${sources[@]}" tests/settings_sync_auto.c -Isrc -Itests -I"$work" -o "$out" -O0 -g -pthread \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
  -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations -Wno-macro-redefined -Wno-unused-function; }
extra=(); if [ "${SANITIZE:-0}" = 1 ]; then extra+=(-fsanitize=undefined -fno-sanitize-recover=undefined); fi
build "$work/test" ${extra[@]+"${extra[@]}"}
build "$work/test-ro" -DNV_VALIDATION_READONLY ${extra[@]+"${extra[@]}"}
log="$work/log.txt"
run(){ local bin=$1 dev=$2 step=$3; shift 3; if ! env "$@" "$bin" "$work/$dev" "$step" >>"$log" 2>&1; then echo "FAIL: device $dev step $step"; tail -40 "$log"; exit 1; fi; }
mkdir "$work/server" "$work/a" "$work/b" "$work/ro-server" "$work/r"
export J7_SERVER="$work/server"
run "$work/test" a a-edit
run "$work/test" b b-restore
run "$work/test" b b-edit
run "$work/test" a a-receive
run "$work/test" a a-offline-edit J7_OFFLINE=1
run "$work/test" a a-reconnect
run "$work/test" b b-follow
run "$work/test" b other-account
mkdir "$work/fi-server" "$work/fi-a" "$work/fi-b" "$work/fi-c"
J7_SERVER="$work/fi-server" run "$work/test" fi-a files-a
J7_SERVER="$work/fi-server" run "$work/test" fi-b files-b
printf '%s' 'tv-c-own-fanart-key' > "$work/fi-c/fanart.txt"
J7_SERVER="$work/fi-server" run "$work/test" fi-c files-c
mkdir "$work/ru-server" "$work/ru-a" "$work/ru-b"
printf '%s' '{"features":{"player_settings":{"stream_auto_play_regex":{"type":"string","value":""},"stream_auto_play_selected_addons":{"type":"string_set","value":[]},"stream_auto_play_selected_plugins":{"type":"string_set","value":[]}}}}' > "$work/ru-server/tv-settings-test-p1.json"
J7_SERVER="$work/ru-server" run "$work/test" ru-a rules-a
J7_SERVER="$work/ru-server" run "$work/test" ru-b rules-b
J7_SERVER="$work/ru-server" run "$work/test" ru-a rules-official
mkdir "$work/st-server" "$work/st-a" "$work/st-b"
J7_SERVER="$work/st-server" run "$work/test" st-a store-a
J7_SERVER="$work/st-server" run "$work/test" st-b store-b
mkdir "$work/dw-server" "$work/dw"
J7_SERVER="$work/dw-server" run "$work/test" dw device-wide
mkdir "$work/up-server" "$work/u" "$work/u2"
J7_SERVER="$work/up-server" run "$work/test" u upgrade
J7_SERVER="$work/up-server" run "$work/test" u2 reinstall
J7_SERVER="$work/ro-server" run "$work/test-ro" r readonly
grep '^PASS' "$log"
if grep -E '\[(bn-sync|ajustes-oficial)\]' "$log" | grep -qE 'settings-(test|other)|"value"|https?://'; then
 echo "FAIL: a sync log line carries an account id or a value"; exit 1; fi
echo "PASS: automatic settings sync two-device mock suite"

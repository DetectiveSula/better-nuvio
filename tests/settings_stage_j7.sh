#!/usr/bin/env bash
# Stage J7a: two installations (separate processes and data dirs) against one
# file-backed mock account. Never contacts the live backend.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/bn-j7.XXXXXX")
trap '[ -n "${J7_KEEP:-}" ] && cp "$work/log.txt" "$J7_KEEP"; rm -rf "$work"' EXIT
python3 - "$work/j4-transport.inc" <<'PYGEN'
from pathlib import Path
import sys
s=Path('tests/settings_stage_i_transport.c').read_text().split('int main(int argc,char **argv)')[0]
Path(sys.argv[1]).write_text(s.replace('#include "../src/ajustes.c"','#include "ajustes.c"'))
PYGEN
sources=()
for source in src/*.c src/dts/*.c; do
 case "$source" in src/main.c|src/ajustes.c|src/nuvem.c)continue;;esac
 sources+=("$source")
done
flags=(-fno-omit-frame-pointer -DNV_J7_TESTE)
if [ "${J7_READONLY_TEST:-0}" = 1 ]; then flags+=(-DNV_VALIDATION_READONLY); fi
# Superseded (owner decision, 12 Nov 2026): settings sync is automatic, so the
# J7a "default Off" and arming scenarios no longer describe the product. The
# two-device coverage moved to tests/settings_sync_auto.sh; only the read-only
# validation check below still applies here.
if [ "${J7_READONLY_TEST:-0}" != 1 ]; then
 echo "SKIP: J7a arming suite superseded by tests/settings_sync_auto.sh (automatic settings sync)"; exit 0
fi
if [ "${J7_LOGIN_TEST:-0}" = 1 ]; then
 [ "${J7_READONLY_TEST:-0}" = 1 ] || { echo "Login test requires read-only test"; exit 2; }
 flags+=(-DNV_VALIDATION_LOGIN)
fi
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=undefined -fno-sanitize-recover=undefined); fi
cc "${flags[@]}" "${sources[@]}" tests/settings_stage_j7.c -Isrc -Itests -I"$work" -o "$work/test" -O0 -g -pthread \
 -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
 -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations -Wno-macro-redefined
mkdir "$work/server" "$work/a" "$work/b"
export J7_SERVER="$work/server"
log="$work/log.txt"
run(){ local dev=$1 step=$2; shift 2; if ! env "$@" "$work/test" "$work/$dev" "$step" >>"$log" 2>&1; then echo "FAIL: device $dev step $step"; tail -40 "$log"; exit 1; fi; }
if [ "${J7_READONLY_TEST:-0}" = 1 ]; then
 run a readonly-arming
 grep '^PASS' "$log"
 exit 0
fi
run a a-default-off
run a a-arm
run b b-restore
run b b-edit
run a a-receive
run a a-offline-edit J7_OFFLINE=1
run a a-reconnect
run b b-follow
run a conflict-a J7_OFFLINE=1
run b conflict-b J7_OFFLINE=1
run a sync; run b sync; run a sync; run b sync
cmp -s "$work/server/final-a.txt" "$work/server/final-b.txt" || { echo "FAIL: devices diverged after conflict"; diff "$work/server/final-a.txt" "$work/server/final-b.txt" | head; exit 1; }
run a settle; run b settle
cmp -s "$work/server/settled-a.txt" "$work/server/settled-b.txt" || { echo "FAIL: devices diverged after settling"; exit 1; }
grep '^STATE' "$log"
echo "PASS: same-field offline conflict converged on both devices (J4 per-field revision/device ordering; J6 three-way merge; no first-writer guarantee for J4)"
run a a-profiles
run a a-account
run a a-expiry
run a a-expired-restart
run a a-tampered-restart
run a a-local-only
run a a-disarm

# J0 probe compatibility: fresh account whose profiles 1 and 2 hold the exact
# historical probe document (src/j0sonda.c), two fresh devices.
mkdir "$work/server2" "$work/c" "$work/d"
printf '%s' '{"schema":"better-nuvio-probe","version":0,"probe":"0A1B2C3D"}' > "$work/server2/better_nuvio-settings-test-p1.json"
printf '%s' '{"probe": "FFFF0000", "schema": "better-nuvio-probe", "version": 0}' > "$work/server2/better_nuvio-settings-test-p2.json"
cp "$work/server2/better_nuvio-settings-test-p2.json" "$work/probe-p2.orig"
export J7_SERVER="$work/server2"
run c p-first
run d p-restore
run c p-offline-edit J7_OFFLINE=1
run c p-reconnect
run d p-follow
run c p-profile2
mkdir "$work/server3" "$work/e"
printf '%s' '{"schema":"better-nuvio-probe","version":0,"probe":"0A1B2C3D","note":"x"}' > "$work/server3/better_nuvio-settings-test-p1.json"
export J7_SERVER="$work/server3"
run e u-unknown
# Native Home layout is Better-only, including all three modes and profile isolation.
mkdir "$work/layout-server" "$work/layout-a" "$work/layout-b"
printf '%s' '{"features":{"unrelated":{"sentinel":{"type":"string","value":"keep"}}}}' > "$work/layout-server/tv-settings-test-p1.json"
export J7_SERVER="$work/layout-server"
run layout-a layout-offline J7_OFFLINE=1
run layout-a layout-send-2
run layout-b layout-receive-2
for mode in 1 2 0; do
 run layout-a "layout-send-$mode"
 run layout-b "layout-receive-$mode"
done
export J7_SERVER="$work/server3"
# Independent permissions: J4/J5 and J6 armed separately (fresh account, two restarts).
mkdir "$work/server4" "$work/f"
export J7_SERVER="$work/server4"
mark=$(wc -l < "$log" | tr -d " ")
run f s-j4only
tail -n +"$mark" "$log" | grep '\[ajustes-oficial\] perfil 1 pendentes com uploads J6 desligados 1 \[mdblist_show_imdb\]' >/dev/null || { echo "FAIL: J6 pending names not visible while J6 is off"; exit 1; }
tail -n +"$mark" "$log" | grep '\[j7\] uploads J4/J5 LIGADOS, J6 desligados (perfil 1)' >/dev/null || { echo "FAIL: scope state not logged"; exit 1; }
run f s-restart
run f s-codes
run f s-profile
run f s-legacy
run f s-legacy-check
grep -q 'documento J0 de teste: tratado como nao inicializado' "$log" || { echo "FAIL: probe diagnostic missing"; exit 1; }
grep '^PASS' "$log"
# Logs carry names, counts, HTTP codes and fingerprints only.
run a privacy
if grep -qE 'settings-(test|other)' "$log"; then echo "FAIL: an account id reached the logs"; grep -nE 'settings-(test|other)' "$log" | sed -E 's/settings-(test|other)/<id>/' | head; exit 1; fi
for f in "$work"/[a-f]/settings-official-j6-*.txt; do head -c 3 "$f" | grep -q '^v2 ' && ! grep -qE 'settings-(test|other)' "$f" || { echo "FAIL: J6 journal $f holds an account id"; exit 1; }; done
if grep -qE 'eyJ|j7-secret-credential-value|ative com token|codigo [0-9A-F]{8}' "$log"; then echo "FAIL: a token, activation code or credential reached the logs"; exit 1; fi
if grep -E '\[(bn-sync|ajustes-oficial|j7)\]' "$log" | grep -qE 'settings-(test|other)|"value"|https?://'; then
 echo "FAIL: a J7/J4/J6 log line carries an account id or a value"; grep -E '\[(bn-sync|ajustes-oficial|j7)\]' "$log" | grep -E 'settings-|value|http' | head; exit 1; fi
grep -c '\[bn-sync\]\|\[ajustes-oficial\]\|\[j7\]' "$log" | sed 's/^/log lines with names only: /'
grep -E '\[j7\] foto' "$log" | tail -8
echo "PASS: J7a two-device mock suite"

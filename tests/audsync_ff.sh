#!/bin/bash
# Subtitle AutoSync on the Samsung .tpk: the software audio decoder backend
# (src/audsync_ff.c) over real HTTP (rede.c + DtsEngine) on generated MKV/MP4
# samples, through audsync -> legsync -> autosync, with known offsets; and the embedded
# text track (legref) winning over audio when the file has one.
# Needs the host FFmpeg with the .tpk component set:
#   NUVIO_DTS_TARGET=tpk NUVIO_DTS_SRC=<ffmpeg-7.1.5 source> NUVIO_DTS_ROOT=/tmp/nuvio-dts-tpk-host \
#     CC=cc sh tools/build-dts-ffmpeg.sh
#   NUVIO_DTS_ROOT=/tmp/nuvio-dts-tpk-host bash tests/audsync_ff.sh     SANITIZE=1 / SANITIZE=thread
set -euo pipefail
cd "$(dirname "$0")/.."
T=${TMPDIR:-/tmp}; T=${T%/}
PREFIX=${NUVIO_DTS_ROOT:-$T/nuvio-dts-tpk-host}
[ -f "$PREFIX/include/libavcodec/avcodec.h" ] || { echo "host FFmpeg missing in $PREFIX (see the header of this script)" >&2; exit 1; }
D=$(mktemp -d "$T/nv-audsync-ff.XXXXXX")
PID=""
trap '[ -n "$PID" ] && kill "$PID" 2>/dev/null; rm -rf "$D"' EXIT
flags=()
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
elif [ "${SANITIZE:-0}" = thread ]; then flags+=(-fsanitize=thread -fno-omit-frame-pointer); fi
FF=(-L"$PREFIX/lib" -lavformat -lavcodec -lswresample -lavutil -lz)
# Without FFmpeg the backend is a stub that never registers (Tizen 4/5, other platforms).
cc -Isrc -Wall -Wextra -Werror -fsyntax-only src/audsync_ff.c
cc -Isrc -Wall -Wextra -Werror -fsyntax-only -DNV_DTS_FFMPEG -I"$PREFIX/include" src/audsync_ff.c
# legref.c as on the .tpk 6+ (NV_TPK: the embedded-subtitle reference is on there; audio is the fallback).
cc ${flags[@]+"${flags[@]}"} -c -DNV_TPK -Isrc -O1 -g src/legref.c -o "$D/legref.o"
cc ${flags[@]+"${flags[@]}"} -std=gnu11 -DNV_DTS_FFMPEG -Isrc -Itests -I"$PREFIX/include" -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -O1 -g -Wall -Wextra -Wno-misleading-indentation \
  src/audsync_ff.c src/dts/dts_engine.c src/rede.c src/redeurl.c \
  src/audvad.c src/audsync.c src/legsync.c src/legsyncui.c src/legenda2.c "$D/legref.o" src/autosync.c src/legenda.c src/assrender.c \
  tests/audsync_ff.c "${FF[@]}" -pthread -lm -ldl -o "$D/t"
python3 tests/audsync_ff_server.py "$D" "$D/port" "$D/requests" &
PID=$!
for _ in $(seq 50); do [ -s "$D/port" ] && break; sleep .1; done
"$D/t" "$D" "$(cat "$D/port")" 2>&1 | grep -v '^\[legenda\]'
python3 - "$D/requests" <<'PY'
import json, sys
rows = [json.loads(l) for l in open(sys.argv[1])]
with_hdr = [r for r in rows if r['fixture'] == 'yes']
assert with_hdr and all(r['range'] for r in with_hdr), 'media read without Range'
assert len(rows) - len(with_hdr) >= 1, 'the header-less case never reached the origin'
print(f'origin: {len(with_hdr)} ranged media requests with the playing headers')
PY

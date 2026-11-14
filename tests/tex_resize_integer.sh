#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-resize-integer.XXXXXX")
trap 'rm -rf "$work"' EXIT
python3 - "$work" <<'PY'
from pathlib import Path
import sys
source=Path('src/tex_cache.c').read_text()
start=source.index('static SDL_Surface *reduzirBilinear(')
end=source.index('// PIRAMIDE NA CPU',start)
code=source[start:end]
Path(sys.argv[1],'resize.inc').write_text(code)
reference=code.replace('dst=reduzirInteiro(src,lw,lh);','dst=NULL;')
assert reference!=code
reference=reference.replace('reduzirBilinear','referenceBilinear').replace('reduzirInteiro','referenceInteiro').replace('tex_reduzir','referenceResize')
Path(sys.argv[1],'reference.inc').write_text(reference)
PY
flags=()
if [ "${SANITIZE:-0}" = 1 ];then
  # SDL's dynamic API loader opens an error dialog before main under ASAN
  # on this macOS release. UBSAN runs without that unrelated startup failure.
  sanitizer=address,undefined
  [ "$(uname -s)" != Darwin ] || sanitizer=undefined
  flags+=(-fsanitize="$sanitizer" -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
fi
cc -O2 "${flags[@]+"${flags[@]}"}" tests/tex_resize_integer.c -I"$work" \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -o "$work/test"
"$work/test"

#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc tests/psestilos_loop.c -Isrc -O1 -g -Wall -Wno-deprecated-declarations \
  -Wno-macro-redefined -o "${TMPDIR:-/tmp}/nuvio-psestilos-loop"
"${TMPDIR:-/tmp}/nuvio-psestilos-loop"
# The old catalog mural is drawn by none of today's five backgrounds; asking for
# its 36 posters anyway kept them "hot" (never evicted) and, with the texture cap
# halved under memory pressure, starved the visible wall (S90C, 14 Nov 2026:
# "tela=58/47.9MB fila-tex=20"). Request and draw must share one condition.
grep -q 'if (!muralVisivel(modoFundo())) { muralTexturas\[i\] = 0; continue; }' src/perfilsel.c
grep -q 'else if (muralVisivel(modo) && cob < 0.999f)' src/perfilsel.c
echo "PASS: the hidden catalog mural requests no textures"

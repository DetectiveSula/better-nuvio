#!/usr/bin/env bash
# Stage G parity with regular NuvioTV's Home: realistic manifests and the synced
# payload regular Nuvio writes, through Better Nuvio's real catordem/fileiras
# code, compared with a port of the official ordering. No network, no account.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-home-official.XXXXXX")
trap 'rm -rf "$work"' EXIT
python3 - "$work/official-eligibility.inc" <<'PY'
from pathlib import Path
import sys
s=Path('src/descoberta.c').read_text()
a=s.index('static int foraDaHome(')
b=s.index('static int exigeBusca(')
e=s.index('\n}\n',b)+3
Path(sys.argv[1]).write_text(s[a:e])
PY
cc -O1 -g -DNV_ANDROID -DFIL_TESTE -Isrc -I"$work" -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  src/fileiras.c src/catordem.c src/js.c tests/home_rows_official.c -pthread -o "$work/test"
for follow in 0 1; do
  for variant in default personal; do
    python3 tests/home_rows_official.py gen "$work/scenario" "$follow" "$variant"
    "$work/test" "$work/scenario" > "$work/actual" 2> "$work/log" || { cat "$work/log"; exit 1; }
    python3 tests/home_rows_official.py check "$work/actual" "$follow" "$variant"
  done
done
python3 - <<'PY'
from pathlib import Path
d=Path('src/descoberta.c').read_text(); h=Path('src/home.c').read_text()
# The remote pre-sort would discard manifest order; fil_unir owns the projected order.
assert 'catordem_tem_ordem() && nOrdem > 0 && !fil_tem_projecao()) {' in d and 'catordem_tem_ordem() && nCat > 0 && !fil_tem_projecao()) {' in d
assert 'fil_tem_projecao() ? 0 : catordem_unir(' in h
assert 'foraDaHome(p, f)' in d and 'catordem_tipo_chave(tipoK, tipoK' in d
print('PASS: discovery and Home leave the projected order to fil_unir; showInHome and full-type identity wired')
PY

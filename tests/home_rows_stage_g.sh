#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-home-g-build.XXXXXX")
trap 'rm -rf "$work"' EXIT
python3 - "$work/stage-g-editor.inc" <<'PY'
from pathlib import Path
import sys
s=Path('src/ajustes.c').read_text();a=s.index('static void filMontarLista(void) {');b=s.index('// Linha da tela -> indice em fil_*',a)
Path(sys.argv[1]).write_text(s[a:b])
s=Path('src/perfis.c').read_text();a=s.index('int perfis_home_config(void) {');b=s.index('void perfis_carregar_ativo(void)',a)
Path(sys.argv[1]).with_name('stage-g-profile.inc').write_text(s[a:b])
s=Path('src/descoberta.c').read_text();a=s.index('static int prioCatalogo(');b=s.index('// Os catalogos que a cota',a)
Path(sys.argv[1]).with_name('stage-g-quota.inc').write_text(s[a:b])
PY
cc -O1 -g -DNV_ANDROID -DFIL_TESTE -Isrc -I"$work" -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
 src/fileiras.c src/catordem.c src/catordemcache.c src/js.c tests/home_rows_stage_g.c -pthread -o "$work/test"
"$work/test"

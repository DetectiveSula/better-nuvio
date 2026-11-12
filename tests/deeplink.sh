#!/bin/bash
# Parser de links externos (tests/deeplink.c).
#
#   bash tests/deeplink.sh
set -eu
cd "$(dirname "$0")/.."
out="${TMPDIR:-/tmp}/nv-deeplink-tests"
cc ${SANITIZE:+-fsanitize=address,undefined} src/deeplink.c tests/deeplink.c -Isrc -o "$out" -O1 -g -Wall
"$out"
python3 - <<'PY'
import os
import subprocess
import tempfile
from pathlib import Path

source = Path('src/app.c').read_text()
update=source[source.index('void app_atualizar('):]
assert update.index('deeplink_desarmar_pendente()') < update.index('if (tela == TELA_ESCOLHA_PERFIL')
start = source.index('static void deeplinkAbrir(')
end = source.index('\nstatic void spotAbrir(', start)
with tempfile.TemporaryDirectory(prefix='deeplink-app-') as tmp:
    Path(tmp, 'deeplink_abrir.inc').write_text(source[start:end])
    dispatch = source.index('  { DeepLink dl;')
    dispatch_end = source.index('\n  //', dispatch)
    Path(tmp, 'deeplink_dispatch.inc').write_text(source[dispatch:dispatch_end])
    binary = str(Path(tmp, 'test'))
    flags = ['-fsanitize=address,undefined'] if os.environ.get('SANITIZE') else []
    subprocess.run(['cc', *flags, 'src/deeplink.c', 'tests/deeplink_app.c',
                    '-Isrc', '-I' + tmp, '-O1', '-g', '-Wall', '-o', binary], check=True)
    subprocess.run([binary], check=True)
    subprocess.run([binary, 'pending'], check=True)
    debug = str(Path(tmp, 'test-debug'))   # Android debug variant of the J0 probe links
    subprocess.run(['cc', *flags, '-DNV_J0_TESTE', '-DNV_J7_TESTE', 'src/deeplink.c', 'tests/deeplink_app.c',
                    '-Isrc', '-I' + tmp, '-O1', '-g', '-Wall', '-o', debug], check=True)
    subprocess.run([debug], check=True)
    print('deeplink app: ok')
PY

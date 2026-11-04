"""Protect the existing native settings ABI during the restoration.

This baseline must not be regenerated merely to make a failure pass: enum/key
changes require a reviewed migration and defaults changes require explicit tests.
No application startup, account access or credentials are needed.
"""
import hashlib
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BASELINE = ROOT / 'tests/fixtures/settings-restoration/bindings.json'


def preprocess(text):
    result = subprocess.run(
        ['clang', '-E', '-P', '-x', 'c', '-DNV_ANDROID', '-DNV_TOUCH_UI',
         '-DNV_TECLADO_SIS', '-'], input=text, text=True,
        capture_output=True, check=True)
    return result.stdout


def contract():
    source = (ROOT / 'src/ajustes.c').read_text()
    enum = source[source.index('typedef enum {'):source.index('} OpcaoId;')]
    ids = [name for name in re.findall(r'\bAJ_\w+\b', preprocess(enum))
           if name != 'AJ_N']
    start = source.index('static const char *CHAVE[] = {')
    keys = re.findall(r'"((?:\\.|[^"\\])*)"', preprocess(
        source[start:source.index('\n};', start)]))
    assert len(ids) == len(keys), 'Enum and storage key arrays diverged'
    defaults = (ROOT / 'src/ajustes_ux_padrao.inc').read_text()
    defaults = defaults.split('// Stage C appended defaults (original binding prefix ends above).')[0]
    defaults = re.sub(r'/\*.*?\*/|//[^\n]*', '', defaults, flags=re.S)
    defaults = re.sub(r'\s+', '', defaults)
    return {
        'ordered_bindings': list(zip(ids, keys)),
        'defaults_sha256': hashlib.sha256(defaults.encode()).hexdigest(),
    }


def main():
    expected = json.loads(BASELINE.read_text())
    actual = contract()
    # Appending new bindings is permitted; existing numeric IDs/keys are not.
    assert actual['ordered_bindings'][:len(expected['ordered_bindings'])] == [
        tuple(item) for item in expected['ordered_bindings']
    ], 'Existing settings IDs/storage keys changed; migration required'
    assert actual['defaults_sha256'] == expected['defaults_sha256'], (
        'Existing positional defaults changed; explicit compatibility review required')
    for path in ('src/progresso.c', 'src/progresso.h'):
        digest = hashlib.sha256((ROOT / path).read_bytes()).hexdigest()
        assert digest == expected['progress_sources'][path], (
            path + ' changed: run/review progress-format compatibility tests')
    print('PASS: %d existing bindings, defaults and progress sources preserved' %
          len(expected['ordered_bindings']))


if __name__ == '__main__':
    main()

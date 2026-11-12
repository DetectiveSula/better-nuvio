"""J5 enrollment boundaries, no provider/backend calls."""
from pathlib import Path
import json,hashlib,re
root=Path(__file__).resolve().parents[1]
ledger=json.loads((root/'docs/settings-restoration/stage-j5-bindings.json').read_text())
assert ledger['j1_inventory_sha256']==hashlib.sha256((root/'docs/settings-restoration/stage-j-inventory.json').read_bytes()).hexdigest()
assert ledger['namespace']=='better_nuvio' and ledger['default_writes'] is False
assert len(ledger['fields'])==6
wire=(root/'src/homewire.h').read_text()
assert set(re.findall(r"case '([a-z])':",wire))==set('szova')
controller=(root/'src/ajustes_better.inc').read_text()
assert 'bna_open(defs,JB_N+1)' in controller and 'JB_N==93' in controller
assert 'bna_home_seed' in controller and 'fil_home_apply' in controller
rows=(root/'src/fileiras_home.inc').read_text()
assert 'base,addonsConta[source].base' in rows  # private rank; no URL digest
assert 'canonical' in rows and 'catalogue:' in rows and '#%d/%d' in rows
assert 'fil_registrar(' not in rows and 'nuvem_' not in rows
assert 'homeReorder(chaves,saida,q)' in (root/'src/fileiras.c').read_text()
assert 'homeClear();' in (root/'src/fileiras.c').read_text()
print('PASS: J5 closed portable field families; unchanged J1, 93 approved J4 controls, no catalogue insertion, no transport or credential fields in row adapter')

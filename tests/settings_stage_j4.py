"""J4 allowlist/protocol boundary checks. No account or provider operations."""
from pathlib import Path
import json,re
ROOT=Path(__file__).resolve().parents[1]
inventory=json.loads((ROOT/'docs/settings-restoration/stage-j-inventory.json').read_text())
portable=[r for r in inventory['controls'] if r['tier']=='better_nuvio tier: portable']
actual=re.findall(r'\{(AJ_\w+),"([^"]+)"\}',(ROOT/'src/ajustes_better_map.inc').read_text())
extensions=json.loads((ROOT/'docs/settings-restoration/stage-j4-bindings.json').read_text())['reviewed_extensions']
assert [(r['native_id'],r['storage_key']) for r in extensions]==[('AJ_HOME_LAYOUT','homeLayoutLocal'),('AJ_FAMILIA','familyWatchingLocal'),('AJ_FAMILIA_PARTILHAR','familyShareActivityLocal')]
assert actual==[(r['native_id'],r['storage_key']) for r in portable+extensions]
assert len(actual)==93 and len(set(actual))==93
excluded=[r for r in inventory['controls'] if r['tier']!='better_nuvio tier: portable' and r['native_id']!='AJ_HOME_LAYOUT']
assert all((r['native_id'],r['storage_key']) not in actual for r in excluded)
assert sum(r['tier']=='official: synced (Stage I)' for r in inventory['controls'])==32
assert sum(r['tier']=='official tier: key unresolved' for r in inventory['controls'])==26
assert sum(r['tier']=='device-local' for r in inventory['controls'])==87
assert all(r['native_scope']=='profile' for r in portable)
source=(ROOT/'src/ajustes.c').read_text();sync=(ROOT/'src/sync.c').read_text()
assert 'ajustes_better_observar();' in source and 'ajustes_better_boot();' in source
assert 'ajustes_better_ciclo(sessaoDoCiclo)' in sync and 'ajustes_better_passo()' in sync
assert 'ajustes_compartilhados_aplicar(ajustesBlob)' in sync
assert 'ajustes_better_esquecer()' in sync
controller=(ROOT/'src/ajustes_better.inc').read_text()
assert 'legfontes_id(' in controller and 'ling_opcao_codigo(' in controller
assert 'font:' in controller and 'legfontes_caminho(' not in controller
assert 'fil_gravar' not in controller and 'home_catalog_shared' not in controller
print('PASS: 90 original J1 portable controls plus reviewed Home layout and two pre-existing Family controls; 32 shared, 26 unresolved, 87 device-local and all other controls excluded; Native save/worker/frame/lifecycle hooks present')

assert 'AJ_HOME_LAYOUT' not in (ROOT/'src/ajustes_official_map.inc').read_text()
assert 'homeLayoutLocal' not in (ROOT/'src/ajustes_shared.inc').read_text()

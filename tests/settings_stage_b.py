"""Evidence-backed Legacy allowlist and preservation tests; no account/network access."""
import json
import re
import subprocess
import tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ledger=json.loads((ROOT/'docs/settings-restoration/stage-b-inventory.json').read_text())
baseline=json.loads((ROOT/'tests/fixtures/settings-restoration/bindings.json').read_text())
rows=ledger['native_controls']; originals=ledger['official_controls']
assert len(rows)==413 and len(originals)==354
assert [(r['id'],r['key']) for r in rows]==[tuple(x) for x in baseline['ordered_bindings']]
assert len({r['id'] for r in rows})==413
badge_ids={r['id'] for r in rows if r['legacy_badge']}
assert len(badge_ids)==125
for r in rows:
    assert r['reason'] and r['native_refs'],r['id']
    assert r['legacy_badge']==(r['classification']=='Verified additional')
    if r['legacy_badge']:
        family=ledger['families'][r['family']]
        assert r['id'] in family['ids'] and family['reason']==r['reason']
        assert family['official_files'] and family['native_files'] and family['evidence']
        assert r['category'] and r['platform_gate']
    assert all(0<=i<len(originals) for i in r['official_controls'])
# Explicit semantic regression cases: name similarity and audit mismatches must not merge controls.
negative={'AJ_TEMA','AJ_HOME_LAYOUT','AJ_HERO','AJ_HERO_NOTAS','AJ_NOTAS_HOME','AJ_DV',
'AJ_ATMOS','AJ_ADDONS','AJ_PLUGINS','AJ_TRAKT','AJ_SIMKL','AJ_TMDB_LIGADO','AJ_MDB_CHAVE',
'AJ_FONTE_MANUAL','AJ_FONTE_PREPARAR','AJ_AUD_LINGUA2','AJ_LEG_LINGUA2','AJ_LEG1_TAMANHO',
'AJ_TUNNELING','AJ_BUFFER','AJ_HDR_UI','AJ_TMDB_DATAS','AJ_DET_DATA_CHEIA',
'AJ_CW_MINIMO','AJ_CW_CONCLUIDO','AJ_CW_PROXIMO','AJ_HDR_INTERFACE','AJ_IPTV_SALVAR','AJ_IPTV_CANCELAR'}
assert badge_ids.isdisjoint(negative)
assert {'AJ_VIDRO_CONTORNO','AJ_SEEKR_AJUSTE','AJ_LEG2_TAMANHO','AJ_FALHA_PROVEDOR_ORDEM','AJ_DEBRID_AD','AJ_DISCORD'}<=badge_ids
source=(ROOT/'src/ajustes.c').read_text()
enum=source[source.index('typedef enum {'):source.index('} OpcaoId;')+len('} OpcaoId;')]
unit=enum+'\n#include "ajustes_legacy.inc"\n#include <assert.h>\nint main(void) {\n'
for r in rows:
    if r['id']=='AJ_BAIXAR_REGISTRO':unit+='#ifdef NV_ANDROID\n'
    if r['legacy_badge']:
        unit+='#if '+r['platform_gate']+'\nassert(ajustes_opcao_legacy('+r['id']+'));\n#else\nassert(!ajustes_opcao_legacy('+r['id']+'));\n#endif\n'
    else:unit+='assert(!ajustes_opcao_legacy('+r['id']+'));\n'
    if r['id']=='AJ_BAIXAR_REGISTRO':unit+='#endif\n'
unit+='assert(!ajustes_opcao_legacy(-1));assert(!ajustes_opcao_legacy(AJ_N));return 0;}\n'
with tempfile.TemporaryDirectory(prefix='settings-b-') as tmp:
    p=Path(tmp)/'badges.c';p.write_text(unit)
    for flags in [[],['-DNV_ANDROID','-DNV_TOUCH_UI','-DNV_TECLADO_SIS'],['-DNV_ANDROID','-DNV_P2P_MOTOR'],['-DNV_TPK'],['-DNV_TPK','-DNV_TPK40'],['-D__EMSCRIPTEN__']]:
        binary=Path(tmp)/'badges'
        subprocess.run(['cc','-Isrc',*flags,str(p),'-o',str(binary)],cwd=ROOT,check=True)
        subprocess.run([str(binary)],check=True)
# The decoration must not become another remote/pointer target.
drawing=(ROOT/'src/ajustes_ux_desenho.inc').read_text()
body=drawing[drawing.index('float ajustes_legacy_selo('):drawing.index('// Caixa alta espacada')]
assert 'ponteiro_' not in body and 'foco' not in body
assert 'available < width' in body and '"Legacy"' in body
assert 'ajustes_legacy_selo' in (ROOT/'src/spotlight.c').read_text()
# No consumer, serialization, defaults, visibility/platform gate or callback change is allowed here.
import hashlib
contract=json.loads((ROOT/'tests/fixtures/settings-restoration/stage-b-wiring.json').read_text())
stage_c=json.loads((ROOT/'tests/fixtures/settings-restoration/stage-c-wiring.json').read_text())
stage_d=json.loads((ROOT/'tests/fixtures/settings-restoration/stage-d-wiring.json').read_text())
assert set(stage_d)=={'src/debrid.c','src/streams.c'}
optin=json.loads((ROOT/'tests/fixtures/settings-restoration/stage-d-optin-wiring.json').read_text())
for path,digest in contract.items():
    if path in stage_c:
        assert stage_c[path]['before']==digest, 'Stage B baseline changed: '+path
        digest=stage_c[path]['after']
        if path.startswith('function:'):
            lines=iter(digest.splitlines())
            assert all(any(line==candidate for candidate in lines) for line in contract[path].splitlines()), 'Original statements removed: '+path
    if path in stage_d:
        assert stage_d[path]['before']==digest and stage_d[path]['reason']
        digest=stage_d[path]['after']
    if path in optin:
        assert optin[path]['before']==digest and optin[path]['reason']
        if path.startswith('function:'):
            lines=iter(optin[path]['after'].splitlines())
            assert all(any(line==candidate for candidate in lines) for line in digest.splitlines()), path
        digest=optin[path]['after']
    if path.startswith('function:'):
        name=path.split(':',1)[1]
        current=source
        # Exact frozen function text; source line numbers may move.
        assert digest in current, 'Existing setting wiring/visibility changed: '+name
    else:assert hashlib.sha256((ROOT/path).read_bytes()).hexdigest()==digest,path
print('PASS: all 413 provenance records, 125 explicit Legacy controls, negative equivalence cases and gates across 6 builds; original wiring retained with explicit Stage C/D policy additions')

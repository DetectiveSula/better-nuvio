"""Wire contract and consumer evidence; no account/provider operations."""
import json
import re
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'src/ajustes_shared.inc').read_text()
inventory=json.loads((ROOT/'docs/settings-restoration/stage-i-inventory.json').read_text())
assert inventory['official_commit']=='99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f'
controls=inventory['controls']
assert len(controls)==32
actual=re.findall(r'\{(AJ_\w+), "(\w+)", "(\w+)"\}',source)
assert actual==[(r['native_id'],r['feature'],r['key']) for r in controls]
assert len({(r['feature'],r['key']) for r in controls})==32
assert not {'AJ_NOME_ADDON','AJ_GRAD_CLASSICO','AJ_DET_TRAILER','AJ_TUNNELING','AJ_RAIL_BLUR','AJ_FIL_LIMITE','AJ_FONTE_AUTO','AJ_TMDB_DATAS'}&{r['native_id'] for r in controls}
for row in controls:
    assert row['type']=='boolean' and row['scope']=='selected profile'
    assert row['official_default_reads'] and row['native_getters']
    for getter in row['native_getters']:
        assert getter['name']+'(' in (ROOT/getter['file']).read_text()
        assert getter['consumers']
        for consumer in getter['consumers']:
            assert consumer['code'] in (ROOT/consumer['file']).read_text()
sync=(ROOT/'src/sync.c').read_text()
push=sync[sync.index('static void empurrarAjustes(void)'):sync.index('static void empurrarAjustes(void)')+4000]
assert 'ajustes_compartilhados_mesclar' in push and 'ajustes_mesclar_blob(' not in push
assert 'ajustes_compartilhados_aplicar(ajustesBlob)' in sync
assert 'ajustes_aplicar_blob(ajustesBlob)' not in sync
assert 'ajustes_compartilhados_edicao(edPerfil, antes, valor)' in (ROOT/'src/ajustes.c').read_text()
print('PASS: 32 explicitly scoped official boolean controls with original defaults and Native runtime-consumer evidence; unsupported/device/Stage G rows excluded')
# Account refresh on resume and after playback (sync_pedir_atualizacao), wired
# where the app actually observes those events.
main=(ROOT/'src/main.c').read_text()
fg=main[main.index('if (e.type == SDL_APP_DIDENTERFOREGROUND) {'):][:400]
assert 'sync_pedir_atualizacao(' in fg
app=(ROOT/'src/app.c').read_text()
loop=app[app.index('static int playerAntes;'):][:400]
assert 'if (playerAntes && !aberto) sync_pedir_atualizacao(' in loop and 'sync_periodico((unsigned)agora)' in loop
print('PASS: resume and player-close refresh requests are wired to the real SDL foreground event and player transition')

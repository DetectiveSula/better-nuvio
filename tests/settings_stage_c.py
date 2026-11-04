"""Stage C append-only policy and pipeline routing contract (no network)."""
import json,re
from pathlib import Path
from settings_preservation import contract,main,ROOT
main()
old=json.loads((ROOT/'tests/fixtures/settings-restoration/bindings.json').read_text())
actual=contract()['ordered_bindings']
assert actual[413:417]==[
 ('AJ_RESOLVE_LINKS','resolvePlayableLinksLocal'),
 ('AJ_RESOLVE_PROVIDER','preferredResolverProviderLocal'),
 ('AJ_RESOLVE_PREPARE','-preparePlayableLinks'),
 ('AJ_RESOLVE_LIMIT','playableLinkPreparationLimitLocal')]
assert actual[417:]==[('AJ_TB_CACHED_ADD','torboxCachedAddLocal')]
defaults=(ROOT/'src/ajustes_ux_padrao.inc').read_text().split('// Stage C appended defaults (original binding prefix ends above).')[1]
old,added=defaults.split('// Stage D cached-addition opt-in (after all Stage C defaults).')
assert re.sub(r'\s+|/\*.*?\*/','',old,flags=re.S)=='1,0,1,0,'
assert re.sub(r'\s+|/\*.*?\*/','',added,flags=re.S)=='1,'
s=(ROOT/'src/streams.c').read_text()
# Every automatic resolution path is guarded centrally; no raw legacy resolver calls.
assert 'debrid_resolver_cache(' not in s and 'debrid_resolver_servico_cache(' not in s
assert s.count('debrid_resolver_escolhido(')==1
availability=(ROOT/'src/streams_cache.inc').read_text()
assert 'debrid_cache_consultar_lote(' in availability
assert 'streamResolver' not in availability and 'debrid_resolver' not in availability
assert 'debrid_resolver_existente(' in (ROOT/'src/resolver_policy.c').read_text()
assert 'manualPermit=-1;' in s and 'manualPermitGen==geracao' in s
assert 'manualPermitUntil-SDL_GetTicks()' in s and 'manualPermitSession' in s
# Both TV sheet/island layouts draw their confirmation after the final shared surfaces.
app=(ROOT/'src/app.c').read_text();assert app.index('stream_resolver_confirmacao_desenhar();')>app.rindex('ilha_desenhar(agora);')
print('PASS: four preserved Stage C settings plus one default-Off opt-in; all automatic paths gated, availability read-only, one-shot manual consent and shared-surface modal routing')

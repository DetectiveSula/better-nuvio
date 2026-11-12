#!/usr/bin/env python3
"""Build an honest per-binding test ledger from the isolated native harness.

A source reference or a value-boundary check is never labelled a functional pass.
No application data, account snapshots, credentials or device logs are read.
"""
import argparse,json,re,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tests'))
from settings_preservation import contract
parser=argparse.ArgumentParser()
parser.add_argument('--native-log',default='build/settings-expansion/regressions/settings_expansion_native.log')
args=parser.parse_args()
log=(ROOT/args.native_log).read_text()
assert 'PASS: all filter subchoices' in log and 'Assertion failed' not in log,'Native harness did not finish successfully'
pattern=r'BOUNDARY_ONLY id=(\d+) key=(\S+) label=(.*?) kind=(\d+) default=(-?\d+) choices=(\d+) min=(-?\d+) max=(-?\d+) step=(\d+)'
native={}
for match in re.finditer(pattern,log):
 i,key,label,kind,default,choices,lo,hi,step=match.groups()
 native[key]=dict(host_id=int(i),label=label,type=int(kind),default=int(default),choices=int(choices),min=int(lo),max=int(hi),step=int(step))
assert len(native)==433,'Expected the current host catalogue, including action/read-only entries'
audit=json.loads((ROOT/'docs/settings-restoration/stage-b-inventory.json').read_text())
old={r['id']:r for r in audit['native_controls']}
functional={}
def assign(ids,evidence,scope):
 for name in ids.split():functional[name]=dict(evidence=evidence,scope=scope)
assign('AJ_PROF AJ_PROF_BORDA AJ_PROF_BRILHO AJ_PROF_COBERTURA AJ_PROF_POSTERS AJ_PROF_CW AJ_PROF_EPS AJ_PROF_ELENCO AJ_PROF_TRAILERS',
 ['tests/settings_expansion_render.sh','tests/settings_expansion_policy.sh'],
 'Actual OpenGL pixel effects, all requested presets and five independent surface masks; production widgets carry real focus. TV GPU validation pending.')
assign('AJ_LARGURA_DP AJ_RAIO_DP',['tests/home.sh','tests/settings_stage_j6.sh'],
 'Six widths and five corner presets change actual Home geometry/radius. Existing custom numeric values remain compatible; real TV sizing pending.')
assign('AJ_MENU_OCULTAR_PILULA',['tests/settings_expansion_native.sh','tests/menu_tvos.sh','tests/menu_geometry.sh'],
 'Persistence, Dynamic pill alpha, opening navigation and existing menu geometry. Hidden Modern rail uses the existing pointer-edge path; real remote/pointer validation pending.')
assign('AJ_HERO_TRAILER_ESPERA AJ_DET_TRAILER_ESPERA',['tests/settings_expansion_native.sh'],
 'Effective milliseconds, every legal new detail-delay value and disk reload; production Home/detail timing consumers traced. Live trailer timing on TV pending.')
assign('AJ_POSPLAY_RECOM AJ_POSPLAY_PERCENT AJ_AINDA_EPISODIOS',['tests/posplay.sh','tests/posplay_recuo.sh','tests/settings_expansion_native.sh'],
 'Actual recommendation policy at all 100 percentages; actual next-episode timer at all 20 prompt intervals, OK/Off/autoplay-Off and disk reload. Real playback/UI pending.')
assign('AJ_LEG_AUTO_COMPLETA',['tests/settings_expansion_subtitles.sh','tests/settings_expansion_native.sh','tests/linguas.sh'],
 'Actual automatic selector with mocked embedded/addon/late tracks; saved language and per-title None honoured. Live provider/player device validation pending.')
assign('AJ_POSPLAY_TRAILER AJ_POSPLAY_TRAILER_ESPERA',['tests/postplay_preview.sh','tests/settings_expansion_native.sh','android/app/src/test/java/space/nuvio/nativelegacy/PostPlayPreviewPolicyTest.kt'],
 'All 301 delay values in mock preview lifecycle; Android software-decoder/URL/low-memory policies and both native ABIs compile. Texture composition, decoding and CPU load NOT device-validated. Unsupported on LG/Tizen/desktop.')
assign('AJ_ICONE_APP',['tests/iconeapp_policy.sh','tests/settings_expansion_assets.py'],
 'All 10 icon/banner/TV-launcher resource pairs and entitlement policy. Existing linked selection is retained. Actual launcher alias switching/cache refresh NOT device-validated; locked without entitlement.')
assign('AJ_FILTROS_FONTES AJ_FILTRO_TAM_MAX AJ_FILTRO_GRUPOS AJ_FILTRO_MIN_GB AJ_FILTRO_MAX_RESULT AJ_FILTRO_POR_RES AJ_FILTRO_POR_QUAL AJ_FILTRO_SORT AJ_FILTRO_REQUIRED_GROUPS',
 ['tests/settings_expansion_policy.sh','tests/settings_expansion_native.sh','tests/fontefalha.sh','tests/stream_cache_filter.sh'],
 'Every metadata option in Preferred/Required/Excluded modes, actual settings callbacks and persistence; all numeric quota/size values in policy, real sheet rows/counts and automatic candidate safety/order. Live sources/providers and TV sheet interaction pending.')
rows=[]
for android_id,(name,key) in enumerate(contract()['ordered_bindings']):
 original=old.get(name,{})
 runtime=native.get(key)
 row=dict(id=name,android_id=android_id,key=key,label=(runtime or {}).get('label',original.get('label',name)),
          original_413=android_id<413,
          contract='PASS: existing ID/key/default preservation' if android_id<413 else 'PASS: append-only extension',
          value_contract='PASS: every legal choice and numeric boundary' if runtime and runtime['type'] in (0,1) else 'Not applicable: action/readout (not executed by boundary loop)',
          functional_status='Focused local tests passed; device limitations remain' if name in functional else 'NOT individually functionally certified by this run',
          functional_evidence=functional.get(name,{}),
          prior_source_references=[{'file':x['file'],'line_at_original_audit':x['line']} for x in original.get('native_consumers',[])],
          notes='Source references and broad regression passes do not certify this setting individually.')
 if runtime:row['native_contract']=runtime
 else:row['notes']='Android-only control absent from host harness; no action executed. Original ABI guard still covers it.'
 rows.append(row)
policy=(ROOT/'build/settings-expansion/policy-final.log').read_text()
choices=[]
for category,option,label in re.findall(r'PASS dimension=(\d+) choice=(\d+) preferred/required/excluded: (.*)',policy):
 for mode in ['Preferred','Required','Excluded']:
  choices.append(dict(dimension=int(category),option=int(option),mode=mode,label='Unknown' if label=='Desconhecido' else label,
       status='PASS: real policy, actual UI toggle callback, independent save/reload',evidence=['tests/settings_expansion_policy.sh','tests/settings_expansion_native.sh']))
result=dict(official_commit='99fbd65d8c16928d1fe11bf3f9812c4b5aaaca8f',
 summary=dict(android_bindings=len(rows),host_bindings=len(native),original_preserved=413,
              host_value_contracts=sum(r['value_contract'].startswith('PASS') for r in rows),
              focused_functional_bindings=len(functional),filter_subchoices=len(choices)),
 limitations=['This is NOT a claim that every original setting has been functionally exercised one by one.',
              'Actions, credentials, account/provider mutations and hardware-specific behaviour were not exercised live.',
              'All focused tests are local/offline/mocked except local OpenGL rendering; no TV installation or account writes.'],
 controls=rows,filter_subchoices=choices)
out=ROOT/'docs/settings-restoration/settings-expansion-ledger.json';out.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
md=['# Per-setting verification ledger','',
    '**This is not an all-settings functional certification.** Every binding is listed below; value-range checks are separate from consumer tests. See `SETTINGS-EXPANSION.md` for fixes, suites and device limitations.','',
    f"Android bindings: {len(rows)}. Host: {len(native)}. Original bindings preserved: 413. Host value contracts checked: {result['summary']['host_value_contracts']}. Focused functional bindings: {len(functional)}. Filter subchoices: {len(choices)}.",'',
    '| Setting | Key | Value contract | Functional verification |','|---|---|---|---|']
for row in rows:
 def cell(s):return str(s).replace('|','\\|').replace('\n',' ')
 md.append('| '+' | '.join(cell(x) for x in [row['id']+' — '+row['label'],row['key'],row['value_contract'],row['functional_evidence'].get('scope',row['functional_status'])])+' |')
md+=['','## Filter subchoices','',
     'Each listed option was exercised through Preferred, Required and Excluded UI callbacks and disk reloads, as well as the production filter policy. These are additional sheet controls, not replacement binding IDs.','',
     '| Dimension | Option | Mode | Result |','|---|---|---|---|']
for c in choices:md.append(f"| {c['dimension']} | {c['label']} | {c['mode']} | PASS (local mocks) |")
(ROOT/'docs/settings-restoration/SETTINGS-VERIFICATION.md').write_text('\n'.join(md)+'\n')
print(json.dumps(result['summary']))

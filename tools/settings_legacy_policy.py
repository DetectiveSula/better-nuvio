"""Check/regenerate the badge switch from the reviewed provenance allowlist.

This never infers eligibility from unmatched original-audit rows. Edit and review
stage-b-inventory.json with source evidence before intentionally regenerating.
"""
import argparse
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def policy():
    ledger=json.loads((ROOT/'docs/settings-restoration/stage-b-inventory.json').read_text())
    out='// Verified additional user controls only. Evidence: docs/settings-restoration/stage-b-inventory.json\n// Generated from its explicit allowlist; never derive eligibility from unmatched audit rows.\nint ajustes_opcao_legacy(int op) { switch (op) {\n'
    seen=set()
    for row in ledger['native_controls']:
        assert row['id'] not in seen
        seen.add(row['id'])
        if not row['legacy_badge']:continue
        assert row['classification']=='Verified additional' and row['family'] in ledger['families']
        assert row['id'] in ledger['families'][row['family']]['ids']
        assert row['category'] and row['platform_gate'] and row['native_refs']
        out+='#if '+row['platform_gate']+'\ncase '+row['id']+': return 1;\n#endif\n'
    return out+'default: return 0;\n} }\n'

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--write',action='store_true',help='Intentionally regenerate the reviewed switch')
    args=parser.parse_args();target=ROOT/'src/ajustes_legacy.inc'
    expected=policy()
    if args.write:target.write_text(expected)
    else:assert target.read_text()==expected,'Badge switch diverges from reviewed provenance evidence'
    print('PASS: Legacy badge policy matches the explicit reviewed inventory')

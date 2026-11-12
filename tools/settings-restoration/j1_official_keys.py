"""Official control -> DataStore keys via the ViewModel method body; apply official sync rules."""
import json,re,sys
from pathlib import Path
ROOT=Path.home()/'Downloads/better-nuvio'
OFF=Path.home()/'Documents/Nuvio-Official/app/src/main/java/com/nuvio/tv'
APPSRC=Path.home()/'Documents/Nuvio-Official'
b=json.loads((ROOT/'docs/settings-restoration/stage-b-inventory.json').read_text())
sync=json.loads((ROOT/'docs/settings-restoration/stage-i-official-sync.json').read_text())
SYNCED=set(sync['features'])
EXCL={}
for k,v in sync['exclusion_sets'].items():
    feat={'catalogKeysExcludedFromProfileSettingsBlob':'layout_settings','localOnlyLayoutProfileSettingsKeys':'layout_settings',
          'localOnlyPlayerProfileSettingsKeys':'player_settings','localOnlyTrailerProfileSettingsKeys':'trailer_settings'}.get(k)
    if feat: EXCL.setdefault(feat,set()).update(v)
EXCL.setdefault('layout_settings',set()).add('search_discover_enabled')
CRED={'debrid_settings':{'torbox_api_key','premiumize_api_key','real_debrid_api_key'},'mdblist_settings':{'mdblist_api_key'},'animeskip_settings':{'animeskip_client_id'}}
# store class -> (feature, profile, setter->keys)
stores={}
for f in (OFF/'data/local').glob('*.kt'):
    s=f.read_text(); name=f.stem
    feat=(re.search(r'FEATURE\s*=\s*"([a-z_]+)"',s) or [None,None])[1]
    keys={v:(n,t) for v,t,n in re.findall(r'val\s+(\w+)\s*=\s*(boolean|int|long|float|double|string|stringSet)PreferencesKey\(\s*"([^"]+)"',s)}
    setters={}
    for fm in re.finditer(r'fun\s+(set\w+|toggle\w+|update\w+)\s*\([^)]*\)[^{=]*\{',s):
        i=fm.end(); d=1; j=i
        while j<len(s) and d: d+={'{':1,'}':-1}.get(s[j],0); j+=1
        used=[keys[v] for v in re.findall(r'\[(\w+)\]\s*=',s[i:j]) if v in keys]
        if used: setters[fm.group(1)]=used
    stores[name]={'keys':keys,'feature':feat,'profile':'ProfileDataStoreFactory' in s,'setters':setters,'file':f.name}
def body(path,line):
    s=(APPSRC/path).read_text().splitlines()
    t='\n'.join(s[line-1:line+60]); i=t.find('{')
    if i<0: return t[:400]
    d=0
    for j,c in enumerate(t[i:],i):
        d+={'{':1,'}':-1}.get(c,0)
        if d==0: return t[i:j+1]
    return t
res={}
for oc in b['official_controls']:
    keys=[]
    for w in oc.get('writes',[]):
        p=APPSRC/w['file']
        if not p.exists(): continue
        src=p.read_text()
        types=dict(re.findall(r'va[lr]\s+(\w+)\s*:\s*(\w+DataStore|\w+Preferences)\b',src))
        bd=body(w['file'],w['line'])
        ev=re.match(r'\s*(?:data\s+class\s+|class\s+|object\s+)?(\w+)\s*(?:\(|:)',w['code'])
        if ev and re.search(r'Event\b',w['code']):
            name=ev.group(1)
            # Follow the event handler: `is XEvent.Name -> receiver.setY(...)` (same file, then any file).
            pats=[src]+[q.read_text() for q in p.parent.glob('*.kt') if q!=p]
            for cand in pats:
                for m in re.finditer(r'is\s+\w+\.'+name+r'\b\s*->\s*',cand):
                    seg=cand[m.end():m.end()+600]
                    seg=re.split(r'\n\s*is\s+\w+\.',seg)[0]
                    calls=re.findall(r'(\w+)\(',seg)
                    bd+='\n'+seg
                    # one indirection: a private ViewModel helper
                    for c in calls:
                        hm=re.search(r'fun\s+'+c+r'\s*\([^)]*\)[^{]*\{',cand)
                        if hm: bd+='\n'+cand[hm.end():hm.end()+500]
                    types.update(dict(re.findall(r'va[lr]\s+(\w+)\s*:\s*(\w+DataStore|\w+Preferences)\b',cand)))
                if name in bd: pass
        for recv,meth in re.findall(r'(\w+)\.(set\w+|toggle\w+|update\w+)\(',bd):
            st=stores.get(types.get(recv,''))
            if st and meth in st['setters']:
                for k,t in st['setters'][meth]: keys.append({'store':types[recv],'feature':st['feature'],'profile':st['profile'],'key':k,'type':t})
    if not keys:
        # No recorded write: match UI properties to store keys (<prop>Key or snake_case).
        for prop in oc.get('properties',[]):
            snake=re.sub(r'(?<!^)([A-Z])',r'_\1',prop).lower()
            for sname,st in stores.items():
                for var,(kn,kt) in st['keys'].items():
                    if var in (prop+'Key',prop) or kn==snake:
                        e={'store':sname,'feature':st['feature'],'profile':st['profile'],'key':kn,'type':kt,'match':'property'}
                        if e not in keys: keys.append(e)
    for k in keys:
        f=k['feature']
        if not k['profile']: k['official_sync']='device/global store (not synced)'
        elif f not in SYNCED: k['official_sync']='profile store, feature not synced'
        elif k['key'] in CRED.get(f,()): k['official_sync']='credential (separate RPC)'
        elif k['key'] in EXCL.get(f,()): k['official_sync']='excluded: device-local in official'
        elif f=='plugin_settings' and k['key']!='group_streams_by_repository': k['official_sync']='excluded'
        else: k['official_sync']='exported in "tv" blob'
    res[oc['id']]=keys
json.dump(res,sys.stdout,indent=0)

import json,re,sys
from pathlib import Path
from collections import Counter
J=Path(sys.argv[1]); R=Path('docs/settings-restoration')
b=json.loads((R/'stage-b-inventory.json').read_text())
si=json.loads((R/'stage-i-inventory.json').read_text())
sync=json.loads((R/'stage-i-official-sync.json').read_text())
resolved=json.loads((J/'official_resolved.json').read_text())
hits=json.loads((J/'keyname_hits.json').read_text())
SYNCED=set(sync['features'])
EXCL={'player_settings':set(sync['exclusion_sets']['localOnlyPlayerProfileSettingsKeys']),
      'layout_settings':set(sync['exclusion_sets']['localOnlyLayoutProfileSettingsKeys'])|set(sync['exclusion_sets']['catalogKeysExcludedFromProfileSettingsBlob'])|{'search_discover_enabled'},
      'trailer_settings':{'trailer_allow_4k'}}   # ProfileSettingsSyncService.localOnlyTrailerProfileSettingsKeys
for k,v in sync['exclusion_sets'].items():
    if 'Trailer' in k: EXCL['trailer_settings'].update(v)
shared={c['native_id']:c for c in si['controls']}
# Hand review of Better-Nuvio-exclusive controls (reasons are the decision record).
DEVICE={
 'hardware/display/GPU/audio capability':['AJ_DV','AJ_ATMOS','AJ_QUALIDADE','AJ_AMPLIAR_UI','AJ_AMPLIACAO_ALGO','AJ_AMPLIACAO_NITIDEZ','AJ_VIDRO','AJ_VIDRO_FOSCO','AJ_TAMANHO_UI','AJ_ESCALA_TEXTO','AJ_TAMANHO_AJUSTES','AJ_SUAVIZAR_DEGRADES','AJ_FONTE_HDR','AJ_EPG_MEMORIA'],
 'network/connection behaviour':['AJ_FONTE_TOCAR_CONFERINDO','AJ_FONTE_AQUECER','AJ_FONTE_CONFERIR_VARIAS'],
 'consent or diagnostics (given per device)':['AJ_SOCIAL','AJ_ENQUETES','AJ_HIST_CONTA','AJ_LOG_REDE'],
 'depends on device-only IPTV/guide sources':['AJ_GUIA_FONTE','AJ_GUIA_ORDEM','AJ_LIVETV_SUGERIR'],
 'profile picker (shown before a profile is selected)':['AJ_PS_FUNDO','AJ_PS_CONTINUAR'],
 'download-capable safety opt-in (Stage D)':['AJ_TB_CACHED_ADD'],
}
DEV={i:r for r,ids in DEVICE.items() for i in ids}
# Setter-path mappings accepted after comparing label and official key (J1 review).
SETTER_OK={'AJ_AUD_LINGUA','AJ_AUD_LINGUA2','AJ_FONTE_MANUAL','AJ_CW_FONTE','AJ_SALVOS_DEST','AJ_HERO_TRAILER_ESPERA',
 'AJ_CW_CONCLUIDO','AJ_FUNDO','AJ_LEG_FORCADA','AJ_PLR_CLASSIF','AJ_LAYOUT_AJUSTES','AJ_TELA_ABERTURA','AJ_PULAR_ABERTURA',
 'AJ_PROXIMO_AUTO','AJ_HERO_NOTAS','AJ_FONTE_GRUPO','AJ_FONTE_GRUPO_SESSOES','AJ_LINK_REUSAR','AJ_LINK_PRAZO','AJ_PLR_FORMATO',
 'AJ_PAUSA_COMENTARIOS','AJ_CW_DIAS','AJ_RESOLVE_PROVIDER','AJ_RESOLVE_PREPARE','AJ_RESOLVE_LIMIT','AJ_TRAILER_QUAL',
 'AJ_FONTE_UI','AJ_FONTE_PRAZO'}
RECONSIDER={'AJ_RELOGIO_12H','AJ_FONTE_AUTO','AJ_FONTE_TEXTO','AJ_DET_SO_CINEMETA','AJ_BUSCA_CINEMETA','AJ_DESCANSO_ESTILO'}
EXTRA=[('AJ_RESOLVE_LINKS','resolvePlayableLinksLocal','Resolve playable links',3),('AJ_RESOLVE_PROVIDER','resolverProviderLocal','Resolve with',3),
       ('AJ_RESOLVE_PREPARE','resolverPrepareLocal','Prepare instant playback',3),('AJ_RESOLVE_LIMIT','resolverLimitLocal','Preparation limit',3),
       ('AJ_TB_CACHED_ADD','torboxCachedAddLocal','Add cached TorBox torrents',3)]
OFFICIAL_EXTRA={'AJ_RESOLVE_PROVIDER':{'feature':'debrid_settings','key':'preferred_resolver_provider_id','type':'string'},
                'AJ_RESOLVE_PREPARE':{'feature':'debrid_settings','key':'instant_playback_preparation_limit','type':'int'},
                'AJ_RESOLVE_LIMIT':{'feature':'debrid_settings','key':'instant_playback_preparation_limit','type':'int'}}
ctrls=list(b['native_controls'])+[{'id':i,'key':k,'label':l,'scope':s,'category':'Playback','classification':'Stage C/D appended','official_controls':[],'type':0,'choices':[]} for i,k,l,s in EXTRA]
def status(feat,key):
    if feat not in SYNCED: return 'not synced by regular Nuvio (feature outside the profile blob)'
    if key in EXCL.get(feat,()): return 'device-local in regular Nuvio (excluded from the blob)'
    if key.endswith('_api_key') or key=='animeskip_client_id': return 'credential (separate RPC)'
    return 'exported in "tv" blob'
out=[]
for c in ctrls:
    nid=c['id']; sc=c['scope']
    offs=[]
    if nid in hits: offs.append({**hits[nid],'evidence':'identical preference key'})
    if nid in OFFICIAL_EXTRA: offs.append({**OFFICIAL_EXTRA[nid],'evidence':'DebridSettingsDataStore (Stage C mapping)'})
    if c['classification']!='Related, distinct behaviour':
        for oid in c.get('official_controls',[]):
            for k in resolved.get(str(oid),[]):
                e={'feature':k['feature'],'key':k['key'],'type':k['type'],'evidence':'official setter/property ('+k['store']+')'}
                if not any(x['key']==e['key'] for x in offs): offs.append(e)
    for o in offs: o['official_sync']=status(o['feature'],o['key'])
    rejected=[o for o in offs if o['evidence'].startswith('official setter') and nid not in SETTER_OK]
    offs=[o for o in offs if o not in rejected]
    ntype='enum' if c.get('choices') else 'int'
    row={'native_id':nid,'storage_key':c['key'],'label':c['label'],'category':c.get('category'),
         'native_scope':{0:'none',1:'device (Native TV-only)',2:'profile',3:'profile'}[sc],'native_value':ntype,'official':offs}
    exported=[o for o in offs if o['official_sync']=='exported in "tv" blob']
    if nid in shared:
        tier,why='official: synced (Stage I)','bidirectional through the "tv" blob; 32 verified fields'
    elif sc==0:
        tier,why='not a preference','action, read-only row or section'
    elif nid in DEV:
        tier,why='device-local',DEV[nid]
    elif sc==1 and exported:
        tier,why='device-local','Native TV-only scope; regular Nuvio exports '+exported[0]['feature']+'.'+exported[0]['key']+' (revisit after J6)'
    elif exported:
        o=exported[0]
        conv='boolean, same meaning' if o['type']=='boolean' else f"{o['type']} value map required"
        tier,why='official tier: J6 candidate',f"regular Nuvio exports {o['feature']}.{o['key']} ({conv})"
    elif offs and all(o['official_sync'].startswith('device-local') for o in offs):
        tier,why='device-local','regular Nuvio keeps its equivalent device-local'
    elif sc==1 and not rejected:
        tier,why='device-local','Native TV-only scope (hardware, display, input, local service or install)'
        if nid in RECONSIDER: why+='; could be portable, revisit after J4'
    elif sc==1:
        tier,why='device-local','Native TV-only scope (hardware, display, input, local service or install)'+('; candidate official mapping rejected in review' if rejected else '')
        if nid in RECONSIDER: why+='; could be portable, revisit after J4'
    elif rejected or (c.get('official_controls') and c['classification'] not in ('Related, distinct behaviour','Verified additional')):
        tier,why='official tier: key unresolved',('candidate official mapping rejected in review; ' if rejected else '')+'counterpart key composite or unresolved; local until J6 resolves it'
        if sc==1: why='Native TV-only scope; '+why
    elif sc==1:
        tier,why='device-local','Native TV-only scope (hardware, display, input, local service or install)'
        if nid in RECONSIDER: why+='; could be portable, revisit after J4'
    else:
        tier,why='better_nuvio tier: portable','Better Nuvio-exclusive (or distinct behaviour), profile preference'
    row['tier']=tier; row['reason']=why
    out.append(row)
doc={'generated':'2026-11-04','stage':'J1','official_commit':si['official_commit'],
 'tiers':{'official: synced (Stage I)':'Already bidirectional via "tv"',
          'official tier: J6 candidate':'Regular Nuvio exports it; sync through "tv" after a tested value conversion (never also in better_nuvio)',
          'official tier: key unresolved':'Counterpart exists; stored key composite/unresolved; local until J6',
          'better_nuvio tier: portable':'Synced per profile through p_platform "better_nuvio" (J4)',
          'device-local':'Never uploaded',
          'not a preference':'Actions, read-only rows, sections'},
 'policy':['One owner per setting: an exported official equivalent always wins over better_nuvio.',
           'Device-local rows never leave the device, including when regular Nuvio would sync an equivalent elsewhere.',
           'Credentials, API keys, tokens, URLs and endpoint configuration are not settings rows; they stay in their existing stores or the provider-credential RPC.',
           'Profile scope: every synced row is per profile; nothing is account-wide.'],
 'controls':out}
(R/'stage-j-inventory.json').write_text(json.dumps(doc,indent=1,ensure_ascii=False)+'\n')
print(len(out)); print(Counter(r['tier'] for r in out).most_common())
print('J6 by type:',Counter(r['official'][0]['type'] for r in out if r['tier']=='official tier: J6 candidate'))
print('unresolved:',[r['native_id'] for r in out if r['tier']=='official tier: key unresolved'][:40])

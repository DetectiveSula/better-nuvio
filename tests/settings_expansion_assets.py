"""Every existing Android icon/banner pair: real resources and TV entry points."""
from pathlib import Path
import re,xml.etree.ElementTree as E
root=Path(__file__).resolve().parents[1];res=root/'android/app/src/main/res'
a='{http://schemas.android.com/apk/res/android}'
m=E.parse(root/'android/app/src/main/AndroidManifest.xml').getroot()
aliases=m.findall('./application/activity-alias')
ids=re.search(r'private val ICONES = arrayOf\((.*?)\)',(root/'android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt').read_text(),re.S)
ids=re.findall(r'"([a-z]+)"',ids[1]);assert len(ids)==10
for name in ids:
 alias=next(x for x in aliases if x.get(a+'name','').endswith('Icone_'+name))
 for field in ['icon','banner']:
  kind,value=alias.get(a+field)[1:].split('/')
  assert any(p.stem==value for d in res.glob(kind+'*') for p in d.iterdir()),(name,field,value)
 assert any(c.get(a+'name')=='android.intent.category.LEANBACK_LAUNCHER' for c in alias.findall('./intent-filter/category'))
 assert alias.get(a+'enabled')==('true' if name=='original' else 'false')
 print('PASS icon/banner/TV launcher pair:',name)
assert 'apoiador_ativo()' in (root/'src/iconeapp.c').read_text()
print('PASS all 10 pairs and original default; entitlement retained. Real launcher refresh is device-only.')

"""Stage A routing/gate and English coverage regressions; no application startup."""
import ast
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
records = json.loads((ROOT / 'tests/fixtures/settings-restoration/presentation.json').read_text())
source = (ROOT / 'src/ajustes.c').read_text()
enum = source[source.index('typedef enum {'):source.index('} OpcaoId;') + len('} OpcaoId;')]
# Standalone native catalogue exercises the actual production scene/art routing.
unit = enum + '''
typedef struct { int tipo, op; const char *titulo, *sub, *icone; } Item;
enum { IT_SEC, IT_GRP, IT_ROT, IT_OPC, IT_MAIS };
#define SEC(t,s,i) {IT_SEC,-1,t,s,i}
#define ROT(t) {IT_ROT,-1,t,0,0}
#define GRP(t,s,i) {IT_GRP,-1,t,s,i}
#define ROTS(t,s) {IT_ROT,-1,t,s,0}
#define OPC(o) {IT_OPC,o,0,0,0}
#define MAIS() {IT_MAIS,-1,"More options",0,0}
#include "ajustes_scenes.inc"
#include "ajustes_ux_tela.inc"
#include <assert.h>
#include <string.h>
static void montarTela(void) {}
int main(void) {
 const char *expected[] = {"Account","Switch Profile","Appearance","Layout",
 "Content & Discovery","Playback","Integrations","Tracking","Notifications","Advanced","About"};
 int categories=0, counts[AJ_N]={0}, category[AJ_N]={0}, advanced[AJ_N]={0}, more=0;
 for (int i=0;i<AJ_N_TELA;i++) {
  if(TELA[i].tipo==IT_SEC) { assert(categories<11);assert(!strcmp(TELA[i].titulo,expected[categories++])); more=0; }
  if(TELA[i].tipo==IT_MAIS) more=1;
  if(TELA[i].tipo==IT_OPC) { assert(++counts[TELA[i].op]==1); category[TELA[i].op]=categories-1; advanced[TELA[i].op]=more; }
 }
 assert(categories==11);
 assert(ajSceneForCategory(0)==AJS_CONTAS);
 assert(ajSceneForCategory(5)==AJS_REPRODUCAO);
 assert(ajSceneForOption(AJ_LEG_LINGUA)==AJS_IDIOMAS);
 assert(ajSceneForOption(AJ_TMDB_EPS)==AJS_TITULO);
 assert(counts[AJ_TMDB_CHAVE_LOCAL]==1&&category[AJ_TMDB_CHAVE_LOCAL]==6);
 assert(ajSceneForOption(AJ_TMDB_CHAVE_LOCAL)==AJS_TITULO);
 assert(ajSceneForOption(AJ_XTREAM_CONEXOES)==AJS_TVAOVIVO);
'''
routing=json.loads((ROOT/'tests/fixtures/settings-restoration/stage-b-routing.json').read_text())
assert set(routing)=={'AJ_FALHA_PROVEDORES','AJ_FALHA_PROVEDOR_ORDEM','AJ_DISCORD'}
assert all(x['to']=='Integrations' for x in routing.values())
presentation_overrides={'AJ_FILTROS_FONTES':{'category':'Integrations'},'AJ_PROF_BRILHO':{'advanced':0}}
# Custom provider configuration is now adjacent to Custom posters in Layout.
for option in ('AJ_POSTER_INST','AJ_POSTER_TOKEN','AJ_POSTER_EXTRA','AJ_POSTER_CHAVE','AJ_POSTER_MODELO','AJ_POSTER_TESTAR'):
    presentation_overrides[option]={'category':'Layout','advanced':0}
categories=['Account','Switch Profile','Appearance','Layout','Content & Discovery','Playback','Integrations','Tracking','Notifications','Advanced','About']
for r in records:
    unit += '#if ' + r['gate'] + '\n'
    unit += 'assert(counts[%s]==1); assert(ajSceneForOption(%s)==%s); assert(ajArtBlockForOption(%s)==%d);\n' % (r['id'], r['id'], r['scene'], r['id'], r['block'])
    unit += 'assert(category[%s]==%d); assert(advanced[%s]==%d);\n' % (r['id'], categories.index(presentation_overrides.get(r['id'],{}).get('category',routing.get(r['id'],{}).get('to',r['category']))), r['id'], presentation_overrides.get(r['id'],{}).get('advanced',r['advanced']))
    unit += '#endif\n'
unit += 'return 0;}\n'
import tempfile
with tempfile.TemporaryDirectory(prefix='nuvio-settings-a-') as tmp:
    path=Path(tmp)/'routing.c'; path.write_text(unit)
    for flags in [[], ['-DNV_ANDROID','-DNV_TOUCH_UI','-DNV_TECLADO_SIS'], ['-DNV_ANDROID','-DNV_P2P_MOTOR'], ['-DNV_TPK'], ['-DNV_TPK','-DNV_TPK40'], ['-D__EMSCRIPTEN__']]:
        binary=Path(tmp)/'routing'
        subprocess.run(['cc','-Isrc',*flags,str(path),'-o',str(binary)],cwd=ROOT,check=True)
        subprocess.run([str(binary)],check=True)
# English table covers labels, help and all explicit formatted settings translations.
string = r'"(?:\\.|[^"\\])*"'
def decode(literal):
    # Decode C hex byte runs without reinterpreting adjacent literal Unicode.
    literal=re.sub(r'(?:\\x[0-9a-fA-F]{2})+',
        lambda m: bytes.fromhex(m.group().replace('\\x','')).decode('utf8'),literal)
    return ast.literal_eval(literal)
pairs={decode(a):decode(b) for a,b in re.findall(r'\{\s*('+string+')\s*,\s*('+string+')\s*\}', (ROOT/'src/idioma_tab.h').read_text())}
extras={decode(a) for a,b in re.findall(r'\{\s*('+string+')\s*,\s*('+string+')\s*\}', (ROOT/'src/idioma.c').read_text())}
stage_c_english=set(json.loads((ROOT/'tests/fixtures/settings-restoration/stage-c-english.json').read_text()))
stage_c_english.update(json.loads((ROOT/'tests/fixtures/settings-restoration/stage-d-optin-english.json').read_text()))
stage_c_english.update(json.loads((ROOT/'tests/fixtures/settings-restoration/stage-g-english.json').read_text()))
stage_c_english.update(json.loads((ROOT/'tests/fixtures/settings-restoration/expansion-english.json').read_text()))
technical={'Dolby Vision','Dolby Atmos','TMDB','MDBList','IMDb','Letterboxd','MyAnimeList',
'Discord','Xtream','M3U','Stalker','IMDb + TMDB','HLS (.m3u8)','TS (.ts)','Chips',
'SpatialPosters','RPDB','YouTube','English','Inter','LG Display','Droid Sans','Montserrat',
'Roboto','Atkinson Hyperlegible Next','JetBrains Sans','SGSR','FSR 1 (EASU+RCAS)',
'Lanczos','Bilinear'}
labels=source[source.index('static const Opcao OPCOES'):]
labels=labels[:labels.index('\n};')]
for literal in re.findall(r'(?:ESC|NUM|LER|ACAO)\(('+string+')',labels):
    value=decode(literal)
    assert value in pairs or value in extras or value in stage_c_english or value in technical, 'Untranslated settings label: '+value
help_source=source[source.index('static const char *ajudaOpcaoBase('):]
help_source=help_source[:help_source.index('\n}\n')+3]
for literal in re.findall(r'return\s+('+string+')',help_source):
    value=decode(literal)
    assert not value or value in pairs or value in extras or value in stage_c_english, 'Untranslated settings help: '+value
print('PASS: source category order, all 388 presentation entries/gates/scenes/art blocks across 6 builds; English labels/help')

# Every static choice must be translated or an explicit technical name/numeric unit.
for name, body in re.findall(r'static const char \*(V_\w+)\[\]\s*=\s*\{(.*?)\};',source,re.S):
    body=re.sub(r'/\*.*?\*/|//[^\n]*','',body,flags=re.S)
    for literal in re.findall(string,body):
        value=decode(literal)
        assert (not value or value in pairs or value in extras or value in stage_c_english or value in technical or
                re.fullmatch(r'[\d .%×x/,+:–−\-]*\w{0,3}', value)), (name,value)
print('PASS: all static settings choice text translated, with explicit technical-name exceptions')

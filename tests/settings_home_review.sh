#!/usr/bin/env bash
# Isolated rendering and policy checks for the November settings defect review.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/bn-home-review.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc tests/family_row_styles.c -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -lm -o "$work/family" -Wno-deprecated-declarations -Wno-macro-redefined
"$work/family"
python3 - "$work" <<'PY'
from pathlib import Path
import re,subprocess,sys
s=Path('src/ctxmenu.c').read_text()
m=re.search(r'static int infoPossivel\([^\n]+\) \{.*?\n\}',s,re.S)
assert m
code='''
#include <assert.h>
#include <string.h>
static int simple,doLista,soFileira;
typedef struct { char imdb[64],titulo[100],tipo[16]; } CatItem;
int ajustes_menu_cartaz_simples(void){return simple;}
'''+m.group()+'''
int main(void){
 CatItem ci={"tt1234567","Fixture","movie"};
 assert(infoPossivel(&ci));simple=1;assert(!infoPossivel(&ci));
 simple=0;assert(infoPossivel(&ci));doLista=1;assert(!infoPossivel(&ci));
 doLista=0;soFileira=1;assert(!infoPossivel(&ci));soFileira=0;
 strcpy(ci.tipo,"channel");assert(!infoPossivel(&ci));
 strcpy(ci.tipo,"movie");ci.imdb[0]=0;assert(!infoPossivel(&ci));
 assert(!infoPossivel(0));
}
'''
p=Path(sys.argv[1])/'context.c';p.write_text(code)
subprocess.run(['cc',str(p),'-o',sys.argv[1]+'/context'],check=True)
subprocess.run([sys.argv[1]+'/context'],check=True)
print('PASS: actual enriched context-menu gate, On/Off, row-only menus, channels and missing metadata')
PY

#!/usr/bin/env bash
# Actual player request/EOF seams, with isolated metadata and overlay mocks.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/bn-postplay-wiring.XXXXXX")
trap 'rm -rf "$work"' EXIT
python3 - "$work" <<'PY'
from pathlib import Path
import re,subprocess,sys
s=Path('src/player.c').read_text()
def fn(name):
 m=re.search(r'static (?:void|int) '+name+r'\([^\n]*\) \{.*?\n\}',s,re.S)
 assert m,name
 return m.group()
code='''
#include <assert.h>
#include <string.h>
typedef struct { char tipo[16],imdb[64];long tmdb; } CatItem;
static int enabled=1,visible,waiting,requests,requestedSeries;
static long requestedTmdb;static char requestedId[64];
int ajustes_posplay_recomendacoes(void){return enabled;}
int posplay_visivel(void){return visible;}
int posplay_aguarda_escolha(void){return waiting;}
void extras_pedir(const char *id,int series,long tmdb){requests++;strcpy(requestedId,id);requestedSeries=series;requestedTmdb=tmdb;}
'''+fn('prepararRecomendacoesPlayer')+'\n'+fn('posplayReterNoFim')+'''
int main(void){
 CatItem ci={"movie","tt0123456",123};
 prepararRecomendacoesPlayer(&ci);assert(requests==1&&!strcmp(requestedId,ci.imdb)&&requestedSeries==0&&requestedTmdb==123);
 enabled=0;prepararRecomendacoesPlayer(&ci);assert(requests==1);
 enabled=1;strcpy(ci.tipo,"series");prepararRecomendacoesPlayer(&ci);assert(requests==1);
 strcpy(ci.tipo,"channel");prepararRecomendacoesPlayer(&ci);prepararRecomendacoesPlayer(0);assert(requests==1);
 for(visible=0;visible<2;visible++)for(waiting=0;waiting<2;waiting++)for(int next=0;next<2;next++)
  assert(posplayReterNoFim(next)==(visible||(next&&waiting)));
 // Movie recommendations at 100%, or a still-watching prompt when the
 // episode catalogue refreshes: EOF must retain the visible choice.
 visible=1;waiting=1;assert(posplayReterNoFim(0));
 visible=0;waiting=0;assert(!posplayReterNoFim(1));
}
'''
p=Path(sys.argv[1])/'test.c';p.write_text(code)
subprocess.run(['cc',str(p),'-o',sys.argv[1]+'/test'],check=True)
subprocess.run([sys.argv[1]+'/test'],check=True)
assert 'prepararRecomendacoesPlayer(temFixo ? &itemFixo : NULL);' in s
assert '!posplayReterNoFim(player_proximo_jogavel() != NULL)' in s  # unaired next = last episode
print('PASS: direct movie metadata request/master-Off/live exclusion and actual EOF retention for recommendations/presence prompt/dismissal')
PY

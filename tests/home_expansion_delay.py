"""Exercise Home's production dwell timer without a device, GL or network."""
from pathlib import Path
import subprocess
import tempfile

source = Path(__file__).resolve().parents[1] / 'src/home.c'
text = source.read_text()
start = text.index('  // Relogio da expansao.')
end = text.index('\n  { int alvo = palcoFundoDestino();', start)
timer = text[start:end]
with tempfile.TemporaryDirectory(prefix='nuvio-expansion-delay-') as directory:
    path = Path(directory) / 'timer.c'
    path.write_text(r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
typedef uint32_t Uint32;
static int enabled=1, portrait, context, expandable=1, motionReduzido;
static int expFileira=-1, expColuna=-1, okPressionando;
static float expAbre, okHold, delay;
static Uint32 expDesde;
static struct {int fileira,coluna;} foco;
static int ajustes_expandir_poster(void){return enabled;}
static int homeRetratoTelefone(void){return portrait;}
static int ctx_aberto(void){return context;}
static float ajustes_expandir_poster_atraso(void){return delay;}
static int podeExpandir(int r){(void)r;return expandable;}
#define NV_MOLA_TELA 10
static float anim_mola(float x,float target,float dt,int speed){
 (void)dt;(void)speed;return (x+target)*.5f;
}
static void tick(Uint32 agora){float dt=.016f;
''' + timer + r'''
}
int main(void){
 for(int seconds=0;seconds<=10;seconds++){
  delay=seconds;enabled=1;portrait=context=motionReduzido=0;
  expandable=1;expFileira=-1;foco.fileira=foco.coluna=0;
  tick(1000);assert(expDesde==1000);
  if(seconds){tick(1000+seconds*1000-1);assert(expAbre==0);}
  tick(1000+seconds*1000);assert(expAbre>0);
  foco.coluna=1;tick(20000);assert(expDesde==20000);
  if(seconds)assert(expAbre==0);
  enabled=0;tick(21000);assert(expAbre==0&&expFileira==-1&&expColuna==-1);
  printf("PASS actual Home expansion dwell: %d s, focus reset and master Off\n",seconds);
 }
 delay=2;enabled=1;portrait=0;expFileira=-1;tick(1000);
 context=1;tick(5000);assert(expAbre==0&&expDesde==5000);
 context=0;tick(6999);assert(expAbre==0);tick(7000);assert(expAbre>0);
 portrait=1;tick(8000);assert(expAbre==0&&expFileira==-1);
 portrait=0;expandable=0;tick(9000);tick(12000);assert(expAbre==0);
 expandable=1;motionReduzido=1;tick(12001);assert(expAbre==1);
 // SDL's 32-bit timer can wrap while a TV remains on for weeks.
 expFileira=-1;motionReduzido=0;tick(UINT32_MAX-1000u);
 tick(998);assert(expAbre==0);tick(999);assert(expAbre>0);
 puts("PASS context-menu suspension, portrait/landscape gates, reduced motion and timer wrap");
}
''')
    executable = Path(directory) / 'timer'
    subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                    str(path), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)

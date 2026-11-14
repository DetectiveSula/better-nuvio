"""Exercise production Home preload planning without a window, network or GPU."""
from pathlib import Path
import subprocess
import tempfile

root=Path(__file__).resolve().parents[1]
home=(root/'src/home.c').read_text()
texture=(root/'src/tex_cache.c').read_text()
def function(source,name):
    start=source.index(name+'(')
    start=source.rfind('\n',0,start)+1
    return source[start:source.index('\n}',start)+3]

code=r'''
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define MAX_FIL 8
#define NV_TELA_W 1000
#define NV_TELA_H 600
#define NV_LEGACY_ROW_HEAD_H 20
typedef enum {FILEIRA_NORMAL,FILEIRA_CATALOGOS,FILEIRA_SOCIAL,FILEIRA_LARGA,
 FILEIRA_HEROI,FILEIRA_TOP10_NUM,FILEIRA_TOP10,FILEIRA_CONTINUE,
 FILEIRA_POSTER_GRANDE,FILEIRA_HEROI_DIVIDIDO,FILEIRA_RETORNO} TipoFileira;
typedef struct {TipoFileira tipo;int n,ini,folders[24];} Fileira;
typedef struct {float scroll;unsigned long quadro;int parados,direcao;} TexLookahead;
typedef struct {char cover[40];} ColFolder;
static Fileira fileiras[MAX_FIL];static int nFileiras=MAX_FIL;
static ColFolder folders[800];static char posters[800][40];
static struct {int fileira;} foco;
static float velX[MAX_FIL],scrollX[MAX_FIL],scrollY,velY;
static struct {float x;} bordaPag;
static unsigned long quadroAtual=1;
static const char *wanted;static char requested[80];static int attempts,moving,markedCovers;
static const ColFolder *col_folder(int i){return i>=0&&i<800?folders+i:NULL;}
static const char *col_capa(const ColFolder *f){return f->cover;}
static int fileiraItemIndice(const Fileira *f,int c){return f->ini+c;}
static const char *arte_por_identidade(int i,int landscape){(void)landscape;return i>=0&&i<800?posters[i]:NULL;}
static float homeFileiraX(int r,int c){return c*120-scrollX[r];}
static float larguraFil(int r){(void)r;return 100;}
static float alturaFil(int r){(void)r;return 300;}
static float alturaTotalFil(int r){(void)r;return 300;}
static float fileiraGap(void){return 10;}
static int editorial(TipoFileira t){(void)t;return 0;}
static int ajustes_posteres_deitados(void){return 0;}
static int fileiraEhCanal(const Fileira *f){(void)f;return 0;}
static void tex_grade_movendo(int v){moving=v;}
static void tex_grade_velocidade(float v){(void)v;}
static void tex_pedido_fora(int v){assert(v==0);}
static void tex_residencia_inicio(int r){(void)r;}
static int tex_residencia_fileiras(void){return 2;}
static void tex_residencia_marcar(const char *url,int r){(void)r;if(url&&!strncmp(url,"cover-",6))markedCovers++;}
static int tex_lookahead_pedir(const char *url,float width){
 if(!url)return 0;assert(width==100);attempts++;
 if(wanted&&!strcmp(url,wanted)){snprintf(requested,sizeof requested,"%s",url);return 1;}
 return 0; /* Cached/in-flight candidates must not stall the lookahead. */
}
static void reset(const char *target){wanted=target;requested[0]=0;attempts=0;quadroAtual++;}
'''
code+=function(texture,'int tex_lookahead_parado')
for name in ['static const char *homeArteFileira','static void homePrecarregarHorizontal',
             'static void homeResidencia','static void homePrecarregarVertical']:
    code+=function(home,name)
code+=r'''
int main(void){
 for(int r=0;r<MAX_FIL;r++){
  fileiras[r].tipo=FILEIRA_CATALOGOS;fileiras[r].n=20;fileiras[r].ini=r*100;
  for(int c=0;c<20;c++){
   int i=r*100+c;fileiras[r].folders[c]=i;
   snprintf(folders[i].cover,sizeof folders[i].cover,"cover-%d",i);
   snprintf(posters[i],sizeof posters[i],"poster-%d",i);
  }
 }
 foco.fileira=0;
 assert(!strcmp(homeArteFileira(0,0,0),"cover-0"));
 fileiras[0].tipo=FILEIRA_NORMAL;assert(!strcmp(homeArteFileira(0,0,0),"poster-0"));
 fileiras[0].tipo=FILEIRA_CATALOGOS;
 assert(!homeArteFileira(0,30,0));
 homeResidencia();assert(markedCovers>0);
 reset("cover-11");homePrecarregarHorizontal(0,0,100);assert(!requested[0]);
 reset("cover-11");homePrecarregarHorizontal(0,0,100);assert(!requested[0]);
 reset("cover-11");homePrecarregarHorizontal(0,0,100);assert(!strcmp(requested,"cover-11")&&attempts==3);
 velX[0]=600;reset("cover-9");homePrecarregarHorizontal(0,0,100);assert(!strcmp(requested,"cover-9")&&moving);
 scrollX[0]=1200;velX[0]=-600;reset("cover-9");homePrecarregarHorizontal(0,0,100);assert(!strcmp(requested,"cover-9"));
 velX[0]=0;scrollX[0]=0;velY=600;
 reset("cover-300");homePrecarregarVertical(0,0,0);assert(!strcmp(requested,"cover-300")&&!moving);
 scrollY=1500;reset(NULL);homePrecarregarVertical(0,0,0);
 scrollY=1000;velY=-600;reset("cover-100");homePrecarregarVertical(0,0,0);assert(!strcmp(requested,"cover-100"));
 reset("cover-100");homePrecarregarVertical(0,1,0);assert(!attempts&&!moving);
 velY=600;velX[0]=600;reset("cover-9");homePrecarregarHorizontal(0,0,100);assert(!attempts);
 puts("PASS: poster and collection-cover lookahead before visibility; cached neighbours skipped; vertical scroll preloads without waiting for idle; collection covers retained");
}
'''
# Assert that the tested planners are called by the real draw path, including
# the collection-row branch that previously skipped every preload mechanism.
draw=home[home.index('static void home_desenhar_('):]
assert 'homePrecarregarVertical(topoFil,pd,cinema);' in draw
collection=draw[draw.index('if (tipo == FILEIRA_CATALOGOS) {'):]
assert 'homePrecarregarHorizontal(r,0,lw);' in collection[:collection.index('continue;')]
with tempfile.TemporaryDirectory(prefix='nuvio-home-prefetch-') as directory:
    source=Path(directory)/'test.c';binary=Path(directory)/'test'
    source.write_text(code)
    subprocess.run(['cc','-O1','-g','-fsanitize=undefined','-fno-sanitize-recover=undefined',str(source),'-lm','-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)

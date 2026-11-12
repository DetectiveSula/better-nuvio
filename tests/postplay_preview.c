#define NV_POSTPLAY_PREVIEW_TEST
#include "gfx.h"
#include <assert.h>
#include <string.h>
static int supported=1,state,requests,opens,closes,holes,answered;
static const char *url;
static int backend_supported(void){return supported;}
static int backend_state(void){return state;}
static void backend_close(void){closes++;state=0;}
static void backend_open(const char *u,GfxRect r){assert(u&&r.w>0);opens++;state=1;}
void trailerimdb_pedir(const char*i){assert(!strncmp(i,"tt",2));requests++;}
const char *trailerimdb_url(const char*i,const char**name){(void)i;(void)name;return url;}
int trailerimdb_respondeu(const char*i){(void)i;return answered;}
void gfx_furo_raio(GfxRect r,float radius){assert(r.w>0&&radius>0);holes++;}
#include "../src/postplay_preview.c"
int main(void){
 GfxRect r={100,100,200,300};
 postplay_preview_draw("tt1",0,2500,0,r);assert(!requests&&!opens);
 supported=0;postplay_preview_draw("tt1",1,2500,0,r);assert(!requests);supported=1;
 for(unsigned delay=0;delay<=30000;delay+=100){
  postplay_preview_close();url="https://fixture.invalid/trailer.mp4";int before=opens;
  postplay_preview_draw("tt1",1,delay,1000,r);
  if(delay)assert(opens==before);
  if(delay)postplay_preview_draw("tt1",1,delay,1000+delay-1,r);
  if(delay)assert(opens==before);
  postplay_preview_draw("tt1",1,delay,1000+delay,r);assert(opens==before+1);
  int h=holes;postplay_preview_draw("tt1",1,delay,1001+delay,r);assert(holes==h);
  state=2;postplay_preview_draw("tt1",1,delay,1002+delay,r);assert(holes==h+1);
  state=-1;postplay_preview_draw("tt1",1,delay,1003+delay,r);assert(failed&&!opened);
  postplay_preview_draw("tt1",1,delay,50000+delay,r);assert(opens==before+1);
 }
 postplay_preview_close();url=NULL;answered=0;postplay_preview_draw("tt2",1,0,100,r);assert(asked&&!opened);
 postplay_preview_draw("tt3",1,0,101,r);assert(!strcmp(current,"tt3"));
 url="http://fixture.invalid/unsafe";postplay_preview_draw("tt3",1,0,102,r);assert(failed&&!opened);
 postplay_preview_close();url="https://fixture.invalid/safe";postplay_preview_draw("tt4",1,0,100,r);int before=closes;
 postplay_preview_draw("tt4",1,0,12101,r);assert(closes==before+1&&failed);
 postplay_preview_close();postplay_preview_draw("tmdb:42",1,0,100,r);assert(!opened);
 puts("PASS post-play preview: all 301 delays, opt-in/unsupported gates, first-frame-only transparency, focus cancellation, errors/timeouts/no retry loop and independent closure");
}

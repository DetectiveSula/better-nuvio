#define NV_POSTPLAY_PREVIEW_TEST
#include "gfx.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
/* Fakes for the player backend and the three trailer sources; the order
 * itself is the real trailerfonte.c (linked by postplay_preview.sh). */
static int supported=1,state,opens,closes,holes;
static int appleAsks,ytAsks,imdbAsks,appleAnswered,ytAnswered,imdbAnswered,ytPending;
static const char *appleUrl,*ytUrl,*imdbUrl;
static char lastOpen[256],lastTitle[160],lastMeta[96];
static int lastSerie;static long lastTmdb;
static int fonteAjuste;
int ajustes_trailer_fonte(void){return fonteAjuste;}
static int backend_supported(void){return supported;}
static int backend_state(void){return state;}
static void backend_close(void){closes++;state=0;}
static void backend_open(const char *u,GfxRect r){assert(u&&r.w>0);opens++;state=1;snprintf(lastOpen,sizeof lastOpen,"%s",u);}
void trailerapple_pedir(const char *i,const char *t,const char *m,int s){assert(!strncmp(i,"tt",2));appleAsks++;
 snprintf(lastTitle,sizeof lastTitle,"%s",t);snprintf(lastMeta,sizeof lastMeta,"%s",m);lastSerie=s;}
const char *trailerapple_url(const char *i){(void)i;return appleUrl;}
int trailerapple_respondeu(const char *i){(void)i;return appleAnswered;}
void extras_hero_trailer_pedir(const char *i,int s,long t){assert(!strncmp(i,"tt",2));ytAsks++;lastTmdb=t;(void)s;}
int extras_hero_trailers_proprios(const char *i,char dst[][16],int max){(void)i;if(!ytUrl&&!ytPending)return 0;assert(max>0);snprintf(dst[0],16,"dQw4w9WgXcQ");return 1;}
int extras_hero_trailer_respondeu(const char *i){(void)i;return ytAnswered;}
const char *trailerytb_primeiro_1080p(const char *const *ids,int n,int *pronto){assert(n==1&&ids[0]);*pronto=!ytPending;return ytPending?NULL:ytUrl;}
void trailerimdb_pedir(const char*i){assert(!strncmp(i,"tt",2));imdbAsks++;}
const char *trailerimdb_url(const char*i,const char**name){(void)i;(void)name;return imdbUrl;}
int trailerimdb_respondeu(const char*i){(void)i;return imdbAnswered;}
void gfx_furo_raio(GfxRect r,float radius){assert(r.w>0&&radius>0);holes++;}
#include "../src/postplay_preview.c"
static PostplayPreviewItem item(const char *id){PostplayPreviewItem it={id,"Fixture Title","2024",0,77};return it;}
static void fontes(const char *a,int aOk,const char *y,int yOk){appleUrl=a;appleAnswered=aOk;ytUrl=y;ytAnswered=yOk;ytPending=0;imdbUrl=NULL;imdbAnswered=1;}
int main(void){
 GfxRect r={100,100,200,300};
 PostplayPreviewItem t1=item("tt1");
 postplay_preview_draw(&t1,0,2500,0,r);assert(!appleAsks&&!opens);
 supported=0;postplay_preview_draw(&t1,1,2500,0,r);assert(!appleAsks);supported=1;
 postplay_preview_draw(NULL,1,0,0,r);assert(!opens);
 // The delay gate, first-frame transparency, error and no retry loop, for every delay.
 for(unsigned delay=0;delay<=30000;delay+=100){
  postplay_preview_close();fontes("https://fixture.invalid/apple.m3u8",1,NULL,1);int before=opens;
  postplay_preview_draw(&t1,1,delay,1000,r);
  if(delay)assert(opens==before);
  if(delay)postplay_preview_draw(&t1,1,delay,1000+delay-1,r);
  if(delay)assert(opens==before);
  postplay_preview_draw(&t1,1,delay,1000+delay,r);assert(opens==before+1&&!strcmp(lastOpen,appleUrl));
  int h=holes;postplay_preview_draw(&t1,1,delay,1001+delay,r);assert(holes==h);
  state=2;postplay_preview_draw(&t1,1,delay,1002+delay,r);assert(holes==h+1);
  state=-1;postplay_preview_draw(&t1,1,delay,1003+delay,r);assert(!opened);
  postplay_preview_draw(&t1,1,delay,1004+delay,r);assert(failed&&!opened);   // no YouTube: done
  postplay_preview_draw(&t1,1,delay,50000+delay,r);assert(opens==before+1);
 }
 // The lookups get the card's title, year and TMDB id; IMDb is not asked in Automatic.
 assert(!strcmp(lastTitle,"Fixture Title")&&!strcmp(lastMeta,"2024")&&lastSerie==0&&lastTmdb==77&&ytAsks&&!imdbAsks);
 // Apple first even when YouTube is ready.
 postplay_preview_close();fontes("https://fixture.invalid/apple.m3u8",1,"https://fixture.invalid/yt.mp4",1);
 postplay_preview_draw(&t1,1,0,100,r);assert(!strcmp(lastOpen,"https://fixture.invalid/apple.m3u8"));
 // Apple still looking: wait, YouTube does not win by being faster.
 postplay_preview_close();fontes(NULL,0,"https://fixture.invalid/yt.mp4",1);int o=opens;
 postplay_preview_draw(&t1,1,0,100,r);assert(opens==o&&!failed);
 // Apple answers without a trailer: YouTube.
 appleAnswered=1;postplay_preview_draw(&t1,1,0,200,r);assert(opens==o+1&&!strcmp(lastOpen,"https://fixture.invalid/yt.mp4"));
 // Apple fails in the player: YouTube next.
 postplay_preview_close();fontes("file:///data/user/0/app/cache/trailer/tt1-apple-play.m3u8",1,"http://127.0.0.1:40123/ytb/dQw4w9WgXcQ.m3u8",1);
 postplay_preview_draw(&t1,1,0,100,r);assert(!strncmp(lastOpen,"file:///",8));
 state=-1;postplay_preview_draw(&t1,1,0,200,r);postplay_preview_draw(&t1,1,0,300,r);
 assert(opened&&!strcmp(lastOpen,"http://127.0.0.1:40123/ytb/dQw4w9WgXcQ.m3u8"));
 // Neither plays: stop, no loop.
 state=-1;postplay_preview_draw(&t1,1,0,400,r);postplay_preview_draw(&t1,1,0,500,r);assert(failed&&!opened);
 o=opens;postplay_preview_draw(&t1,1,0,90000,r);assert(opens==o);
 // Apple stalls without a first frame for 12 s: YouTube.
 postplay_preview_close();fontes("https://fixture.invalid/apple.m3u8",1,"https://fixture.invalid/yt.mp4",1);
 postplay_preview_draw(&t1,1,0,100,r);int c=closes;postplay_preview_draw(&t1,1,0,12101,r);assert(closes==c+1);
 postplay_preview_draw(&t1,1,0,12102,r);assert(!strcmp(lastOpen,"https://fixture.invalid/yt.mp4"));
 // A link the player may not open is skipped, not opened.
 postplay_preview_close();fontes("http://fixture.invalid/unsafe",1,"https://fixture.invalid/yt.mp4",1);o=opens;
 postplay_preview_draw(&t1,1,0,100,r);assert(opens==o);postplay_preview_draw(&t1,1,0,101,r);assert(!strcmp(lastOpen,"https://fixture.invalid/yt.mp4"));
 // YouTube still resolving after Apple gave none: wait, then give up after the window.
 postplay_preview_close();fontes(NULL,1,NULL,0);ytPending=1;o=opens;
 postplay_preview_draw(&t1,1,0,100,r);assert(opens==o&&!failed);
 postplay_preview_draw(&t1,1,0,12101,r);assert(failed&&opens==o);
 // Focus moves: the new card starts over.
 PostplayPreviewItem t3=item("tt3");postplay_preview_draw(&t3,1,0,101,r);assert(!strcmp(current,"tt3"));
 // No IMDb id: nothing.
 postplay_preview_close();PostplayPreviewItem tm=item("tmdb:42");postplay_preview_draw(&tm,1,0,100,r);assert(!opened);
 // IMDb chosen as the trailer source: IMDb only.
 postplay_preview_close();fonteAjuste=TRF_IMDB;fontes("https://fixture.invalid/apple.m3u8",1,"https://fixture.invalid/yt.mp4",1);
 imdbUrl="https://fixture.invalid/imdb.mp4";postplay_preview_draw(&t1,1,0,100,r);assert(imdbAsks==1&&!strcmp(lastOpen,imdbUrl));
 fonteAjuste=0;
 puts("PASS post-play preview: Apple then YouTube (Trailer source order), waits for Apple, skips a failed or blocked source, delays, first-frame transparency, focus cancellation, timeouts without retry loops");
}

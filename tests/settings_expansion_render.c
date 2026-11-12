#include "gfx.h"
#include "ajustes.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned char pixels[400*400*4];
static unsigned long render(GLuint tex,int surface,int avatar){
 glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
 gfx_card_para(surface,((GfxRect){100,80,200,280}),tex,avatar?GFX_AVATAR:GFX_CARD,1,0,0,.08f,0,0,0,1);
 assert(gfx_card_superficie_atual==NV_CARD_POSTERS);
 glReadPixels(0,0,400,400,GL_RGBA,GL_UNSIGNED_BYTE,pixels);assert(glGetError()==GL_NO_ERROR);
 unsigned long sum=0;for(unsigned i=0;i<sizeof pixels;i+=4)sum+=pixels[i]+pixels[i+1]+pixels[i+2];return sum;
}
int main(void){
 assert(SDL_Init(SDL_INIT_VIDEO)==0);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
 SDL_Window *w=SDL_CreateWindow("Card depth settings regression",0,0,400,400,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);assert(w);
 SDL_GLContext gl=SDL_GL_CreateContext(w);assert(gl);gfx_tamanho_alvo(400,400);glViewport(0,0,400,400);assert(gfx_iniciar());
 GLuint tex;glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);unsigned char rgba[]={40,40,40,255};glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 const char *surfaceKeys[]={"cardDepthPostersEnabled","cardDepthContinueWatchingEnabled","cardDepthEpisodeCardsEnabled","cardDepthCastEnabled","cardDepthTrailersEnabled"};
 /* Storage identifiers are looked up, never positional. */
 for(int i=0;i<5;i++)assert(ajustes_shot_valor(surfaceKeys[i],0));
 assert(ajustes_shot_valor("cardDepthEnabled",0));
 assert(ajustes_shot_valor("cardDepthEdgeStrength",56));assert(ajustes_shot_valor("cardDepthSheenStrength",100));assert(ajustes_shot_valor("cardDepthEdgeCoverage",100));
 for(int s=0;s<5;s++){
  unsigned long on=render(tex,s,s==NV_CARD_CAST);assert(ajustes_shot_valor(surfaceKeys[s],1));unsigned long off=render(tex,s,s==NV_CARD_CAST);assert(on>off);
  assert(ajustes_shot_valor(surfaceKeys[s],0));assert(render(tex,s,s==NV_CARD_CAST)==on);
  printf("PASS surface=%d enabled changes actual framebuffer; disabled/restored independently\n",s);
 }
 assert(ajustes_shot_valor("cardDepthSheenStrength",0));unsigned long edge[3];int e[]={28,42,56};for(int i=0;i<3;i++){assert(ajustes_shot_valor("cardDepthEdgeStrength",e[i]));edge[i]=render(tex,0,0);}assert(edge[0]<edge[1]&&edge[1]<edge[2]);
 unsigned long coverage[3];int c[]={0,50,100};for(int i=0;i<3;i++){assert(ajustes_shot_valor("cardDepthEdgeCoverage",c[i]));coverage[i]=render(tex,0,0);}assert(coverage[0]<coverage[1]&&coverage[1]<coverage[2]);
 assert(ajustes_shot_valor("cardDepthEdgeStrength",0));unsigned long sheen[3];for(int i=0;i<3;i++){assert(ajustes_shot_valor("cardDepthSheenStrength",i*50));sheen[i]=render(tex,0,0);}assert(sheen[0]<sheen[1]&&sheen[1]<sheen[2]);
 assert(ajustes_shot_valor("cardDepthEnabled",1));assert(render(tex,0,0)==sheen[0]);
 puts("PASS actual pixels: all glow presets/coverage modes/sheen strengths/master off; GL shaders compile");
 glDeleteTextures(1,&tex);gfx_encerrar();SDL_GL_DeleteContext(gl);SDL_DestroyWindow(w);SDL_Quit();return 0;
}

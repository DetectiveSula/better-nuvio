/* Render the actual settings illustration, not a reconstruction of its geometry. */
#include "../src/ajustes.c"
#include <assert.h>
static unsigned char frame[640*360*4], previous[sizeof frame];
static void render(int op,int width,int radius,float time){
 valor[AJ_LARGURA_DP]=width;valor[AJ_RAIO_DP]=radius;ajMovReiniciar();
 memset(&C,0,sizeof C);C.s=1;C.a=1;C.w=640;C.h=360;C.ar=.1f;C.ag=.6f;C.ab=.7f;C.t=time;
 glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);ajcCenaCartazes(op);
 glReadPixels(0,0,640,360,GL_RGBA,GL_UNSIGNED_BYTE,frame);assert(glGetError()==GL_NO_ERROR);
}
int main(void){
 assert(SDL_Init(SDL_INIT_VIDEO)==0);
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
 SDL_Window *w=SDL_CreateWindow("Selected card settings preview",0,0,640,360,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);assert(w);
 SDL_GLContext gl=SDL_GL_CreateContext(w);assert(gl);gfx_tamanho_alvo(640,360);glViewport(0,0,640,360);assert(gfx_iniciar());
 render(AJ_LARGURA_DP,104,12,0);memcpy(previous,frame,sizeof frame);
 render(AJ_LARGURA_DP,104,12,1.5f);assert(!memcmp(previous,frame,sizeof frame));
 render(AJ_LARGURA_DP,140,12,0);assert(memcmp(previous,frame,sizeof frame));
 render(AJ_RAIO_DP,120,0,0);memcpy(previous,frame,sizeof frame);
 render(AJ_RAIO_DP,120,0,1.5f);assert(!memcmp(previous,frame,sizeof frame));
 render(AJ_RAIO_DP,120,16,0);assert(memcmp(previous,frame,sizeof frame));
 puts("PASS actual preview pixels: selected width and sharp/rounded corners change geometry; idle time cannot change either selection");
 gfx_encerrar();SDL_GL_DeleteContext(gl);SDL_DestroyWindow(w);SDL_Quit();return 0;
}

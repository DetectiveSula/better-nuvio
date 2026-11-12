#define NV_PROFILE_EDIT_TEST
#define main existing_touch_main
#include "perfilsel_toque.c"
#undef main
static int editor,editorSlot=-1,opens;
int profile_editor_active(void){return editor;}
int profile_editor_open(int slot,const char *pin){(void)pin;editor=1;editorSlot=slot;opens++;return 1;}
void profile_editor_close(void){editor=0;}
void profile_editor_update(void){}
Uint32 SDL_GetTicks(void){return 1000;}
static void key(Uint32 type,SDL_Keycode code){SDL_Event e={0};e.type=type;e.key.keysym.sym=code;perfilsel_evento(&e);}
int main(void){
  total=2;foco=0;pinDe=-1;preparando=concluido=0;
  key(SDL_KEYDOWN,SDLK_RETURN);assert(profileOkDown && !concluido);
  key(SDL_KEYUP,SDLK_RETURN);assert(concluido && selecionado==1 && !editor);
  concluido=0;key(SDL_KEYDOWN,SDLK_RETURN);perfilEditar(foco);
  assert(editor && editorSlot==0 && !concluido && opens==1);
  key(SDL_KEYUP,SDLK_RETURN);assert(!concluido);
  editor=0;key(SDL_KEYUP,SDLK_RETURN);assert(!concluido);
  key(SDL_KEYDOWN,SDLK_RETURN);key(SDL_KEYDOWN,SDLK_RIGHT);key(SDL_KEYUP,SDLK_RETURN);
  assert(!concluido && foco==1);
  perfilEditar(1);assert(pinDe==1 && editAfterPin==1 && !editor && selecionado==1);
  eventoPin(SDLK_ESCAPE);assert(pinDe==-1 && editAfterPin==-1 && !editor);
  foco=0;touchHold=1;perfilAtivar(0,1);assert(editor && opens==2 && !concluido);
  puts("PASS: short OK opens profile; hold opens editor once; release/cancel/navigation cannot select; locked edit asks for PIN");
  return 0;
}

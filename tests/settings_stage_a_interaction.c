#include "../src/ajustes.c"
#include <assert.h>
static void key(SDL_Keycode k) {
  SDL_Event e = {0}; e.type = SDL_KEYDOWN; e.key.keysym.sym = k;
  ajustes_evento(&e);
}
int main(void) {
  assert(SDL_Init(SDL_INIT_TIMER) == 0);
  memcpy(valor, valorPadrao, sizeof valor);
  montarTela(); focarSecao(0); focoIndice = 1; uxIndice = 2; uxTopo = -1;
  assert(nSecoes == 11);
  for (int s = 0; s < nSecoes; s++) {
    assert(uxIndice == s + 2);
    key(SDLK_RIGHT); assert(!focoIndice);
    assert(secAtual == s);
    assert(focoItem > secIni[s] && focoItem < secFim(s));
    key(SDLK_LEFT); assert(focoIndice && uxIndice == s + 2);
    if (s + 1 < nSecoes) key(SDLK_DOWN);
  }
  for (int s = nSecoes-1; s > 0; s--) { key(SDLK_UP); assert(uxIndice == s + 1); }
  key(SDLK_UP); assert(uxIndice == 1); key(SDLK_RIGHT); assert(!focoIndice);
  key(SDLK_ESCAPE); assert(focoIndice);
  key(SDLK_UP); assert(uxIndice == 0); key(SDLK_RETURN); assert(uxPediuBusca);
  // A short viewport scrolls both ends into view without shrinking the text.
  uxIndice = nSecoes + 1;
  float scroll = ajSidebarEnsureVisible(62, 300);
  assert(scroll > 0 && uxIndice * 62 >= scroll && (uxIndice + 1) * 62 <= scroll + 300);
  uxIndice = 0; assert(ajSidebarEnsureVisible(62, 300) == 0);
  uxNotificar("Ajuste salvo nesta TV."); assert(strstr(uxAviso, "saved"));
  // English search and values are independent of a saved Portuguese UI language.
  valor[AJ_IDIOMA] = IDIOMA_PT + 1;
  AjusteBuscaResultado results[20];
  int n = ajustes_buscar("subtitle", results, 20);
  assert(n > 0);
  for (int i=0;i<n;i++) assert(!strstr(results[i].titulo,"legenda"));
  assert(ajSceneForOption(AJ_LEG_LINGUA) == AJS_IDIOMAS);
  assert(ajSceneForOption(AJ_XTREAM_CONEXOES) == AJS_TVAOVIVO);
  char valueText[64];
  uxValorTexto(AJ_CW_DIAS, 7, valueText, sizeof valueText);
  assert(!strcmp(valueText, "7 days"));
  int days = valor[AJ_CW_DIAS]; valor[AJ_CW_DIAS] = 7;
  assert(!strcmp(textoValor(AJ_CW_DIAS), "7 days")); valor[AJ_CW_DIAS] = days;
  for (int i=0;i<GUIA_NENT;i++) if (GUIA_ENT[i].alvo == AJ_NT_IMDB)
    assert(strstr(guiaOnde(&GUIA_ENT[i]), "Settings › Integrations › Ratings"));
  assert(!strcmp(uxCaminho(AJ_LEG_LINGUA),"Playback"));
  assert(!strcmp(uxCaminho(AJ_XTREAM_CONEXOES),"Content & Discovery"));
  puts("PASS: native remote category entry/return/order, search, English and form routing");
  SDL_Quit(); return 0;
}

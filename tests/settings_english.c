#include "idioma.h"
#include "idiomacod.h"
#include <assert.h>
#include <pthread.h>
#include <string.h>
int ajustes_idioma(void) { return IDIOMA_PT; }
int ajustes_idioma_ingles(void) { return 0; }
static void *worker(void *unused) {
  (void)unused;
  assert(!strcmp(i18n("Voltar"), "Voltar"));
  return 0;
}
int main(void) {
  pthread_t thread;
  assert(!strcmp(i18n("Voltar"), "Voltar"));
  int first = idioma_ingles_entrar();
  assert(!strcmp(i18n("Voltar"), "Back"));
  int nested = idioma_ingles_entrar();
  idioma_ingles_sair(nested);
  assert(!strcmp(i18n("Voltar"), "Back"));
  assert(!pthread_create(&thread, 0, worker, 0));
  assert(!pthread_join(thread, 0));
  idioma_ingles_sair(first);
  assert(!strcmp(i18n("Voltar"), "Voltar"));
  assert(!strcmp(idioma_ingles("Voltar"), "Back"));
  assert(!strcmp(idioma_ingles("TorBox"), "TorBox"));
  assert(!strcmp(idioma_ingles("Menu suspenso"), "Dropdown menu"));
  assert(!strcmp(idioma_ingles("Pacote 1"), "Pack 1"));
  assert(!strcmp(idioma_ingles("日本語"), "Japanese"));
  return 0;
}

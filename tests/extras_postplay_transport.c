// No real HTTP: only a canned TMDB movie response can be returned.
#define rede_baixar postplayUnusedDownload
#define dlopen postplayNoHttpLibrary
#include "../src/rede.c"
#undef rede_baixar
#undef dlopen
#include <stdatomic.h>
#include <assert.h>
static atomic_int requests;
void *postplayNoHttpLibrary(const char *name,int mode) {(void)name;(void)mode;return NULL;}
int postplayMockRequests(void) {return atomic_load(&requests);}
char *rede_baixar(const char *url,int seconds) {
  (void)seconds;
  if(!strncmp(url,"https://api.themoviedb.org/3/movie/",33)) {
    atomic_fetch_add(&requests,1);
    assert(strstr(url,"recommendations"));
    return strdup("{\"id\":354912,\"original_language\":\"en\",\"recommendations\":{\"results\":[{\"id\":101,\"title\":\"Mock related film\",\"release_date\":\"2026-01-01\",\"poster_path\":\"/mock.jpg\"}]}}");
  }
  return NULL;
}

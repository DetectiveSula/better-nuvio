#include "ajustes.h"
#include "descoberta.h"
#include "extras.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int postplayMockRequests(void);
static void waitForMetadata(void) {
  for(int i=0;i<2000&&extras_carregando();i++) SDL_Delay(5);
  assert(!extras_carregando());
}
static void setting(const char *key,int value) {assert(ajustes_shot_valor(key,value));}
int main(void) {
  assert(SDL_Init(SDL_INIT_TIMER)==0);
  ajustes_dir(getenv("NUVIO_DADOS"));
  desc_tmdb_definir("00000000000000000000000000000000"); // synthetic fixture value
  setting("tmdb_enabled",0);
  const char *off[]={"tmdb_use_details","tmdb_use_release_dates","tmdb_use_trailers",
                    "tmdb_use_collections","tmdb_use_productions","tmdb_use_more_like_this"};
  for(size_t i=0;i<sizeof off/sizeof off[0];i++) setting(off[i],1);
  setting("postPlayRecommendationsLocal",1);
  extras_pedir("tt2380307",0,354912);waitForMetadata();
  assert(postplayMockRequests()==0&&extras_n_relacionados()==0);
  // Same title previously loaded without recommendations. Enabling post-play
  // invalidates that request policy and must fetch/parse the real metadata path.
  setting("postPlayRecommendationsLocal",0);
  extras_pedir("tt2380307",0,354912);waitForMetadata();
  assert(postplayMockRequests()==1&&extras_n_relacionados()==1);
  assert(!strcmp(extras_relacionado_titulo(0),"Mock related film"));
  extras_pedir("tt2380307",0,354912);waitForMetadata();
  assert(postplayMockRequests()==1); // no duplicate requests
  // Provider master Off stays authoritative; post-play never bypasses it.
  setting("tmdb_enabled",1);
  extras_pedir("tt2380307",0,354912);waitForMetadata();
  assert(postplayMockRequests()==1&&extras_n_relacionados()==0);
  puts("PASS: real movie metadata request + parsed related list, independent post-play toggle, same-title refresh, idempotence and TMDB master Off (isolated HTTP)");
  return 0;
}

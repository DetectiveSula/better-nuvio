/* Compile the production resize functions in isolation and compare the fast
 * path byte-for-byte with the existing fractional area path, including alpha. */
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define nv_superficie SDL_CreateRGBSurfaceWithFormat
#include "resize.inc"
#include "reference.inc"

static unsigned randomState=0x35af3921;
static unsigned nextByte(void){randomState=randomState*1664525u+1013904223u;return randomState>>24;}
static void fill(SDL_Surface *s,int alpha){
  for(int y=0;y<s->h;y++)for(int x=0;x<s->w;x++){
    unsigned char *p=(unsigned char *)s->pixels+(size_t)y*s->pitch+(size_t)x*4;
    p[0]=(unsigned char)nextByte();p[1]=(unsigned char)nextByte();p[2]=(unsigned char)nextByte();
    p[3]=alpha==0?255:alpha==1?(unsigned char)nextByte():0;
  }
}
static void equal(SDL_Surface *a,SDL_Surface *b){
  assert(a&&b&&a->w==b->w&&a->h==b->h);
  for(int y=0;y<a->h;y++)assert(!memcmp((char *)a->pixels+(size_t)y*a->pitch,(char *)b->pixels+(size_t)y*b->pitch,(size_t)a->w*4));
}
int main(void){
  const int factors[][2]={{3,3},{4,4},{6,6},{8,8},{16,16},{3,1},{1,5},{3,5},{2,3}};
  for(size_t i=0;i<sizeof factors/sizeof *factors;i++)for(int alpha=0;alpha<3;alpha++){
    int w=17*factors[i][0],h=11*factors[i][1],pitch=(w+7)*4;
    void *memory=calloc((size_t)pitch,(size_t)h);assert(memory);
    SDL_Surface *src=SDL_CreateRGBSurfaceWithFormatFrom(memory,w,h,32,pitch,SDL_PIXELFORMAT_ABGR8888);assert(src);fill(src,alpha);
    SDL_Surface *a=tex_reduzir(src,17,11),*b=referenceResize(src,17,11);equal(a,b);
    SDL_FreeSurface(a);SDL_FreeSurface(b);SDL_FreeSurface(src);free(memory);
  }
  /* Fractional ratios, >16x reductions and small bilinear requests retain
   * their established paths; indexed surfaces retain palette conversion. */
  SDL_Surface *src=SDL_CreateRGBSurfaceWithFormat(0,96,72,32,SDL_PIXELFORMAT_ABGR8888);assert(src);fill(src,1);
  const int sizes[][2]={{31,23},{48,36},{3,2},{120,90}};
  for(size_t i=0;i<sizeof sizes/sizeof *sizes;i++){
    SDL_Surface *a=tex_reduzir(src,sizes[i][0],sizes[i][1]),*b=referenceResize(src,sizes[i][0],sizes[i][1]);equal(a,b);SDL_FreeSurface(a);SDL_FreeSurface(b);
  }
  SDL_FreeSurface(src);
  src=SDL_CreateRGBSurfaceWithFormat(0,3840,2160,32,SDL_PIXELFORMAT_ABGR8888);assert(src);fill(src,1);
  Uint64 t=SDL_GetPerformanceCounter();SDL_Surface *old=referenceResize(src,1280,720);
  double before=(SDL_GetPerformanceCounter()-t)*1000.0/SDL_GetPerformanceFrequency();
  t=SDL_GetPerformanceCounter();SDL_Surface *fast=tex_reduzir(src,1280,720);
  double after=(SDL_GetPerformanceCounter()-t)*1000.0/SDL_GetPerformanceFrequency();equal(old,fast);
  printf("PASS: exact RGBA match for opaque, mixed-alpha, transparent and padded images; fractional and fallback sizes unchanged\n");
  printf("4K to 1280x720 on this host: area %.2f ms, integer %.2f ms (%.2fx)\n",before,after,before/after);
  SDL_FreeSurface(old);SDL_FreeSurface(fast);SDL_FreeSurface(src);return 0;
}

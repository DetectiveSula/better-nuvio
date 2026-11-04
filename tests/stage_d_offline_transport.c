// Keep Native network utility APIs but disable dynamic HTTP transport entirely.
// Stage D supplies three strict mocked provider entry points in its fixture.
#define rede_baixar_st stageDUnusedGet
#define rede_postar_st stageDUnusedPost
#define rede_colocar_st stageDUnusedPut
#define dlopen stageDNoDlopen
#include "../src/rede.c"
#undef dlopen
void *stageDNoDlopen(const char *path,int mode) {(void)path;(void)mode;return NULL;}

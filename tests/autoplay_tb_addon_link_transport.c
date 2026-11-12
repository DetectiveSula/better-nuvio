// Real rede.c utilities with every HTTP entry point the autoplay test touches
// renamed out of the way; tests/autoplay_tb_addon_link.c supplies strict mocks.
#define rede_baixar_st tbLinkUnusedGet
#define rede_postar_st tbLinkUnusedPost
#define rede_colocar_st tbLinkUnusedPut
#define rede_url_final_tipo tbLinkUnusedFinal
#define dlopen tbLinkNoDlopen
#include "../src/rede.c"
#undef dlopen
void *tbLinkNoDlopen(const char *path,int mode) {(void)path;(void)mode;return NULL;}

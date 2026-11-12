// No dynamic network transport in local playback/presence tests.
#define dlopen posplayTestNoDlopen
#include "../src/rede.c"
#undef dlopen
void *posplayTestNoDlopen(const char *path,int mode) {(void)path;(void)mode;return NULL;}

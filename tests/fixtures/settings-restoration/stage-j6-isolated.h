/* The isolated Stage I unit fixture has only its original 32 Native IDs. The
 * complete app always includes ajustes_official.inc immediately before this. */
#ifndef AJ6_NATIVE_BRIDGE
static void aj6Context(void){}
static void aj6Edit(int p,const int *a,const int *b){(void)p;(void)a;(void)b;}
static void aj6Forget(void){}
static void aj6Observe(void){}
static int aj6Pending(void){return 0;}
static int aj6Apply(const char *j){(void)j;return 0;}
static int aj6Merge(const char *j,char **out){(void)j;*out=NULL;return 0;}
static void aj6Ack(const char *j){(void)j;}
#endif

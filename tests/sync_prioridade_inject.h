/* Transport/thread-creation control only; actual sync.c remains unchanged. */
#include <pthread.h>
int sync_teste_criar(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
#define pthread_create sync_teste_criar
/* Tell the cached-row republish at sync start (restaurarOrdemLocal, main
 * thread, per-profile guarded) apart from structure applied after the network. */
#include "colfileiras.h"   /* declare the real prototype before the macro */
void sync_teste_colfileiras(const char *caller);
#define colfileiras_sincronizar() sync_teste_colfileiras(__func__)

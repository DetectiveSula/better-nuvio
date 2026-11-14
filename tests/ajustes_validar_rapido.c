// 14 Nov 2026, Samsung S90C: every account sync froze the TV for 5.3 s in
// ajustes_compartilhados_validar. Its duplicate-key check re-read AND
// re-validated every earlier sibling for each new key, recursively, so the
// cost snowballed with the size of the settings document. A large, nested
// document must validate in one pass; duplicates (also escaped) still fail.
#include "ajustes.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double agoraMs(void) {
  struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

int main(void) {
  size_t cap = 4u * 1024u * 1024u, n = 0;
  char *big = malloc(cap);
  double t0, ms;
  assert(big);
  assert(ajustes_compartilhados_validar("{\"version\":1,\"a\":{\"b\":[1,{\"c\":true}]},\"d\":\"x\"}"));
  assert(!ajustes_compartilhados_validar("{\"version\":1,\"a\":1,\"a\":2}"));
  assert(!ajustes_compartilhados_validar("{\"version\":1,\"a\":1,\"\\u0061\":2}"));
  assert(!ajustes_compartilhados_validar("{\"version\":1,\"x\":{\"k\":1,\"k\":2}}"));
  assert(!ajustes_compartilhados_validar("{\"version\":1,\"x\":{\"k\":1,}}"));
  n += (size_t)snprintf(big + n, cap - n, "{\"version\":1,\"features\":{");
  for (int g = 0; g < 40; g++) {
    n += (size_t)snprintf(big + n, cap - n, "%s\"group%d\":{", g ? "," : "", g);
    for (int k = 0; k < 40; k++)
      n += (size_t)snprintf(big + n, cap - n,
        "%s\"setting%d\":{\"type\":\"object\",\"value\":{\"a\":{\"b\":[1,2,{\"c\":true,\"e\":{\"f\":null}}]},\"d\":\"x\"}}",
        k ? "," : "", k);
    n += (size_t)snprintf(big + n, cap - n, "}");
  }
  snprintf(big + n, cap - n, "}}");
  t0 = agoraMs();
  assert(ajustes_compartilhados_validar(big));
  ms = agoraMs() - t0;
  printf("ajustes_validar_rapido: %zu bytes in %.1f ms\n", n, ms);
  assert(ms < 250.0);   /* old code: 18.9 s on the Mac for this document */
  free(big);
  puts("ajustes_validar_rapido: PASS");
  return 0;
}

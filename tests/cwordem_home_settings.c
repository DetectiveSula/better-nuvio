// Fixture initial state, using the actual native settings getters/defaults.
// Home ordering must not pull in account loading, credentials or GPU initialisation.
#include "../src/ajustes.c"
void test_cw_enabled(int enabled) { valor[AJ_CW_LIGADO] = enabled ? 0 : 1; }
void test_cw_order(int order) { valor[AJ_CW_ORDEM] = order; }

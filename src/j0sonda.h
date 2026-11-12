// Stage J0: one-off probe of a separate per-profile `better_nuvio` namespace in
// the account's profile-settings RPCs (docs/settings-restoration/STAGE-J.md,
// section 8.2). Android DEBUG builds only (NV_ANDROID without NDEBUG); every
// other build compiles the stubs below and the deep links are refused.
//
//   nuvio-legacy://debug/j0-probe                     read-only + baseline, issues a token
//   nuvio-legacy://debug/j0-probe/write?token=XXXXXXXX write, read back, isolation check
//   nuvio-legacy://debug/j0-probe/clean?token=XXXXXXXX overwrite the probe with {}
//
// Logs and notices carry HTTP status, sizes, 64-bit fingerprints and the token,
// never document contents, owner ids or credentials.
#ifndef NV_J0SONDA_H
#define NV_J0SONDA_H

#if (defined(NV_ANDROID) && !defined(NDEBUG)) || defined(NV_J0_TESTE)
#define NV_J0_SONDA 1
#endif

enum { J0_LER = 1, J0_GRAVAR, J0_LIMPAR };

// Main loop: start the requested phase on its own thread (one at a time).
void j0sonda_pedir(int fase, const char *token);
// Main loop, every frame: shows a finished phase's summary once.
void j0sonda_passo(void);
// Last summary (tests); "" while none.
const char *j0sonda_resultado(void);
#endif

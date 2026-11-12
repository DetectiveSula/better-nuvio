// Stage J7a: debug-only arming of J4/J5 (better_nuvio) and J6 (official "tv")
// account uploads for the enrolled dedicated test account (STAGE-J.md 15).
//
//   nuvio-legacy://debug/j7/arm                     shows account fingerprint + J4/J5 code
//   nuvio-legacy://debug/j7/arm?token=XXXXXXXX      enrols this account, arms J4/J5 (better_nuvio) only
//   nuvio-legacy://debug/j7/arm-j6                  separate code for J6 official "tv" writes
//   nuvio-legacy://debug/j7/arm-j6?token=XXXXXXXX   arms J6 for this profile (never implied by arm)
//   nuvio-legacy://debug/j7/disarm                  clears enrolment and arming
//   nuvio-legacy://debug/j7/snapshot                read-only fingerprints + leak counts
//
// Arming is per profile, at most 4 hours (wall clock, enforced after restarts),
// only for the enrolled account (stored as a hash, never the id) and is cleared
// as soon as another account or no account is active. Stage I's 32 automatic
// uploads are an explicit, unchanged exception. Logs and notices carry names,
// counts, HTTP codes and 64-bit fingerprints only.
#ifndef NV_J7ARME_H
#define NV_J7ARME_H

#if (defined(NV_ANDROID) && !defined(NDEBUG)) || defined(NV_J7_TESTE)
#define NV_J7_ARME 1
#endif

#define J7_JANELA_S (4 * 60 * 60)

enum { J7_CODIGO = 1, J7_ARMAR, J7_DESARMAR, J7_FOTO, J7_CODIGO_J6, J7_ARMAR_J6 };
enum { J7_J4 = 1, J7_J6 = 2 };   // independent permission scopes

// Worker or main thread. Final gate in front of every J4/J5 and J6 upload:
// Android release builds always 0; Android debug builds only for the enrolled,
// armed, unexpired owner/profile; other platforms keep their explicit permissions.
// Main loop: j7arme_pedir handles a debug deep link (DL_J7); j7arme_passo runs
// every frame (account change, expiry, notices); j7arme_resultado is the last
// summary line ("" while none).
#ifdef NV_J7_ARME
int j7arme_permite_j4(const char *owner, int profile);
int j7arme_permite_j6(const char *owner, int profile);
#ifndef NV_J7_IMPL
// Fail-closed fallback for a unit that sees the debug declaration without
// src/j7arme.c being linked (a test that sets NV_ANDROID for one file): refuse.
// The real definition in src/j7arme.c (always in the Android build, which globs
// src/*.c) overrides it.
__attribute__((weak)) int j7arme_permite_j4(const char *owner, int profile) {
  (void)owner; (void)profile;
  return 0;
}
__attribute__((weak)) int j7arme_permite_j6(const char *owner, int profile) {
  (void)owner; (void)profile;
  return 0;
}
#endif
void j7arme_pedir(int fase, const char *token);
void j7arme_passo(void);
const char *j7arme_resultado(void);
#else
// Inline so that builds without arming need no extra source file and Android
// release contains no arming code at all.
static inline int j7arme_permite_j4(const char *owner, int profile) {
  (void)owner; (void)profile;
#ifdef NV_ANDROID
  return 0;   // release Android: J4/J5/J6 can never upload, whatever flags are stored
#else
  return 1;   // other platforms: the existing explicit permissions decide
#endif
}
static inline int j7arme_permite_j6(const char *owner, int profile) { return j7arme_permite_j4(owner, profile); }
static inline void j7arme_pedir(int fase, const char *token) { (void)fase; (void)token; }
static inline void j7arme_passo(void) {}
static inline const char *j7arme_resultado(void) { return ""; }
#endif

#ifdef NV_J7_TESTE
void j7arme_teste_avancar(long long segundos);   // moves the wall clock
void j7arme_teste_foto(void);                    // runs the snapshot synchronously
#endif
#endif

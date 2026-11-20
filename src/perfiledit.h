// EDITAR PERFIL fora do Android (owner, 18 Nov 2026: "an option on the profile
// page to make changes to profile"). The Android app has a native panel for
// this (ProfileEditor.kt); everywhere else there was none. This is the same
// set of changes, drawn by the app and saved through the same account calls:
//
//   name + photo   sync_patch_profile (sparse, like the Android panel)
//   colour/background
//                  sync_pull_profiles, then sync_push_profiles with every other
//                  profile copied back unchanged (official Nuvio's way: those
//                  two fields have no sparse call)
//   PIN            set_profile_pin / clear_profile_pin, at once, with the
//                  current PIN when the profile already has one
//   catalogues     get_avatar_catalog (public images),
//                  get_member_profile_background_catalog (supporters only; an
//                  error simply means "no supporter backgrounds")
//
// Network work runs on one worker thread; results are applied on the app
// thread through perfis_aplicar_* under the profile-session lock, so a profile
// switch in the middle never writes into the wrong list. Opened from the
// profile picker (perfilsel.c), which hosts its keys, updates and drawing.
#ifndef NV_PERFILEDIT_H
#define NV_PERFILEDIT_H
#include <SDL2/SDL.h>
#include <stddef.h>

// Opens the editor for profile `indice` (profile_index). A profile with a PIN
// needs that PIN, already checked by the caller (the picker's PIN pad); it
// also stands in for "current PIN" when the PIN is changed. 0 = refused.
int  perfiledit_abrir_perfil(int indice, const char *pinVerificado);
int  perfiledit_aberto(void);
void perfiledit_fechar(void);
void perfiledit_evento(const SDL_Event *e);
void perfiledit_atualizar(float dt, Uint32 agora);
void perfiledit_desenhar(Uint32 agora);
// 1 once after a save that changed the profile (the host refreshes itself).
int  perfiledit_mudou(void);
// 1 when profiles can be edited: an account session.
int  perfiledit_disponivel(void);
#if defined(NV_SOCIALVIS_DEMO) || defined(NV_PERFILSEL_TEST)
// Screenshots only (tests/perfilsel.sh --capturas): a made-up profile and catalogue,
// no session and no network.
void perfiledit_demo(int liga);
// Opens straight on a view: 0 options, 1 editor, 2 remove-PIN; tab, zone, item.
void perfiledit_demo_estado(int vista, int aba, int zona, int item);
#endif

// --- pure rules (tests/perfiledit.c) -------------------------------------
#define PE_NOME_MAX_CHARS 20
#define PE_NOME_MAX_BYTES 63
#define PE_URL_MAX       300
#define PE_ID_MAX         64
#define PE_CORES           8
extern const char *const PE_CORES_HEX[PE_CORES];   // official Nuvio TV's profile colours

int perfiledit_nome_valido(const char *s);          // trimmed, 1..20 code points, <= 63 bytes, no control chars
void perfiledit_nome_aparar(const char *in, char *out, size_t n);   // trim + clamp to the limits
int perfiledit_url_valida(const char *s);           // "" or http(s) without spaces, < 300 bytes
int perfiledit_cor_normal(const char *in, char out[8]);   // "#RRGGBB" from RRGGBB/#RRGGBB/#AARRGGBB
int perfiledit_pin_valido(const char *s);           // exactly 4 digits

typedef struct { char id[PE_ID_MAX]; char nome[64]; char url[PE_URL_MAX]; char cor[8]; } PEAvatar;
typedef struct { char id[PE_ID_MAX]; char nome[64]; } PEFundo;
// Catalogue rows -> entries (max 64 read). `base` is the account server URL.
int perfiledit_ler_avatares(const char *json, const char *base, PEAvatar *out, int max);
int perfiledit_ler_fundos(const char *json, PEFundo *out, int max);

// Request bodies (malloc'd JSON; NULL = refused). `foto` mode: 0 unchanged,
// 1 catalogue avatar (`avatarId`), 2 no photo, 3 image link (`fotoUrl`,
// http(s); sent as avatar_url with no avatar_id, like official Nuvio).
char *perfiledit_corpo_patch(int perfil, const char *nome, int foto, const char *avatarId, const char *fotoUrl);
// Pull rows -> push body. `cor` NULL keeps it; `fundo` mode: 0 unchanged,
// 1 standard (no background), 2 supporter background `fundoId`.
char *perfiledit_corpo_push(const char *pull, int perfil, const char *nome, int foto,
                            const char *avatarId, const char *fotoUrl, const char *cor, int fundo,
                            const char *fundoId, const char *cliente);
#endif

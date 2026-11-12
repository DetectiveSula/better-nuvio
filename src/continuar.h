#ifndef NV_CONTINUAR_H
#define NV_CONTINUAR_H
#include "catalogo.h"
#include "gfx.h"
#include "badges.h"

// Conteudo sobre a capa; a home continua responsavel por imagem e foco.
// `raio`: o MESMO raio da arte do cartao (fracao da altura, raioDe em home.c).
void continuar_desenhar(const CatItem *item, GfxRect card, float raio);
// Stage E (regular Nuvio's ContinueWatchingCard): the top-right badge text and
// style for an item, and the episode-title line. `curto` = Wide/Poster labels.
// Empty when the item has no badge. Shared by the card, the poster-style row
// and the Home hero so they never disagree.
void continuar_selo(const CatItem *item, int curto, long long agoraMs,
                    char *dst, size_t cap, BadgeEstilo *estilo);
// A next-up (official ContinueWatchingItem.NextUp), aired or not.
int  continuar_e_a_seguir(const CatItem *item);
// A next-up that has not aired yet (official `nextUp && !hasAired`).
int  continuar_proximo_inedito(const CatItem *item, long long agoraMs);
void continuar_titulo_episodio(const CatItem *item, long long agoraMs, char *dst, size_t cap);
// O veu da base que continuar_desenhar pede (gfx_veu_base): a home o poe
// dentro da arte (gfx_veu_card_por) antes de desenha-la.
#define NV_CW_VEU_F 0.66f
#define NV_CW_VEU_A 0.88f
#endif

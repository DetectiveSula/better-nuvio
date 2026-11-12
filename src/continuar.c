#include "continuar.h"
#include "ajustes.h"
#include "idioma.h"
#include "layout.h"
#include "recomenda.h"
#include "badges.h"
#include "text.h"
#include "anim.h"
#include "revela.h"
#include "proximo.h"
#include "trakt.h"
#include "simkl.h"
#include "cwordem.h"
#include "descoberta.h"
#include "epdata.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// The raw release of a next-up and whether it has aired. Official
// hasEpisodeAired(released, fallback = true): unknown counts as aired.
static int jaSaiu(const char *id, long long agoraMs, char *cru, size_t cap) {
  long long ms;
  int r;
  if (cwo_estreia_cru(id, cru, cap) && (r = ep_ja_saiu(cru, agoraMs)) >= 0) return r;
  cru[0] = 0;
  ms = cwo_estreia(id);
  return ms == CWO_SEM_DATA || ms <= agoraMs;
}
static int ehASeguir(const CatItem *ci) {
  return ci->progresso == 0 &&
         (trakt_e_a_seguir(ci->imdb) || simkl_e_a_seguir(ci->imdb) || cwo_conta_a_seguir(ci->imdb));
}

int continuar_e_a_seguir(const CatItem *ci) { return ci && ehASeguir(ci); }

int continuar_proximo_inedito(const CatItem *ci, long long agoraMs) {
  char cru[48];
  return ci && ehASeguir(ci) && !jaSaiu(ci->imdb, agoraMs, cru, sizeof cru);
}

void continuar_selo(const CatItem *ci, int curto, long long agoraMs,
                    char *dst, size_t cap, BadgeEstilo *estilo) {
  char cru[48];
  int nova = 0;
  if (!dst || !cap) return;
  dst[0] = 0;
  if (estilo) *estilo = BADGE_NEUTRO;
  if (!ci) return;
  // In progress (official ContinueWatchingItem.InProgress): the progress label.
  // A next-up has progresso 0 but may still carry the episode runtime in
  // restanteMin; it is a NextUp and never shows "left" (official: separate
  // item types). Anything else with a known remaining time keeps the label.
  if (ci->progresso > 0) {
    ep_rotulo_progresso(ci->restanteMin, ci->progresso, dst, cap);
    return;
  }
  if (!ehASeguir(ci)) {
    if (ci->restanteMin > 0) ep_rotulo_progresso(ci->restanteMin, ci->progresso, dst, cap);
    return;
  }
  if (cwo_alerta(ci->imdb, ci->temporada, agoraMs, &nova)) {
    if (nova) { snprintf(dst, cap, "%s", i18n("Nova temporada")); if (estilo) *estilo = BADGE_NOVA_TEMPORADA; }
    else { snprintf(dst, cap, "%s", i18n(curto ? "Novo" : "Novo episódio")); if (estilo) *estilo = BADGE_NOVO_EPISODIO; }
    return;
  }
  if (!jaSaiu(ci->imdb, agoraMs, cru, sizeof cru)) {
    if (!cru[0] || !ep_rotulo_estreia(cru, agoraMs, curto, ajustes_idioma(), dst, cap))
      snprintf(dst, cap, "%s", i18n("A estrear"));
    return;
  }
  snprintf(dst, cap, "%s", i18n(curto ? "Próximo" : "Próximo a assistir"));
}

void continuar_titulo_episodio(const CatItem *ci, long long agoraMs, char *dst, size_t cap) {
  char cru[48];
  if (!dst || !cap) return;
  dst[0] = 0;
  if (!ci) return;
  if (ci->nomeEpisodio[0]) { snprintf(dst, cap, "%s", ci->nomeEpisodio); return; }
  if (ehASeguir(ci) && !jaSaiu(ci->imdb, agoraMs, cru, sizeof cru) && cru[0])
    ep_rotulo_estreia(cru, agoraMs, 0, ajustes_idioma(), dst, cap);
}

// Cards carry the actual playable episode. Completed seeds are resolved by
// discovery before publication, never relabelled only while drawing.
void continuar_desenhar(const CatItem *ci, GfxRect r, float raio) {
  if (!ci) return;
  float esc = r.w / NV_DESTAQUE_W;
  float pad = NV_CW_PAD * esc, largura = r.w - pad * 2;
  // Veu so na base (gfx.h, gfx_veu_base): o nome e o episodio ficam embaixo;
  // o selo de cima tem o proprio fundo.
  //
  // O RAIO E O DA ARTE (#144). Era NV_RAIO_CARD fixo (~13 px neste cartao),
  // enquanto a arte usa o raio de Ajustes: com raio maior os cantos do veu
  // passavam por fora da curva da arte e apareciam como cantos escuros e
  // "cortados" na base do cartao.
  gfx_veu_base(r, raio, NV_CW_VEU_F, NV_CW_VEU_A);

  // STAGE E — THE BADGE OF REGULAR NUVIO (ContinueWatchingCard): in progress,
  // the remaining-time label; a next-up, "New Season" / "New Episode" when the
  // release alert holds, the countdown while it has not aired ("Upcoming"
  // without a usable date), otherwise "Next Up". Wide and Poster styles use
  // the short forms. Nunca inventar status de estreia: sem data registrada o
  // episodio conta como exibido, como no oficial (hasEpisodeAired fallback).
  char selo[64];
  BadgeEstilo estilo = BADGE_NEUTRO;
  continuar_selo(ci, ajustes_cw_estilo() != 0, (long long)time(NULL) * 1000LL,
                 selo, sizeof selo, &estilo);
  if (selo[0]) {
    float bw = badge_largura(selo);
    if (bw <= largura)
      badge_desenhar(r.x + r.w - pad - bw, r.y + pad, selo, estilo, 1.0f);
  }

  // O selo do IMDb, no CANTO INFERIOR DIREITO (issue #87): o card ja tinha a
  // nota no CatItem — quem nao a lia era o enfeite do Trakt — e a fileira e o
  // unico lugar da home onde ela nao aparecia. O desenho e o mesmo selo da
  // aba Salvos (rec_selo_imdb), pela base da linha do titulo e com o mesmo
  // `pad` lateral do badge de "restam". A largura dele e medida ANTES do
  // texto e sai da pista das linhas, que encurtam em vez de passar por baixo.
  float seloLarg = 0.0f;
  if (ci->nota > 0 && ajustes_notas_home())
    seloLarg = badge_imdb_largura(ci->nota);
  // Quando o selo existe as linhas de texto perdem seloLarg + uma folga de
  // 16*esc — e o preco ja sai na largura, porque rec_selo_imdb ancora pela
  // esquerda e so devolve a medida depois de desenhar.
  float livre = largura - (seloLarg > 0 ? seloLarg + 16.0f * esc : 0.0f);
  if (livre < 40.0f) livre = 40.0f;   // texto sempre ganha um minimo de pista

  float base = r.y + r.h - 30*esc;
  int serie = !strcmp(ci->tipo, "series") && ci->temporada > 0 && ci->episodio > 0;
  // Episode title; an unaired next-up without one shows the long air-date
  // text instead (official `episodeTitle`).
  char nomeEp[160];
  continuar_titulo_episodio(ci, (long long)time(NULL) * 1000LL, nomeEp, sizeof nomeEp);
  if (serie && nomeEp[0]) {
    TxtLinha ep = txt_linha_corta(TXT_CW_META, nomeEp, 230, 232, 238, 255, livre);
    base -= ep.h;
    txt_desenhar_alpha(ep, r.x + pad, base, 1);
    base -= 4*esc;
  }
  if (seloLarg > 0)
    badge_imdb(r.x + r.w - pad - seloLarg, base - BADGE_H, ci->nota, 0, 1.0f);
  TxtLinha titulo = txt_linha_corta(TXT_CW_TITULO, ci->titulo, 247, 248, 250, 255, livre);
  base -= titulo.h;
  txt_desenhar_alpha(titulo, r.x + pad, base, 1);
  if (serie) {
    // Stage E: the official `season_episode_format` ("S1 E2"), above the title.
    char te[32];
    snprintf(te, sizeof te, i18n("T%d E%d"), ci->temporada, ci->episodio);
    TxtLinha ep = txt_linha(TXT_CW_META, te, 230, 232, 238, 255);
    txt_desenhar_alpha(ep, r.x + pad, base - ep.h - 4*esc, 1);
  }

  // Linha fina, recuada da moldura. A parte vazia nao vira uma faixa cinza.
  // A barra ANDA do valor que mostrava ate o novo (revela.h): na volta do
  // player o card cresce ate onde a pessoa parou, em vez de saltar.
  // O zero tambem e registrado: o "a seguir" que comeca a ser visto cresce
  // do nada ate o valor novo.
  float prog = revela_progresso(ci->imdb, ci->temporada, ci->episodio,
                                (float)ci->progresso, SDL_GetTicks());
  if (ci->progresso > 0) {
    float h = NV_CW_BAR_H * esc;
    float preenchido = largura * anim_clamp(prog / 100.f, 0, 1);
    if (preenchido < h) preenchido = h;
    GfxRect barra = {r.x + pad, r.y + r.h - NV_CW_BAR_BOTTOM*esc - h, preenchido, h};
    gfx_cor(barra, .5f, .96f, .965f, .98f, .98f);
  }
}

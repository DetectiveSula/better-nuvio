// STAGE H — FAMILY WATCHING (replaces the Home "Friends Watching" row).
//
// One card per OTHER profile of the signed-in Nuvio account, with that
// profile's most recent movie or episode: in progress (bar + time left) or
// recently marked watched. Selecting a card opens the title's details under the
// CURRENT profile: nothing is copied, no profile switch (home.c treats the row
// as an ordinary details row).
//
// PRIVACY RULES (owner decisions, 6 Nov 2026), all in familia_perfil_elegivel /
// familia_linha_visivel and covered by tests/familia.c:
//   - the row is Off by default (AJ_FAMILIA, per profile, portable Better key);
//   - a profile appears only if it opted in (AJ_FAMILIA_PARTILHAR, Off by
//     default, portable Better key), read per profile on this device;
//   - PIN-locked profiles never appear;
//   - child profiles (perfilinfantil.h) never appear, and the whole row is hidden
//     while the ACTIVE profile is a child profile.
//
// LIVE READS (familia_rede.c): any signed-in account, read-only. The data comes through a FamBuscar
// callback (familia_definir_busca). The intended transport is the existing
// per-profile account RPCs (sync_pull_watch_progress with p_profile_id/p_limit
// and the first page of sync_pull_watched_items), whose bodies the parsers below
// already read; registering it changes live synchronisation behaviour and waits
// for the owner's approval (NEXT-STAGES-REVIEW.md, "Family Watching"). With no
// callback the row simply has no cards and is not shown. Logs carry counts only.
#ifndef NV_FAMILIA_H
#define NV_FAMILIA_H
#include <stddef.h>

#define FAM_MAX        8      // cards (other profiles) at most
#define FAM_ATIV_MAX   24     // activity rows read per profile
#define FAM_REFAZ_MS   120000 // at most one read per profile every 2 minutes
#define FAM_VISTO_PCT  90     // progress at/above this counts as watched (as CW)

enum { FAM_EM_CURSO = 1, FAM_VISTO = 2 };

typedef struct {
  int  perfil;            // profile_index
  char nome[64];
  char avatar[300];       // avatar_url; "" = initial on the profile colour
  char cor[10];           // avatar_color_hex
  int  temPin, infantil, partilha;
} FamPerfil;

typedef struct {
  char id[64];            // content_id (base id, no :season:episode)
  char tipo[8];           // "movie" | "series"
  int  temporada, episodio;
  double posMs, durMs;    // progress rows; 0 for watched items
  long long quandoMs;     // last_watched / watched_at
  int  visto;             // 1 = a watched item
} FamAtividade;

typedef struct {
  int  perfil;
  char nome[64], avatar[300], cor[10];
  char id[64], tipo[8];
  int  temporada, episodio;
  int  pct;               // 0..100, in progress only
  int  restanteMin;       // in progress only; -1 unknown
  int  estado;            // FAM_EM_CURSO | FAM_VISTO
  long long quandoMs;
} FamCartao;

// ---- pure rules (no I/O) ----------------------------------------------------
int familia_linha_visivel(int ligado, int ativoInfantil);
int familia_perfil_elegivel(const FamPerfil *p, int ativo);
// Most recent usable activity; ties prefer in-progress. 0 = none.
int familia_escolher(const FamAtividade *a, int n, FamAtividade *out);
int familia_cartao_de(const FamPerfil *p, const FamAtividade *a, FamCartao *out);
// Bodies of sync_pull_watch_progress / sync_pull_watched_items. -1 = invalid.
int familia_ler_progresso(const char *json, FamAtividade *out, int max);
int familia_ler_vistos(const char *json, FamAtividade *out, int max);
// Pill ("In Progress" / "Recently Watched") and the detail line
// ("S2 E5 · 22m left", "Movie · Watched 3 h ago"), already translated.
void familia_rotulos(const FamCartao *c, long long agoraMs, char *pilula, size_t tp,
                     char *linha, size_t tl);

typedef int (*FamBuscar)(int perfil, FamAtividade *out, int max, void *u);
// Cards for the eligible profiles, newest first. 0 when the row is hidden.
int familia_montar(const FamPerfil *perfis, int n, int ativo, int ativoInfantil,
                   int ligado, FamBuscar buscar, void *u, FamCartao *out, int max);

// ---- app state --------------------------------------------------------------
// Live transport. None is registered in this build (see the header comment).
void familia_definir_busca(FamBuscar fn, void *u);
// familia_rede.c: the read-only transport over the existing per-profile RPCs,
// for any signed-in account (the profile consent gates still apply).
void familia_rede_registrar(void);
int  familia_rede_conta_permitida(const char *impressao);
int  familia_tem_busca(void);
// Main thread, every frame: snapshots the profile list and the settings that
// gate the row (cheap; at most once a second).
void familia_passo(void);
// Worker (Home catalogue cycle): refreshes when stale or asked, returns cards.
int  familia_atualizar(FamCartao *out, int max);
// Cache only (no read): the cards for the Home catalogue cycle.
int  familia_cartoes(FamCartao *out, int max);
// Main thread: the card drawn for a profile ("familia:<n>" slug). 0 = none.
int  familia_cartao_do_perfil(int perfil, FamCartao *out);
void familia_home_aberta(void);     // Home opened: a fresh read (throttled)
// 1 = familia_passo would start a Family-only read now (row visible, transport,
// an eligible profile, stale or asked, none running).
int  familia_quer_ler(void);
void familia_ajuste_mudou(void);    // a Family setting changed: rebuild Home
void familia_esquecer(void);        // account/profile change
unsigned familia_revisao(void);

#endif

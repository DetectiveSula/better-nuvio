// Stage E: regular Nuvio's countdown and progress labels (AirDateUtils.kt,
// formatEpisodeAirDateLabel, ContinueWatchingProgressLabel.kt). Separate from
// the pure date rules in epdata.c so subset-linked tests need no i18n.
#include "epdata.h"
#include "idioma.h"
#include "idiomacod.h"
#include <stdio.h>
#include <time.h>

static int diaDeHoje(long long agoraMs, EpDia *o) {
  time_t t = (time_t)(agoraMs / 1000LL);
  struct tm tm;
  if (!localtime_r(&t, &tm)) return 0;
  o->ano = tm.tm_year + 1900; o->mes = tm.tm_mon + 1; o->dia = tm.tm_mday;
  return 1;
}

// formatEpisodeAirDateLabel: day and short month, with the year only when it
// is not the current (local) year. Better's locale order (idioma_data_curta).
static void rotuloData(const EpDia *r, long long agoraMs, int idioma, char *dst, size_t cap) {
  EpDia h;
  int ano = (diaDeHoje(agoraMs, &h) && h.ano == r->ano) ? 0 : r->ano;
  idioma_data_curta(idioma, r->dia, idioma_mes_curto(idioma, r->mes - 1), ano, dst, cap);
}

int ep_rotulo_estreia(const char *raw, long long agoraMs, int curto, int idioma,
                      char *dst, size_t cap) {
  EpDia r;
  long dias;
  char data[48];
  if (!dst || !cap) return 0;
  dst[0] = 0;
  // `releasedIso` blank: the official label comes from that same value, so
  // there is nothing to show.
  if (ep_ja_saiu(raw, agoraMs) != 0) return 0;     // aired, or unparseable
  if (!ep_dia_local(raw, &r) || !ep_dias_ate(raw, agoraMs, &dias)) return 0;
  if (dias < 0) return 0;
  if (dias == 0) snprintf(dst, cap, "%s", i18n(curto ? "Hoje" : "Estreia hoje"));
  else if (dias == 1) snprintf(dst, cap, "%s", i18n(curto ? "Amanhã" : "Estreia amanhã"));
  else if (dias <= 7)
    snprintf(dst, cap, i18n(curto ? "Em %d dias" : "Estreia em %d dias"), (int)dias);
  else {
    rotuloData(&r, agoraMs, idioma, data, sizeof data);
    if (curto) snprintf(dst, cap, "%s", data);
    else snprintf(dst, cap, i18n("Estreia %s"), data);
  }
  return dst[0] != 0;
}

void ep_rotulo_progresso(int restanteMin, int progresso, char *dst, size_t cap) {
  if (!dst || !cap) return;
  if (restanteMin > 0) {
    int h = restanteMin / 60, m = restanteMin % 60;
    if (h > 0) snprintf(dst, cap, i18n("%dh %dm restantes"), h, m);
    else snprintf(dst, cap, i18n("%dm restantes"), m < 1 ? 1 : m);
    return;
  }
  if (progresso > 0) snprintf(dst, cap, i18n("%d%% visto"), progresso > 100 ? 100 : progresso);
  else snprintf(dst, cap, "%s", i18n("Retomar"));
}

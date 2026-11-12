// Stage E port of regular Nuvio's episode release rules (pure: no UI text).
// Labels live in eprotulo.c. See epdata.h for the source of each function.
#include "epdata.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define EP_DIA_MS (24LL * 60LL * 60LL * 1000LL)

typedef struct {
  int tipo;               // 0 none, 1 plain date, 2 zoned instant, 3 local date-time
  int ano, mes, dia, h, m, s;
  long ms;                // fraction of a second, in milliseconds
  int offMin;             // zone offset east of UTC, minutes (tipo 2)
} EpCru;

static int dig(const char *s, int n, int *v) {
  int r = 0;
  for (int i = 0; i < n; i++) {
    if (!isdigit((unsigned char)s[i])) return 0;
    r = r * 10 + (s[i] - '0');
  }
  *v = r;
  return 1;
}
static int bissexto(int a) { return (a % 4 == 0 && a % 100 != 0) || a % 400 == 0; }
static int diaValido(int a, int m, int d) {
  static const int dm[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (a < 1 || m < 1 || m > 12 || d < 1) return 0;
  return d <= dm[m - 1] + (m == 2 && bissexto(a));
}
// Days since 1970-01-01 of a proleptic Gregorian date (H. Hinnant).
static long long diasCivis(int a, int m, int d) {
  a -= m <= 2;
  long long era = (a >= 0 ? a : a - 399) / 400;
  unsigned yoe = (unsigned)(a - era * 400);
  unsigned doy = (153u * (unsigned)(m + (m > 2 ? -3 : 9)) + 2u) / 5u + (unsigned)d - 1u;
  unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
  return era * 146097LL + (long long)doe - 719468LL;
}
// "YYYY-MM-DD" at s; returns chars consumed (10) or 0.
static int lerData(const char *s, int *a, int *m, int *d) {
  if (!dig(s, 4, a) || s[4] != '-' || !dig(s + 5, 2, m) || s[7] != '-' || !dig(s + 8, 2, d)) return 0;
  return diaValido(*a, *m, *d) ? 10 : 0;
}

// Trimmed copy, as `raw?.trim()`.
static int aparar(const char *raw, char *dst, size_t cap) {
  size_t n;
  if (!raw) return 0;
  while (*raw && isspace((unsigned char)*raw)) raw++;
  n = strlen(raw);
  while (n && isspace((unsigned char)raw[n - 1])) n--;
  if (!n || n >= cap) return 0;
  memcpy(dst, raw, n);
  dst[n] = 0;
  return 1;
}

// The whole value as one ISO form: LocalDate, Instant/OffsetDateTime/
// ZonedDateTime, or LocalDateTime. Anything left over means "not this form".
static int lerInteiro(const char *v, EpCru *c) {
  const char *p = v;
  memset(c, 0, sizeof *c);
  if (!lerData(p, &c->ano, &c->mes, &c->dia)) return 0;
  p += 10;
  if (!*p) { c->tipo = 1; return 1; }
  if (*p != 'T') return 0;
  p++;
  if (!dig(p, 2, &c->h) || p[2] != ':' || !dig(p + 3, 2, &c->m)) return 0;
  p += 5;
  if (*p == ':') {
    if (!dig(p + 1, 2, &c->s)) return 0;
    p += 3;
    if (*p == '.') {
      int nd = 0; long f = 0;
      p++;
      while (isdigit((unsigned char)*p) && nd < 9) { if (nd < 3) f = f * 10 + (*p - '0'); nd++; p++; }
      if (!nd) return 0;
      while (nd < 3) { f *= 10; nd++; }
      c->ms = f;
    }
  }
  if (c->h > 23 || c->m > 59 || c->s > 59) return 0;
  if (!*p) { c->tipo = 3; return 1; }
  if (*p == 'Z') { p++; c->offMin = 0; }
  else if (*p == '+' || *p == '-') {
    int sinal = *p == '-' ? -1 : 1, oh, om = 0;
    if (!dig(p + 1, 2, &oh)) return 0;
    p += 3;
    if (*p == ':') { if (!dig(p + 1, 2, &om)) return 0; p += 3; }
    if (oh > 18 || om > 59) return 0;
    c->offMin = sinal * (oh * 60 + om);
  } else return 0;
  // ZonedDateTime may carry a region id; the offset already fixes the instant.
  if (*p == '[') { const char *f = strchr(p, ']'); if (!f || f[1]) return 0; p = f + 1; }
  if (*p) return 0;
  c->tipo = 2;
  return 1;
}

// parseEmbeddedReleaseDate: the FIRST YYYY-MM-DD not touching other digits;
// an invalid first match is not retried (Regex.find then LocalDate.parse).
static int lerEmbutida(const char *v, int *a, int *m, int *d) {
  for (const char *p = v; *p; p++) {
    int y, mo, da;
    if (p > v && isdigit((unsigned char)p[-1])) continue;
    if (!dig(p, 4, &y) || p[4] != '-' || !dig(p + 5, 2, &mo) || p[7] != '-' || !dig(p + 8, 2, &da)) continue;
    if (isdigit((unsigned char)p[10])) continue;
    if (!diaValido(y, mo, da)) return 0;
    *a = y; *m = mo; *d = da;
    return 1;
  }
  return 0;
}

static long long instanteZonado(const EpCru *c) {
  long long seg = diasCivis(c->ano, c->mes, c->dia) * 86400LL + c->h * 3600LL + c->m * 60LL + c->s;
  return (seg - c->offMin * 60LL) * 1000LL + c->ms;
}
static int diaDoInstante(long long ms, EpDia *o) {
  time_t t = (time_t)(ms >= 0 ? ms / 1000LL : (ms - 999) / 1000LL);
  struct tm tm;
  if (!localtime_r(&t, &tm)) return 0;
  o->ano = tm.tm_year + 1900; o->mes = tm.tm_mon + 1; o->dia = tm.tm_mday;
  return 1;
}

int ep_dia_local(const char *raw, EpDia *out) {
  char v[96];
  EpCru c;
  int a, m, d;
  if (!out || !aparar(raw, v, sizeof v)) return 0;
  if (lerInteiro(v, &c)) {
    if (c.tipo == 2) return diaDoInstante(instanteZonado(&c), out);
    out->ano = c.ano; out->mes = c.mes; out->dia = c.dia;   // plain date or local date-time
    return 1;
  }
  if (!lerEmbutida(v, &a, &m, &d)) return 0;
  out->ano = a; out->mes = m; out->dia = d;
  return 1;
}

int ep_instante_ms(const char *raw, long long *ms) {
  char v[96];
  EpCru c;
  int a, m, d;
  if (!ms || !aparar(raw, v, sizeof v)) return 0;
  if (lerInteiro(v, &c)) {
    if (c.tipo == 2) { *ms = instanteZonado(&c); return 1; }
    if (c.tipo == 3) {
      struct tm tm;
      time_t t;
      memset(&tm, 0, sizeof tm);
      tm.tm_year = c.ano - 1900; tm.tm_mon = c.mes - 1; tm.tm_mday = c.dia;
      tm.tm_hour = c.h; tm.tm_min = c.m; tm.tm_sec = c.s; tm.tm_isdst = -1;
      t = mktime(&tm);
      if (t == (time_t)-1) return 0;
      *ms = (long long)t * 1000LL + c.ms;
      return 1;
    }
    *ms = diasCivis(c.ano, c.mes, c.dia) * EP_DIA_MS;   // plain date: UTC midnight
    return 1;
  }
  if (!lerEmbutida(v, &a, &m, &d)) return 0;
  *ms = diasCivis(a, m, d) * EP_DIA_MS;
  return 1;
}

int ep_ja_saiu(const char *raw, long long agoraMs) {
  long long ms;
  if (!ep_instante_ms(raw, &ms)) return -1;
  return ms <= agoraMs;
}

int ep_dias_ate(const char *raw, long long agoraMs, long *dias) {
  EpDia r, h;
  if (!dias || !ep_dia_local(raw, &r) || !diaDoInstante(agoraMs, &h)) return 0;
  *dias = (long)(diasCivis(r.ano, r.mes, r.dia) - diasCivis(h.ano, h.mes, h.dia));
  return 1;
}

int ep_alerta_lancamento(long long lancamentoMs, long long vistoMs, long long agoraMs) {
  const long long sessentaDias = 60LL * EP_DIA_MS;
  return lancamentoMs <= agoraMs && lancamentoMs > vistoMs &&
         (agoraMs - lancamentoMs) < sessentaDias;
}

int ep_alerta_temporada(int alerta, int temporadaSemente, int temporadaProxima) {
  return alerta && temporadaSemente >= 0 && temporadaProxima != temporadaSemente;
}

int ep_arte_usa_fundo(int inedito, const char *thumb, const char *poster,
                      const char *backdrop, float aspecto) {
  if (!inedito || !backdrop || !backdrop[0]) return 0;
  if (!thumb || !thumb[0]) return 1;
  if (poster && poster[0] && !strcmp(thumb, poster)) return 1;
  return aspecto > 0.0f && aspecto < 1.0f;
}

const char *ep_arte_paisagem(const char *provedor, float aspecto, const char *metahub,
                             const char *catalogo) {
  int retrato = aspecto > 0.0f && aspecto < 1.0f;
  if (provedor && provedor[0] && !retrato) return provedor;
  if (metahub && metahub[0]) return metahub;
  if (provedor && provedor[0]) return provedor;
  return catalogo && catalogo[0] ? catalogo : NULL;
}

int ep_arte_cobrir(float aspecto) { return aspecto > 0.0f && aspecto < 1.0f; }

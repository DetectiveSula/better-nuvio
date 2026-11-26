// YouTube trailers for the native apps: see trailerytb.h.
//
// Based on InAppYouTubeExtractor.kt and YouTubeRateLimitGate.kt from NuvioTV
// (https://github.com/NuvioMedia/NuvioTV, GPL-3.0). Translated to C for
// Nuvio+ on 19 Nov 2026 and reduced to the single-URL path: the adaptive
// (separate video + audio) path, captions and the decoder probe are left out,
// because the trailer players here take one url with picture and sound.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE   // strcasestr on glibc (Tizen)
#endif
#include "trailerytb.h"
#include "rede.h"
#include "js.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

// --- constants, as in InAppYouTubeExtractor.kt --------------------------------
#define YTB_UA_PADRAO "Mozilla/5.0 (Linux; Android 12; Android TV) AppleWebKit/537.36 " \
                      "(KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36"
#define YTB_CHAVE_RESERVA "AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8"
#define YTB_PAGINA "https://www.youtube.com/watch?v=dQw4w9WgXcQ&hl=en"
#define YTB_CONFIG_TTL_S      (3 * 60 * 60)   // a fresh watch config lives 3 h
#define YTB_RESERVA_TTL_S     (10 * 60)       // the fallback key, 10 min
#define YTB_PAUSA_BASE_S      60              // rate limit: 1 min, doubling...
#define YTB_PAUSA_MAX_S       (15 * 60)       // ...up to 15 min
#define YTB_DOBRAS_MAX        4
#define YTB_NEGATIVO_S        (30 * 60)       // "no trailer" is believed 30 min
#define YTB_PRAZO_MS          20000u

// The three identities, in NuvioTV's priority order. visionOS is the one that
// returns the HLS master playlist; Android carries the progressive MP4.
typedef struct {
  const char *chave, *id, *versao, *ua, *contexto;
} YtbCliente;
static const YtbCliente CLIENTES[] = {
  { "visionos", "101", "1.02",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 15_7_3) AppleWebKit/605.1.15 "
    "(KHTML, like Gecko) Version/26.0 Safari/605.1.15",
    "\"clientName\":\"VISIONOS\",\"clientVersion\":\"1.02\",\"deviceMake\":\"Apple\","
    "\"deviceModel\":\"RealityDevice17,1\",\"osName\":\"visionOS\","
    "\"osVersion\":\"26.5.23O471\",\"hl\":\"en\",\"gl\":\"US\"" },
  { "android", "3", "20.10.35",
    "com.google.android.youtube/20.10.35 (Linux; U; Android 14; en_US) gzip",
    "\"clientName\":\"ANDROID\",\"clientVersion\":\"20.10.35\",\"osName\":\"Android\","
    "\"osVersion\":\"14\",\"platform\":\"MOBILE\",\"androidSdkVersion\":34,"
    "\"hl\":\"en\",\"gl\":\"US\"" },
  { "ios", "5", "20.10.1",
    "com.google.ios.youtube/20.10.1 (iPhone16,2; U; CPU iOS 17_4 like Mac OS X)",
    "\"clientName\":\"IOS\",\"clientVersion\":\"20.10.1\",\"deviceModel\":\"iPhone16,2\","
    "\"osName\":\"iPhone\",\"osVersion\":\"17.4.0.21E219\",\"platform\":\"MOBILE\","
    "\"hl\":\"en\",\"gl\":\"US\"" },
};
#define N_CLIENTES ((int)(sizeof CLIENTES / sizeof CLIENTES[0]))

// --- pure parts ----------------------------------------------------------------
int trailerytb_id_valido(const char *s) {
  int i;
  if (!s) return 0;
  for (i = 0; s[i]; i++) {
    char c = s[i];
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
          c == '_' || c == '-')) return 0;
    if (i >= 11) return 0;
  }
  return i == 11;
}

// "\"NAME\":\"value\"" anywhere in `txt`.
static int campoTexto(const char *txt, const char *nome, char *dst, size_t n) {
  char padrao[64];
  const char *p, *q;
  size_t k;
  if (!txt || !dst || !n) return 0;
  snprintf(padrao, sizeof padrao, "\"%s\":\"", nome);
  p = strstr(txt, padrao);
  if (!p) return 0;
  p += strlen(padrao);
  q = strchr(p, '"');
  if (!q || q == p) return 0;
  k = (size_t)(q - p);
  if (k >= n) return 0;
  memcpy(dst, p, k); dst[k] = 0;
  return 1;
}

int trailerytb_config_da_pagina(const char *html, char *chave, size_t nChave,
                                char *visitante, size_t nVisitante) {
  if (visitante && nVisitante) visitante[0] = 0;
  if (chave && nChave) chave[0] = 0;
  if (visitante) campoTexto(html, "VISITOR_DATA", visitante, nVisitante);
  return campoTexto(html, "INNERTUBE_API_KEY", chave, nChave);
}

// The object that is the value of "chave" in [ini, fim): '{' in *o, its end in
// *oFim. Returns 1 when found.
static int objeto(const char *ini, const char *fim, const char *chave,
                  const char **o, const char **oFim) {
  char padrao[64];
  const char *p;
  size_t n;
  snprintf(padrao, sizeof padrao, "\"%s\"", chave);
  n = strlen(padrao);
  for (p = ini; p && (!fim || p < fim); ) {
    p = strstr(p, padrao);
    if (!p || (fim && p >= fim)) return 0;
    p += n;
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    if (*p != ':') continue;
    p++;
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    if (*p != '{') continue;
    *o = p;
    *oFim = js_fim(p);
    return *oFim != NULL;
  }
  return 0;
}

static int temN(const char *url) {
  return url && (strstr(url, "?n=") || strstr(url, "&n="));
}

// NuvioTV's videoScore: height first, then frame rate, then bit rate.
static double pontuar(int altura, int fps, double bitrate) {
  return altura * 1e9 + fps * 1e6 + bitrate;
}

void trailerytb_ler_resposta(const char *json, TytbResposta *r) {
  const char *o, *of;
  double melhor = -1.0;
  int melhorN = 1, melhorWebm = 1;
  if (!r) return;
  memset(r, 0, sizeof *r);
  if (!json) return;
  if (objeto(json, NULL, "playabilityStatus", &o, &of)) {
    char motivo[256] = "";
    js_texto(o, of, "status", r->status, sizeof r->status);
    js_texto(o, of, "reason", motivo, sizeof motivo);
    if (!strcmp(r->status, "LOGIN_REQUIRED")) {
      // "Sign in to confirm you're not a bot": YouTube is throttling this IP.
      // An age check belongs to the video; anything else may be a stale visitor.
      const char *b = strcasestr(motivo, "bot"), *a = strcasestr(motivo, "age");
      r->botCheck = b && (b == motivo || b[-1] == ' ') && (b[3] == 0 || b[3] == ' ' || b[3] == '.');
      r->ageCheck = a && (a == motivo || a[-1] == ' ') && (a[3] == 0 || a[3] == ' ' || a[3] == '.' || a[3] == '-');
    }
  }
  if (!objeto(json, NULL, "streamingData", &o, &of)) return;
  js_texto(o, of, "hlsManifestUrl", r->hls, sizeof r->hls);
  // "formats" = the muxed (picture + sound) files. Order as NuvioTV's
  // sortCandidates: best score, then no "n" throttling parameter, then MP4
  // before WebM (the client order is the order we ask in).
  { const char *p = js_array(o, of, "formats");
    for (; p && p < of; p = js_prox(js_fim(p))) {
      const char *pe = js_fim(p);
      char url[4096] = "", mime[96] = "", rotulo[24] = "";
      int altura, fps, n, webm;
      double sc;
      if (!pe) break;
      if (*p != '{') continue;
      if (!js_texto(p, pe, "url", url, sizeof url) || !url[0]) continue;   // signatureCipher: skip
      js_texto(p, pe, "mimeType", mime, sizeof mime);
      if (mime[0] && !strstr(mime, "video/")) continue;
      altura = (int)js_num(p, pe, "height", 0);
      if (altura <= 0 && js_texto(p, pe, "qualityLabel", rotulo, sizeof rotulo)) altura = atoi(rotulo);
      fps = (int)js_num(p, pe, "fps", 0);
      sc = pontuar(altura, fps, js_num(p, pe, "bitrate", js_num(p, pe, "averageBitrate", 0)));
      n = temN(url);
      webm = strstr(mime, "webm") != NULL;
      if (sc > melhor || (sc == melhor && (n < melhorN || (n == melhorN && webm < melhorWebm)))) {
        melhor = sc; melhorN = n; melhorWebm = webm;
        snprintf(r->progressivo, sizeof r->progressivo, "%s", url);
        r->progAltura = altura;
      }
    } }
}

int trailerytb_hls_melhor(const char *m3u8, long *banda) {
  const char *p = m3u8;
  int melhorAlt = 0, melhorLarg = 0;
  long melhorBanda = 0;
  if (banda) *banda = 0;
  if (!m3u8) return 0;
  while ((p = strstr(p, "#EXT-X-STREAM-INF:")) != NULL) {
    const char *fimLinha = strchr(p, '\n'), *r, *b, *prox;
    int larg = 0, alt = 0;
    long bw = 0;
    if (!fimLinha) break;
    r = strstr(p, "RESOLUTION=");
    if (r && r < fimLinha) sscanf(r + 11, "%dx%d", &larg, &alt);
    b = strstr(p, "BANDWIDTH=");
    if (b && b < fimLinha) bw = atol(b + 10);
    // The next non-empty line must be the variant's uri, not another tag.
    prox = fimLinha + 1;
    while (*prox == '\r' || *prox == '\n' || *prox == ' ') prox++;
    if (*prox && *prox != '#' &&
        (alt > melhorAlt || (alt == melhorAlt && bw > melhorBanda) ||
         (alt == melhorAlt && bw == melhorBanda && larg > melhorLarg))) {
      melhorAlt = alt; melhorBanda = bw; melhorLarg = larg;
      if (!melhorAlt) melhorAlt = 1;   // a variant without RESOLUTION still counts
    }
    p = fimLinha;
  }
  if (banda) *banda = melhorBanda;
  return melhorAlt;
}

// --- the playlist the TV gets (owner, 20/11/2026) ------------------------------
// "Very low quality", "the audio kicks in delayed", "a different language" on
// the Samsung TV. YouTube's master lists 16 variants from 144p up, H.264 and
// VP9, with the sound as a SEPARATE rendition in two groups (HE-AAC for the
// small variants, AAC-LC for 360p up), dubbed tracks as more renditions, and
// auto captions with AUTOSELECT=YES. AVPlay starts on the first, smallest
// variant and climbs slowly (a 2-minute trailer stays blurry), and the climb
// changes the audio group, so the sound restarts a few seconds in.
// What the TV gets instead: ONE H.264 variant (the tallest up to 1080p, then
// the highest bit rate), the ONE audio rendition of its group (the original
// track over a dub, then the default) and no captions.
static const char *attr(const char *ini, const char *fim, const char *nome, char *dst, size_t n) {
  size_t ln = strlen(nome);
  const char *p = ini;
  dst[0] = 0;
  while ((p = strstr(p, nome)) != NULL && p < fim) {
    if ((p == ini || p[-1] == ',' || p[-1] == ':') && p[ln] == '=') {
      const char *v = p + ln + 1, *e;
      if (*v == '"') { v++; e = memchr(v, '"', (size_t)(fim - v)); }
      else { e = v; while (e < fim && *e != ',') e++; }
      if (!e) e = fim;
      snprintf(dst, n, "%.*s", (int)(e - v), v);
      return dst;
    }
    p += ln;
  }
  return NULL;
}

// The line [ini, fim) without the attribute `nome` (and its comma).
static void semAttr(const char *ini, const char *fim, const char *nome, char *dst, size_t n) {
  size_t ln = strlen(nome), k = 0;
  const char *p = ini;
  while (p < fim && k + 1 < n) {
    if (p > ini && p[-1] == ',' && !strncmp(p, nome, ln) && p[ln] == '=') {
      const char *e = p + ln + 1;
      if (*e == '"') { e = memchr(e + 1, '"', (size_t)(fim - e - 1)); e = e ? e + 1 : fim; }
      else while (e < fim && *e != ',') e++;
      if (k && dst[k - 1] == ',') k--;   // drop the comma before it
      p = e;
      continue;
    }
    dst[k++] = *p++;
  }
  dst[k] = 0;
}

static const char *fimDaLinha(const char *p) {
  const char *e = strchr(p, '\n');
  if (!e) e = p + strlen(p);
  while (e > p && e[-1] == '\r') e--;
  return e;
}

int trailerytb_hls_reduzir(const char *m3u8, char *out, size_t n, int *altura) {
  const char *p, *vIni = NULL, *vFim = NULL, *uIni = NULL, *uFim = NULL, *aIni = NULL, *aFim = NULL;
  int melhorAlt = 0, aPontos = -1;
  long melhorBw = -1;
  char grupo[64] = "", v[128];
  size_t k;
  if (altura) *altura = 0;
  if (!m3u8 || !out || n < 64 || strncmp(m3u8, "#EXTM3U", 7)) return 0;
  for (p = m3u8; (p = strstr(p, "#EXT-X-STREAM-INF:")) != NULL; ) {
    const char *f = fimDaLinha(p), *u = f;
    int larg = 0, alt = 0;
    long bw;
    while (*u == '\r' || *u == '\n' || *u == ' ') u++;
    if (!*u || *u == '#') { p = f; continue; }
    if (!attr(p + 18, f, "CODECS", v, sizeof v) || strncmp(v, "avc1", 4)) { p = f; continue; }
    if (attr(p + 18, f, "RESOLUTION", v, sizeof v)) sscanf(v, "%dx%d", &larg, &alt);
    bw = attr(p + 18, f, "BANDWIDTH", v, sizeof v) ? atol(v) : 0;
    if (alt <= 1080 && (alt > melhorAlt || (alt == melhorAlt && bw > melhorBw))) {
      melhorAlt = alt; melhorBw = bw;
      vIni = p; vFim = f; uIni = u; uFim = fimDaLinha(u);
      if (!attr(p + 18, f, "AUDIO", grupo, sizeof grupo)) grupo[0] = 0;
    }
    p = f;
  }
  if (!vIni) return 0;
  if (grupo[0]) {
    for (p = m3u8; (p = strstr(p, "#EXT-X-MEDIA:")) != NULL; ) {
      const char *f = fimDaLinha(p);
      int pts;
      if (!attr(p + 13, f, "TYPE", v, sizeof v) || strcmp(v, "AUDIO") ||
          !attr(p + 13, f, "GROUP-ID", v, sizeof v) || strcmp(v, grupo)) { p = f; continue; }
      pts = 1;
      if (attr(p + 13, f, "DEFAULT", v, sizeof v) && !strcmp(v, "YES")) pts = 2;
      if (attr(p + 13, f, "NAME", v, sizeof v) && strcasestr(v, "original")) pts = 3;
      if (pts > aPontos) { aPontos = pts; aIni = p; aFim = f; }
      p = f;
    }
    if (!aIni) return 0;   // a variant without its sound would play mute
  }
  k = (size_t)snprintf(out, n, "#EXTM3U\n%s",
                       strstr(m3u8, "#EXT-X-INDEPENDENT-SEGMENTS") ? "#EXT-X-INDEPENDENT-SEGMENTS\n" : "");
  if (aIni) {
    char lin[4096], a2[4096];
    // The one rendition is the default; nothing else for the player to pick.
    semAttr(aIni, aFim, "DEFAULT", lin, sizeof lin);
    semAttr(lin, lin + strlen(lin), "AUTOSELECT", a2, sizeof a2);
    k += (size_t)snprintf(out + k, k < n ? n - k : 0, "%s,DEFAULT=YES,AUTOSELECT=YES\n", a2);
  }
  { char lin[2048], l2[2048];
    semAttr(vIni, vFim, "SUBTITLES", lin, sizeof lin);
    semAttr(lin, lin + strlen(lin), "CLOSED-CAPTIONS", l2, sizeof l2);
    k += (size_t)snprintf(out + k, k < n ? n - k : 0, "%s\n%.*s\n", l2, (int)(uFim - uIni), uIni); }
  if (k >= n) return 0;
  if (altura) *altura = melhorAlt;
  return 1;
}

long trailerytb_expira(const char *url) {
  const char *p;
  if (!url) return 0;
  if ((p = strstr(url, "expire=")) != NULL) return atol(p + 7);   // googlevideo query
  if ((p = strstr(url, "/expire/")) != NULL) return atol(p + 8);  // manifest path
  return 0;
}

// --- state ---------------------------------------------------------------------
#define YTB_MAX 32
typedef struct {
  char id[12];
  char url[4096];
  char *manif;        // the reduced playlist the local server hands the TV
  long valeAte;       // epoch; result believed until then
  int  emVoo, respondeu;
  int  larg, alt;     // what the TV gets (0 = unknown), for trailerytb_1080p
} Entrada;
static Entrada tab[YTB_MAX];
static int nTab, proxSlot;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cfgMtx = PTHREAD_MUTEX_INITIALIZER;
#define YTB_VIS_MAX 2048   // the visitor id grew to ~450 characters (Nov 2026)
static char cfgChave[96], cfgVisitante[YTB_VIS_MAX];
static long cfgEm, cfgTtl;
static int  greves;          // YouTubeRateLimitGate: strikes
static long bloqueadoAte;    // epoch seconds

static long agora(void) { return (long)time(NULL); }

static Entrada *achar(const char *id) {
  int i;
  for (i = 0; i < nTab; i++) if (!strcmp(tab[i].id, id)) return &tab[i];
  return NULL;
}
static Entrada *reservar(const char *id) {
  Entrada *e = achar(id);
  if (e) return e;
  if (nTab < YTB_MAX) e = &tab[nTab++];
  else { e = &tab[proxSlot]; proxSlot = (proxSlot + 1) % YTB_MAX; }
  free(e->manif);
  memset(e, 0, sizeof *e);
  snprintf(e->id, sizeof e->id, "%s", id);
  return e;
}

int trailerytb_bloqueado(void) {
  int b;
  pthread_mutex_lock(&mtx);
  b = agora() < bloqueadoAte;
  pthread_mutex_unlock(&mtx);
  return b;
}
// Each strike doubles the pause, up to 15 min; a success resets it.
static long pausar(void) {
  long s;
  int d;
  pthread_mutex_lock(&mtx);
  greves++;
  d = greves - 1 < YTB_DOBRAS_MAX ? greves - 1 : YTB_DOBRAS_MAX;
  s = (long)YTB_PAUSA_BASE_S << d;
  if (s > YTB_PAUSA_MAX_S) s = YTB_PAUSA_MAX_S;
  bloqueadoAte = agora() + s;
  pthread_mutex_unlock(&mtx);
  return s;
}
static void sucesso(void) {
  pthread_mutex_lock(&mtx);
  greves = 0; bloqueadoAte = 0;
  pthread_mutex_unlock(&mtx);
}

// --- network -------------------------------------------------------------------
// GET or POST; body in *corpo (caller frees), returns the HTTP status (0 = no
// answer at all).
static int pedir(const char *metodo, const char *url, const char *const *cab,
                 const char *corpo, char **saida) {
  RedeResposta r = {0};
  RedePedido q = {0};
  int st;
  *saida = NULL;
  q.metodo = metodo; q.url = url; q.cabecalhos = cab;
  q.corpo = corpo; q.n_corpo = corpo ? strlen(corpo) : 0;
  q.prazo_ms = YTB_PRAZO_MS; q.seguir = 1;
  // YouTube's headers (cookies, security policy) pass the 16 KB default.
  q.max_cabecalhos = REDE_CAB_MAXIMO;
  rede_pedir(&q, &r);
  st = r.erro == REDE_OK ? r.status : 0;
  if (st >= 200 && st < 300 && r.corpo) { *saida = r.corpo; r.corpo = NULL; }
  rede_resposta_limpar(&r);
  return st;
}

enum { CFG_OK = 0, CFG_LIMITE = 1 };
// The api key and visitor id, from a cached watch page. `forcar` refetches.
static int garantirConfig(int forcar, char *chave, size_t nChave, char *vis, size_t nVis) {
  static const char *const cab[] = {
    "Accept-Language: en-US,en;q=0.9", "User-Agent: " YTB_UA_PADRAO, NULL };
  int r = CFG_OK;
  pthread_mutex_lock(&cfgMtx);
  if (forcar || !cfgChave[0] || agora() - cfgEm > cfgTtl) {
    char *html = NULL;
    int st = pedir(NULL, YTB_PAGINA, cab, NULL, &html);
    char k[96] = "", v[YTB_VIS_MAX] = "";
    if (st == 429) r = CFG_LIMITE;
    else if (html && trailerytb_config_da_pagina(html, k, sizeof k, v, sizeof v)) {
      snprintf(cfgChave, sizeof cfgChave, "%s", k);
      snprintf(cfgVisitante, sizeof cfgVisitante, "%s", v);
      cfgEm = agora(); cfgTtl = YTB_CONFIG_TTL_S;
    } else if (!cfgChave[0] || forcar) {
      // A consent or bot-check page can replace the watch page while the
      // player API still answers: keep going with the fallback key, briefly.
      snprintf(cfgChave, sizeof cfgChave, "%s", YTB_CHAVE_RESERVA);
      cfgVisitante[0] = 0;
      cfgEm = agora(); cfgTtl = YTB_RESERVA_TTL_S;
    }
    free(html);
  }
  snprintf(chave, nChave, "%s", cfgChave);
  snprintf(vis, nVis, "%s", cfgVisitante);
  pthread_mutex_unlock(&cfgMtx);
  return r;
}

// One player request as client `c`. Returns the HTTP status.
static int perguntar(const YtbCliente *c, const char *id, const char *chave,
                     const char *vis, TytbResposta *r) {
  char url[256], corpo[1024], h1[64], h2[64], h3[320], h4[YTB_VIS_MAX + 32];
  const char *cab[10];
  int n = 0, st;
  char *json = NULL;
  snprintf(url, sizeof url, "https://www.youtube.com/youtubei/v1/player?key=%s", chave);
  snprintf(corpo, sizeof corpo,
           "{\"videoId\":\"%s\",\"contentCheckOk\":true,\"racyCheckOk\":true,"
           "\"context\":{\"client\":{%s}},\"playbackContext\":{\"contentPlaybackContext\":"
           "{\"html5Preference\":\"HTML5_PREF_WANTS\"}}}", id, c->contexto);
  snprintf(h1, sizeof h1, "X-YouTube-Client-Name: %s", c->id);
  snprintf(h2, sizeof h2, "X-YouTube-Client-Version: %s", c->versao);
  snprintf(h3, sizeof h3, "User-Agent: %s", c->ua);
  cab[n++] = "Content-Type: application/json";
  cab[n++] = "Origin: https://www.youtube.com";
  cab[n++] = "Accept-Language: en-US,en;q=0.9";
  cab[n++] = h1; cab[n++] = h2; cab[n++] = h3;
  if (vis && vis[0]) { snprintf(h4, sizeof h4, "X-Goog-Visitor-Id: %s", vis); cab[n++] = h4; }
  cab[n] = NULL;
  st = pedir("POST", url, cab, corpo, &json);
  trailerytb_ler_resposta(json, r);
  free(json);
  return st;
}

// The manifest is only trusted once it downloads and lists a variant.
// `*master` gets the playlist (caller frees) when it serves.
static int hlsServe(const char *url, int *altura, char **master) {
  static const char *const cab[] = { "User-Agent: " YTB_UA_PADRAO, NULL };
  char *m = NULL;
  long banda;
  int st = pedir(NULL, url, cab, NULL, &m);
  *altura = (st >= 200 && st < 300 && m) ? trailerytb_hls_melhor(m, &banda) : 0;
  if (*altura > 0) *master = m;
  else free(m);
  return *altura > 0;
}

// The local server's hook (main.c): the url it serves `id`'s playlist at.
static int (*urlLocal)(const char *id, char *dst, size_t n);
void trailerytb_definir_local(int (*fn)(const char *id, char *dst, size_t n)) { urlLocal = fn; }

// A progressive url is used once one byte of it downloads (NuvioTV probes the
// CDN the same way; the alternate CDN hosts are left out here).
static int progressivoServe(const char *url) {
  static const char *const cab[] = { "Range: bytes=0-0", "User-Agent: " YTB_UA_PADRAO, NULL };
  RedeResposta r = {0};
  RedePedido q = {0};
  int ok;
  q.url = url; q.cabecalhos = cab; q.prazo_ms = 4000; q.seguir = 1; q.max_bytes = 4096;
  rede_pedir(&q, &r);
  ok = r.erro == REDE_OK && (r.status == 200 || r.status == 206);
  rede_resposta_limpar(&r);
  return ok;
}

enum { EX_NADA = 0, EX_ACHOU = 1, EX_LIMITE = 2, EX_LOGIN = 3 };
// The clients in order, stopping at the first HLS manifest that plays; the
// best progressive is the fallback. `dst` gets the url.
// RESOLUTION of the first variant (the reduced playlist has one).
static void resolucaoDe(const char *m3u8, int *larg, int *alt) {
  const char *p = m3u8 ? strstr(m3u8, "#EXT-X-STREAM-INF:") : NULL;
  char v[64];
  if (!p) return;
  if (attr(p + 18, fimDaLinha(p), "RESOLUTION", v, sizeof v)) sscanf(v, "%dx%d", larg, alt);
}

static int extrair(const char *id, int forcarConfig, char *dst, size_t n, const char **como,
                   char **manif, int *larg, int *altura) {
  char chave[96], vis[YTB_VIS_MAX], prog[4096] = "";
  int i, logins = 0, bots = 0, progAlt = 0, respostas = 0;
  if (garantirConfig(forcarConfig, chave, sizeof chave, vis, sizeof vis) == CFG_LIMITE) return EX_LIMITE;
  for (i = 0; i < N_CLIENTES; i++) {
    TytbResposta r;
    int st = perguntar(&CLIENTES[i], id, chave, vis, &r);
    if (st == 429) return EX_LIMITE;
    if (st < 200 || st >= 300) continue;
    if (!strcmp(r.status, "LOGIN_REQUIRED")) {
      if (r.botCheck) bots++;
      else if (!r.ageCheck) logins++;
      continue;
    }
    if (r.status[0] && strcmp(r.status, "OK")) continue;
    respostas++;
    if (r.hls[0]) {
      int alt = 0, altRed = 0;
      char *master = NULL;
      if (hlsServe(r.hls, &alt, &master)) {
        size_t cap = strlen(master) + 64;
        char *red = urlLocal ? malloc(cap) : NULL;
        snprintf(dst, n, "%s", r.hls);
        *como = CLIENTES[i].chave;
        if (red && trailerytb_hls_reduzir(master, red, cap, &altRed) && urlLocal(id, dst, n)) {
          *manif = red;
          *altura = altRed; *larg = altRed * 16 / 9;
          resolucaoDe(red, larg, altura);
          printf("[trailer] youtube %s: HLS %dp H.264, um audio, servido local (%s)\n",
                 id, altRed, CLIENTES[i].chave);
        } else {
          free(red);
          snprintf(dst, n, "%s", r.hls);
          *altura = alt; *larg = alt * 16 / 9;
          printf("[trailer] youtube %s: HLS ate %dp (%s)\n", id, alt, CLIENTES[i].chave);
        }
        free(master);
        return EX_ACHOU;
      }
    }
    if (r.progressivo[0] && r.progAltura > progAlt) {
      snprintf(prog, sizeof prog, "%s", r.progressivo);
      progAlt = r.progAltura;
    }
  }
  if (prog[0] && progressivoServe(prog)) {
    snprintf(dst, n, "%s", prog);
    *como = "progressivo";
    *altura = progAlt; *larg = progAlt * 16 / 9;
    printf("[trailer] youtube %s: MP4 %dp\n", id, progAlt);
    return EX_ACHOU;
  }
  if (!respostas && bots) return EX_LIMITE;
  if (!respostas && logins == N_CLIENTES) return EX_LOGIN;
  return EX_NADA;
}

static void *buscar(void *arg) {
  char id[12], url[4096] = "", *manif = NULL;
  const char *como = "";
  int r, larg = 0, alt = 0;
  long vale;
  Entrada *e;
  snprintf(id, sizeof id, "%s", (const char *)arg);
  free(arg);
  r = extrair(id, 0, url, sizeof url, &como, &manif, &larg, &alt);
  // Only a stale visitor id is fixed by a fresh watch page; anything else
  // would fail the same way, and every request counts toward YouTube's limit.
  if (r == EX_LOGIN) r = extrair(id, 1, url, sizeof url, &como, &manif, &larg, &alt);
  if (r == EX_ACHOU) sucesso();
  else if (r == EX_LIMITE)
    printf("[trailer] youtube %s: YouTube recusando pedidos, pausa de %ld s\n", id, pausar());
  else printf("[trailer] youtube %s: sem trailer tocavel\n", id);
  fflush(stdout);
  vale = r == EX_ACHOU ? trailerytb_expira(url) - 600 : agora() + YTB_NEGATIVO_S;
  if (r == EX_ACHOU && vale < agora() + 600) vale = agora() + 3600;   // no expiry in the url
  pthread_mutex_lock(&mtx);
  e = reservar(id);
  e->emVoo = 0;
  e->respondeu = 1;
  if (r == EX_ACHOU) snprintf(e->url, sizeof e->url, "%s", url);
  else e->url[0] = 0;
  e->larg = r == EX_ACHOU ? larg : 0;
  e->alt = r == EX_ACHOU ? alt : 0;
  free(e->manif);
  e->manif = r == EX_ACHOU ? manif : NULL;
  if (r != EX_ACHOU) free(manif);
  // A pause is not an answer about the video: try again after it.
  e->valeAte = r == EX_LIMITE ? 0 : vale;
  pthread_mutex_unlock(&mtx);
  return NULL;
}

void trailerytb_pedir(const char *id) {
  Entrada *e;
  pthread_t f;
  char *arg;
  if (!trailerytb_id_valido(id)) return;
  pthread_mutex_lock(&mtx);
  if (agora() < bloqueadoAte) { pthread_mutex_unlock(&mtx); return; }
  e = reservar(id);
  if (e->emVoo || (e->respondeu && agora() < e->valeAte)) { pthread_mutex_unlock(&mtx); return; }
  e->emVoo = 1; e->respondeu = 0;
  pthread_mutex_unlock(&mtx);
  arg = strdup(id);
  if (!arg) { pthread_mutex_lock(&mtx); e->emVoo = 0; e->respondeu = 1; pthread_mutex_unlock(&mtx); return; }
  if (pthread_create(&f, NULL, buscar, arg) == 0) pthread_detach(f);
  else { free(arg); pthread_mutex_lock(&mtx); e->emVoo = 0; e->respondeu = 1; pthread_mutex_unlock(&mtx); }
}

const char *trailerytb_url(const char *id) {
  Entrada *e;
  const char *u = NULL;
  if (!trailerytb_id_valido(id)) return NULL;
  pthread_mutex_lock(&mtx);
  e = achar(id);
  if (e && e->respondeu && e->url[0] && agora() < e->valeAte) u = e->url;
  pthread_mutex_unlock(&mtx);
  return u;
}

// 1080p OR BETTER (owner, 24/11/2026: "quality minimum 1080p"). YouTube calls
// a 1920x804 scope trailer 1080p, so the width counts as much as the height.
int trailerytb_1080p(const char *id) {
  Entrada *e;
  int ok = 0;
  if (!trailerytb_id_valido(id)) return 0;
  pthread_mutex_lock(&mtx);
  e = achar(id);
  if (e && e->respondeu && e->url[0] && agora() < e->valeAte)
    ok = e->larg >= 1900 || e->alt >= 1060;
  pthread_mutex_unlock(&mtx);
  return ok;
}

// The first of `ids` (best first) that plays at 1080p or better. Asks for
// them in order and waits on the first unanswered one: never more than one
// lookup in flight per title, and the better pick is never skipped for being
// slower. NULL with *pronto 1 = none of them qualifies.
const char *trailerytb_primeiro_1080p(const char *const *ids, int n, int *pronto) {
  int i;
  *pronto = 1;
  for (i = 0; i < n; i++) {
    if (!trailerytb_id_valido(ids[i])) continue;
    trailerytb_pedir(ids[i]);
    if (!trailerytb_respondeu(ids[i])) { *pronto = 0; return NULL; }
    if (trailerytb_1080p(ids[i])) return trailerytb_url(ids[i]);
  }
  return NULL;
}

int trailerytb_respondeu(const char *id) {
  Entrada *e;
  int r;
  if (!trailerytb_id_valido(id)) return 1;
  pthread_mutex_lock(&mtx);
  // While YouTube refuses requests there is nothing to wait for.
  if (agora() < bloqueadoAte) { pthread_mutex_unlock(&mtx); return 1; }
  e = achar(id);
  r = e ? e->respondeu && !e->emVoo : 0;
  pthread_mutex_unlock(&mtx);
  return r;
}

// The local server asks for `id`'s reduced playlist (0 = none or expired).
int trailerytb_manifesto(const char *id, char *dst, size_t n) {
  Entrada *e;
  int ok = 0;
  if (!trailerytb_id_valido(id) || !dst || !n) return 0;
  pthread_mutex_lock(&mtx);
  e = achar(id);
  if (e && e->manif && agora() < e->valeAte && strlen(e->manif) < n) {
    snprintf(dst, n, "%s", e->manif);
    ok = 1;
  }
  pthread_mutex_unlock(&mtx);
  return ok;
}

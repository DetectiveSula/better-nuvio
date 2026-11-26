// YOUTUBE TRAILERS IN THE NATIVE APPS (owner, 19/11/2026).
//
// IMDb's trailer API now answers every request with CloudFront's "Request
// blocked" 403, and Apple only has the titles it sells. The official Nuvio
// apps (Android TV, sideloaded phones) get their trailers from YouTube: TMDB
// gives the video id, the app asks YouTube's own player API for a playable
// stream and plays it in its own player. This module is a C port of that
// extractor, in its single-URL mode, for the players here (AVPlay, Media3,
// the LG pipeline), which take ONE url with picture and sound together.
//
// Based on InAppYouTubeExtractor.kt and YouTubeRateLimitGate.kt from NuvioTV
// (https://github.com/NuvioMedia/NuvioTV, GPL-3.0), translated to C and
// trimmed to what a single-URL player can use: the HLS master playlist first
// (H.264 up to 1080p with the sound muxed into every variant), then the best
// progressive MP4. Modified 19 Nov 2026 for Nuvio+, also GPL-3.0.
//
// It asks YouTube as YouTube's own visionOS, Android and iOS apps do, which is
// outside YouTube's terms; YouTube also changes this API from time to time,
// so expect to follow NuvioTV's fixes. Network and threads stay inside the
// .c; the parsing below is pure, for tests/trailerytb.c.
#ifndef NV_TRAILERYTB_H
#define NV_TRAILERYTB_H
#include <stddef.h>

// Starts (once) the lookup of YouTube video `id` (11 characters, from TMDB's
// /videos). Cheap and idempotent: call it every frame if convenient.
void        trailerytb_pedir(const char *id);
// The playable url (HLS master or MP4) when the lookup found one and it has
// not expired; NULL otherwise.
const char *trailerytb_url(const char *id);
// 1 when there is nothing more to wait for: found, not found, failed, or
// YouTube is refusing requests right now (the caller moves on to the next
// source instead of waiting).
int         trailerytb_respondeu(const char *id);
// 1 when `id` resolved to a stream of 1080p or better (width >= 1900 or
// height >= 1060). trailerytb_primeiro_1080p: the first such of `ids`, asking
// in order; *pronto 0 = still waiting on one.
int         trailerytb_1080p(const char *id);
const char *trailerytb_primeiro_1080p(const char *const *ids, int n, int *pronto);
// 1 while YouTube is rate limiting and lookups are paused.
int         trailerytb_bloqueado(void);
// THE PLAYLIST THE TV GETS (20/11/2026): with a local server (main.c wires
// proxyts), an HLS trailer plays from a reduced copy of YouTube's master: one
// H.264 variant, its one audio rendition, no captions (trailerytb_hls_reduzir).
// `fn` writes the local url for `id`; trailerytb_manifesto is what it serves.
void trailerytb_definir_local(int (*fn)(const char *id, char *dst, size_t n));
int  trailerytb_manifesto(const char *id, char *dst, size_t n);

// --- pure parts (no network), for the tests ---------------------------------
// 1 when `s` is an 11-character YouTube id ([A-Za-z0-9_-]).
int  trailerytb_id_valido(const char *s);
// INNERTUBE_API_KEY and VISITOR_DATA from a watch page; returns 1 when the key
// was found (the visitor id is optional).
int  trailerytb_config_da_pagina(const char *html, char *chave, size_t nChave,
                                 char *visitante, size_t nVisitante);
// What one player response says. `status`: "OK", "LOGIN_REQUIRED", ...
typedef struct {
  char status[24];
  int  botCheck, ageCheck;        // LOGIN_REQUIRED because of a bot / age check
  char hls[2048];                 // streamingData.hlsManifestUrl
  char progressivo[4096];         // best playable progressive (muxed) url
  int  progAltura;                // its height (0 = none)
} TytbResposta;
void trailerytb_ler_resposta(const char *json, TytbResposta *r);
// Best variant of an HLS master playlist: returns its height (0 = no variant)
// and its bandwidth in *banda.
int  trailerytb_hls_melhor(const char *m3u8, long *banda);
// The master reduced to the tallest H.264 variant up to 1080p (then the highest
// bit rate) and the one audio rendition of its group (the original track over
// a dub, then the default), captions dropped. 1 = `out` holds it, *altura its
// height; 0 = no H.264 variant, or its sound is missing (keep the master).
int  trailerytb_hls_reduzir(const char *m3u8, char *out, size_t n, int *altura);
// Seconds-since-epoch expiry written in a googlevideo / manifest url, 0 = none.
long trailerytb_expira(const char *url);

#endif

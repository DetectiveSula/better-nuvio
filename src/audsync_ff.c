// Ver audsync_ff.h. Software decode of the playing source's audio for the
// subtitle audio sync (audsync.c) on players without a PCM tap (.tpk AVPlay).
#include "audsync_ff.h"
#include "audsync.h"
#include <stdio.h>
#include <string.h>

#ifndef NV_DTS_FFMPEG
int  audsyncff_disponivel(void) { return 0; }
void audsyncff_registrar(AudsyncFfProvedor p) { (void)p; }
int  audsyncff_parar(int ms) { (void)ms; return 1; }
int  audsyncff_teste_ativos(void) { return 0; }
AudsyncFfRelato audsyncff_teste_relato(void) { AudsyncFfRelato r; memset(&r, 0, sizeof r); r.stream = -1; return r; }
#else

#include "dts/dts_engine.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#define ENTREGA      1600            // samples per audsync_pcm (100 ms; audsync takes <= 4096)
#define SALTO_S      0.10            // decoded pts this far from the expected one = re-anchor
#define FOLGA_FIM_S  10.0            // window placed so it ends this far before the end
#define ESPERA_MS    20

static pthread_mutex_t FM = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t FC = PTHREAD_COND_INITIALIZER;
static AudsyncFfProvedor provedor;
static int registrado;
static atomic_uint geracao;          // bumped by every ligar(): older workers stop
static DtsEngine *engAtual;          // FM: reader of the newest worker (cancel target)
static int ativos;                   // FM: worker threads alive
static AudsyncFfRelato relato;       // FM

typedef struct { AudsyncFfFonte f; unsigned ger; uint64_t pedido; } Tarefa;

static void dormirMs(int ms) {
  struct timespec t = { ms / 1000, (long)(ms % 1000) * 1000000L };
  while (nanosleep(&t, &t) && errno == EINTR) {}
}

static int vivo(const Tarefa *t) { return atomic_load(&geracao) == t->ger; }

// Same request still listening? Paused (seek/buffer) = wait; anything else = stop.
// Returns 1 to deliver now, 0 to stop.
static int esperarVez(const Tarefa *t, int n) {
  for (;;) {
    AudSyncStatus st;
    if (!vivo(t)) return 0;
    st = audsync_status();
    if (st.pedido != t->pedido || st.fase != AUDSYNC_OUVINDO) return 0;
    if (!st.pausado && audsync_espaco() >= n) return 1;
    dormirMs(ESPERA_MS);
  }
}

// Delivers n samples in ENTREGA pieces. 0 = stop (session over or cancelled).
static int entregar(const Tarefa *t, const int16_t *a, int n, int64_t ptsUs) {
  while (n > 0) {
    int k = n < ENTREGA ? n : ENTREGA;
    if (!esperarVez(t, k)) return 0;
    if (!audsync_pcm(a, k, ptsUs)) {
      // Refused without overflow: paused between the check and the call. The
      // window restarts on resume anyway; retry the same piece.
      continue;
    }
    a += k; n -= k; ptsUs += (int64_t)k * 1000000 / AUDVAD_HZ;
  }
  return 1;
}

static int casaIdioma(const char *quer, const char *tem) {
  size_t a, b;
  if (!quer[0] || !tem || !tem[0]) return 0;
  if (!strcasecmp(quer, tem)) return 1;
  a = strlen(quer); b = strlen(tem);
  // "en" vs "eng": the TV reports either form.
  return (a == 2 && b == 3 && !strncasecmp(quer, tem, 2)) || (a == 3 && b == 2 && !strncasecmp(quer, tem, 2));
}

// The audio stream to listen to: the n-th audio stream when the platform knows
// it, else the selected track's language, else the default, else the first.
static int escolherStream(AVFormatContext *ic, const AudsyncFfFonte *f) {
  int i, k = 0, primeiro = -1, padrao = -1, idioma = -1;
  for (i = 0; i < (int)ic->nb_streams; i++) {
    AVStream *s = ic->streams[i];
    AVDictionaryEntry *l;
    if (s->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) continue;
    if (!avcodec_find_decoder(s->codecpar->codec_id)) { k++; continue; }
    if (f->ordinal >= 0 && k == f->ordinal) return i;
    if (primeiro < 0) primeiro = i;
    if (padrao < 0 && (s->disposition & AV_DISPOSITION_DEFAULT)) padrao = i;
    l = av_dict_get(s->metadata, "language", NULL, 0);
    if (idioma < 0 && l && casaIdioma(f->idioma, l->value)) idioma = i;
    k++;
  }
  return idioma >= 0 ? idioma : padrao >= 0 ? padrao : primeiro;
}

// One listening run. 1 = stopped on purpose (window done, cancel), 0 = the
// source failed or ended first (the session fails).
static int ouvir(DtsEngine *e, const Tarefa *t) {
  AVFormatContext *ic;
  AVCodecContext *dc = NULL;
  SwrContext *swr = NULL;
  AVPacket *pk = NULL;
  AVFrame *fr = NULL;
  AVChannelLayout mono = AV_CHANNEL_LAYOUT_MONO;
  int16_t *saida = NULL;
  int cap = 0, si, r, ok = 0, ancorado = 0, fimDemux = 0;
  int64_t inicioUs = 0, ancoraUs = 0, saidas = 0;
  double proximo = 0, ini;
  long long total = 0;
  AVStream *st;

  if (dts_engine_open_demux(e, t->f.url, t->f.cabecalhos) < 0) {
    printf("[audsync-ff] source open failed: %s\n", dts_engine_error(e));
    return 0;
  }
  ic = dts_engine_avformat(e);
  si = ic ? escolherStream(ic, &t->f) : -1;
  if (si < 0) { printf("[audsync-ff] no decodable audio stream\n"); return 0; }
  st = ic->streams[si];
  for (r = 0; r < (int)ic->nb_streams; r++) if (r != si) ic->streams[r]->discard = AVDISCARD_ALL;
  if (ic->start_time != AV_NOPTS_VALUE) inicioUs = ic->start_time;
  {
    const AVCodec *c = avcodec_find_decoder(st->codecpar->codec_id);
    dc = avcodec_alloc_context3(c);
    if (!dc || avcodec_parameters_to_context(dc, st->codecpar) < 0) goto fim;
    dc->pkt_timebase = st->time_base;
    // Only the core of DTS-HD / TrueHD would be cheaper, but any decode works.
    if (avcodec_open2(dc, c, NULL) < 0) { printf("[audsync-ff] decoder open failed (%s)\n", c->name); goto fim; }
    pthread_mutex_lock(&FM);
    relato.stream = si; relato.canais = st->codecpar->ch_layout.nb_channels; relato.taxa = st->codecpar->sample_rate;
    snprintf(relato.codec, sizeof relato.codec, "%s", c->name);
    pthread_mutex_unlock(&FM);
  }
  // The window starts at the playback position, pulled back when it would not
  // fit before the end. Any window of the film measures the same offset.
  ini = t->f.pos > 0 ? t->f.pos : 0;
  if (ic->duration > 0) {
    double dur = ic->duration / (double)AV_TIME_BASE, ultimo = dur - AUDSYNC_ALVO_SEG - FOLGA_FIM_S;
    if (ini > ultimo) ini = ultimo > 0 ? ultimo : 0;
  }
  printf("[audsync-ff] listening: stream %d %s %dch %d Hz from %.1fs\n", si, avcodec_get_name(st->codecpar->codec_id),
         st->codecpar->ch_layout.nb_channels, st->codecpar->sample_rate, ini);
  fflush(stdout);
  if (ini > 1.0) {
    int64_t alvo = inicioUs + (int64_t)(ini * AV_TIME_BASE);
    if (avformat_seek_file(ic, -1, INT64_MIN, alvo, alvo, AVSEEK_FLAG_BACKWARD) < 0)
      printf("[audsync-ff] seek failed: listening from the start\n");
  }
  pk = av_packet_alloc(); fr = av_frame_alloc();
  if (!pk || !fr) goto fim;
  while (vivo(t)) {
    if (!fimDemux) {
      r = av_read_frame(ic, pk);
      if (r == AVERROR_EOF) { fimDemux = 1; avcodec_send_packet(dc, NULL); }
      else if (r < 0) { if (vivo(t)) printf("[audsync-ff] read failed (%d)\n", r); break; }
      else {
        if (pk->stream_index == si) r = avcodec_send_packet(dc, pk);
        av_packet_unref(pk);
        if (r < 0 && r != AVERROR(EAGAIN) && r != AVERROR_INVALIDDATA) break;
      }
    }
    for (;;) {
      int64_t ptsQ;
      double pts;
      int n, max;
      r = avcodec_receive_frame(dc, fr);
      if (r == AVERROR(EAGAIN)) break;
      if (r == AVERROR_EOF) { printf("[audsync-ff] source ended before the window\n"); goto fim; }
      if (r < 0) break;   // one bad frame: the next packet goes on
      ptsQ = fr->best_effort_timestamp;
      if (ptsQ == AV_NOPTS_VALUE || fr->sample_rate <= 0) { av_frame_unref(fr); continue; }
      pts = (av_rescale_q(ptsQ, st->time_base, AV_TIME_BASE_Q) - inicioUs) / 1e6;
      if (!ancorado || fabs(pts - proximo) > SALTO_S || !swr) {
        // First frame, a gap/jump or a format change: a fresh resampler and a
        // new anchor (audsync sees the pts jump and restarts its window).
        swr_free(&swr);
        if (swr_alloc_set_opts2(&swr, &mono, AV_SAMPLE_FMT_S16, AUDVAD_HZ, &fr->ch_layout,
                                fr->format, fr->sample_rate, 0, NULL) < 0 || swr_init(swr) < 0) {
          printf("[audsync-ff] resampler setup failed\n"); goto fim;
        }
        ancorado = 1; ancoraUs = (int64_t)llround(pts * 1e6); saidas = 0;
      }
      proximo = pts + fr->nb_samples / (double)fr->sample_rate;
      max = (int)av_rescale_rnd(swr_get_delay(swr, fr->sample_rate) + fr->nb_samples, AUDVAD_HZ, fr->sample_rate, AV_ROUND_UP) + 32;
      if (max > cap) {
        int16_t *nb = realloc(saida, (size_t)max * sizeof *saida);
        if (!nb) goto fim;
        saida = nb; cap = max;
      }
      n = swr_convert(swr, (uint8_t **)&saida, cap, (const uint8_t **)fr->extended_data, fr->nb_samples);
      av_frame_unref(fr);
      if (n < 0) goto fim;
      if (n > 0) {
        if (!entregar(t, saida, n, ancoraUs + saidas * 1000000 / AUDVAD_HZ)) { ok = 1; goto fim; }
        saidas += n; total += n;
        pthread_mutex_lock(&FM); relato.amostras = total; pthread_mutex_unlock(&FM);
      }
    }
  }
  ok = !vivo(t);   // cancelled = on purpose; a read/decode failure is not
fim:
  printf("[audsync-ff] stopped after %.1fs of audio (%s)\n", total / (double)AUDVAD_HZ,
         ok ? "window done or cancelled" : "source failed");
  fflush(stdout);
  swr_free(&swr); av_frame_free(&fr); av_packet_free(&pk); avcodec_free_context(&dc); free(saida);
  return ok;
}

static void *trabalhar(void *u) {
  Tarefa *t = u;
  DtsEngine *e = dts_engine_create();
  int ok = 0;
  if (e) {
    pthread_mutex_lock(&FM);
    if (vivo(t)) engAtual = e; else dts_engine_cancel(e);
    pthread_mutex_unlock(&FM);
    ok = ouvir(e, t);
    pthread_mutex_lock(&FM);
    if (engAtual == e) engAtual = NULL;
    pthread_mutex_unlock(&FM);
    dts_engine_destroy(e);   // nobody else holds it: ligar() cancels only under FM via engAtual
  }
  if (!ok && vivo(t)) audsync_backend_falhou(t->pedido, AUDSYNC_M_JANELA);
  free(t);
  pthread_mutex_lock(&FM); ativos--; pthread_cond_broadcast(&FC); pthread_mutex_unlock(&FM);
  return NULL;
}

// audsync_passo, UI thread. Never blocks: the old worker is cancelled and exits
// on its own; a new one starts for the current request.
static void ligar(int on) {
  Tarefa *t;
  pthread_t th;
  pthread_attr_t at;
  unsigned g = atomic_fetch_add(&geracao, 1) + 1;
  pthread_mutex_lock(&FM);
  if (engAtual) { dts_engine_cancel(engAtual); engAtual = NULL; }
  pthread_mutex_unlock(&FM);
  if (!on) return;
  t = calloc(1, sizeof *t);
  if (!t) return;
  t->ger = g; t->pedido = audsync_status().pedido; t->f.ordinal = -1;
  if (!provedor || !provedor(&t->f) || strncmp(t->f.url, "http", 4)) {
    free(t);
    printf("[audsync-ff] nothing to listen to (no progressive source)\n"); fflush(stdout);
    audsync_backend_falhou(audsync_status().pedido, AUDSYNC_M_PLATAFORMA);
    return;
  }
  pthread_mutex_lock(&FM); ativos++; memset(&relato, 0, sizeof relato); relato.stream = -1; pthread_mutex_unlock(&FM);
  pthread_attr_init(&at);
  pthread_attr_setdetachstate(&at, PTHREAD_CREATE_DETACHED);
  if (pthread_create(&th, &at, trabalhar, t) != 0) {
    pthread_mutex_lock(&FM); ativos--; pthread_mutex_unlock(&FM);
    audsync_backend_falhou(t->pedido, AUDSYNC_M_MEMORIA);
    free(t);
  }
  pthread_attr_destroy(&at);
}

int audsyncff_disponivel(void) {
  return avcodec_find_decoder(AV_CODEC_ID_AC3) || avcodec_find_decoder(AV_CODEC_ID_AAC);
}

void audsyncff_registrar(AudsyncFfProvedor p) {
  provedor = p;
  if (registrado || !audsyncff_disponivel()) return;
  registrado = 1;
  audsync_backend(ligar);
  // Decoded here, never bitstream: capability is OK whatever the TV outputs.
  audsync_formato(AUDSYNC_FMT_PCM);
}

int audsyncff_parar(int ms) {
  struct timespec ts;
  int r = 0;
  atomic_fetch_add(&geracao, 1);
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += ms / 1000; ts.tv_nsec += (long)(ms % 1000) * 1000000L;
  if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
  pthread_mutex_lock(&FM);
  if (engAtual) { dts_engine_cancel(engAtual); engAtual = NULL; }
  while (ativos > 0 && r != ETIMEDOUT) r = pthread_cond_timedwait(&FC, &FM, &ts);
  r = ativos == 0;
  pthread_mutex_unlock(&FM);
  return r;
}

int audsyncff_teste_ativos(void) { int n; pthread_mutex_lock(&FM); n = ativos; pthread_mutex_unlock(&FM); return n; }
AudsyncFfRelato audsyncff_teste_relato(void) {
  AudsyncFfRelato r; pthread_mutex_lock(&FM); r = relato; pthread_mutex_unlock(&FM); return r;
}
#endif

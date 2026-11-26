// Subtitle sync "by audio" on the Samsung .tpk (audsync_ff.c): the software
// decoder backend over real HTTP (rede.c + DtsEngine) on generated sample files,
// through the whole session (audsync.c -> legsync.c -> autosync.c).
//   bash tests/audsync_ff.sh        (needs the host FFmpeg of build-dts-ffmpeg.sh, tpk target)
// The fixture: an MKV with a fake video track, a Portuguese AAC track with music
// only (default) and an English AC3 5.1 track with speech on the centre channel,
// the speech 3.0 s off the subtitle; the same with an English text track at the
// true timing (the embedded reference must win, audio unused); and an MP4 with
// one AAC track, -1.5 s off.
#include "audsync.h"
#include "audsync_ff.h"
#include "legsync.h"
#include "legref.h"
#include "autosync.h"
#include "audsync_sint.h"
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <unistd.h>

// --- stubs (legsyncui.c / legenda.c without SDL or the UI) ---------------------------
#include "legendasui.h"
void legendasui_definir_sync(const LegendasSyncProvider *p) { (void)p; }
const char *i18n(const char *s) { return s; }
void plrui_decimal(char *s) { (void)s; }
const char *player_id_canal(void) { return ""; }

static int casos;
#define CASO(nome) do { casos++; fprintf(stderr, "-- %s\n", nome); } while (0)

// --- fixture writer (libavformat to a FILE*: the test FFmpeg has no file protocol) ----
static int escreverCb(void *u, const uint8_t *b, int n) { return fwrite(b, 1, (size_t)n, u) == (size_t)n ? n : AVERROR(EIO); }
static int64_t buscarCb(void *u, int64_t o, int w) {
  if (w == AVSEEK_SIZE) return -1;
  return fseeko(u, o, w & ~AVSEEK_FORCE) ? -1 : ftello(u);
}

typedef struct { AVCodecContext *c; AVStream *s; AVFrame *f; int64_t n; double offset; int tipo, centro; } Trilha;

static int16_t amostra16(double t, double off, int tipo) {   // 16 kHz synthetic, linear interpolation
  int16_t a[2]; double i = floor(t * 16000.0), fr = t * 16000.0 - i;
  sint_pcm(a, 2, i / 16000.0, off, tipo);
  return (int16_t)(a[0] + (a[1] - a[0]) * fr);
}

static void abrirTrilha(AVFormatContext *oc, Trilha *t, enum AVCodecID id, AVChannelLayout lay, int br, const char *lang, int padrao) {
  const AVCodec *c = avcodec_find_encoder(id);
  assert(c);
  t->c = avcodec_alloc_context3(c);
  t->c->sample_rate = 48000; t->c->sample_fmt = AV_SAMPLE_FMT_FLTP; t->c->bit_rate = br;
  av_channel_layout_copy(&t->c->ch_layout, &lay);
  t->c->time_base = (AVRational){ 1, 48000 };
  if (oc->oformat->flags & AVFMT_GLOBALHEADER) t->c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  assert(avcodec_open2(t->c, c, NULL) == 0);
  t->s = avformat_new_stream(oc, NULL);
  avcodec_parameters_from_context(t->s->codecpar, t->c);
  t->s->time_base = t->c->time_base;
  av_dict_set(&t->s->metadata, "language", lang, 0);
  t->s->disposition = padrao ? AV_DISPOSITION_DEFAULT : 0;
  t->f = av_frame_alloc();
  t->f->nb_samples = t->c->frame_size; t->f->format = t->c->sample_fmt;
  av_channel_layout_copy(&t->f->ch_layout, &t->c->ch_layout); t->f->sample_rate = 48000;
  assert(av_frame_get_buffer(t->f, 0) == 0);
}

static void drenar(AVFormatContext *oc, Trilha *t, AVPacket *p) {
  while (avcodec_receive_packet(t->c, p) == 0) {
    av_packet_rescale_ts(p, t->c->time_base, t->s->time_base);
    p->stream_index = t->s->index;
    assert(av_interleaved_write_frame(oc, p) == 0);
  }
}

// One audio frame of track t: speech (centre only for 5.1) or music, plus noise.
static void quadro(AVFormatContext *oc, Trilha *t, AVPacket *p) {
  int i, ch, nch = t->c->ch_layout.nb_channels;
  av_frame_make_writable(t->f);
  for (i = 0; i < t->f->nb_samples; i++) {
    double s = (t->n + i) / 48000.0;
    float v = amostra16(s, t->offset, t->tipo) / 32768.0f, ruido = amostra16(s + 1000, 0, SINT_RUIDO) / 32768.0f;
    for (ch = 0; ch < nch; ch++)
      ((float *)t->f->data[ch])[i] = (t->centro < 0 || ch == t->centro) ? v : 0.3f * ruido;
  }
  t->f->pts = t->n; t->n += t->f->nb_samples;
  assert(avcodec_send_frame(t->c, t->f) == 0);
  drenar(oc, t, p);
}

// legEmbutida: an English S_TEXT/UTF8 track with the TRUE timing (speech time),
// i.e. the external subtitle's cues minus `offset`.
static void gerar(const char *arq, const char *formato, double dur, double offset, int comVideo, int legEmbutida) {
  AVFormatContext *oc = NULL;
  FILE *fp = fopen(arq, "wb");
  unsigned char *buf = av_malloc(65536);
  Trilha tr[2]; int nt = 0, k;
  AVStream *vs = NULL, *ls = NULL;
  int lq = 0;
  AVPacket *p = av_packet_alloc();
  int64_t vq = 0;
  assert(fp && buf && avformat_alloc_output_context2(&oc, NULL, formato, NULL) >= 0);
  oc->pb = avio_alloc_context(buf, 65536, 1, fp, NULL, escreverCb, buscarCb);
  memset(tr, 0, sizeof tr);
  if (comVideo) {   // opaque "H.264" payload, 25 fps, keyframe every second: real bytes to skip
    vs = avformat_new_stream(oc, NULL);
    vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO; vs->codecpar->codec_id = AV_CODEC_ID_H264;
    vs->codecpar->width = 320; vs->codecpar->height = 180; vs->time_base = (AVRational){ 1, 1000 };
    { static const uint8_t avcc[] = { 1, 0x42, 0, 0x1e, 0xff, 0xe1, 0, 4, 0x67, 0x42, 0, 0x1e, 1, 0, 2, 0x68, 0xce };
      vs->codecpar->extradata = av_mallocz(sizeof avcc + AV_INPUT_BUFFER_PADDING_SIZE);
      memcpy(vs->codecpar->extradata, avcc, sizeof avcc); vs->codecpar->extradata_size = sizeof avcc; }
  }
  if (comVideo) {
    abrirTrilha(oc, &tr[nt], AV_CODEC_ID_AAC, (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO, 128000, "por", 1);
    tr[nt].tipo = SINT_MUSICA | SINT_RUIDO; tr[nt].centro = -1; nt++;
    abrirTrilha(oc, &tr[nt], AV_CODEC_ID_AC3, (AVChannelLayout)AV_CHANNEL_LAYOUT_5POINT1, 384000, "eng", 0);
    tr[nt].tipo = SINT_FALA | SINT_RUIDO; tr[nt].centro = 2; tr[nt].offset = offset; nt++;
  } else {
    abrirTrilha(oc, &tr[nt], AV_CODEC_ID_AAC, (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO, 128000, "eng", 1);
    tr[nt].tipo = SINT_FALA | SINT_RUIDO; tr[nt].centro = -1; tr[nt].offset = offset; nt++;
  }
  if (legEmbutida) {
    ls = avformat_new_stream(oc, NULL);
    ls->codecpar->codec_type = AVMEDIA_TYPE_SUBTITLE; ls->codecpar->codec_id = AV_CODEC_ID_SUBRIP;
    ls->time_base = (AVRational){ 1, 1000 };
    av_dict_set(&ls->metadata, "language", "eng", 0);
  }
  { int w = avformat_write_header(oc, NULL); if (w < 0) fprintf(stderr, "write_header %s: %s\n", formato, av_err2str(w)); assert(w >= 0); }
  for (;;) {
    double ta = tr[0].n / 48000.0;
    for (k = 1; k < nt; k++) if (tr[k].n / 48000.0 < ta) ta = tr[k].n / 48000.0;
    if (ta >= dur) break;
    while (vs && vq * 40 <= ta * 1000 + 40) {
      uint8_t lixo[3000]; int i, n = vq % 25 ? 1200 : 3000;
      for (i = 0; i < n; i++) lixo[i] = (uint8_t)(vq * 31 + i * 7);
      av_new_packet(p, n); memcpy(p->data, lixo, (size_t)n);
      p->pts = p->dts = vq * 40; p->duration = 40; p->stream_index = vs->index;
      p->flags = vq % 25 ? 0 : AV_PKT_FLAG_KEY;
      assert(av_interleaved_write_frame(oc, p) == 0);
      vq++;
    }
    while (ls && lq < sintN && sintCues[lq].ini - offset <= ta) {
      char linha[64]; int n = snprintf(linha, sizeof linha, "Line %d says something new", lq + 1);
      double a = sintCues[lq].ini - offset;
      if (a >= 0) {
        av_new_packet(p, n); memcpy(p->data, linha, (size_t)n);
        p->pts = p->dts = (int64_t)llround(a * 1000); p->duration = (int64_t)llround((sintCues[lq].fim - sintCues[lq].ini) * 1000);
        p->stream_index = ls->index; p->flags = AV_PKT_FLAG_KEY;
        assert(av_interleaved_write_frame(oc, p) == 0);
      }
      lq++;
    }
    for (k = 0; k < nt; k++) if (tr[k].n / 48000.0 <= ta) quadro(oc, &tr[k], p);
  }
  for (k = 0; k < nt; k++) { avcodec_send_frame(tr[k].c, NULL); drenar(oc, &tr[k], p); }
  assert(av_write_trailer(oc) == 0);
  avio_flush(oc->pb);
  for (k = 0; k < nt; k++) { avcodec_free_context(&tr[k].c); av_frame_free(&tr[k].f); }
  av_free(oc->pb->buffer); avio_context_free(&oc->pb); avformat_free_context(oc); av_packet_free(&p);
  fclose(fp);
}

// --- the platform side (what video_tpk.c's provider fills) --------------------------
static char base[64], fonteUrl[256], sessaoUrl[256], fonteCab[64] = "X-Fixture: yes", fonteIdioma[8] = "en";
static double fontePos = 100;
static int provedorChamadas;
static const char *cabsFonte(void) { return fonteCab; }   // player.c: legref_fonte_cabecalhos(video_cabecalhos)
static int provedor(AudsyncFfFonte *f) {
  provedorChamadas++;
  if (!fonteUrl[0]) return 0;
  snprintf(f->url, sizeof f->url, "%s", fonteUrl);
  snprintf(f->cabecalhos, sizeof f->cabecalhos, "%s", fonteCab);
  snprintf(f->idioma, sizeof f->idioma, "%s", fonteIdioma);
  f->ordinal = -1; f->pos = fontePos;
  return 1;
}

static unsigned agora = 1000;
static void passo(void) { agora += 50; legsync_passo(sessaoUrl, fontePos, 60.0, 0, agora); }

static LegSyncVisao esperar(LegSyncFase f, int seg) {
  LegSyncVisao v;
  int i;
  for (i = 0; i < seg * 200; i++) { passo(); v = legsync_visao(0); if (v.fase == f) return v; usleep(5000); }
  fprintf(stderr, "expected phase %d, got %d reason %d\n", f, v.fase, v.motivo);
  assert(!"timeout");
  return v;
}

static int esperarOcioso(void) {
  int i;
  for (i = 0; i < 600 && audsyncff_teste_ativos() > 0; i++) usleep(5000);
  return audsyncff_teste_ativos() == 0;
}

static void abrir(const char *arquivo, int automatico) {
  char sub[128];
  snprintf(fonteUrl, sizeof fonteUrl, "%s/%s", base, arquivo);
  snprintf(sessaoUrl, sizeof sessaoUrl, "%s", fonteUrl);
  snprintf(sub, sizeof sub, "%s/legenda.srt", base);
  legsync_teste_auto(automatico);
  legsync_iniciar(sessaoUrl);
  legsync_audio_habilitar(1);
  legsync_primaria_externa(sub, "en", "OpenSubtitles");
}

int main(int argc, char **argv) {
  LegSyncVisao v;
  AudsyncFfRelato r;
  char txt[300], caminho[512];
  FILE *srt;
  assert(argc == 3);
  setvbuf(stdout, NULL, _IONBF, 0);   // backend logs interleave with the case lines
  snprintf(base, sizeof base, "http://127.0.0.1:%s", argv[2]);
  legref_fonte_cabecalhos(cabsFonte);

  sint_timeline(0, 600, 99);
  snprintf(caminho, sizeof caminho, "%s/legenda.srt", argv[1]);
  { char *s = sint_srt(); srt = fopen(caminho, "wb"); fputs(s, srt); fclose(srt); free(s); }
  snprintf(caminho, sizeof caminho, "%s/filme.mkv", argv[1]); gerar(caminho, "matroska", 430, 3.0, 1, 0);
  snprintf(caminho, sizeof caminho, "%s/filme.mp4", argv[1]); gerar(caminho, "mp4", 430, -1.5, 0, 0);
  snprintf(caminho, sizeof caminho, "%s/filme-leg.mkv", argv[1]); gerar(caminho, "matroska", 610, 3.0, 1, 1);   /* as long as the external subtitle */
  fprintf(stderr, "fixtures written\n");

  CASO("no backend yet: 'by audio' unavailable on this platform");
  assert(audsync_capacidade() == AUDSYNC_CAP_PLATAFORMA);
  abrir("filme.mkv", 0);
  { int i; for (i = 0; i < 400; i++) { passo(); v = legsync_visao(0); if (v.motivoAudio == LEGSYNC_M_AUD_PLATAFORMA) break; usleep(5000); } }
  legsync_texto(&v, txt, sizeof txt);
  fprintf(stderr, "   row: %s\n", txt);
  assert(!(v.acoes & LEGSYNC_ACAO_AUDIO) && v.motivoAudio == LEGSYNC_M_AUD_PLATAFORMA);
  assert(strstr(txt, "indispon\xc3\xadvel nesta plataforma"));

  CASO("FFmpeg present: backend registers, 'by audio' offered");
  assert(audsyncff_disponivel());
  audsyncff_registrar(provedor);
  assert(audsync_capacidade() == AUDSYNC_CAP_OK);
  v = esperar(LEGSYNC_PRONTA, 10);
  legsync_texto(&v, txt, sizeof txt);
  fprintf(stderr, "   row: %s\n", txt);
  assert((v.acoes & LEGSYNC_ACAO_AUDIO) && v.motivoAudio == LEGSYNC_M_NENHUM);

  CASO("MKV, English AC3 5.1 picked by language over the default Portuguese AAC: +3.0 s accepted");
  assert(legsync_acao(LEGSYNC_ACAO_AUDIO));
  passo();
  v = legsync_visao(0); assert(v.fase == LEGSYNC_OUVINDO);
  v = esperar(LEGSYNC_ACEITA, 120);
  r = audsyncff_teste_relato();
  fprintf(stderr, "   accepted %d ms (stream %d %s %dch %d Hz, %.1f s decoded)\n", v.offsetAutoMs, r.stream, r.codec,
          r.canais, r.taxa, r.amostras / 16000.0);
  assert(v.audio && abs(v.offsetAutoMs - 3000) <= 100);
  assert(r.stream == 2 && !strcmp(r.codec, "ac3") && r.canais == 6);
  assert(r.amostras >= AUDSYNC_ALVO_SEG * 16000LL && r.amostras < (AUDSYNC_ALVO_SEG + 20) * 16000LL);
  assert(esperarOcioso());
  assert(legsync_acao(LEGSYNC_ACAO_DESFAZER));

  CASO("automatic plan (AJ_LEG_SYNC_AUTO + AJ_LEG_SYNC_AUDIO), file without a text track: the embedded reference finds none, audio used");
  legsync_encerrar(); passo();
  abrir("filme.mkv", 1);
  v = esperar(LEGSYNC_ACEITA, 120);
  fprintf(stderr, "   automatic: %d ms\n", v.offsetAutoMs);
  assert(v.audio && abs(v.offsetAutoMs - 3000) <= 100);
  assert(esperarOcioso());

  CASO("automatic plan, file WITH an embedded English text track: synced against it, audio never started");
  legsync_encerrar(); passo();
  { int antes = provedorChamadas;
    abrir("filme-leg.mkv", 1);
    v = esperar(LEGSYNC_ACEITA, 120);
    fprintf(stderr, "   embedded reference: %d ms (audio=%d)\n", v.offsetAutoMs, v.audio);
    assert(!v.audio && abs(v.offsetAutoMs - 3000) <= 100);
    assert(provedorChamadas == antes && audsyncff_teste_ativos() == 0); }

  CASO("MP4 AAC near the end: the window is pulled back to fit, -1.5 s accepted");
  legsync_encerrar(); passo();
  fontePos = 400;
  abrir("filme.mp4", 0);
  esperar(LEGSYNC_PRONTA, 10);
  assert(legsync_acao(LEGSYNC_ACAO_AUDIO));
  v = esperar(LEGSYNC_ACEITA, 120);
  r = audsyncff_teste_relato();
  fprintf(stderr, "   accepted %d ms (%s)\n", v.offsetAutoMs, r.codec);
  assert(abs(v.offsetAutoMs + 1500) <= 100 && !strcmp(r.codec, "aac"));
  assert(esperarOcioso());
  fontePos = 100;

  CASO("cancel: player close mid-listen stops the decoder");
  legsync_encerrar(); passo();
  abrir("filme.mkv", 0);
  esperar(LEGSYNC_PRONTA, 10);
  assert(legsync_acao(LEGSYNC_ACAO_AUDIO)); passo();
  { int i; for (i = 0; i < 1000 && audsyncff_teste_relato().amostras < 16000 * 20; i++) { passo(); usleep(2000); } }
  assert(audsyncff_teste_ativos() == 1);
  { long long antes = audsyncff_teste_relato().amostras, depois;
    legsync_encerrar(); passo();
    assert(audsync_status().fase == AUDSYNC_PARADO);
    assert(esperarOcioso());
    depois = audsyncff_teste_relato().amostras;
    fprintf(stderr, "   %.1f s heard at close, %.1f s when the decoder exited\n", antes / 16000.0, depois / 16000.0);
    assert(depois - antes < 16000 * 2); }   // stops within a couple of deliveries

  CASO("cancel: Parar stops the decoder; listening again starts a fresh one");
  abrir("filme.mkv", 0);
  esperar(LEGSYNC_PRONTA, 10);
  assert(legsync_acao(LEGSYNC_ACAO_AUDIO)); passo();
  usleep(200000);
  assert(legsync_acao(LEGSYNC_ACAO_PARAR)); passo();
  assert(esperarOcioso());
  assert(legsync_acao(LEGSYNC_ACAO_AUDIO));
  v = esperar(LEGSYNC_ACEITA, 120);
  assert(abs(v.offsetAutoMs - 3000) <= 100);
  assert(esperarOcioso());

  CASO("source missing (404): refused, nothing changed, decoder gone");
  legsync_encerrar(); passo();
  abrir("filme.mkv", 0);
  esperar(LEGSYNC_PRONTA, 10);
  snprintf(fonteUrl + strlen(base), sizeof fonteUrl - strlen(base), "/sumiu.mkv");
  assert(legsync_acao(LEGSYNC_ACAO_AUDIO));
  v = esperar(LEGSYNC_RECUSADA, 30);
  assert(v.offsetAutoMs == 0 && legsync_offset_ms(0) == 0);
  assert(esperarOcioso());

  CASO("the playing URL's headers go with every request: without them the origin refuses");
  legsync_encerrar(); passo();
  abrir("filme.mkv", 0);
  esperar(LEGSYNC_PRONTA, 10);
  fonteCab[0] = 0;
  assert(legsync_acao(LEGSYNC_ACAO_AUDIO));
  v = esperar(LEGSYNC_RECUSADA, 30);
  snprintf(fonteCab, sizeof fonteCab, "X-Fixture: yes");
  assert(esperarOcioso());

  CASO("nothing to listen to (live channel, trailer): refused at once, no decoder");
  legsync_encerrar(); passo();
  abrir("filme.mkv", 0);
  esperar(LEGSYNC_PRONTA, 10);
  { int antes = provedorChamadas;
    fonteUrl[0] = 0;
    legsync_acao(LEGSYNC_ACAO_AUDIO);   // fails inside the request already: 0 is fine
    v = esperar(LEGSYNC_INDISPONIVEL, 5);
    assert(v.motivo == LEGSYNC_M_AUD_PLATAFORMA);
    assert(provedorChamadas == antes + 1 && audsyncff_teste_ativos() == 0); }

  legsync_encerrar(); passo();
  assert(audsyncff_parar(2000));
  legsync_destruir();
  fprintf(stderr, "audsync_ff: %d cases ok\n", casos);
  return 0;
}

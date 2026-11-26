// SOFTWARE AUDIO TAP for subtitle AutoSync "by audio" (audsync.h) on players
// that never hand decoded PCM to the app: the Samsung .tpk (AVPlay, host
// tizen-tpk/Video.cs). Instead of tapping the TV's player, a worker thread opens
// the SAME source a second time (DtsEngine's ranged HTTP reader, the playing
// URL's headers), demuxes the selected audio track from around the playback
// position, decodes it with the FFmpeg the .so already links for the DTS
// converter, and feeds mono 16 kHz int16 with media timestamps to audsync_pcm.
// The AVPlay playback path is not touched.
//
// Contract with audsync.c: the tap is armed/disarmed through the backend's
// ligar(1/0) (UI thread, from audsync_passo). The worker never overflows the
// ring (it waits for room, so nothing is dropped and the timeline stays
// contiguous), waits while the session is paused (seek/buffer), and stops as
// soon as the session generation changes (cancel, new request, window done).
// A source that cannot be opened or ends before the window fails the session.
//
// Only built where the .so links FFmpeg (the DTS converter flag, in the .c only:
// the Tizen 4/5 .so has none);
// without it audsyncff_disponivel() is 0 and nothing registers, so the
// subtitle sync row keeps "Por audio: indisponivel nesta plataforma".
#ifndef NV_AUDSYNC_FF_H
#define NV_AUDSYNC_FF_H

typedef struct {
  char url[8192];          // the source the player is playing (http/https)
  char cabecalhos[2048];   // "Name: value" lines, as video_definir_cabecalhos
  char idioma[16];         // language of the selected audio track ("" = unknown)
  int ordinal;             // n-th audio stream of the file (0-based), -1 = by language
  double pos;              // playback position in seconds: the window starts here
} AudsyncFfFonte;

// Fills `f` with the source playing now; 0 = nothing to listen to (no source,
// live channel, trailer). Called on the UI thread when the tap is armed.
typedef int (*AudsyncFfProvedor)(AudsyncFfFonte *f);

int  audsyncff_disponivel(void);
// Registers the audsync backend (and PCM format) once; no-op without FFmpeg.
void audsyncff_registrar(AudsyncFfProvedor provedor);
// Stops the worker (if any) and waits up to `ms` for it to exit. 1 = idle.
int  audsyncff_parar(int ms);

// Tests: decoder threads alive now, and bytes/samples of the last run.
int  audsyncff_teste_ativos(void);
typedef struct { long long amostras; int stream; int canais, taxa; char codec[24]; } AudsyncFfRelato;
AudsyncFfRelato audsyncff_teste_relato(void);
#endif

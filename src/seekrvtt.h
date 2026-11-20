// SEEKR (seekr.tv): a parte PURA — ler a resposta do /sprites, ler o WebVTT
// das miniaturas e escolher a miniatura de uma posicao. Sem rede, sem SDL, sem
// GL, para o teste rodar sozinho (tests/seekr.sh). O resto mora em seekr.c.
//
// O formato, conferido no SDK oficial (github.com/AKhalil609/seekr-android-sdk,
// seekr-core/.../internal/Vtt.kt e LookupModels.kt):
//
//   GET https://api.seekr.tv/sprites?duration_ms=..&imdb_id=tt..      (filme)
//   GET https://api.seekr.tv/sprites?duration_ms=..&show_imdb_id=tt..&season=S&episode=E
//   cabecalho X-API-Key: <chave pessoal>
//   200 {"vtt_url":"https://sprites.seekr.tv/...vtt?sig=..","scale":1.0,
//        "source_duration_ms":7200000, ...}
//   401 {"error":"invalid or missing API key"}               (medido 02/10/2026)
//
// O VTT (sem chave, assinado) e um WEBVTT comum: uma linha "INICIO --> FIM" e
// a linha seguinte com a URL da FOLHA de miniaturas e o fragmento
// "#xywh=x,y,w,h" do recorte de 320x180 dentro dela.
#ifndef NV_SEEKRVTT_H
#define NV_SEEKRVTT_H
#include <stddef.h>

typedef struct {
  long ini, fim;          // ms
  int  folha;             // indice em SeekrVtt.folhas
  int  x, y, w, h;        // recorte dentro da folha, em pixels
} SeekrCue;

typedef struct {
  SeekrCue *cues; int nCues;
  char **folhas;  int nFolhas;   // URLs absolutas, sem o fragmento
} SeekrVtt;

// vtt_url da resposta do /sprites, ja absoluta (relativa ganha
// https://sprites.seekr.tv na frente, como a pagina do servico diz). 1 se leu.
int  seekr_ler_lookup(const char *json, char *vtt, size_t tam);
// Monta a URL do /sprites. `t`/`e` zero = filme. 1 se coube.
int  seekr_url_lookup(char *dst, size_t tam, const char *imdb, int t, int e,
                      long durMs);
// Le o VTT inteiro. Devolve o numero de cues (0 = nada aproveitavel). As cues
// saem em ordem de inicio. Liberar com seekr_vtt_liberar.
int  seekr_vtt_ler(const char *vtt, SeekrVtt *out);
void seekr_vtt_liberar(SeekrVtt *v);
// Indice da cue para `posMs`, ou -1 sem cues. A cue que CONTEM a posicao so e
// a mais proxima na primeira metade dela (o quadro foi tirado no INICIO);
// passada a metade vale a seguinte — a regra do "Preview accuracy" da
// documentacao. Antes da primeira, a primeira; depois da ultima, a ultima.
int  seekr_vtt_cue(const SeekrVtt *v, long posMs);

// WHERE A SKIP LANDS when a preview was on screen (player.c, terminarSalto):
// at `previa`, the video time of the picture shown (its slot's start minus
// the sync setting), so the landing frame is the frame that was previewed.
// Only within one slot (15 s) of the raw position `pos` and inside the video;
// otherwise, or with no preview (previa < 0), the raw position.
static inline double seekr_pouso(double previa, double pos, double dur) {
  double d = previa - pos;
  if (previa < 0.0 || d > 15.0 || d < -15.0 || (dur > 0.0 && previa >= dur)) return pos;
  return previa;
}
#endif

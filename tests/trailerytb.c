// tests/trailerytb.sh: the pure parts of the YouTube trailer extractor
// (trailerytb.h), and, with NUVIO_YT_VIVO=<video id>, one real extraction
// against YouTube from this machine (not part of the suite: it needs the
// network and YouTube's goodwill).
#include "trailerytb.h"
#include "rede.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
  // ids
  assert(trailerytb_id_valido("Way9Dexny3w"));
  assert(trailerytb_id_valido("a-b_c123XYZ"));
  assert(!trailerytb_id_valido("Way9Dexny3"));      // 10
  assert(!trailerytb_id_valido("Way9Dexny3wx"));    // 12
  assert(!trailerytb_id_valido("Way9Dexny3!"));
  assert(!trailerytb_id_valido(NULL) && !trailerytb_id_valido(""));

  // watch page config
  { char k[96], v[256];
    const char *html = "...\"INNERTUBE_API_KEY\":\"AIzaKEY123\",...\"VISITOR_DATA\":\"Cgt2aXNpdG9y\"...";
    assert(trailerytb_config_da_pagina(html, k, sizeof k, v, sizeof v));
    assert(!strcmp(k, "AIzaKEY123") && !strcmp(v, "Cgt2aXNpdG9y"));
    assert(!trailerytb_config_da_pagina("<html>consent</html>", k, sizeof k, v, sizeof v));
    assert(!k[0] && !v[0]); }

  // player responses
  { TytbResposta r;
    const char *ok =
      "{\"playabilityStatus\":{\"status\":\"OK\"},"
      "\"streamingData\":{\"expiresInSeconds\":\"21540\","
      "\"formats\":["
      "{\"itag\":18,\"url\":\"https://rr1---sn-a.googlevideo.com/videoplayback?expire=1790000000\\u0026itag=18\","
      "\"mimeType\":\"video/mp4; codecs=\\\"avc1.42001E, mp4a.40.2\\\"\",\"height\":360,\"fps\":25,\"bitrate\":500000},"
      "{\"itag\":22,\"signatureCipher\":\"s=abc\\u0026url=https%3A%2F%2Fx\",\"mimeType\":\"video/mp4\",\"height\":720},"
      "{\"itag\":43,\"url\":\"https://x.googlevideo.com/videoplayback?itag=43\\u0026n=zzz\",\"mimeType\":\"video/webm\",\"height\":360,\"fps\":25,\"bitrate\":500000}"
      "],"
      "\"hlsManifestUrl\":\"https://manifest.googlevideo.com/api/manifest/hls_variant/expire/1790000123/id/x/file/index.m3u8\""
      "}}";
    trailerytb_ler_resposta(ok, &r);
    assert(!strcmp(r.status, "OK") && !r.botCheck && !r.ageCheck);
    assert(strstr(r.hls, "manifest.googlevideo.com") && strstr(r.hls, "index.m3u8"));
    // the ciphered 720p is skipped; of the two 360p, the one without "n" wins,
    // and the JSON escape & is decoded
    assert(r.progAltura == 360 && strstr(r.progressivo, "itag=18") && strstr(r.progressivo, "&itag"));
    assert(trailerytb_expira(r.progressivo) == 1790000000L);
    assert(trailerytb_expira(r.hls) == 1790000123L);
    assert(trailerytb_expira("https://example.com/a.mp4") == 0);

    trailerytb_ler_resposta("{\"playabilityStatus\":{\"status\":\"LOGIN_REQUIRED\","
                            "\"reason\":\"Sign in to confirm you\\u2019re not a bot\"}}", &r);
    assert(!strcmp(r.status, "LOGIN_REQUIRED") && r.botCheck && !r.ageCheck && !r.hls[0]);
    trailerytb_ler_resposta("{\"playabilityStatus\":{\"status\":\"LOGIN_REQUIRED\","
                            "\"reason\":\"Sign in to confirm your age\"}}", &r);
    assert(r.ageCheck && !r.botCheck);
    trailerytb_ler_resposta("{\"playabilityStatus\":{\"status\":\"LOGIN_REQUIRED\","
                            "\"reason\":\"This video is private\"}}", &r);
    assert(!r.ageCheck && !r.botCheck);
    trailerytb_ler_resposta(NULL, &r);
    assert(!r.status[0] && !r.hls[0] && !r.progressivo[0]); }

  // HLS master: best variant by height, then bandwidth
  { long bw;
    const char *m =
      "#EXTM3U\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=300000,CODECS=\"avc1.4D400C,mp4a.40.5\",RESOLUTION=256x144\n"
      "https://x/144.m3u8\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=4000000,CODECS=\"avc1.640028,mp4a.40.2\",RESOLUTION=1920x1080\n"
      "https://x/1080.m3u8\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=4500000,CODECS=\"vp09.00.40.08,mp4a.40.2\",RESOLUTION=1920x1080\n"
      "https://x/1080vp9.m3u8\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=9000000,RESOLUTION=1280x720\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=900000,RESOLUTION=640x360\n"
      "https://x/360.m3u8\n";
    assert(trailerytb_hls_melhor(m, &bw) == 1080 && bw == 4500000);
    assert(trailerytb_hls_melhor("#EXTM3U\n", &bw) == 0 && bw == 0);
    assert(trailerytb_hls_melhor(NULL, &bw) == 0); }

  // The playlist the TV gets: one H.264 variant up to 1080p, its group's
  // original audio (not the first dub), no captions (20/11/2026).
  { char o[4096];
    int alt;
    const char *m =
      "#EXTM3U\n#EXT-X-INDEPENDENT-SEGMENTS\n"
      "#EXT-X-MEDIA:URI=\"https://a/ar.m3u8\",TYPE=AUDIO,GROUP-ID=\"234\",LANGUAGE=\"ar\",NAME=\"Arabic - dubbed\",DEFAULT=NO,AUTOSELECT=YES\n"
      "#EXT-X-MEDIA:URI=\"https://a/en.m3u8\",TYPE=AUDIO,GROUP-ID=\"234\",LANGUAGE=\"en-US\",NAME=\"English - original\",DEFAULT=YES,AUTOSELECT=YES\n"
      "#EXT-X-MEDIA:URI=\"https://a/he.m3u8\",TYPE=AUDIO,GROUP-ID=\"233\",NAME=\"Default\",DEFAULT=YES,AUTOSELECT=YES\n"
      "#EXT-X-MEDIA:URI=\"https://a/vtt.m3u8\",TYPE=SUBTITLES,GROUP-ID=\"vtt\",LANGUAGE=\"en\",NAME=\"English\",DEFAULT=NO,AUTOSELECT=YES\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=135417,CODECS=\"avc1.4D400C,mp4a.40.5\",RESOLUTION=256x144,AUDIO=\"233\",SUBTITLES=\"vtt\"\n"
      "https://v/144.m3u8\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=3295980,CODECS=\"avc1.640028,mp4a.40.2\",RESOLUTION=1920x1080,AUDIO=\"234\",SUBTITLES=\"vtt\"\n"
      "https://v/1080.m3u8\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=3847963,CODECS=\"vp09.00.40.08,mp4a.40.2\",RESOLUTION=1920x1080,AUDIO=\"234\"\n"
      "https://v/1080vp9.m3u8\n"
      "#EXT-X-STREAM-INF:BANDWIDTH=9000000,CODECS=\"avc1.640033,mp4a.40.2\",RESOLUTION=3840x2160,AUDIO=\"234\"\n"
      "https://v/2160.m3u8\n";
    assert(trailerytb_hls_reduzir(m, o, sizeof o, &alt) == 1 && alt == 1080);
    assert(strstr(o, "https://v/1080.m3u8") && !strstr(o, "144.m3u8") && !strstr(o, "vp9") && !strstr(o, "2160"));
    assert(strstr(o, "https://a/en.m3u8") && !strstr(o, "ar.m3u8") && !strstr(o, "he.m3u8"));
    assert(!strstr(o, "SUBTITLES") && !strstr(o, "vtt"));
    assert(strstr(o, "DEFAULT=YES,AUTOSELECT=YES\n") && strstr(o, "#EXT-X-INDEPENDENT-SEGMENTS"));
    assert(!strstr(o, "DEFAULT=NO"));
    // Without its audio rendition, or without H.264, the master stays as is.
    assert(trailerytb_hls_reduzir("#EXTM3U\n#EXT-X-STREAM-INF:BANDWIDTH=1,CODECS=\"avc1.6\",RESOLUTION=1x720,AUDIO=\"9\"\nhttps://v/x\n",
                                  o, sizeof o, &alt) == 0);
    assert(trailerytb_hls_reduzir("#EXTM3U\n#EXT-X-STREAM-INF:BANDWIDTH=1,CODECS=\"vp09.0\",RESOLUTION=1x720\nhttps://v/x\n",
                                  o, sizeof o, &alt) == 0);
    // Muxed variant (no AUDIO group): kept alone.
    assert(trailerytb_hls_reduzir("#EXTM3U\n#EXT-X-STREAM-INF:BANDWIDTH=1,CODECS=\"avc1.6,mp4a.40.2\",RESOLUTION=1x720\nhttps://v/m\n",
                                  o, sizeof o, &alt) == 1 && alt == 720 && strstr(o, "https://v/m\n"));
    assert(trailerytb_hls_reduzir(m, o, 64, &alt) == 0);   // too small: refuse, never cut
    assert(trailerytb_hls_reduzir(NULL, o, sizeof o, &alt) == 0); }

  puts("PASS: trailerytb parsing (ids, watch config, player responses, HLS variants, reduced playlist, expiry)");

  { const char *vivo = getenv("NUVIO_YT_VIVO");
    if (vivo && *vivo) {
      int i;

      trailerytb_pedir(vivo);
      for (i = 0; i < 300 && !trailerytb_respondeu(vivo); i++) usleep(100000);
      printf("live %s: %s\n", vivo, trailerytb_url(vivo) ? "PLAYABLE URL" : "nothing");
      if (trailerytb_url(vivo)) printf("  %.90s...\n", trailerytb_url(vivo));
      return trailerytb_url(vivo) ? 0 : 1;
    } }
  return 0;
}

/* Real stream-to-filter metadata conversion. No account, provider or network. */
#include "streams.h"
#include "fontefiltro.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  Stream s = {0}; FonteFiltroDados d;
  snprintf(s.arquivo,sizeof s.arquivo,
    "Sample.2160p.BluRay.REMUX.HDR10.DV.TrueHD.Atmos.7.1.HEVC.[en]-TESTGROUP.mkv");
  snprintf(s.rotulo,sizeof s.rotulo,"Addon 4K Cached");
  snprintf(s.descricao,sizeof s.descricao,"Original addon description");
  s.altura=2160;s.tamanhoBytes=UINT64_C(4000000000);
  Stream original=s;
  stream_filtro_dados(&s,&d);
  const int options[]={0,0,0,0,0,1,0};
  for(int category=0;category<FONTEFILTRO_N;category++) {
    uint64_t expected=fontefiltro_bit(category,options[category]);
    assert(d.valores[category]&expected);
    FonteFiltroConfig c={0};c.exigir[category]=expected;
    assert(fontefiltro_permite(&c,&d));
    c.exigir[category]=0;c.preferir[category]=expected;
    assert(fontefiltro_pontuacao(&c,&d)>0);
    c.preferir[category]=0;c.excluir[category]=expected;
    assert(!fontefiltro_permite(&c,&d));
  }
  assert(!memcmp(&s,&original,sizeof s));
  assert(d.tamanhoBytes==UINT64_C(4000000000));
  assert(!strcmp(d.grupo,"TESTGROUP"));
  FonteFiltroConfig c={0};strcpy(c.gruposObrigatorios,"testgroup");
  assert(fontefiltro_permite(&c,&d));
  strcpy(c.grupos,"testgroup");assert(!fontefiltro_permite(&c,&d));
  puts("PASS real Stream filename/label -> all seven filter dimensions and release groups; source unchanged");

  memset(&s,0,sizeof s);s.altura=720;s.tamanhoMB=1024;
  stream_filtro_dados(&s,&d);
  assert(d.valores[FONTEFILTRO_RESOLUCAO]==fontefiltro_bit(FONTEFILTRO_RESOLUCAO,3));
  assert(d.tamanhoBytes==UINT64_C(1073741824));
  for(int category=1;category<FONTEFILTRO_N;category++)assert(!d.valores[category]);
  c=(FonteFiltroConfig){.minGB=2};assert(!fontefiltro_permite(&c,&d));
  c=(FonteFiltroConfig){.maxGB=2};assert(fontefiltro_permite(&c,&d));
  puts("PASS structured height and MiB fallback reach actual resolution and size controls");

  memset(&s,0,sizeof s);
  snprintf(s.url,sizeof s.url,"https://media.invalid/en/2160p/hdr/hevc/file");
  stream_filtro_dados(&s,&d);
  for(int category=0;category<FONTEFILTRO_N;category++)assert(!d.valores[category]);
  c=(FonteFiltroConfig){0};assert(fontefiltro_permite(&c,&d));
  c.excluir[FONTEFILTRO_LINGUAS]=fontefiltro_desconhecido(FONTEFILTRO_LINGUAS);
  assert(!fontefiltro_permite(&c,&d));
  s.filtro.naoVod=1;stream_filtro_dados(&s,&d);
  assert(d.naoVod&&fontefiltro_permite(&c,&d));
  s.filtroLido=1;s.filtro=d;s.filtro.valores[FONTEFILTRO_ENCODE]=1;
  stream_filtro_dados(&s,&d);assert(!memcmp(&d,&s.filtro,sizeof d));
  stream_filtro_dados(NULL,&d);
  FonteFiltroDados empty={0};assert(!memcmp(&d,&empty,sizeof d));
  puts("PASS URL text is not media metadata; Unknown/live/cached-classification/null handling preserved");
  return 0;
}

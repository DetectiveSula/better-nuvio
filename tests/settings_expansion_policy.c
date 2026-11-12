#include "settings_expansion_policy.h"
#include "card_depth_policy.h"
#include "fontefiltro.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static void dimensions(void){
 for(int category=0;category<FONTEFILTRO_N;category++){
  int options=fontefiltro_opcoes_n(category);
  for(int option=0;option<options;option++){
   FonteFiltroConfig c={0};FonteFiltroDados yes={0},no={0};
   uint64_t bit=fontefiltro_bit(category,option);
   yes.valores[category]=bit;no.valores[category]=fontefiltro_bit(category,(option+1)%options);
   c.exigir[category]=bit;assert(fontefiltro_config_validar(&c));
   assert(fontefiltro_permite(&c,&yes));assert(!fontefiltro_permite(&c,&no));
   c.exigir[category]=0;c.excluir[category]=bit;
   assert(!fontefiltro_permite(&c,&yes));assert(fontefiltro_permite(&c,&no));
   c.excluir[category]=0;c.preferir[category]=bit;
   assert(fontefiltro_pontuacao(&c,&yes)==1);assert(fontefiltro_pontuacao(&c,&no)==0);
   FonteFiltroDados sources[]={no,yes};int out[2];
   assert(fontefiltro_selecionar(&c,sources,2,NULL,out)==2);assert(out[0]==1&&out[1]==0);
   printf("PASS dimension=%d choice=%d preferred/required/excluded: %s\n",category,option,fontefiltro_opcao(category,option));
  }
 }
}
/* Classify real release-name tokens before filtering: masks alone cannot prove
   that a source carrying the advertised tag actually matches the control. */
static void parsed_dimensions(void){
 static const char *const samples[FONTEFILTRO_N][16]={
  {"2160p","1440p","1080p","720p","576p","480p","360p",""},
  {"BluRay REMUX","BluRay","WEB-DL","WEBRip","HDRip","HC HD-Rip","DVDRip","HDTV","CAM","TS","TC","SCR",""},
  {"HDR DV","DV","HDR","HDR10+","HDR10","DV","HDR","HLG","10bit","3D","IMAX","AI","SDR","H-OU","H-SBS",""},
  {"Atmos","DD+","DD","DTS:X","DTS-HD MA","DTS-HD","DTS-ES","DTS","TrueHD","OPUS","FLAC","AAC",""},
  {"7.1","6.1","5.1","2.0",""},
  {"AV1","HEVC","AVC","XviD","DivX",""},
  {"en","hi","it","es","fr","de","pt","pl","cs","es-419","ja","ko","zh","multi",""}
 };
 for(int category=0;category<FONTEFILTRO_N;category++){
  for(int option=0;option<fontefiltro_opcoes_n(category);option++){
   char filename[200];
   // Short language codes must be explicit tags, not arbitrary title words
   // (for example, the film "It" must not be classified as Italian).
   snprintf(filename,sizeof filename,category==FONTEFILTRO_LINGUAS ?
            "Sample.1080p.[%s].mkv" : "Sample.%s.mkv",samples[category][option]);
   FonteFiltroEntrada input={.filename=filename};FonteFiltroDados parsed={0};
   fontefiltro_classificar(&input,&parsed);
   uint64_t expected=fontefiltro_bit(category,option);
   if(option==fontefiltro_opcoes_n(category)-1)assert(!parsed.valores[category]);
   else assert(parsed.valores[category]&expected);
   FonteFiltroConfig c={0};c.exigir[category]=expected;
   assert(fontefiltro_permite(&c,&parsed));
   c.exigir[category]=0;c.preferir[category]=expected;
   assert(fontefiltro_pontuacao(&c,&parsed)>0);
   c.preferir[category]=0;c.excluir[category]=expected;
   assert(!fontefiltro_permite(&c,&parsed));
   printf("PASS parsed release dimension=%d choice=%d: preferred/required/excluded\n",category,option);
  }
 }
 FonteFiltroEntrada title={.filename="It.2017.mkv"};FonteFiltroDados parsed;
 fontefiltro_classificar(&title,&parsed);
 assert(!parsed.valores[FONTEFILTRO_LINGUAS]);
}
static void limits(void){
 FonteFiltroDados d[6]={0};int out[6];FonteFiltroConfig c={0};
 for(int i=0;i<6;i++){d[i].tamanhoBytes=(uint64_t)(i+1)*1000000000;d[i].valores[0]=1ULL<<(i%2);d[i].valores[1]=1ULL<<(i%3);}
 for(unsigned limit=0;limit<=10000;limit++){
  c=(FonteFiltroConfig){0};c.maxResultados=limit;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==(limit&&limit<6?(int)limit:6));
  c=(FonteFiltroConfig){0};c.porResolucao=limit;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==(limit&&limit<3?(int)limit*2:6));
  c=(FonteFiltroConfig){0};c.porQualidade=limit;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==(limit&&limit<2?(int)limit*3:6));
 }
 for(unsigned gb=0;gb<=1000;gb++){
  c=(FonteFiltroConfig){0};c.minGB=gb;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==(!gb?6:gb>6?0:7-(int)gb));
  c=(FonteFiltroConfig){0};c.maxGB=gb;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==(!gb||gb>6?6:(int)gb));
 }
 c=(FonteFiltroConfig){0};
 d[0].valores[0]=d[1].valores[0]=0;c.porResolucao=1;
 assert(fontefiltro_selecionar(&c,d,6,NULL,out)==3); /* Unknown has its own quota. */
 d[0].valores[0]=1;d[1].valores[0]=2;c=(FonteFiltroConfig){0};
 c.maxResultados=3;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==3);
 c.maxResultados=0;c.porResolucao=1;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==2);
 c.porResolucao=0;c.porQualidade=1;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==3);
 c.porQualidade=0;c.minGB=2;c.maxGB=4;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==3);assert(out[0]==1&&out[2]==3);
 d[0].tamanhoBytes=0;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==4); /* missing sizes retained */
 c=(FonteFiltroConfig){0};for(unsigned sort=0;sort<6;sort++){c.ordenar=sort;assert(fontefiltro_selecionar(&c,d,6,NULL,out)==6);if(sort==3)assert(out[0]==1&&out[5]==0);if(sort==4)assert(out[0]==5&&out[5]==0);}
 unsigned char eligible[]={1,0,1,0,1,0};c=(FonteFiltroConfig){0};assert(fontefiltro_selecionar(&c,d,6,eligible,out)==3);assert(out[0]==0&&out[1]==2&&out[2]==4);
 strcpy(c.gruposObrigatorios,"test");strcpy(d[0].grupo,"TEST");assert(fontefiltro_permite(&c,&d[0]));assert(!fontefiltro_permite(&c,&d[1]));strcpy(c.grupos,"test");assert(!fontefiltro_permite(&c,&d[0]));
 d[0].naoVod=1;assert(fontefiltro_permite(&c,&d[0]));
 puts("PASS sizes/six sort modes/quotas/eligibility/required+excluded groups/live bypass");
}
int main(void){
 for(unsigned i=0;i<sizeof nvWidthPresets/sizeof *nvWidthPresets;i++)assert(nv_settings_preset(nvWidthPresets,6,nvWidthPresets[i].value));
 for(unsigned i=0;i<5;i++)assert(nv_settings_preset(nvRadiusPresets,5,nvRadiusPresets[i].value));
 for(unsigned i=0;i<3;i++){assert(nv_settings_preset(nvEdgePresets,3,nvEdgePresets[i].value));assert(nv_settings_preset(nvCoveragePresets,3,nvCoveragePresets[i].value));}
 for(int s=0;s<NV_CARD_SURFACES;s++)for(unsigned mask=0;mask<32;mask++){assert(nv_card_depth_enabled(1,mask,s)==!!(mask&(1u<<s)));assert(!nv_card_depth_enabled(0,mask,s));}
 for(int t=0;t<=300;t++)assert(nv_settings_delay_ms(t,25)==(unsigned)t*100);
 assert(nv_settings_delay_ms(-1,25)==2500);assert(nv_settings_delay_ms(301,25)==2500);
 for(int p=1;p<=100;p++){assert(!nv_settings_threshold(p*10-1,1000,p));assert(nv_settings_threshold(p*10,1000,p));}
 assert(!nv_settings_threshold(100,0,50));assert(!nv_settings_threshold(100,100,0));
 for(int n=0;n<=20;n++)for(unsigned count=0;count<=21;count++)assert(nv_settings_still_due(count,n)==(n>0&&count>=(unsigned)n));
 dimensions();parsed_dimensions();limits();
 {int n=20000;FonteFiltroDados *d=calloc((size_t)n,sizeof *d);int *out=malloc((size_t)n*sizeof *out);assert(d&&out);
  for(int i=0;i<n;i++)d[i].tamanhoBytes=(uint64_t)(n-i+1)/2;
  FonteFiltroConfig c={0};c.ordenar=3;c.maxResultados=10000;
  assert(fontefiltro_selecionar(&c,d,n,NULL,out)==10000);
  for(int i=1;i<10000;i++){assert(d[out[i-1]].tamanhoBytes<=d[out[i]].tamanhoBytes);if(d[out[i-1]].tamanhoBytes==d[out[i]].tamanhoBytes)assert(out[i-1]<out[i]);}
  free(d);free(out);puts("PASS 20,000 sources: stable size order, 10,000-result cap, no arbitrary source truncation");}
 puts("PASS all requested preset values, five surface masks, delays, percentages and episode intervals");return 0;
}

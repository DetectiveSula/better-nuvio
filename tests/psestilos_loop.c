// Exercise production draw calls across row and complete-sequence seams.
#define main light_regression_main
#include "psestilos_luz_barra.c"
#undef main
#include "../src/psestilos.c"

int main(void) {
  assert(light_regression_main()==0);
  char urls[PSEST_PAREDE_MAX][32];const char *mural[PSEST_PAREDE_MAX];
  for(int i=0;i<PSEST_PAREDE_MAX;i++) {
    snprintf(urls[i],sizeof urls[i],"poster-%02d",i);mural[i]=savedWallUrls[i]=urls[i];
  }
  for(int profile=2;profile<=8;profile++) {
    PSCena c={0};c.perfil=profile;c.mural=mural;c.muralN=PSEST_PAREDE_MAX;
    savedWallCount=profile==2 ? 12 : 0;
    psestilos_iniciar();psestilos_atualizar(.016f,&c,PS_FUNDO_FILMES);
    assert(parAtual.perfil==profile && parAtual.n==PSEST_PAREDE_MAX);
    for(int i=0;i<parAtual.n;i++)for(int j=0;j<i;j++)assert(strcmp(parAtual.url[i],parAtual.url[j]));
  }
  PSCena late={0};late.perfil=2;late.mural=mural;savedWallCount=12;
  psestilos_iniciar();psestilos_atualizar(.016f,&late,PS_FUNDO_FILMES);assert(parAtual.n==12);
  late.muralN=PSEST_PAREDE_MAX;psestilos_atualizar(.016f,&late,PS_FUNDO_FILMES);
  assert(parAtual.n==PSEST_PAREDE_MAX && parTemAnt);
  late.semParede=1;int reads=savedWallReads;
  psestilos_atualizar(.016f,&late,PS_FUNDO_FILMES);assert(savedWallReads==reads);
  savedWallCount=0;
  puts("PASS: profile 2 legacy cache, future profiles, late catalog and PIN fallback fill unique 36-title walls");
  for(int count=1;count<=PSEST_PAREDE_MAX;count++) {
    ParSet set={0};set.n=count;
    for(int i=0;i<count;i++)set.tex[i]=i+1;
    for(int speed=20;speed<=36;speed+=8) {
      for(int epoch=0;epoch<2;epoch++) for(int seam=0;seam<=count*2;seam++) {
        RectChamado before[256];int nb;
        tempo=(double)(seam+epoch*count*10000)*(PAR_CH+PAR_GAP)/speed-.0001;
        nRects=nCores=0;parDesenhar(&set,1);
        nb=nRects;memcpy(before,rects,nb*sizeof *before);
        tempo+=.0002;nRects=nCores=0;parDesenhar(&set,1);
        if(count>=PAR_COLS) for(int i=0;i<nRects;i++) for(int j=0;j<i;j++) {
          // Neighbouring columns cannot display the same title at any height,
          // regardless of their direction, speed or position in the loop.
          if(fabsf(rects[i].r.x-rects[j].r.x)>1)
            assert(rects[i].tex!=rects[j].tex);
        }
        if(count>=PAR_COLS*3) for(int i=0;i<nRects;i++) for(int j=0;j<i;j++) {
          // A normal wall has enough art for at least three visible rows.
          // Those rows must not stack copies of the same poster in a column.
          if(fabsf(rects[i].r.x-rects[j].r.x)<1 &&
             fabsf(rects[i].r.y-rects[j].r.y)<3*(PAR_CH+PAR_GAP)-1)
            assert(rects[i].tex!=rects[j].tex);
        }
        for(int i=0;i<nb;i++) {
          GfxRect r=before[i].r;
          if(r.x<200 || r.x>1500 || r.y<0 || r.y>800)continue;
          int found=0;
          for(int j=0;j<nRects;j++)
            if(rects[j].tex==before[i].tex && fabsf(rects[j].r.x-r.x)<.01f &&
               fabsf(rects[j].r.y-r.y)<.02f)found=1;
          if(!found) fprintf(stderr,"count=%d speed=%d seam=%d texture=%u x=%f y=%f\n",count,speed,seam,before[i].tex,r.x,r.y);
          assert(found); // artwork identity and position both cross continuously
        }
      }
    }
  }
  puts("PASS: seamless poster loops; distinct column title pools whenever at least 9 titles are available");
  return 0;
}

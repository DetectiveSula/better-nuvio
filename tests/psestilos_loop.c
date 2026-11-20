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
  { // A poster that fails to load leaves the wall instead of staying a blank
    // card (owner's S90C, 14 Nov 2026): the wall is reassembled without it,
    // cross-fades, and never asks for it again.
    char more[PSEST_PAREDE_MAX+4][32];const char *big[PSEST_PAREDE_MAX+4];
    for(int i=0;i<PSEST_PAREDE_MAX+4;i++){snprintf(more[i],sizeof more[i],"title-%02d",i);big[i]=more[i];}
    PSCena f={0};f.perfil=5;f.mural=big;f.muralN=PSEST_PAREDE_MAX+4;
    failedUrl=NULL;psestilos_iniciar();psestilos_atualizar(.016f,&f,PS_FUNDO_FILMES);
    assert(parAtual.n==PSEST_PAREDE_MAX && !strcmp(parAtual.url[7],"title-07"));
    psestilos_atualizar(2.0f,&f,PS_FUNDO_FILMES);assert(!parTemAnt);
    // The stub decoded everything above; a failed poster never had a texture,
    // so give its slot one frame to lose it before the failure is noticed.
    failedUrl="title-07";psestilos_atualizar(.016f,&f,PS_FUNDO_FILMES);
    psestilos_atualizar(.016f,&f,PS_FUNDO_FILMES);
    assert(parAtual.n==PSEST_PAREDE_MAX && parTemAnt);   // refilled from the spare titles
    for(int i=0;i<parAtual.n;i++)assert(strcmp(parAtual.url[i],"title-07"));
    for(int i=0;i<parAtual.n;i++)for(int j=0;j<i;j++)assert(strcmp(parAtual.url[i],parAtual.url[j]));
    psestilos_atualizar(2.0f,&f,PS_FUNDO_FILMES);assert(!parTemAnt);
    psestilos_atualizar(.016f,&f,PS_FUNDO_FILMES);assert(!parTemAnt);   // settled: no churn
    f.muralN=PSEST_PAREDE_MAX;f.perfil=6;   // no spare title: the wall is one short, never blank
    psestilos_atualizar(.016f,&f,PS_FUNDO_FILMES);assert(parAtual.n==PSEST_PAREDE_MAX-1);
    failedUrl=NULL;
    puts("PASS: a poster that failed to load leaves the wall and a spare title takes its place");
  }
  puts("PASS: profile 2 legacy cache, future profiles, late catalog and PIN fallback fill unique 36-title walls");
  { // NO POSTER SEEN LOADING (psestilos.c, PAR_ESPERA_S): the first wall waits
    // hidden for its posters, a new wall waits behind the old one, and late
    // posters fade in.
    PSCena a={0};a.perfil=3;a.mural=mural;a.muralN=PSEST_PAREDE_MAX;savedWallCount=0;
    texLoading=1;psestilos_iniciar();
    for(int i=0;i<10;i++)psestilos_atualizar(.1f,&a,PS_FUNDO_FILMES);
    assert(parEntra==0.0f);                          // 1.0 s, nothing decoded: still hidden
    texLoading=0;psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    assert(parEntra>0.0f && parAtual.fade[0]<1.0f);  // ready: reveal starts; late posters fade
    for(int i=0;i<30;i++)psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    assert(parEntra==1.0f && parAtual.fade[0]==1.0f);
    // A slow network never leaves the screen black past PAR_ESPERA_S.
    texLoading=1;psestilos_iniciar();
    for(int i=0;i<16;i++)psestilos_atualizar(.1f,&a,PS_FUNDO_FILMES);
    assert(parEntra>0.0f);
    // Focus moves to a profile whose wall is not decoded: the old wall stays
    // until the new one is ready, then they cross-fade.
    texLoading=0;psestilos_iniciar();
    for(int i=0;i<40;i++)psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    char own[12][32];for(int i=0;i<12;i++){snprintf(own[i],sizeof own[i],"own-%02d",i);savedWallUrls[i]=own[i];}
    savedWallCount=12;a.perfil=4;texLoading=1;
    for(int i=0;i<5;i++)psestilos_atualizar(.1f,&a,PS_FUNDO_FILMES);
    assert(parTemAnt && parTroca==0.0f);             // waiting: only the old wall shows
    texLoading=0;psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    assert(parTroca>0.0f);
    // Cached on arrival: shown at once, no per-poster fade.
    assert(parAtual.fade[0]==0.0f||parAtual.visto[0]);
    // Flicking past a profile before its wall showed keeps the visible wall.
    savedWallCount=0;a.perfil=5;texLoading=1;psestilos_iniciar();
    for(int i=0;i<30;i++)psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);  // perfil 5 (mural) hidden
    texLoading=0;for(int i=0;i<40;i++)psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    savedWallCount=12;a.perfil=6;texLoading=1;psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    assert(parTemAnt && parAnt.perfil==5);
    a.perfil=7;psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    assert(parTemAnt && parAnt.perfil==5 && parAtual.perfil==7);
    // Neighbours are warmed only once the visible wall is ready.
    texLoading=0;a.vizinhos[0]=8;lookaheadAsks=0;
    for(int i=0;i<200;i++)psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    assert(lookaheadAsks>0);
    texLoading=1;a.perfil=9;lookaheadAsks=0;psestilos_atualizar(.016f,&a,PS_FUNDO_FILMES);
    assert(lookaheadAsks==0);
    texLoading=0;savedWallCount=0;
    puts("PASS: walls wait for their posters (1.5 s ceiling), keep the visible wall while flicking, fade late posters, warm neighbours");
  }
  for(int count=1;count<=PSEST_PAREDE_MAX;count++) {
    ParSet set={0};set.n=count;
    for(int i=0;i<count;i++){set.tex[i]=i+1;set.fade[i]=1;}
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

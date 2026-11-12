// Real parser/cache transport and real source-sheet rows; no live HTTP.
#define main stageDOriginalMain
#include "settings_stage_d.c"
#undef main
static void sheet(void) { montar(automaticaDaFolha()); }
static void visible(int count) { sheet();if(nFiltrados()!=count||nOrdem!=count)fprintf(stderr,"Expected %d visible, got %d/%d of %d raw\n",count,nFiltrados(),nOrdem,n);assert(nFiltrados()==count && nOrdem==count); }
static void addonOrder(void) {
  int tb=debrid_servico_disponivel(DEBRID_TB),pm=debrid_servico_disponivel(DEBRID_PM);
  char path[1024];snprintf(path,sizeof path,"%s/ajustes.txt",dir);
  FILE *f=fopen(path,"a");assert(f);fprintf(f,"fontePrioridadeLocal 3\n");fclose(f);ajustes_dir(dir);
  debrid_definir_chave_local("torbox",tb?"synthetic-tb":"");
  debrid_definir_chave_local("premiumize",pm?"synthetic-pm":"");
  assert(ordemDoAddon());
}
int main(void) {
  char temp[1024];snprintf(temp,sizeof temp,"%s/nuvio-filter-XXXXXX",nvt_dir());assert(mkdtemp(temp));dir=temp;
  setenv("NUVIO_DADOS",dir,1);assert(SDL_Init(SDL_INIT_TIMER)==0);
  configure(1,1,1,0,1,0,0,0);addonOrder();load();
  filtro=soCache=soMp4=soDub=0;preferida=atual=-1;grupo=1;
  visible(4);for(int i=0;i<4;i++)assert(ordem[i]==i);
  Stream original[4];memcpy(original,lista,sizeof original);
  // Focus a retained source below the source about to disappear.
  aberta=1;foco=focoVisto=linhaDe(3);assert(foco==3);
  stream_cache_verificar();stream_folha_atualizar(.016f,SDL_GetTicks());
  visible(3);assert(ordem[0]==0&&ordem[1]==2&&ordem[2]==3);
  assert(filtrado(foco)==3); // source identity survives the asynchronous removal
  assert(n==4&&folhaContagens[0]==3&&folhaContagens[1]==3);
  expectLabel(&lista[0],"Cached");expectLabel(&lista[1],"Uncached");
  assert(cacheOcultarConfirmada(&lista[1])&&!passaFiltro(1));
  for(int i=0;i<4;i++) {
    Stream unchanged=lista[i];unchanged.cacheConsulta=0;unchanged.cacheConsultaServico=0;
    unchanged.cacheConsultaGeracao=0;unchanged.cacheConsultaSessao=0;unchanged.cacheConsultaQuando=0;
    assert(!memcmp(&unchanged,&original[i],sizeof unchanged));
  }
  // Existing explicit Cached-only chip remains stricter than provider-negative hiding.
  soCache=1;visible(1);assert(ordem[0]==0&&folhaOcultos==2);
  soCache=0;visible(3);
  // Resolve Off/on, provider and credential changes invalidate cached UI counts
  // without replacing the addon list. Stale negative evidence cannot hide a source.
  configure(0,1,1,0,1,0,0,0);addonOrder();visible(4);
  configure(1,1,1,0,1,0,0,0);addonOrder();visible(4); // key generation changed
  stream_cache_verificar();visible(3);
  configure(1,2,1,0,1,1,0,0);addonOrder();visible(4);
  stream_cache_verificar();visible(2);expectLabel(&lista[0],"Uncached");
  // Unknown hashes and API errors/timeouts/malformed/missing results stay visible.
  configure(1,1,1,0,1,0,0,0);addonOrder();
  for(mode=1;mode<=5;mode++) {load();stream_cache_verificar();visible(4);expectLabel(&lista[0],"Error");}
  mode=0;load();visible(4); // addon labels alone are not provider verification
  stream_cache_verificar();visible(3);
  // HTTP URLs remain available even with negative provider evidence, with or
  // without clientResolve. Live sources are outside the VOD presentation filter.
  Stream direct=lista[1];snprintf(direct.url,sizeof direct.url,"https://fixture.invalid/ready");
  assert(!cacheOcultarConfirmada(&direct));direct.cacheRequerResolver=1;assert(!cacheOcultarConfirmada(&direct));
  direct.url[0]=0;direct.filtro.naoVod=1;assert(!cacheOcultarConfirmada(&direct));
  // The focused row itself can disappear; clamp to a remaining remote-focus row.
  load();visible(4);foco=focoVisto=linhaDe(1);stream_cache_verificar();stream_folha_atualizar(.016f,SDL_GetTicks());
  visible(3);assert(foco>=0&&foco<nOrdem&&filtrado(foco)!=1);
  // Real HTTP addon entries with a known negative hash are NOT resolver-only
  // torrents. Exercise both URL-only and clientResolve forms through the UI.
  char json[1600];snprintf(json,sizeof json,"{\"streams\":[{\"name\":\"Direct 1080p\",\"url\":\"https://fixture.invalid/one\",\"infoHash\":\"%s\"},{\"name\":\"Direct resolve 1080p\",\"url\":\"https://fixture.invalid/two\",\"clientResolve\":{\"infoHash\":\"%s\",\"service\":\"torbox\",\"isCached\":false}}]}",H2,H2);
  Stream *http=NULL;assert(stream_extrair(json,"Fixture addon",&http)==2);
  stream_definir_lista(http,2);free(http);stream_cache_verificar();visible(2);
  expectLabel(&lista[0],"Uncached");expectLabel(&lista[1],"Uncached");
  // Existing source filters still apply independently to retained direct URLs.
  FonteFiltroConfig limits={0};limits.maxGB=1;
  lista[0].tamanhoBytes=lista[0].filtro.tamanhoBytes=UINT64_C(2000000000);
  assert(fontefalha_filtro_definir(&limits));stream_filtro_atualizar();visible(1);
  assert(ordem[0]==1);limits.maxGB=0;assert(fontefalha_filtro_definir(&limits));
  stream_filtro_atualizar();visible(2);
  /* Expansion: exercise actual rows/counts, stable indices and automatic gates. */
  mode=0;load();stream_cache_verificar();
  FonteFiltroConfig expanded={0};expanded.maxResultados=2;
  assert(fontefalha_filtro_definir(&expanded));stream_filtro_atualizar();visible(2);
  assert(ordem[0]==0&&ordem[1]==2);expectLabel(&lista[0],"Cached");
  expanded.maxResultados=1;assert(fontefalha_filtro_definir(&expanded));stream_filtro_atualizar();visible(1);assert(ordem[0]==0);
  /* An earlier verified-negative torrent must not consume an autoplay quota. */
  Stream quotaSources[]={lista[1],lista[0]};int sourceOrder[]={0,1};unsigned char excluded[]={0,0};long scores[]={100,100};
  streamUserRestrict(quotaSources,sourceOrder,excluded,scores,2,&expanded,NULL,NULL,NULL,NULL);
  for(int q=0;q<2;q++)assert(!!excluded[q]==(sourceOrder[q]==0));
  expanded=(FonteFiltroConfig){0};expanded.ordenar=4;
  lista[0].filtro.tamanhoBytes=lista[0].tamanhoBytes=100;lista[2].filtro.tamanhoBytes=lista[2].tamanhoBytes=500;lista[3].filtro.tamanhoBytes=lista[3].tamanhoBytes=300;
  assert(fontefalha_filtro_definir(&expanded));stream_filtro_atualizar();visible(3);assert(ordem[0]==2&&ordem[1]==3&&ordem[2]==0);
  assert(ajustes_fonte_primeira());
  /* First in list uses exactly the sorted eligible source, not addon arrival. */
  assert(automaticoPrimeira()==0); /* Sorted Unknown is still ineligible. */
  assert(motivoForaDoAuto(2,&expanded)==MOT_CACHE_INDEFINIDO);
  /* Mock a fresh provider-positive response for a second real hash. */
  compatFixture=compatSecondCached=1;compatHash=H1;
  for(int i=0;i<n;i++)lista[i].cacheConsultaServico=0;
  stream_cache_verificar();expectLabel(&lista[1],"Cached");
  lista[1].altura=1080;lista[1].filtro.tamanhoBytes=lista[1].tamanhoBytes=600;
  assert(automaticoPrimeira()==1); /* First eligible cached source in size order. */
  compatFixture=compatSecondCached=0;
  for(int i=0;i<n;i++)lista[i].cacheConsultaServico=0;
  stream_cache_verificar();
  int ordered[]={0,2,3},ranks[]={10,20,30},prefSlot=0;
  unsigned char excludedSlots[]={0,0,0},aboveSlots[]={1,0,1};signed char groups[]={4,5,6};long ptsSlots[]={100,200,300};
  streamUserRestrict(lista,ordered,excludedSlots,ptsSlots,3,&expanded,aboveSlots,groups,ranks,&prefSlot);
  assert(ordered[0]==2&&ordered[1]==3&&ordered[2]==0&&prefSlot==2);
  assert(ranks[0]==20&&ranks[1]==30&&ranks[2]==10&&groups[0]==5&&aboveSlots[0]==0&&ptsSlots[0]==200);
  expanded.ordenar=0;expanded.maxResultados=0;assert(fontefalha_filtro_definir(&expanded));stream_filtro_atualizar();visible(3);assert(ordem[0]==0&&ordem[1]==2&&ordem[2]==3);
  puts("PASS expanded filters: actual UI counts/limits/stable sorting, cached autoplay quota eligibility and reset restores original ordering");
  assert(!mutations&&!compatCreateCalls);aberta=0;SDL_Quit();
  puts("PASS: provider-verified uncached torrent filtering, real visible rows/counts, stable order/metadata/focus, strict Cached chip, Off/provider/key changes, errors/unknown/direct/live preservation and zero mutations");
  return 0;
}

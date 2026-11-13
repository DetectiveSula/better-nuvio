/* Automatic settings sync (owner decision, 12 Nov 2026): with no arming, any
 * signed-in TV uploads its J4/J5 (better_nuvio), J6 (official tv) and Stage I
 * edits, and another TV on the same account/profile receives them. Two
 * installations (separate processes and data dirs) share one file-backed mock
 * account. The harness (mock server, boot, sync1, expectations) is the Stage J7
 * suite's, extracted by settings_sync_auto.sh. Never contacts the live backend. */
#include "j7-harness.inc"

/* Rewrites this TV's anchor and checkpoint as an older build saved them: the
 * bindings added after JB_ANTIGAS are absent. */
static void dropNewKeys(const char *dir){
 char cmd[4000];size_t n=(size_t)snprintf(cmd,sizeof cmd,"python3 -I -c 'import json,sys,glob\nnew=set(sys.argv[2:])\nfor f in glob.glob(sys.argv[1]+\"/bn-*-v1-*-p1.json\"):\n d=json.load(open(f)); d[\"slots\"]=[x for x in d[\"slots\"] if x[\"key\"] not in new]; json.dump(d,open(f,\"w\"))' '%s'",dir);
 for(size_t i=JB_ANTIGAS;i<JB_N;i++)n+=(size_t)snprintf(cmd+n,sizeof cmd-n," %s",jbMap[i].key);
 assert(n<sizeof cmd&&system(cmd)==0);
}

int main(int argc,char **argv){
 setvbuf(stdout,NULL,_IOLBF,0);assert(argc==3);server=getenv("J7_SERVER");assert(server);offline=getenv("J7_OFFLINE")!=NULL;
 const char *step=argv[2];
 boot(argv[1]);
 if(!strcmp(step,"readonly")){
  assert(!j7arme_permite_j4(sessao_usuario(),1)&&!j7arme_permite_j6(sessao_usuario(),1));
  valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];assert(gravar());sync1();
  assert(betterPushes==0);
  pass("read-only validation build: automatic sync still refuses J4/J5/J6 uploads");
  return 0;
 }
 if(!strcmp(step,"a-edit")){
  /* No arming, no permission calls: a plain signed-in TV. */
  assert(j7arme_permite_j4(sessao_usuario(),1)&&j7arme_permite_j6(sessao_usuario(),1));
  for(size_t i=0;i<JB_N;i++)valor[jbMap[i].op]=nextJ4(jbMap[i].op);
  for(size_t i=0;i<AJ6_N;i++)valor[aj6Map[i].op]=nextJ6(i);
  valor[AJ_DET_TRAILER_SOM]=1; /* retired key (#509): real builds keep it Off; On is normalised once per boot */
  valor[AJ_HERO]=!valor[AJ_HERO];assert(gravar());assert(fil_definir_tipo("j7.fixture_movie_top",FIL_TIPO_FICHA));sync1();
  assert(betterPushes==1&&tvPushes==1);
  assert(serverHas("better_nuvio",1,"heroGeneroLocal")&&serverHas("better_nuvio",1,"\"home_"));
  assert(serverHas("tv",1,"mdblist_show_imdb")&&serverHas("tv",1,"hero_section_enabled"));
  writeExpect("expect-a.txt");int before=betterPushes+tvPushes;sync1();sync1();assert(betterPushes+tvPushes==before);
  pass("A (no arming): J4, J5 Home, J6 and Stage I edits uploaded automatically, once, no loop");
 }else if(!strcmp(step,"b-restore")){
  sync1();sync1();
  assert(matchExpect("expect-a.txt")==0);assert(betterPushes==0&&tvPushes==0);
  pass("B fresh install receives every A setting and Home layout; uploads nothing (no default echo)");
 }else if(!strcmp(step,"b-edit")){
  sync1();
  for(size_t i=0;i<JB_N;i+=3)valor[jbMap[i].op]=nextJ4(jbMap[i].op);
  for(size_t i=0;i<AJ6_N;i+=3)valor[aj6Map[i].op]=nextJ6(i);
  valor[AJ_DET_TRAILER_SOM]=1;
  assert(gravar());assert(fil_definir_tipo("j7.fixture_movie_top",FIL_TIPO_PALCO));sync1();
  assert(betterPushes==1&&tvPushes==1);writeExpect("expect-b.txt");
  pass("B edits J4, J6 and a Home shape: uploaded automatically");
 }else if(!strcmp(step,"a-receive")){
  sync1();sync1();
  assert(matchExpect("expect-b.txt")==0&&betterPushes==0&&tvPushes==0);
  pass("A after restart receives B's changes; no echo upload");
 }else if(!strcmp(step,"a-offline-edit")){
  assert(offline);valor[AJ_HERO_SINOPSE]=!valor[AJ_HERO_SINOPSE];valor[AJ_MDB_IMDB]=!valor[AJ_MDB_IMDB];
  assert(gravar());sync1();assert(betterPushes==0&&tvPushes==0);
  pass("A offline: edits kept locally");
 }else if(!strcmp(step,"a-reconnect")){
  sync1();assert(betterPushes==1&&tvPushes==1);writeExpect("expect-a2.txt");sync1();assert(betterPushes==1&&tvPushes==1);
  pass("A reconnects: offline edits uploaded once (permission survives restart)");
 }else if(!strcmp(step,"b-follow")){
  sync1();assert(matchExpect("expect-a2.txt")==0&&betterPushes==0);
  pass("B receives A's offline edits");
 }else if(!strcmp(step,"upgrade")){
  /* An existing TV updates to the build with the personal-preferences keys.
   * Skip intro was customised before the update; skip recap is at its default. */
  sync1();int before=betterPushes;
  ajustes_better_esquecer();
  int custom=!valorPadrao[AJ_PULAR_ABERTURA];valor[AJ_PULAR_ABERTURA]=custom;valor[AJ_PULAR_RESUMO]=valorPadrao[AJ_PULAR_RESUMO];
  assert(perfilGravarVetor(1,valor)&&gravar());
  dropNewKeys(argv[1]);
  sync1();sync1();
  assert(betterPushes==before+1);
  assert(serverHas("better_nuvio",1,"pularAberturaLocal")&&!serverHas("better_nuvio",1,"pularResumoLocal"));
  pass("existing TV after the update: a preference customised before it uploads once; defaults are not uploaded");
 }else if(!strcmp(step,"reinstall")){
  /* Fresh install on the same account: the customised preference comes back. */
  sync1();sync1();
  assert(valor[AJ_PULAR_ABERTURA]==!valorPadrao[AJ_PULAR_ABERTURA]&&betterPushes==0);
  pass("reinstall: the customised personal preference is restored from the account");
 }else if(!strcmp(step,"device-wide")){
  /* A device-wide setting (12/24-hour clock) is kept per profile in the account:
   * switching profile on this TV neither copies profile 1's choice into profile 2
   * nor loses it. */
  sync1();int clock1=!valorPadrao[AJ_RELOGIO_12H];valor[AJ_RELOGIO_12H]=clock1;assert(gravar());sync1();
  unsigned long long p1=serverPrint("better_nuvio","settings-test",1);int pushes=betterPushes;assert(pushes>=1&&p1);
  selectProfile(2);sync1();sync1();
  assert(betterPushes==pushes);                       /* nothing of profile 1 copied into profile 2 */
  valor[AJ_RELOGIO_12H]=!clock1;assert(gravar());sync1();
  assert(betterPushes==pushes+1&&serverHas("better_nuvio",2,"relogio12hLocal")&&serverPrint("better_nuvio","settings-test",1)==p1);
  selectProfile(1);sync1();sync1();
  assert(valor[AJ_RELOGIO_12H]==clock1&&serverPrint("better_nuvio","settings-test",1)==p1);
  pass("device-wide setting: each profile keeps its own synced copy; switching profiles copies nothing");
 }else if(!strcmp(step,"store-a")){
  /* Stage 2a: values kept in their own stores (main subtitle style, source filters). */
  sync1();
  int size=player_leg_estilo()->tamanho==120?100:120;assert(ajLegendaDefinir(AJ_LEG1_TAMANHO,size));
  FonteFiltroConfig f;fontefalha_filtro(&f);unsigned gb=f.maxGB==7?9:7;assert(ajFalhaDefinir(AJ_FILTRO_TAM_MAX,(int)gb));
  sync1();assert(betterPushes==1&&serverHas("better_nuvio",1,"legenda1Tamanho")&&serverHas("better_nuvio",1,"filtroTamanhoMax"));
  char file[600],buf[64];snprintf(file,sizeof file,"%s/store-expect.txt",server);snprintf(buf,sizeof buf,"%d %u\n",size,gb);spit(file,buf);
  pass("subtitle size and maximum source size set through their own stores are uploaded");
 }else if(!strcmp(step,"store-b")){
  sync1();sync1();
  char file[600];snprintf(file,sizeof file,"%s/store-expect.txt",server);char *t=slurp(file);int size;unsigned gb;assert(t&&sscanf(t,"%d %u",&size,&gb)==2);free(t);
  FonteFiltroConfig f;fontefalha_filtro(&f);
  assert(player_leg_estilo()->tamanho==size&&f.maxGB==gb&&betterPushes==0);
  pass("the other TV's subtitle style and source filter stores receive them; no echo");
 }else if(!strcmp(step,"rules-a")){
  /* Auto-play rules live in the official document (player_settings). */
  sync1();assert(fonteregra_definir_regex("2160p|1080p"));sync1();
  assert(tvPushes>=1&&serverHas("tv",1,"2160p|1080p"));
  pass("an auto-play rule edited here is merged back into the official settings");
 }else if(!strcmp(step,"rules-b")){
  sync1();sync1();char r[200];fonteregra_regex(r,sizeof r);assert(!strcmp(r,"2160p|1080p"));
  pass("the other TV receives the auto-play rule");
 }else if(!strcmp(step,"rules-official")){
  /* Official Nuvio changed the rule meanwhile: this TV's older copy must not win. */
  char file[600];path(file,sizeof file,"tv","settings-test",1);char *t=slurp(file);assert(t);
  char *at=strstr(t,"2160p|1080p");assert(at);memcpy(at,"0720p|0480p",11);spit(file,t);free(t);
  sync1();sync1();char r[200];fonteregra_regex(r,sizeof r);
  assert(!strcmp(r,"0720p|0480p")&&serverHas("tv",1,"0720p|0480p"));
  pass("a change made in official Nuvio wins over this TV's unchanged copy");
 }else if(!strcmp(step,"files-a")){
  /* Stage 3: keys kept in data files sync too (owner opted in). */
  sync1();
  assert(dados_gravar("fanart.txt","fanart-test-key-0123456789abcdef"));fanartCarregar();
  char deb[128];assert(ajustes_provedores_arquivo(deb,sizeof deb,"debrid"));assert(dados_gravar(deb,"torbox=tb-test-key-abc\n"));debCarregar();
  jbCurrent->jtProximo=0;sync1();jbCurrent->jtProximo=0;sync1();
  assert(serverHas("better_nuvio",1,"fileFanart0")&&serverHas("better_nuvio",1,"fileDebrid0")&&!serverHas("better_nuvio",1,"fanart-test-key"));
  pass("keys in data files are uploaded, encoded, never as plain text");
 }else if(!strcmp(step,"files-b")){
  for(int k=0;k<3;k++){if(jbCurrent)jbCurrent->jtProximo=0;sync1();}
  char *f=dados_ler("fanart.txt");assert(f&&!strcmp(f,"fanart-test-key-0123456789abcdef"));free(f);
  char deb[128];assert(ajustes_provedores_arquivo(deb,sizeof deb,"debrid"));char *d=dados_ler(deb);assert(d&&strstr(d,"tb-test-key-abc"));free(d);
  pass("a fresh TV receives the keys");
 }else if(!strcmp(step,"files-c")){
  /* A TV with its own copy before its first download takes the account's. */
  for(int k=0;k<3;k++){if(jbCurrent)jbCurrent->jtProximo=0;sync1();}
  char *f=dados_ler("fanart.txt");assert(f&&!strcmp(f,"fanart-test-key-0123456789abcdef"));free(f);
  pass("a TV's own copy from before its first download does not overwrite the account's");
 }else if(!strcmp(step,"other-account")){
  unsigned long long bn=serverPrint("better_nuvio","settings-test",1),tv=serverPrint("tv","settings-test",1);assert(bn&&tv);
  signIn(OTHER_TOKEN);sync1();sync1();
  assert(serverPrint("better_nuvio","settings-test",1)==bn&&serverPrint("tv","settings-test",1)==tv);
  pass("another account never receives or overwrites the first account's documents");
 }else{fprintf(stderr,"unknown step %s\n",step);return 2;}
 return 0;
}

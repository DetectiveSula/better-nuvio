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
 if(!strcmp(step,"existing-layout")||!strcmp(step,"existing-layout-offline")){
  /* Preferences from a build before automatic sync, saved before first boot. */
  char file[700];snprintf(file,sizeof file,"%s/ajustes-p1.txt",argv[1]);
  spit(file,"homeLayoutLocal 2\nheroSinopseLocal 1\n");
  snprintf(file,sizeof file,"%s/ajustes.txt",argv[1]);
  spit(file,"homeLayoutLocal 2\nheroSinopseLocal 1\n");
 }
 boot(argv[1]);
 if(!strcmp(step,"copy-all")||!strcmp(step,"copy-active")){
  sync1();selectProfile(2);sync1();
  valor[AJ_HOME_LAYOUT]=1;valor[AJ_HERO]=1;valor[AJ_MDB_IMDB]=1;valor[AJ_RESOLVE_LINKS]=1;
  assert(gravar());sync1();sync1();selectProfile(1);sync1();
  valor[AJ_HOME_LAYOUT]=HOME_LAYOUT_DINAMICA;valor[AJ_HERO]=0;valor[AJ_MDB_IMDB]=0;valor[AJ_RESOLVE_LINKS]=0;
  assert(gravar());
  assert(dados_gravar("debrid.txt","torbox=copy-torbox-key-fixture\n"));debCarregar();
  assert(dados_gravar("fanart.txt","copy-fanart-key-fixture"));fanartCarregar();
  assert(tmdbLocalDefinir("abcdef0123456789abcdef0123456789"));
  extras_definir_chave("copy-mdblist-key-fixture");
  assert(fonteregra_definir_regex("copy-2160p"));
  ling_conta_audio("fr");
  assert(ajLegendaDefinir(AJ_LEG1_TAMANHO,120));
  const char *ids[]={"j7.fixture"},*bases[]={"https://fixture.invalid/j7-config"};fil_podar_catalogos(ids,bases,1,1);
  assert(fil_definir_tipo("j7.fixture_movie_top",FIL_TIPO_FICHA));
  assert(ajustes_copiar_perfil(1,2)>=0);
  char targetFile[200];snprintf(targetFile,sizeof targetFile,"debrid-%s-p2.txt",sessao_usuario());
  char *localKey=dados_ler(targetFile);assert(localKey&&strstr(localKey,"copy-torbox-key-fixture"));free(localKey);
  if(!strcmp(step,"copy-active")){
   selectProfile(2);for(int k=0;k<6;k++)sync1();
   assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_DINAMICA&&valor[AJ_HERO]==0&&valor[AJ_RESOLVE_LINKS]==0&&valor[AJ_MDB_IMDB]==0);
   pass("opening the target before its queued copy uploads cannot reapply the old account settings");
   return 0;
  }
  offline=1;sync1();assert(!serverHas("better_nuvio",2,"fileDebrid0"));offline=0;
  copySwitchOnPull=1;sync1();assert(!serverHas("better_nuvio",2,"fileDebrid0"));
  signIn(OTHER_TOKEN);sync1();assert(!serverHas("better_nuvio",2,"fileDebrid0"));
  signIn(TEST_TOKEN);copyFailAfterCommit=1;
  char queuedName[160];ajCopyName(sessao_usuario(),2,queuedName,sizeof queuedName);
  for(int k=0;k<6;k++){
   sync1();char *pending=dados_ler(queuedName);if(!pending)break;free(pending);
  }
  char *pending=dados_ler(queuedName);assert(!pending);
  assert(perfis_ativo()==1);
  assert(serverHas("better_nuvio",2,"homeLayoutLocal")&&serverHas("better_nuvio",2,"fileDebrid0")&&serverHas("better_nuvio",2,"fileFanart0"));
  char file[600];path(file,sizeof file,"tv","settings-test",2);char *doc=slurp(file);assert(doc);
  for(size_t i=0;i<AJ_SHARED_N;i++)if(ajSharedMap[i].op==AJ_RESOLVE_LINKS||ajSharedMap[i].op==AJ_HERO)assert(ajSharedRemote(doc,i)==1);
  free(doc);
  pass("copy publishes an inactive target's layout, resolver settings and keys; offline/session changes and ambiguous writes retain the request");
  return 0;
 }else if(!strcmp(step,"copy-peer")){
  selectProfile(2);for(int k=0;k<3;k++){if(jbCurrent)jbCurrent->jtProximo=0;sync1();}
  assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_DINAMICA&&valor[AJ_HERO]==0&&valor[AJ_MDB_IMDB]==0&&valor[AJ_RESOLVE_LINKS]==0);
  char name[160];assert(ajustes_provedores_arquivo(name,sizeof name,"debrid"));
  char *key=dados_ler(name);assert(key&&strstr(key,"copy-torbox-key-fixture"));free(key);
  key=dados_ler("fanart.txt");assert(key&&!strcmp(key,"copy-fanart-key-fixture"));free(key);
  assert(!strcmp(tmdbLocal,"abcdef0123456789abcdef0123456789"));
  char mdb[80];extras_copiar_chave(mdb,sizeof mdb);assert(!strcmp(mdb,"copy-mdblist-key-fixture"));
  char regex[80];fonteregra_regex(regex,sizeof regex);assert(!strcmp(regex,"copy-2160p"));
  assert(!strcmp(ling_conta_audio_valor(),"fr")&&player_leg_estilo()->tamanho==120);
  const char *ids[]={"j7.fixture"},*bases[]={"https://fixture.invalid/j7-config"};fil_podar_catalogos(ids,bases,1,2);
  fil_registrar("j7.fixture_movie_top","Fixture","Fixture","movie",1);
  assert(fil_tipo("j7.fixture_movie_top")==FIL_TIPO_FICHA);
  assert(!betterPushes&&!tvPushes);
  pass("another TV restores the copied target profile, including TorBox, Fanart and TMDB keys");
  return 0;
 }else if(!strcmp(step,"existing-layout-offline")){
  assert(offline);sync1();sync1();
  assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_DINAMICA&&betterPushes==0);
  assert(!serverHas("better_nuvio",1,"homeLayoutLocal"));
  pass("pre-existing preferences stay local until an authoritative pull succeeds");
  return 0;
 }else if(!strcmp(step,"existing-layout")){
  assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_DINAMICA);
  sync1();sync1();sync1();
  assert(serverHas("better_nuvio",1,"homeLayoutLocal"));
  assert(betterPushes==1&&valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_DINAMICA);
  pass("a layout saved before automatic sync uploads after a successful empty pull, once");
  return 0;
 }else if(!strcmp(step,"existing-layout-peer")){
  sync1();sync1();
  assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_DINAMICA&&betterPushes==0);
  pass("another TV restores the pre-existing layout without uploading defaults");
  return 0;
 }else if(!strcmp(step,"existing-layout-cloud-wins")){
  sync1();sync1();
  assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_DINAMICA&&betterPushes==0);
  pass("an account layout wins over a different pre-sync local baseline");
  return 0;
 }else if(!strcmp(step,"readonly")){
  assert(!j7arme_permite_j4(sessao_usuario(),1)&&!j7arme_permite_j6(sessao_usuario(),1));
  valor[AJ_HERO_GENERO]=!valor[AJ_HERO_GENERO];assert(gravar());sync1();
  assert(ajustes_copiar_perfil(1,2)>=0);sync1();
  assert(betterPushes==0&&tvPushes==0);
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
 }else if(!strcmp(step,"share-a")){
  /* Owner, 15 Nov 2026: the Seekr key (a whole-TV store) must reach another TV
   * showing a different profile. TV A edits it on profile 1 only. */
  /* Let each profile settle its file anchors first, as an installed TV has. */
  for(int p=1;p<=3;p++){selectProfile(p==2?2:1);for(int k=0;k<2;k++){if(jbCurrent)jbCurrent->jtProximo=0;sync1();}}
  assert(dados_gravar("seekr.txt","seekr-share-key-0123456789abcdef"));seekrCarregar();
  int prov=(valorPadrao[AJ_POSTER_PROV]+1)%nValores(AJ_POSTER_PROV);valor[AJ_POSTER_PROV]=prov;assert(gravar());
  {char file[600],buf[16];snprintf(file,sizeof file,"%s/share-prov.txt",server);snprintf(buf,sizeof buf,"%d",prov);spit(file,buf);}
  for(int k=0;k<3;k++){if(jbCurrent)jbCurrent->jtProximo=0;sync1();}
  assert(serverHas("better_nuvio",1,"fileSeekr0")&&serverHas("better_nuvio",2,"fileSeekr0"));
  assert(!serverHas("better_nuvio",2,"seekr-share-key"));
  assert(serverHas("better_nuvio",2,"posterProvLocal"));
  pass("share: a Seekr key and poster service changed on profile 1 are saved for profile 2 too");
 }else if(!strcmp(step,"share-b")){
  selectProfile(2);for(int k=0;k<4;k++){if(jbCurrent)jbCurrent->jtProximo=0;sync1();}
  char *f=dados_ler("seekr.txt");assert(f&&!strcmp(f,"seekr-share-key-0123456789abcdef"));free(f);
  {char file[600];snprintf(file,sizeof file,"%s/share-prov.txt",server);char *t=slurp(file);assert(t&&valor[AJ_POSTER_PROV]==atoi(t));free(t);}
  pass("share: another TV on profile 2 receives the Seekr key and the poster service choice");
 }else if(!strcmp(step,"share-a2")){
  /* A received or unchanged key is never re-shared: no loop, no stale overwrite. */
  int before=betterPushes;for(int k=0;k<3;k++){if(jbCurrent)jbCurrent->jtProximo=0;sync1();}
  assert(betterPushes==before);
  pass("share: no repeated uploads after the key is in place");
 }else if(!strcmp(step,"rows-a")){
  /* Owner, 15 Nov 2026: Home row count syncs. TV A changes it on profile 1. */
  for(int k=0;k<2;k++)sync1();
  int n=valorPadrao[AJ_FIL_LIMITE]==12?15:12;fil_ajustar_limite(n);fil_confirmar_limite();
  sync1();sync1();
  assert(fil_limite_gravado()==n&&serverHas("better_nuvio",1,"limiteFileiras"));
  {char file[600],buf[16];snprintf(file,sizeof file,"%s/rows.txt",server);snprintf(buf,sizeof buf,"%d",n);spit(file,buf);}
  pass("rows: a row count changed here is uploaded");
  /* Switching to another profile on this TV must not carry this count into it. */
  selectProfile(2);for(int k=0;k<3;k++)sync1();
  printf("rows-switch: p2 limite=%d\n",fil_limite_gravado());
  assert(!serverHas("better_nuvio",2,"limiteFileiras"));
  selectProfile(1);for(int k=0;k<2;k++)sync1();assert(fil_limite_gravado()==n);
  pass("rows: switching profiles on one TV copies no row count between them");
 }else if(!strcmp(step,"rows-b")){
  for(int k=0;k<3;k++)sync1();
  char file[600];snprintf(file,sizeof file,"%s/rows.txt",server);char *t=slurp(file);assert(t);int n=atoi(t);free(t);
  assert(fil_limite_gravado()==n&&valor[AJ_FIL_LIMITE]==n&&betterPushes==0);
  pass("rows: another TV takes the row count, without echoing it back");
 }else if(!strcmp(step,"rows-wide")){
  /* Android's "Unlimited" (200) reaches a Samsung TV: clamped to its own maximum,
   * never a refused document. */
  char p[600];path(p,sizeof p,"better_nuvio","settings-test",1);char *t=slurp(p);assert(t);
  char *at=strstr(t,"\"limiteFileiras\"");assert(at);char *v=strstr(at,"\"value\":");assert(v);
  char out[200000];size_t pre=(size_t)(v+8-t);char *end=v+8;while(*end>='0'&&*end<='9')end++;
  assert(pre+strlen(end)+8<sizeof out);memcpy(out,t,pre);size_t o=pre+(size_t)sprintf(out+pre,"200");strcpy(out+o,end);
  char *rev=strstr(at,"\"rev\":");assert(rev);free(t);
  spit(p,out);
  /* bump the revision so it wins */
  t=slurp(p);at=strstr(t,"\"limiteFileiras\"");rev=strstr(at,"\"rev\":");long r=strtol(rev+6,NULL,10);
  char o2[200000];size_t pr=(size_t)(rev+6-t);char *e2=rev+6;while(*e2>='0'&&*e2<='9')e2++;memcpy(o2,t,pr);size_t q=pr+(size_t)sprintf(o2+pr,"%ld",r+5);strcpy(o2+q,e2);free(t);spit(p,o2);
  for(int k=0;k<3;k++)sync1();
  assert(fil_limite_gravado()==OPCOES[AJ_FIL_LIMITE].max);
  pass("rows: a larger row count from another platform is clamped to this TV's maximum");
 }else if(!strcmp(step,"other-account")){
  unsigned long long bn=serverPrint("better_nuvio","settings-test",1),tv=serverPrint("tv","settings-test",1);assert(bn&&tv);
  signIn(OTHER_TOKEN);sync1();sync1();
  assert(serverPrint("better_nuvio","settings-test",1)==bn&&serverPrint("tv","settings-test",1)==tv);
  pass("another account never receives or overwrites the first account's documents");
 }else{fprintf(stderr,"unknown step %s\n",step);return 2;}
 return 0;
}

/* Automatic settings sync (owner decision, 12 Nov 2026): with no arming, any
 * signed-in TV uploads its J4/J5 (better_nuvio), J6 (official tv) and Stage I
 * edits, and another TV on the same account/profile receives them. Two
 * installations (separate processes and data dirs) share one file-backed mock
 * account. The harness (mock server, boot, sync1, expectations) is the Stage J7
 * suite's, extracted by settings_sync_auto.sh. Never contacts the live backend. */
#include "j7-harness.inc"

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
 }else if(!strcmp(step,"other-account")){
  unsigned long long bn=serverPrint("better_nuvio","settings-test",1),tv=serverPrint("tv","settings-test",1);assert(bn&&tv);
  signIn(OTHER_TOKEN);sync1();sync1();
  assert(serverPrint("better_nuvio","settings-test",1)==bn&&serverPrint("tv","settings-test",1)==tv);
  pass("another account never receives or overwrites the first account's documents");
 }else{fprintf(stderr,"unknown step %s\n",step);return 2;}
 return 0;
}

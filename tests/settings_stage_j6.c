/* Real Native bridge and sync worker; all account RPCs are isolated fixtures. */
#include "j5-native-fixture.inc"
static char *document(const int *values){
 Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);jsw_ci(&w,"version",1);jsw_cs(&w,"futureRoot","unchanged");jsw_chave(&w,"features");jsw_obj_ini(&w);
 for(size_t first=0;first<AJ6_N;first++){
  int repeated=0;for(size_t j=0;j<first;j++)if(!strcmp(aj6Map[j].feature,aj6Map[first].feature))repeated=1;if(repeated)continue;
  jsw_chave(&w,aj6Map[first].feature);jsw_obj_ini(&w);jsw_cs(&w,"unrelated_field","retain");
  for(size_t i=first;i<AJ6_N;i++)if(!strcmp(aj6Map[i].feature,aj6Map[first].feature))assert(aj6Encode(&w,i,values[i]));
  jsw_obj_fim(&w);
 }jsw_obj_fim(&w);jsw_obj_fim(&w);char *s=strdup(jsw_texto_final(&w));jsw_livre(&w);assert(s);return s;
}
static int position(int op){for(size_t i=0;i<AJ6_N;i++)if(aj6Map[i].op==op)return (int)i;abort();}
static char *single(size_t i,int value){int values[AJ6_N];for(size_t j=0;j<AJ6_N;j++){int n=0;while(!aj6Supported(j,n))n++;values[j]=n;}values[i]=value;return document(values);}
int main(int argc,char **argv){
 setvbuf(stdout,NULL,_IOLBF,0);assert(argc==2);setenv("NUVIO_DADOS",argv[1],1);dados_iniciar(NULL);player_dir(argv[1]);legfontes_dir(argv[1]);ajustes_dir(argv[1]);fil_definir_perfil(1);login();sync_reaplicar_ajustes();
 assert(AJ6_N==40&&AJ_SHARED_N==32&&JB_N==214&&aj6Current()&&!aj6Permission);
 /* One owner: Stage I read-only language imports and Stage G hide_unreleased_content stay out of J6. */
 const int foreign[]={AJ_AUD_LINGUA,AJ_AUD_LINGUA2,AJ_LEG_LINGUA,AJ_LEG_LINGUA2,AJ_TMDB_IDIOMA,AJ_OCULTAR_NLANC};
 int base[AJ6_N],target[AJ6_N];size_t conversions=0;
 for(size_t i=0;i<AJ6_N;i++){
  const Aj6Map *m=&aj6Map[i];for(size_t j=0;j<sizeof foreign/sizeof *foreign;j++)assert(m->op!=foreign[j]);for(size_t j=0;j<JB_N;j++)assert(m->op!=jbMap[j].op);for(size_t j=0;j<AJ_SHARED_N;j++)assert(m->op!=ajSharedMap[j].op);
  const Opcao *o=&OPCOES[m->op];int lo=o->tipo==OP_NUMERO?o->min:0,hi=o->tipo==OP_NUMERO?o->max:nValores(m->op)-1;int first=-1,last=-1;
  for(int n=lo;n<=hi;n++)if(aj6Supported(i,n)){
   Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);jsw_chave(&w,"features");jsw_obj_ini(&w);jsw_chave(&w,m->feature);jsw_obj_ini(&w);assert(aj6Encode(&w,i,n));jsw_obj_fim(&w);jsw_obj_fim(&w);jsw_obj_fim(&w);int out=-1;assert(aj6Remote(jsw_texto_final(&w),i,&out)==1&&out==n);jsw_livre(&w);if(first<0)first=n;last=n;conversions++;
  }
  assert(first>=0&&last!=first);base[i]=first;target[i]=last;
  int out;assert(aj6Remote("{\"features\":{}}",i,&out)==-1);assert(aj6Remote("{\"features\":[]}",i,&out)==-2);
  printf("J6_BINDING %s %s %s %s %d %d\n",CHAVE[m->op],m->feature,m->key,aj6Type(i),first,last);
 }
 printf("PASS: all 40 verified mappings (41 controls incl. the derived prepare switch), %zu lossless value conversions; 214 J4, 32 Stage I, language imports and Stage G owners disjoint\n",conversions);
 free(cloud);cloud=document(base);cycle();for(size_t i=0;i<AJ6_N;i++)assert(valor[aj6Map[i].op]==base[i]);assert(pushes==0&&!aj6Permission);
 for(size_t i=0;i<AJ6_N;i++)valor[aj6Map[i].op]=target[i];assert(gravar());cycle();assert(!pushes&&ajustes_compartilhados_pendentes()==0);
 for(size_t i=0;i<AJ6_N;i++)assert(aj6State[i].pending);
 /* Simulated restart: private journal, no inherited account-write permission. */
 aj6Forget();for(size_t i=0;i<AJ6_N;i++)valor[aj6Map[i].op]=base[i];aj6Context();assert(!aj6Permission);for(size_t i=0;i<AJ6_N;i++)assert(valor[aj6Map[i].op]==target[i]&&aj6State[i].pending);
 assert(ajustes_oficiais_permitir_escritas(1));cycle();assert(pushes==1&&!betterPushes);assert(lastWrite&&strstr(lastWrite,"\"p_platform\":\"tv\"")&&!strstr(lastWrite,"better_nuvio")&&!strstr(lastWrite,"c_settings_v1")&&!strstr(lastWrite,"c_devices_v1")&&strstr(lastWrite,"futureRoot")&&strstr(lastWrite,"unrelated_field"));
 for(size_t i=0;i<AJ6_N;i++){int n;assert(aj6Remote(cloud,i,&n)==1&&n==target[i]&&!aj6State[i].pending);}cycle();assert(pushes==1);
 puts("PASS: actual worker, default-Off, private restart recovery, original TV payload, all mappings bidirectional, unrelated values preserved, no default upload or echo");
 int discover=position(AJ_DESCOBRIR),name=position(AJ_NOME_ADDON);valor[AJ_DESCOBRIR]=1;valor[AJ_NOME_ADDON]=0;assert(gravar());int remote[AJ6_N];memcpy(remote,target,sizeof remote);remote[discover]=0;free(cloud);cloud=document(remote);cycle();assert(valor[AJ_DESCOBRIR]==0&&valor[AJ_NOME_ADDON]==0);
 /* Missing keys / invalid values retain the current preference. */
 int saved=valor[AJ_DESCOBRIR];ajustes_compartilhados_aplicar("{\"version\":1,\"features\":{\"layout_settings\":{\"discover_location\":{\"type\":\"string\",\"value\":\"FUTURE\"},\"focused_poster_backdrop_expand_delay_seconds\":{\"type\":\"int\",\"value\":1.5}}}}");assert(valor[AJ_DESCOBRIR]==saved);
 char *out=NULL;assert(!ajustes_compartilhados_mesclar("{\"version\":1,\"features\":[]}",&out)&&!out);
 /* Malformed duplicate keys are rejected before any projection. */
 assert(!ajustes_compartilhados_aplicar("{\"version\":1,\"features\":{},\"features\":{}}"));assert(valor[AJ_DESCOBRIR]==saved);
 int theme=position(AJ_TEMA);valor[AJ_TEMA]=AJ_N_TEMAS_OPC-1;assert(gravar());assert(aj6State[theme].hold);ajustes_compartilhados_aplicar(cloud);assert(valor[AJ_TEMA]==AJ_N_TEMAS_OPC-1);aj6Forget();aj6Context();assert(valor[AJ_TEMA]==AJ_N_TEMAS_OPC-1&&!aj6Permission);
 /* Deferred ack cannot clear a later local edit. */
 assert(ajustes_oficiais_permitir_escritas(1));valor[AJ_DESCOBRIR]=1;assert(gravar());assert(ajustes_compartilhados_mesclar(cloud,&out)>0&&out);valor[AJ_DESCOBRIR]=2;assert(gravar());ajustes_compartilhados_confirmar(out);free(out);out=NULL;assert(aj6State[discover].pending&&aj6State[discover].desired==2);
 puts("PASS: conflicts, missing/malformed/future fields, compound trailer target, protected Native-only theme choices and edits during acknowledgement");
 /* Offline edits survive a failed pull and context reopening. */
 failPull=1;int before=pushes;cycle();assert(pushes==before&&aj6State[discover].pending);aj6Forget();aj6Context();assert(valor[AJ_DESCOBRIR]==2&&!aj6Permission);failPull=0;cycle();assert(pushes==before);assert(ajustes_oficiais_permitir_escritas(1));cycle();
 int p1=valor[AJ_DESCOBRIR];selectProfile(2);assert(aj6Current()&&!aj6Permission);valor[AJ_DESCOBRIR]=p1==0?1:0;int p2=valor[AJ_DESCOBRIR];assert(gravar());selectProfile(1);assert(valor[AJ_DESCOBRIR]==p1&&!aj6Permission);selectProfile(2);assert(valor[AJ_DESCOBRIR]==p2&&!aj6Permission);selectProfile(1);
 const char *other="eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJzZXR0aW5ncy1vdGhlciIsImV4cCI6NDEwMjQ0NDgwMH0.test\nrefresh\n0\n";assert(dados_gravar("sessao.txt",other));perfis_invalidar_sessao();sessao_iniciar();sync_reaplicar_ajustes();assert(aj6Current()&&!aj6Permission);valor[AJ_DESCOBRIR]=p1==0?1:0;assert(gravar());login();sync_reaplicar_ajustes();assert(valor[AJ_DESCOBRIR]==p1&&!aj6Permission);
 /* Derived prepare checkbox has exactly one wire owner, the 0–5 limit. */
 int limit=position(AJ_RESOLVE_LIMIT);assert(aj6Map[limit].codec==AJ6_INT);for(int n=0;n<=5;n++){char *s=single((size_t)limit,n);ajustes_compartilhados_aplicar(s);free(s);assert(valor[AJ_RESOLVE_LIMIT]==n);}
 before=pushes;stalePull=1;cycle();assert(pushes==before);stalePull=0;
 puts("PASS: offline/restart/reconnect, account/profile isolation, stale response cancellation, default-Off restored and original preparation 0–5 semantics preserved");
 /* Regression: languages arriving in the TV document stay with Stage I's read-only
  * account slots; J6 never changes the Native language choices or queues uploads. */
 int lang[5]={valor[AJ_AUD_LINGUA],valor[AJ_AUD_LINGUA2],valor[AJ_LEG_LINGUA],valor[AJ_LEG_LINGUA2],valor[AJ_TMDB_IDIOMA]};
 int hidden=valor[AJ_OCULTAR_NLANC],pendingBefore=aj6Pending();
 ajustes_compartilhados_aplicar("{\"version\":1,\"features\":{\"player_settings\":{"
  "\"preferred_audio_language\":{\"type\":\"string\",\"value\":\"ja\"},"
  "\"secondary_preferred_audio_language\":{\"type\":\"string\",\"value\":\"fr\"},"
  "\"subtitle_preferred_language\":{\"type\":\"string\",\"value\":\"de\"}},"
  "\"tmdb_settings\":{\"tmdb_language\":{\"type\":\"string\",\"value\":\"it-IT\"}},"
  "\"layout_settings\":{\"hide_unreleased_content\":{\"type\":\"boolean\",\"value\":true}}}}");
 assert(valor[AJ_AUD_LINGUA]==lang[0]&&valor[AJ_AUD_LINGUA2]==lang[1]&&valor[AJ_LEG_LINGUA]==lang[2]&&
        valor[AJ_LEG_LINGUA2]==lang[3]&&valor[AJ_TMDB_IDIOMA]==lang[4]&&valor[AJ_OCULTAR_NLANC]==hidden&&aj6Pending()==pendingBefore);
 /* Trailer enabled without a stored target reads as HERO_MEDIA (Full screen). */
 int focus=position(AJ_FOCO_TRAILER),n=-1;
 assert(aj6Remote("{\"features\":{\"layout_settings\":{\"focused_poster_backdrop_trailer_enabled\":{\"type\":\"boolean\",\"value\":true}}}}",(size_t)focus,&n)==1&&n==1);
 assert(aj6Remote("{\"features\":{\"layout_settings\":{\"focused_poster_backdrop_trailer_enabled\":{\"type\":\"boolean\",\"value\":true},\"focused_poster_backdrop_trailer_playback_target\":{\"type\":\"string\",\"value\":\"FUTURE\"}}}}",(size_t)focus,&n)==-2);
 puts("PASS: audio/subtitle/TMDB languages and Stage G hide_unreleased_content untouched by J6; absent trailer target follows the official HERO_MEDIA default");
 free(cloud);free(lastWrite);ajustes_better_esquecer();return 0;
}

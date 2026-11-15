/* Explicit profile copies are account operations, not ordinary background
 * edits. Capture on the UI thread, then merge against each target's fresh
 * account document on the worker. Never switch the active profile to upload. */
#include "nuvem.h"

static pthread_mutex_t ajCopyLock = PTHREAD_MUTEX_INITIALIZER;
static void ajCopyName(const char *owner, int profile, char *dst, size_t cap) {
  snprintf(dst, cap, "settings-copy-v1-%016llx-p%d.json", aj6OwnerHash(owner), profile);
}
static void ajCopyFree(char *s) { if (s) { memset(s, 0, strlen(s)); free(s); } }
static char *ajCopyMember(const char *json, const char *key) {
  const char *a, *b;
  if (!ajJsonGet(json, json + strlen(json), key, &a, &b) || *a != '{') return NULL;
  return strndup(a, (size_t)(b - a));
}
static int ajCopyField(BnDoc *doc, const char *key, BnValue value) {
  if (doc->count >= BN_MAX) return 0;
  BnField *f = &doc->fields[doc->count++];
  snprintf(f->key, sizeof f->key, "%s", key); f->value = value; f->rev = 1;
  snprintf(f->device, sizeof f->device, "%s", dados_cliente_id());
  return 1;
}
static int ajCopyFileName(size_t i, int profile, const int *source, char *name, size_t cap) {
  const char *key = jtArq[i].chave, *base = NULL;
  if (!strcmp(key, "fileDebrid")) base = "debrid";
  else if (!strcmp(key, "fileFailure")) base = "fontefalha";
  else if (!strcmp(key, "fileTmdbKey")) base = "tmdb-key";
  else if (!strcmp(key, "fileMdbKey")) base = "mdblist-key";
  else if (!strcmp(key, "fileProviderScope")) base = "provider-scope";
  if (base) {
    if (strcmp(base,"tmdb-key") && strcmp(base,"mdblist-key") && strcmp(base,"provider-scope") && !source[AJ_PROVEDORES_ESCOPO])
      return snprintf(name, cap, "%s.txt", base) < (int)cap;
    return snprintf(name, cap, "%s-%s-p%d.txt", base, sessao_usuario(), profile) < (int)cap;
  }
  if (!strcmp(key,"fileLiveTvLists")) return snprintf(name,cap,"ltv-urls-p%d.txt",profile)<(int)cap;
  if (!strcmp(key,"fileJellyfin")) return snprintf(name,cap,"jellyfin-p%d.txt",profile)<(int)cap;
  if (!strcmp(key,"fileEmby")) return snprintf(name,cap,"emby-p%d.txt",profile)<(int)cap;
  if (!strcmp(key,"filePlex")) return snprintf(name,cap,"plex-p%d.txt",profile)<(int)cap;
  return jtArq[i].nome && jtArq[i].nome(name,(unsigned)cap);
}
/* Rebase only the validated profile-owned filenames inside the IPTV bundle. */
static char *ajCopyIptv(const char *raw, int source, int target, size_t cap) {
  char *out = calloc(cap,1); size_t used=0;
  if (!out) return NULL;
  for (const char *p=raw; *p;) {
    char name[64], next[64], needle[24];size_t len;int skip=0;
    if (sscanf(p,"@@bn-file %63s %zu\n%n",name,&len,&skip)!=2 || !skip ||
        strlen(p+skip)<len || !jtIptvDono(name,source)) { free(out); return NULL; }
    snprintf(needle,sizeof needle,"-p%d",source);char *at=strstr(name,needle);
    if (!at) { free(out); return NULL; }
    int n=snprintf(next,sizeof next,"%.*s-p%d%s",(int)(at-name),name,target,at+strlen(needle));
    int head=snprintf(out+used,cap-used,"@@bn-file %s %zu\n",next,len);
    if(n<0 || n>=(int)sizeof next || head<0 || used+(size_t)head+len>=cap){free(out);return NULL;}
    used+=(size_t)head;memcpy(out+used,p+skip,len);used+=len;out[used]=0;p+=skip+len;
  }
  return out;
}
static int ajCopyCaptureFile(size_t i, int source, int target, const int *values, BnDoc *doc) {
  size_t cap=jtCap(i)+1;char *raw=calloc(cap,1);int len=-1;
  if(!raw)return 0;
  if(source==perfis_ativo())len=jtLer(i,raw,cap);
  else {
    char name[200];
    if(ajCopyFileName(i,source,values,name,sizeof name)){
      char *text=dados_ler(name);size_t n=text?strlen(text):0;
      if(n<cap){memcpy(raw,text?text:"",n+1);len=(int)n;}ajCopyFree(text);
    }
  }
  /* A copy owns its credentials per target; never overwrite unselected
   * profiles through the legacy device-wide provider file. */
  if(!strcmp(jtArq[i].chave,"fileProviderScope")){snprintf(raw,cap,"1\n");len=2;}
  if(source==perfis_ativo()&&!strcmp(jtArq[i].chave,"fileDebrid")){
    DebridContexto keys;debrid_contexto_capturar(&keys);size_t n=0;raw[0]=0;len=0;
    for(int k=0;k<DEBRID_N;k++)if(keys.chave[k][0]){
      int w=snprintf(raw+n,cap-n,"%s=%s\n",debrid_servico_id(k),keys.chave[k]);
      if(w<0||n+(size_t)w>=cap){len=-1;break;}n+=(size_t)w;len=(int)n;
    }
    memset(&keys,0,sizeof keys);
  }
  if(source==perfis_ativo()&&!strcmp(jtArq[i].chave,"fileTmdbKey")&&len==0){
    const char *key=desc_chave_tmdb_reserva();
    if(tmdbLocalValida(key)&&strlen(key)<cap){snprintf(raw,cap,"%s",key);len=(int)strlen(raw);}
  }
  if(source==perfis_ativo()&&!strcmp(jtArq[i].chave,"fileMdbKey")&&len==0){
    extras_copiar_chave(raw,cap);len=(int)strlen(raw);
  }
  if(len>=0&&!strcmp(jtArq[i].chave,"fileLiveTv")){
    char *rebased=ajCopyIptv(raw,source,target,cap);ajCopyFree(raw);raw=rebased;
    if(!raw)return 0;len=(int)strlen(raw);
  }
  BnValue parts[JT_PARTES_MAX];int ok=len>=0&&jtPartes(i,raw,(size_t)len,parts);
  if(ok)for(int k=0;k<jtArq[i].partes;k++){char key[JT_CHAVE_MAX];jtChave(i,k,key);if(!ajCopyField(doc,key,parts[k])){ok=0;break;}}
  memset(parts,0,sizeof parts);ajCopyFree(raw);return ok;
}
static int ajCopyLocalFiles(int target,const int *values,const BnDoc *doc){
  int scoped[AJ_N];memcpy(scoped,values,sizeof scoped);scoped[AJ_PROVEDORES_ESCOPO]=1;
  for(size_t i=0;i<JT_N;i++){
    BnValue parts[JT_PARTES_MAX];memset(parts,0,sizeof parts);
    for(int k=0;k<jtArq[i].partes;k++){
      char key[JT_CHAVE_MAX];jtChave(i,k,key);int found=0;
      for(size_t j=0;j<doc->count;j++)if(!strcmp(doc->fields[j].key,key)){parts[k]=doc->fields[j].value;found=1;break;}
      if(!found)return 0;
    }
    size_t cap=jtCap(i)+1,len=0;char *raw=calloc(cap,1);if(!raw)return 0;
    int ok=jtJuntar(i,parts,raw,cap,&len);memset(parts,0,sizeof parts);
    if(ok&&jtArq[i].nome){
      char name[200];ok=ajCopyFileName(i,target,scoped,name,sizeof name)&&debGravarArquivo(name,raw);
    }else if(ok&&!strcmp(jtArq[i].chave,"fileLiveTv")){
      for(const char *p=raw;*p&&ok;){
        char name[64];size_t size;int skip=0;
        ok=sscanf(p,"@@bn-file %63s %zu\n%n",name,&size,&skip)==2&&skip&&strlen(p+skip)>=size&&jtIptvDono(name,target);
        if(!ok)break;char *text=strndup(p+skip,size);ok=text&&debGravarArquivo(name,text);ajCopyFree(text);p+=skip+size;
      }
    }
    ajCopyFree(raw);if(!ok)return 0;
  }
  return 1;
}
static char *ajCopyOfficial(const int *values, int source) {
  Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);jsw_ci(&w,"version",1);jsw_chave(&w,"features");jsw_obj_ini(&w);
  /* Each map has contiguous feature groups. Merge the two patches below. */
  for(size_t first=0;first<AJ_SHARED_N;first++){
    int seen=0;for(size_t j=0;j<first;j++)seen|=!strcmp(ajSharedMap[j].feature,ajSharedMap[first].feature);if(seen)continue;
    jsw_chave(&w,ajSharedMap[first].feature);jsw_obj_ini(&w);
    for(size_t j=first;j<AJ_SHARED_N;j++)if(!strcmp(ajSharedMap[j].feature,ajSharedMap[first].feature))
      aj6Encoded(&w,ajSharedMap[j].key,"boolean",values[ajSharedMap[j].op]==0,NULL);
    jsw_obj_fim(&w);
  }
  jsw_obj_fim(&w);jsw_obj_fim(&w);char *out=strdup(jsw_texto_final(&w));jsw_livre(&w);
  jsw_iniciar(&w);jsw_obj_ini(&w);jsw_chave(&w,"features");jsw_obj_ini(&w);
  for(size_t first=0;first<AJ6_N;first++){
    int seen=0;for(size_t j=0;j<first;j++)seen|=!strcmp(aj6Map[j].feature,aj6Map[first].feature);if(seen)continue;
    jsw_chave(&w,aj6Map[first].feature);jsw_obj_ini(&w);
    for(size_t j=first;j<AJ6_N;j++)if(!strcmp(aj6Map[j].feature,aj6Map[first].feature))aj6Encode(&w,j,values[aj6Map[j].op]);
    jsw_obj_fim(&w);
  }
  jsw_obj_fim(&w);jsw_obj_fim(&w);const char *patch=jsw_texto_final(&w);
  char *merged=out&&patch?ajJsonMerge(out,out+strlen(out),patch,patch+strlen(patch)):NULL;
  free(out);jsw_livre(&w);
  if(merged&&source==perfis_ativo()){
    /* Copy the account fallbacks too: a local language set to "follow the
     * account" must not start following the target's previous languages. */
    jsw_iniciar(&w);jsw_obj_ini(&w);jsw_chave(&w,"features");jsw_obj_ini(&w);
    jsw_chave(&w,"player_settings");jsw_obj_ini(&w);
    const char *keys[]={"subtitle_preferred_language","subtitle_secondary_language","preferred_audio_language","secondary_preferred_audio_language"};
    const char *languages[]={ling_conta_legenda_valor(),ling_conta_legenda2_valor(),ling_conta_audio_valor(),ling_conta_audio2_valor()};
    for(int i=0;i<4;i++)aj6Encoded(&w,keys[i],"string",0,languages[i]);
    const char *rules[]={"stream_auto_play_regex","stream_auto_play_selected_addons","stream_auto_play_selected_plugins"};
    for(int i=0;i<3;i++){
      jsw_chave(&w,rules[i]);jsw_obj_ini(&w);jsw_cs(&w,"type",i?"string_set":"string");jsw_chave(&w,"value");jsw_bruto(&w,i?"[]":"\"\"");jsw_obj_fim(&w);
    }
    jsw_obj_fim(&w);jsw_chave(&w,"tmdb_settings");jsw_obj_ini(&w);
    aj6Encoded(&w,"tmdb_language","string",0,ajustes_tmdb_idioma());jsw_obj_fim(&w);jsw_obj_fim(&w);jsw_obj_fim(&w);
    const char *fallbacks=jsw_texto_final(&w);char *next=fallbacks?ajJsonMerge(merged,merged+strlen(merged),fallbacks,fallbacks+strlen(fallbacks)):NULL;
    free(merged);merged=next;jsw_livre(&w);
    if(merged){char *ruleCopy=NULL;fonteregra_mesclar(merged,&ruleCopy);if(ruleCopy){free(merged);merged=ruleCopy;}}
  }
  return merged;
}
static int ajustesCopiaEnfileirar(int source, int target, const int *values) {
  BnDef *defs=calloc(JB_DEFS_TOTAL,sizeof *defs);BnEngine *engine=calloc(1,sizeof *engine);BnDoc *doc=calloc(1,sizeof *doc);
  char *better=NULL,*official=NULL;int ok=0;uint64_t session=nv_perfil_sessao_ler();char owner[80];snprintf(owner,sizeof owner,"%s",sessao_usuario());
  if(!defs||!engine||!doc||!*owner)goto done;
  size_t nd=jbDefsTodas(values,source==perfis_ativo(),defs);
  if(!nd||!bn_init(engine,owner,target,dados_cliente_id(),defs,nd))goto done;
  snprintf(doc->schema,sizeof doc->schema,"%s",BN_SCHEMA);doc->version=BN_VERSION;doc->profile=target;snprintf(doc->owner,sizeof doc->owner,"%s",owner);
  for(size_t i=0;i<JB_N;i++)if(!ajCopyField(doc,jbMap[i].key,defs[i].initial))goto done;
  for(size_t i=0;i<JT_N;i++)if(!ajCopyCaptureFile(i,source,target,values,doc))goto done;
  if(source==perfis_ativo()){
    size_t count=0;BnField *home=calloc(BN_MAX,sizeof *home);if(!home)goto done;
    int captured=fil_home_snapshot(owner,source,home,BN_MAX,&count);
    if(captured)for(size_t i=0;i<count;i++)if(!ajCopyField(doc,home[i].key,home[i].value)){captured=0;break;}
    free(home);if(!captured)goto done;
  }
  if(!bn_json(engine,doc,&better)||!(official=ajCopyOfficial(values,source)))goto done;
  Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);jsw_chave(&w,"better");jsw_bruto(&w,better);jsw_chave(&w,"tv");jsw_bruto(&w,official);jsw_obj_fim(&w);
  const char *text=jsw_texto_final(&w);char name[160];ajCopyName(owner,target,name,sizeof name);
  pthread_mutex_lock(&ajCopyLock);
  if(text&&nv_perfil_sessao_travar(session)){ok=debGravarArquivo(name,text);nv_perfil_sessao_destravar();}
  pthread_mutex_unlock(&ajCopyLock);jsw_livre(&w);
  if(ok)ok=ajCopyLocalFiles(target,values,doc);
  if(ok)printf("[profile-copy] perfil %d: copia completa guardada para sincronizar\n",target);
done:
  ajCopyFree(better);ajCopyFree(official);if(doc){memset(doc,0,sizeof *doc);free(doc);}free(engine);free(defs);return ok;
}
static int ajCopyCurrent(uint64_t session,const char *owner){return sessao_logada()&&nv_perfil_sessao_atual(session)&&!strcmp(owner,sessao_usuario());}
static char *ajCopyRpc(uint64_t session,const char *owner,const char *fn,int target,const char *platform,const char *document,int *status){
  char *response=NULL;Jsw w;*status=0;
  jsw_iniciar(&w);jsw_obj_ini(&w);jsw_ci(&w,"p_profile_id",target);jsw_cs(&w,"p_platform",platform);
  if(document){jsw_cs(&w,"p_origin_client_id",dados_cliente_id());jsw_chave(&w,"p_settings_json");jsw_bruto(&w,document);}jsw_obj_fim(&w);
  const char *body=jsw_texto_final(&w);
  if(body&&ajCopyCurrent(session,owner))response=sync_rpc_copia_ajustes(session,fn,body,status);
  jsw_livre(&w);
  if(!ajCopyCurrent(session,owner)){ajCopyFree(response);return NULL;}return response;
}
static int ajCopyUpload(uint64_t session,const char *owner,int target,const char *platform,const char *desired){
  int status=0,ok=0;char *response=ajCopyRpc(session,owner,"sync_pull_profile_settings_blob",target,platform,NULL,&status),*merged=NULL;
  if(!response||status<200||status>=300)goto done;
  if(!strcmp(platform,"tv")){
    if(!ajustes_compartilhados_validar(desired))goto done;
    char *base=NULL;if(!ajustes_compartilhados_resposta(response,&base))goto done;
    merged=ajJsonMerge(base,base+strlen(base),desired,desired+strlen(desired));
    if(merged&&!strcmp(base,merged)){ok=1;ajCopyFree(merged);merged=NULL;}ajCopyFree(base);
  }else{
    BnDef *defs=calloc(JB_DEFS_TOTAL,sizeof *defs);BnEngine *e=calloc(1,sizeof *e);BnDoc *remote=calloc(1,sizeof *remote),*copy=calloc(1,sizeof *copy);
    char *wrapped=malloc(strlen(desired)+32);int changed=0;
    if(!defs||!e||!remote||!copy||!wrapped)goto better_done;
    size_t nd=jbDefsTodas(valorPadrao,0,defs);
    if(!nd||!bn_init(e,owner,target,dados_cliente_id(),defs,nd)||!bnc_response(e,response,remote)||!bn_pull(e,remote))goto better_done;
    sprintf(wrapped,"[{\"settings_json\":%s}]",desired);
    if(!bnc_response(e,wrapped,copy))goto better_done;
    /* Validate the captured allowlist before replacing any remote values. */
    char *checked=NULL;if(!bn_json(e,copy,&checked))goto better_done;ajCopyFree(checked);
    uint64_t revision=0;for(size_t j=0;j<remote->count;j++)if(remote->fields[j].rev>revision)revision=remote->fields[j].rev;
    if(revision>=INT64_MAX)goto better_done;revision++;
    for(size_t i=0;i<copy->count;i++){
      size_t j=0;for(;j<remote->count;j++)if(!strcmp(remote->fields[j].key,copy->fields[i].key))break;
      if(j<remote->count&&jbEqual(remote->fields[j].value,copy->fields[i].value))continue;
      if(j==remote->count){if(remote->count==BN_MAX)goto better_done;remote->count++;}
      remote->fields[j]=copy->fields[i];remote->fields[j].rev=revision;snprintf(remote->fields[j].device,sizeof remote->fields[j].device,"%s",dados_cliente_id());changed++;
    }
    if(!changed)ok=1;else bn_json(e,remote,&merged);
better_done:
    if(copy){memset(copy,0,sizeof *copy);free(copy);}if(remote){memset(remote,0,sizeof *remote);free(remote);}ajCopyFree(wrapped);free(e);free(defs);
  }
  if(merged){
    ajCopyFree(response);response=ajCopyRpc(session,owner,"sync_push_profile_settings_blob",target,platform,merged,&status);
    ok=response&&status>=200&&status<300;
  }
done:
  printf("[profile-copy] perfil %d %s HTTP %d: %s\n",target,platform,status,ok?"confirmado":"pendente");
  ajCopyFree(response);ajCopyFree(merged);return ok;
}
/* WHOLE-TV STORES FOLLOW EVERY PROFILE (owner, 15 Nov 2026). The Seekr key,
 * Fanart key, poster service, debrid keys, P2P and failure policy are one file on
 * a TV, shared by all its profiles, but the account keeps one copy per profile.
 * Until now an edit reached only the active profile's copy, so another TV showing
 * a different profile never got it. An edit made here is now queued for every
 * other profile and merged into each one's account document with a newer revision
 * (ajCopyUpload). Only real edits are shared, never a value received or an older
 * copy found at start-up, so a stale TV cannot overwrite a newer key. */
static void ajShareName(const char *owner,int profile,const char *tag,char *dst,size_t cap){
  snprintf(dst,cap,"settings-share-v1-%016llx-p%d-%s.json",aj6OwnerHash(owner),profile,tag);
}
/* Queue `n` fields (one store's parts, or one setting) for every other profile. */
static void ajPartilharCampos(const char *tag,const char *const *keys,const BnValue *values,int n){
  char owner[80];snprintf(owner,sizeof owner,"%s",sessao_usuario());
  int ativo=perfis_ativo(),queued=0;uint64_t session=nv_perfil_sessao_ler();
  if(!sessao_logada()||!*owner||ativo<=0||n<=0)return;
  BnDef *defs=calloc(JB_DEFS_TOTAL,sizeof *defs);BnEngine *e=calloc(1,sizeof *e);BnDoc *doc=calloc(1,sizeof *doc);
  size_t nd=defs?jbDefsTodas(valorPadrao,0,defs):0;
  if(!nd||!e||!doc)goto done;
  for(int k=0;k<perfis_n();k++){
    const ContaPerfil *p=perfis_item(k);int target=p?p->indice:0;char *better=NULL,name[200];
    if(target<=0||target==ativo||!bn_init(e,owner,target,dados_cliente_id(),defs,nd))continue;
    memset(doc,0,sizeof *doc);snprintf(doc->schema,sizeof doc->schema,"%s",BN_SCHEMA);doc->version=BN_VERSION;doc->profile=target;snprintf(doc->owner,sizeof doc->owner,"%s",owner);
    int ok=1;for(int q=0;q<n&&ok;q++)ok=ajCopyField(doc,keys[q],values[q]);
    if(!ok||!bn_json(e,doc,&better))continue;
    Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);jsw_chave(&w,"better");jsw_bruto(&w,better);jsw_obj_fim(&w);ajCopyFree(better);
    const char *text=jsw_texto_final(&w);ajShareName(owner,target,tag,name,sizeof name);
    pthread_mutex_lock(&ajCopyLock);
    if(text&&nv_perfil_sessao_travar(session)){if(debGravarArquivo(name,text))queued++;nv_perfil_sessao_destravar();}
    pthread_mutex_unlock(&ajCopyLock);jsw_livre(&w);
  }
  if(queued)printf("[profile-share] %s: alterado nesta TV, guardado para %d outro(s) perfil(is)\n",tag,queued);
done:
  if(doc){memset(doc,0,sizeof *doc);free(doc);}free(e);free(defs);
}
static void ajPartilharArquivo(size_t i,const char *raw,size_t len){
  BnValue parts[JT_PARTES_MAX];char keys[JT_PARTES_MAX][JT_CHAVE_MAX];const char *kp[JT_PARTES_MAX];
  if(!jtArq[i].dispositivo||!jtPartes(i,raw,len,parts))return;
  for(int q=0;q<jtArq[i].partes;q++){jtChave(i,q,keys[q]);kp[q]=keys[q];}
  ajPartilharCampos(jtArq[i].chave,kp,parts,jtArq[i].partes);memset(parts,0,sizeof parts);
}
/* Whole-TV choices that belong with a shared store: the poster service in use
 * goes with its configuration (filePosters). */
static int ajPartilhaOpcao(int op){return op==AJ_POSTER_PROV;}
static void ajPartilharOpcao(const char *key,BnValue v){const char *k=key;ajPartilharCampos(key,&k,&v,1);}
static const char *ajShareTags[]={"posterProvLocal"};
static void ajPartilhasCiclo(uint64_t session,const char *owner){
  for(int target=1;target<=32&&ajCopyCurrent(session,owner);target++)
    for(size_t i=0;i<JT_N+sizeof ajShareTags/sizeof *ajShareTags&&ajCopyCurrent(session,owner);i++){
      char name[200];const char *tag=i<JT_N?jtArq[i].chave:ajShareTags[i-JT_N];
      if(i<JT_N&&!jtArq[i].dispositivo)continue;ajShareName(owner,target,tag,name,sizeof name);
      pthread_mutex_lock(&ajCopyLock);char *queued=dados_ler(name);pthread_mutex_unlock(&ajCopyLock);
      if(!queued)continue;
      char *better=ajCopyMember(queued,"better");int ok=better&&ajCopyUpload(session,owner,target,"better_nuvio",better);
      pthread_mutex_lock(&ajCopyLock);
      if(ok&&nv_perfil_sessao_travar(session)){char *latest=dados_ler(name);if(latest&&!strcmp(latest,queued))dados_apagar(name);ajCopyFree(latest);nv_perfil_sessao_destravar();}
      pthread_mutex_unlock(&ajCopyLock);ajCopyFree(better);ajCopyFree(queued);
    }
}
int ajustes_copias_ciclo(uint64_t session){
  char owner[80];snprintf(owner,sizeof owner,"%s",sessao_usuario());
  int refresh=0;
  if(!NV_SYNC_AUTO||!j7arme_permite_j4(owner,perfis_ativo())||!j7arme_permite_j6(owner,perfis_ativo()))return 0;
  for(int target=1;target<=32&&ajCopyCurrent(session,owner);target++){
    char name[160];ajCopyName(owner,target,name,sizeof name);pthread_mutex_lock(&ajCopyLock);char *queued=dados_ler(name);pthread_mutex_unlock(&ajCopyLock);
    if(!queued)continue;
    char *better=ajCopyMember(queued,"better"),*tv=ajCopyMember(queued,"tv");
    int ok=better&&tv&&ajCopyUpload(session,owner,target,"better_nuvio",better)&&ajCopyUpload(session,owner,target,"tv",tv);
    if(ok&&target==perfis_ativo())refresh=1;
    pthread_mutex_lock(&ajCopyLock);
    if(ok&&nv_perfil_sessao_travar(session)){
      char *latest=dados_ler(name);if(latest&&!strcmp(latest,queued))dados_apagar(name);ajCopyFree(latest);nv_perfil_sessao_destravar();
    }
    pthread_mutex_unlock(&ajCopyLock);ajCopyFree(better);ajCopyFree(tv);ajCopyFree(queued);
  }
  ajPartilhasCiclo(session,owner);
  return refresh;
}

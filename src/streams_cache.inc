// Per-list read-only cache evidence. No resolver/URL activation here.
static pthread_mutex_t cacheCheckTrava=PTHREAD_MUTEX_INITIALIZER;
static _Atomic int cacheWorker, cacheUiMudou;
static int cacheConsultaAtual(const Stream *s) {
  return s->cacheConsultaServico && ajustes_resolver_ativo() &&
    s->cacheConsultaServico==ajustes_resolver_provedor()+1 &&
    s->cacheConsultaGeracao==debrid_cache_geracao() &&
    s->cacheConsultaSessao==nv_perfil_sessao_ler();
}
int stream_cache_estado(const Stream *s) {
  if(!s)return 0;
  if(cacheConsultaAtual(s))return s->cacheConsulta;
  if(s->cacheRequerResolver && !s->infoHash[0] && !s->hashTorrent[0])return 0;
  if(s->emCache&&!s->cacheNegado&&(!s->foraCache||s->cacheNativo))return 1;
  return s->foraCache||s->cacheNegado?2:0;
}
const char *stream_cache_rotulo(const Stream *s) {
  static const char *labels[]={"Unknown","Cached","Uncached","Error"};
  int state=stream_cache_estado(s);return labels[state>=0&&state<4?state:0];
}
static int streamForaCache(const Stream *s) {
  if(s->cacheRequerResolver && !s->infoHash[0] && !s->hashTorrent[0])return 1;
  if(cacheConsultaAtual(s)) {
    if(s->cacheConsulta==1)return 0;
    if(s->cacheConsulta==2)return 1;
    // Error/Unknown do not prove uncached, but never activate an unverified link.
    return s->infoHash[0]||s->hashTorrent[0] ? 1 : s->foraCache;
  }
  return s->foraCache;
}
static const char *cacheHashDa(const Stream *s) { return s->infoHash[0]?s->infoHash:s->hashTorrent; }
static int cacheFalta(const Stream *s) {
  return cacheHashDa(s)[0] && !(s->url[0]&&s->debridServico>0) && (!cacheConsultaAtual(s) ||
    (Uint32)(SDL_GetTicks()-s->cacheConsultaQuando)>60000);
}
static int cachePendente(void) {
  if(!ajustes_resolver_ativo())return 0;
  int pending=0;pthread_mutex_lock(&verTrava);
  for(int i=0;i<n && !pending;i++)pending=cacheFalta(&lista[i]);
  pthread_mutex_unlock(&verTrava);return pending;
}
void stream_cache_verificar(void) {
  if(!ajustes_resolver_ativo())return;
  // Serialize UI background and foreground autoplay refreshes; no list lock over HTTP.
  pthread_mutex_lock(&cacheCheckTrava);
  int provider=ajustes_resolver_provedor();unsigned generation=debrid_cache_geracao(),list;
  uint64_t profile=nv_perfil_sessao_ler();
  pthread_mutex_lock(&verTrava);list=listaGeracao;pthread_mutex_unlock(&verTrava);
  for(int i=0;;) {
    char hashes[32][48];int count=0,states[32];
    if(!ajustes_resolver_ativo()||provider!=ajustes_resolver_provedor()||generation!=debrid_cache_geracao()||!nv_perfil_sessao_atual(profile))break;
    pthread_mutex_lock(&verTrava);
    if(list!=listaGeracao||i>=n){pthread_mutex_unlock(&verTrava);break;}
    while(i<n && count<32) {
      const Stream *s=&lista[i++];if(!cacheFalta(s))continue;
      const char *hash=cacheHashDa(s);int duplicate=0;
      for(int j=0;j<count;j++)if(!strcasecmp(hash,hashes[j]))duplicate=1;
      if(!duplicate)snprintf(hashes[count++],sizeof hashes[0],"%s",hash);
    }
    pthread_mutex_unlock(&verTrava);if(!count)continue;
    debrid_cache_consultar_lote(provider,hashes,count,states);
    if(!ajustes_resolver_ativo()||provider!=ajustes_resolver_provedor()||generation!=debrid_cache_geracao()||!nv_perfil_sessao_atual(profile))break;
    pthread_mutex_lock(&verTrava);
    if(list!=listaGeracao){pthread_mutex_unlock(&verTrava);break;}
    // All duplicates share a read-only result; original addon fields are untouched.
    for(int j=0;j<count;j++)for(int k=0;k<n;k++)if(!strcasecmp(cacheHashDa(&lista[k]),hashes[j])) {
      int response=states[j];lista[k].cacheConsulta=response==1?1:response==0?2:response==-2?3:0;
      lista[k].cacheConsultaServico=provider+1;lista[k].cacheConsultaGeracao=generation;
      lista[k].cacheConsultaSessao=profile;lista[k].cacheConsultaQuando=SDL_GetTicks();
    }
    atomic_fetch_add(&folhaRevisao,1);atomic_store(&cacheUiMudou,1);
    pthread_mutex_unlock(&verTrava);
  }
  pthread_mutex_unlock(&cacheCheckTrava);
}
static void *cacheTrabalhar(void *unused) {
  (void)unused;stream_cache_verificar();atomic_store(&cacheWorker,0);return NULL;
}
static void cacheAgendar(void) {
  if(!cachePendente())return;
  int expected=0;if(!atomic_compare_exchange_strong(&cacheWorker,&expected,1))return;
  pthread_t thread;if(pthread_create(&thread,NULL,cacheTrabalhar,NULL))atomic_store(&cacheWorker,0);
  else pthread_detach(thread);
}

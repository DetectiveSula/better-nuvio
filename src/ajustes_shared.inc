// Official profile-blob compatibility. Never serialise Native/device snapshots.
// Included after ajustes_device.inc for the object-scoped JSON merge helpers.
#include <pthread.h>
#include <ctype.h>

typedef struct { int op; const char *feature, *key; } AjSharedMap;
static const AjSharedMap ajSharedMap[] = {
  {AJ_RAIL, "layout_settings", "sidebar_collapsed_by_default"},
  {AJ_RAIL_MODERNA, "layout_settings", "modern_sidebar_enabled"},
  {AJ_LANDSCAPE, "layout_settings", "modern_landscape_posters_enabled"},
  {AJ_HERO, "layout_settings", "hero_section_enabled"},
  {AJ_ROTULOS, "layout_settings", "poster_labels_enabled"},
  {AJ_SUFIXO_TIPO, "layout_settings", "catalog_type_suffix_enabled"},
  {AJ_EXPANDIR, "layout_settings", "focused_poster_backdrop_expand_enabled"},
  {AJ_DET_BLUR_NAO_VISTOS, "layout_settings", "blur_unwatched_episodes"},
  {AJ_CW_THUMB, "layout_settings", "use_episode_thumbnails_in_cw"},
  {AJ_CW_LIGADO, "layout_settings", "continue_watching_enabled"},
  {AJ_CW_NAO_EXIBIDOS, "layout_settings", "show_unaired_next_up"},
  {AJ_CW_FURTHEST, "layout_settings", "next_up_from_furthest_episode"},
  {AJ_CW_BLUR_PROX, "layout_settings", "blur_continue_watching_next_up"},
  {AJ_DET_META_EXT, "layout_settings", "prefer_external_meta_addon_detail"},
  {AJ_HERO_CHEIO, "layout_settings", "modern_hero_full_screen_backdrop"},
  {AJ_DET_DATA_CHEIA, "layout_settings", "show_full_release_date"},
  {AJ_NAV_RAPIDA, "layout_settings", "fast_horizontal_navigation_enabled"},
  {AJ_PAUSA_OVERLAY, "player_settings", "pause_overlay_enabled"},
  {AJ_RELOGIO, "player_settings", "osd_clock_enabled"},
  {AJ_TMDB_LIGADO, "tmdb_settings", "tmdb_enabled"},
  {AJ_TMDB_ARTE, "tmdb_settings", "tmdb_use_artwork"},
  {AJ_TMDB_BASICO, "tmdb_settings", "tmdb_use_basic_info"},
  {AJ_TMDB_FICHA, "tmdb_settings", "tmdb_use_details"},
  {AJ_TMDB_ELENCO, "tmdb_settings", "tmdb_use_credits"},
  {AJ_TMDB_PROD, "tmdb_settings", "tmdb_use_productions"},
  {AJ_TMDB_REDES, "tmdb_settings", "tmdb_use_networks"},
  {AJ_TMDB_EPS, "tmdb_settings", "tmdb_use_episodes"},
  {AJ_TMDB_TRAILERS, "tmdb_settings", "tmdb_use_trailers"},
  {AJ_TMDB_MAIS, "tmdb_settings", "tmdb_use_more_like_this"},
  {AJ_TMDB_COL, "tmdb_settings", "tmdb_use_collections"},
  {AJ_TMDB_CW, "tmdb_settings", "tmdb_enrich_continue_watching"},
  // Resolve Playable Links: DebridSettingsDataStore `debrid_enabled`, profile
  // scoped, default false, exported (only the provider API keys are excluded).
  // Native stores the same choice per profile; both gate it on a configured
  // resolver, so the stored choice syncs and the effective state stays local.
  {AJ_RESOLVE_LINKS, "debrid_settings", "debrid_enabled"}
};
#define AJ_SHARED_N (sizeof ajSharedMap / sizeof ajSharedMap[0])
typedef struct { int observed, base, pending, desired; } AjSharedField;
static AjSharedField ajSharedState[AJ_SHARED_N];
static int ajSharedSent[AJ_SHARED_N];
static pthread_mutex_t ajSharedMutex = PTHREAD_MUTEX_INITIALIZER;
static char ajSharedOwner[160];
static int ajSharedProfile, ajSharedApplying;
// layout_settings.follow_addons_order of the selected profile: read-only (Native
// has no control for it), -1 when the account never stored it (official false).
static int ajSharedFollow = -1;
void fil_definir_seguir_addons(int ligado);
// Why the last account document was refused. Never contains account data.
static const char *ajSharedMotivo = "";
static int ajSharedDuplicada;
const char *ajustes_compartilhados_motivo(void) { return ajSharedMotivo; }

// The general Native JSON reader is deliberately tolerant. Account updates
// require a complete, unambiguous document instead, including unknown fields.
static const char *ajSharedString(const char *p, const char *end) {
  if (p == end || *p++ != '"') return NULL;
  while (p < end) {
    unsigned char c = (unsigned char)*p++;
    if (c == '"') return p;
    if (c < 32) return NULL;
    if (c == '\\') {
      if (p == end) return NULL;
      c = (unsigned char)*p++;
      if (c == 'u') { for (int n=0;n<4;n++) if (p == end || !isxdigit((unsigned char)*p++)) return NULL; }
      else if (!strchr("\"\\/bfnrt", c)) return NULL;
    }
  }
  return NULL;
}
static const char *ajSharedJson(const char *p, const char *end, int depth) {
  p = ajJsonSpace(p, end);
  if (p == end || depth > 48) return NULL;
  if (*p == '"') return ajSharedString(p, end);
  if (*p == '{' || *p == '[') {
    int object = *p == '{'; char close = object ? '}' : ']';
    const char *start = ++p;
    p = ajJsonSpace(p, end);
    if (p < end && *p == close) return p+1;
    while (p < end) {
      if (object) {
        char key[512], prior[512];
        const char *k = p, *ke = ajSharedString(p, end);
        if (!ke || !js_cadeia(k, key, sizeof key)) return NULL;
        // Reject duplicate keys, including equivalent escaped spellings.
        const char *q = ajJsonSpace(start, k);
        while (q < k) {
          const char *qe = ajSharedString(q, k);
          if (!qe || !js_cadeia(q, prior, sizeof prior)) return NULL;
          if (!strcmp(key,prior)) { ajSharedDuplicada = 1; return NULL; }
          q = ajJsonSpace(qe, k);
          if (q == k || *q++ != ':') return NULL;
          q = ajSharedJson(q, k, depth+1); if (!q) return NULL;
          q = ajJsonSpace(q,k);
          if (q < k && *q == ',') q = ajJsonSpace(q+1,k);
        }
        p = ajJsonSpace(ke, end);
        if (p == end || *p++ != ':') return NULL;
      }
      p = ajSharedJson(p,end,depth+1); if (!p) return NULL;
      p = ajJsonSpace(p,end);
      if (p == end) return NULL;
      if (*p == close) return p+1;
      if (*p++ != ',') return NULL;
      p = ajJsonSpace(p,end);
    }
    return NULL;
  }
  const char *literals[] = {"true","false","null"};
  for (int i=0;i<3;i++) {
    size_t n=strlen(literals[i]);
    if ((size_t)(end-p)>=n && !memcmp(p,literals[i],n)) return p+n;
  }
  if (*p == '-') p++;
  if (p == end || !isdigit((unsigned char)*p)) return NULL;
  if (*p == '0') p++; else while (p < end && isdigit((unsigned char)*p)) p++;
  if (p < end && *p == '.') {
    p++; if (p == end || !isdigit((unsigned char)*p)) return NULL;
    while (p < end && isdigit((unsigned char)*p)) p++;
  }
  if (p < end && (*p == 'e' || *p == 'E')) {
    p++; if (p < end && (*p == '+' || *p == '-')) p++;
    if (p == end || !isdigit((unsigned char)*p)) return NULL;
    while (p < end && isdigit((unsigned char)*p)) p++;
  }
  return p;
}
int ajustes_compartilhados_validar(const char *json) {
  if (!json) { ajSharedMotivo="no settings document"; return 0; }
  if (strlen(json)>2*1024*1024) { ajSharedMotivo="settings document larger than 2 MB"; return 0; }
  const char *end=json+strlen(json), *p=ajJsonSpace(json,end);
  if (p==end || *p!='{') { ajSharedMotivo="settings_json is not an object"; return 0; }
  ajSharedDuplicada=0;
  p=ajSharedJson(p,end,0);
  if (!p || ajJsonSpace(p,end)!=end) {
    ajSharedMotivo=ajSharedDuplicada ? "duplicate JSON key" : "malformed JSON (or key longer than 511 bytes / nesting deeper than 48)";
    return 0;
  }
  const char *v,*f;
  if (ajJsonGet(json,end,"version",&v,&f) && (f-v!=1 || *v!='1')) { ajSharedMotivo="unsupported settings version"; return 0; }
  ajSharedMotivo="";
  return 1;
}
int ajustes_compartilhados_resposta(const char *response, char **out) {
  *out=NULL;
  if (!response) { ajSharedMotivo="no response body"; return 0; }
  if (strlen(response)>2*1024*1024) { ajSharedMotivo="response larger than 2 MB"; return 0; }
  const char *end=response+strlen(response), *p=ajJsonSpace(response,end);
  ajSharedDuplicada=0;
  const char *f=ajSharedJson(p,end,0);
  if (!f || ajJsonSpace(f,end)!=end) {
    ajSharedMotivo=ajSharedDuplicada ? "duplicate JSON key in response" : "malformed or truncated response";
    return 0;
  }
  if (*p!='[') { ajSharedMotivo="response is not a row array"; return 0; }
  p=ajJsonSpace(p+1,end);
  if (*p==']') { ajSharedMotivo=""; *out=strdup("{}"); return *out!=NULL; }
  if (*p!='{') { ajSharedMotivo="response row is not an object"; return 0; }
  f=ajJsonEnd(p,end);
  const char *a,*b;
  if (!f || *ajJsonSpace(f,end)!=']') { ajSharedMotivo="response has more than one row"; return 0; }
  if (!ajJsonGet(p,f,"settings_json",&a,&b)) { ajSharedMotivo="row has no settings_json"; return 0; }
  if (b-a==4 && !memcmp(a,"null",4)) { ajSharedMotivo=""; *out=strdup("{}"); return *out!=NULL; }
  if (*a!='{') { ajSharedMotivo="settings_json is not an object"; return 0; }
  char *json=strndup(a,(size_t)(b-a));
  if (!json || !ajustes_compartilhados_validar(json)) { free(json); return 0; }
  *out=json; return 1;
}
// -1 absent; -2 malformed/unsupported. No recursive/name-only lookup.
static int ajSharedRemote(const char *json, size_t i) {
  const char *a=json,*b=json+strlen(json),*v,*f;
  const char *path[]={"features",ajSharedMap[i].feature,ajSharedMap[i].key};
  for (int k=0;k<3;k++) {
    if (!ajJsonGet(a,b,path[k],&v,&f)) return -1;
    if (*v!='{') return -2;
    a=v; b=f;
  }
  if (!ajJsonGet(a,b,"type",&v,&f) || f-v!=9 || memcmp(v,"\"boolean\"",9)) return -2;
  if (!ajJsonGet(a,b,"value",&v,&f)) return -2;
  if (f-v==4 && !memcmp(v,"true",4)) return 1;
  if (f-v==5 && !memcmp(v,"false",5)) return 0;
  return -2;
}
static char *ajSharedFeature(const char *json, const char *name) {
  const char *v,*f,*a,*b;
  if (!ajJsonGet(json,json+strlen(json),"features",&v,&f) || *v!='{' ||
      !ajJsonGet(v,f,name,&a,&b) || *a!='{') return NULL;
  return strndup(a,(size_t)(b-a));
}
static int ajSharedEncoded(const char *feature,const char *key,const char *type,
                           const char **value,const char **finish) {
  const char *a,*b,*v,*f; char actual[32];
  if (!ajJsonGet(feature,feature+strlen(feature),key,&a,&b) || *a!='{' ||
      !ajJsonGet(a,b,"type",&v,&f) || *v!='"' || !js_cadeia(v,actual,sizeof actual) || strcmp(actual,type) ||
      !ajJsonGet(a,b,"value",value,finish)) return 0;
  return 1;
}
// Preserve established read-only account source rules, badges and language
// preferences. Feature boundaries prevent similarly named Native extensions
// from being imported. Missing languages retain their current account value.
static void ajSharedReadOnly(const char *json) {
  char *player=ajSharedFeature(json,"player_settings");
  if (player) {
    static const struct { const char *key; void (*set)(const char *); } langs[]={
      {"subtitle_preferred_language",ling_conta_legenda},
      {"subtitle_secondary_language",ling_conta_legenda2},
      {"preferred_audio_language",ling_conta_audio},
      {"secondary_preferred_audio_language",ling_conta_audio2}
    };
    for (size_t i=0;i<sizeof langs/sizeof *langs;i++) {
      const char *a,*b,*v,*f; char text[80];
      if (ajJsonGet(player,player+strlen(player),langs[i].key,&a,&b) && *a=='{' &&
          ajJsonGet(a,b,"type",&v,&f) && f-v==8 && !memcmp(v,"\"string\"",8) &&
          ajJsonGet(a,b,"value",&v,&f) && *v=='"' && js_cadeia(v,text,sizeof text)) {
        langs[i].set(text);
        if (!i) snprintf(contaLegLing,sizeof contaLegLing,"%s",text);
      }
    }
    char *protected=dados_ler("ajustes-locais.txt");
    if (!protected || protected[0]!='1') {
      // The legacy source-rule importer is tolerant; give it only correctly
      // typed original fields, never a malformed list that would clear filters.
      const char *keys[]={"stream_auto_play_regex","stream_auto_play_selected_addons","stream_auto_play_selected_plugins"};
      Jsw w; jsw_iniciar(&w); jsw_obj_ini(&w);
      for (int k=0;k<3;k++) {
        const char *v,*f;
        if (!ajSharedEncoded(player,keys[k],k ? "string_set" : "string",&v,&f)) continue;
        int valid=*v==(k ? '[' : '"');
        if (valid && k) {
          const char *p=ajJsonSpace(v+1,f);
          while (p<f && *p!=']') {
            if (*p!='"' || !(p=ajSharedString(p,f))) { valid=0; break; }
            p=ajJsonSpace(p,f); if (p<f && *p==',') p=ajJsonSpace(p+1,f);
          }
        }
        if (valid) {
          char *raw=strndup(v,(size_t)(f-v));
          if (raw) { jsw_chave(&w,keys[k]); jsw_bruto(&w,raw); free(raw); }
        }
      }
      jsw_obj_fim(&w); const char *rules=jsw_texto_final(&w);
      if (rules) fonteregra_do_blob(rules);
      jsw_livre(&w);
    }
    free(protected);
    free(player);
  }
  char *badges=ajSharedFeature(json,"stream_badge_settings");
  if (badges) {
    const char *v,*f;
    if (ajSharedEncoded(badges,"stream_badge_rules","string",&v,&f) && *v=='"') {
      char *text=malloc((size_t)(f-v)+1);
      if (text && js_cadeia(v,text,(size_t)(f-v)+1)) {
        const char *end=text+strlen(text),*p=ajJsonSpace(text,end);
        const char *done=p==end ? end : ajSharedJson(p,end,0);
        if (done && ajJsonSpace(done,end)==end) selospacote_conta_do_blob(badges);
      }
      free(text);
    }
    free(badges);
  }
  char *tmdb=ajSharedFeature(json,"tmdb_settings");
  if (tmdb) {
    const char *a,*b,*v,*f; char text[80];
    if (ajJsonGet(tmdb,tmdb+strlen(tmdb),"tmdb_language",&a,&b) && *a=='{' &&
        ajJsonGet(a,b,"type",&v,&f) && f-v==8 && !memcmp(v,"\"string\"",8) &&
        ajJsonGet(a,b,"value",&v,&f) && *v=='"' && js_cadeia(v,text,sizeof text))
      snprintf(contaTmdbLing,sizeof contaTmdbLing,"%s",text);
    free(tmdb);
  }
}
static void ajSharedName(const char *owner,int profile,char *file,size_t cap) {
  unsigned long long hash=1469598103934665603ULL;
  ajustesHashBytes(&hash,owner,strlen(owner));
  snprintf(file,cap,"settings-shared-%016llx-p%d.txt",hash,profile);
}
static void ajSharedWrite(const char *owner,int profile,const AjSharedField *state,int follow) {
  if (!*owner) return;
  char file[128],text[8192]; ajSharedName(owner,profile,file,sizeof file);
  size_t n=(size_t)snprintf(text,sizeof text,"v1 %s %d\n",owner,profile);
  for (size_t i=0;i<AJ_SHARED_N;i++) {
    const AjSharedField *s=&state[i];
    n+=(size_t)snprintf(text+n,sizeof text-n,"%s %d %d %d %d\n",ajSharedMap[i].key,s->observed,s->base,s->pending,s->desired);
  }
  // Read-only import for the offline Home order (older readers skip this line).
  if (follow>=0) snprintf(text+n,sizeof text-n,"@follow_addons_order %d\n",follow);
  dados_gravar(file,text);
}
// Returns the stored follow_addons_order (-1 when absent).
static int ajSharedRead(const char *owner,int profile,AjSharedField *state) {
  char file[128],stored[160]; int p=0, follow=-1;
  ajSharedName(owner,profile,file,sizeof file);
  char *text=dados_ler(file);
  if (text && sscanf(text,"v1 %159s %d",stored,&p)==2 && p==profile && !strcmp(stored,owner)) {
    const char *line=strchr(text,'\n');
    while (line && *++line) {
      char key[160]; int obs,base,pending,desired;
      if (sscanf(line,"@follow_addons_order %d",&obs)==1 && (obs==0 || obs==1))
        follow=obs;
      else if (sscanf(line,"%159s %d %d %d %d",key,&obs,&base,&pending,&desired)==5 &&
          obs>=0 && obs<=1 && base>=-1 && base<=1 && pending>=0 && pending<=1 && desired>=0 && desired<=1) {
        for (size_t i=0;i<AJ_SHARED_N;i++) if (!strcmp(key,ajSharedMap[i].key))
          state[i]=(AjSharedField){obs,base,pending,desired};
      }
      line=strchr(line,'\n');
    }
  }
  free(text);
  return follow;
}
static void ajSharedSave(void) { ajSharedWrite(ajSharedOwner,ajSharedProfile,ajSharedState,ajSharedFollow); }
static int ajSharedCurrent(void) {
  return ajSharedOwner[0] && ajSharedProfile==perfis_ativo() &&
         !strcmp(ajSharedOwner,sessao_usuario());
}
// UI thread, after the selected Native profile has been restored. No network.
void ajustes_compartilhados_contexto(void) {
  aj6Context();
  const char *owner=sessao_usuario(); int profile=perfis_ativo();
  if (!owner || !*owner || strlen(owner)>=sizeof ajSharedOwner || profile<=0) return;
  pthread_mutex_lock(&ajSharedMutex);
  if (ajSharedCurrent()) { pthread_mutex_unlock(&ajSharedMutex); return; }
  snprintf(ajSharedOwner,sizeof ajSharedOwner,"%s",owner); ajSharedProfile=profile;
  contaTmdbLing[0]=contaLegLing[0]=0;
  for (size_t i=0;i<AJ_SHARED_N;i++) ajSharedState[i]=(AjSharedField){valor[ajSharedMap[i].op],-1,0,valor[ajSharedMap[i].op]==0};
  ajSharedFollow=ajSharedRead(owner,profile,ajSharedState);
  for (size_t i=0;i<AJ_SHARED_N;i++) {
    AjSharedField *s=&ajSharedState[i];
    if (s->pending || s->base>=0) valor[ajSharedMap[i].op]=s->pending ? !s->desired : !s->base;
    s->observed=valor[ajSharedMap[i].op];
  }
  ajSharedSave(); pthread_mutex_unlock(&ajSharedMutex);
  fil_definir_seguir_addons(ajSharedFollow==1);
}
// Existing admin/profile editor: journal only actual edits for its target,
// without rebinding the active session or publishing the other profile now.
void ajustes_compartilhados_edicao(int profile,const int *before,const int *after) {
  aj6Edit(profile,before,after);
  const char *owner=sessao_usuario();
  if (!owner || !*owner || strlen(owner)>=sizeof ajSharedOwner || profile<=0) return;
  pthread_mutex_lock(&ajSharedMutex);
  AjSharedField state[AJ_SHARED_N]; int changed=0;
  for (size_t i=0;i<AJ_SHARED_N;i++) state[i]=(AjSharedField){before[ajSharedMap[i].op],-1,0,before[ajSharedMap[i].op]==0};
  int follow=ajSharedRead(owner,profile,state);
  for (size_t i=0;i<AJ_SHARED_N;i++) {
    int op=ajSharedMap[i].op;
    if (before[op]!=after[op]) {
      state[i].observed=after[op]; state[i].desired=after[op]==0;
      state[i].pending=state[i].base!=state[i].desired; changed=1;
    }
  }
  if (changed) ajSharedWrite(owner,profile,state,follow);
  pthread_mutex_unlock(&ajSharedMutex);
}
void ajustes_compartilhados_esquecer(void) {
  aj6Forget();
  pthread_mutex_lock(&ajSharedMutex);
  ajSharedOwner[0]=0; ajSharedProfile=0; ajSharedFollow=-1;
  pthread_mutex_unlock(&ajSharedMutex);
  fil_definir_seguir_addons(0);
}
void ajustes_compartilhados_observar(void) {
  aj6Observe();
  if (edPerfil || ajSharedApplying) return;
  pthread_mutex_lock(&ajSharedMutex);
  int changed=0;
  if (ajSharedCurrent()) for (size_t i=0;i<AJ_SHARED_N;i++) {
    AjSharedField *s=&ajSharedState[i]; int v=valor[ajSharedMap[i].op];
    if (s->observed!=v) { s->observed=v; s->desired=v==0; s->pending=s->base!=s->desired; changed=1; }
  }
  if (changed) ajSharedSave();
  pthread_mutex_unlock(&ajSharedMutex);
}
int ajustes_compartilhados_pendentes(void) {
  int n=0; pthread_mutex_lock(&ajSharedMutex);
  if (ajSharedCurrent()) for (size_t i=0;i<AJ_SHARED_N;i++) n+=ajSharedState[i].pending;
  pthread_mutex_unlock(&ajSharedMutex); return n+aj6Pending();
}
// One privacy-safe line per account document: counts and official field names
// only (never values, owner, tokens or other features' contents).
static void ajSharedNomes(char *dst,size_t cap,const unsigned char *marca,unsigned char quem) {
  size_t n=0; dst[0]=0;
  for (size_t i=0;i<AJ_SHARED_N && n+2<cap;i++) if (marca[i]==quem)
    n+=(size_t)snprintf(dst+n,cap-n,"%s%s",n?",":"",ajSharedMap[i].key);
  if (!n) snprintf(dst,cap,"-");
}
int ajustes_compartilhados_aplicar(const char *json) {
  if (edPerfil) { printf("[ajustes-conta] adiado: edicao de outro perfil aberta\n"); return 0; }
  if (!ajustes_compartilhados_validar(json)) {
    printf("[ajustes-conta] documento recusado: %s\n", ajSharedMotivo); return 0;
  }
  enum { F_AUSENTE, F_APLICADO, F_IGUAL, F_LOCAL, F_INVALIDO };
  unsigned char marca[AJ_SHARED_N]; int conta[5]={0};
  int changed=aj6Apply(json); pthread_mutex_lock(&ajSharedMutex);
  if (!ajSharedCurrent()) {
    pthread_mutex_unlock(&ajSharedMutex);
    printf("[ajustes-conta] ignorado: documento de outra conta/perfil\n"); return 0;
  }
  for (size_t i=0;i<AJ_SHARED_N;i++) {
    int remote=ajSharedRemote(json,i);
    marca[i]=remote==-1 ? F_AUSENTE : remote<0 ? F_INVALIDO : F_IGUAL;
    if (remote<0) continue;
    AjSharedField *s=&ajSharedState[i];
    if (s->pending && remote!=s->desired && remote==s->base) { marca[i]=F_LOCAL; continue; }
    // Remote wins a genuine conflict; an unrelated field never loses its edit.
    s->base=remote; s->pending=0; s->desired=remote; s->observed=!remote;
    if (valor[ajSharedMap[i].op]!=!remote) { valor[ajSharedMap[i].op]=!remote; changed++; marca[i]=F_APLICADO; }
  }
  for (size_t i=0;i<AJ_SHARED_N;i++) conta[marca[i]]++;
  { // follow_addons_order: absent keeps the last known value; malformed is ignored.
    const char *a=json,*b=json+strlen(json),*v,*f; int ok=1;
    const char *path[]={"features","layout_settings","follow_addons_order"};
    for (int k=0;k<3 && ok;k++) { ok=ajJsonGet(a,b,path[k],&v,&f) && *v=='{'; a=v; b=f; }
    if (ok && ajJsonGet(a,b,"type",&v,&f) && f-v==9 && !memcmp(v,"\"boolean\"",9) &&
        ajJsonGet(a,b,"value",&v,&f) && (f-v==4 || f-v==5))
      ajSharedFollow=!memcmp(v,"true",4);
  }
  int follow=ajSharedFollow, perfil=ajSharedProfile;
  ajSharedSave(); pthread_mutex_unlock(&ajSharedMutex);
  fil_definir_seguir_addons(follow==1);
  { char aplicados[512],locais[512],invalidos[512];
    ajSharedNomes(aplicados,sizeof aplicados,marca,F_APLICADO);
    ajSharedNomes(locais,sizeof locais,marca,F_LOCAL);
    ajSharedNomes(invalidos,sizeof invalidos,marca,F_INVALIDO);
    printf("[ajustes-conta] perfil %d: %d/%d campos recebidos; aplicados %d [%s]; ja iguais %d; "
           "edicao local pendente %d [%s]; invalidos %d [%s]; ausentes %d; follow_addons_order=%s\n",
           perfil, (int)AJ_SHARED_N-conta[F_AUSENTE], (int)AJ_SHARED_N, conta[F_APLICADO], aplicados,
           conta[F_IGUAL], conta[F_LOCAL], locais, conta[F_INVALIDO], invalidos, conta[F_AUSENTE],
           follow<0 ? "ausente" : follow ? "true" : "false"); }
  ajSharedReadOnly(json);
  idiomaResolver(1); // Existing automatic language policy; explicit local choice wins.
  if (changed) { ajSharedApplying=1; gravar(); ajSharedApplying=0; }
  return changed;
}
// Worker thread, holding the existing account/session guard. Fresh pull only.
int ajustes_compartilhados_mesclar(const char *base, char **out) {
  *out=NULL;
  if (!ajustes_compartilhados_validar(base)) return 0;
  const char *a,*b;
  if (ajJsonGet(base,base+strlen(base),"version",&a,&b) && (b-a!=1 || *a!='1')) return 0;
  // Do not replace a malformed enclosing feature object.
  if (ajJsonGet(base,base+strlen(base),"features",&a,&b) && *a!='{') return 0;
  Jsw patch; jsw_iniciar(&patch); jsw_obj_ini(&patch); jsw_ci(&patch,"version",1);
  jsw_chave(&patch,"features"); jsw_obj_ini(&patch);
  int n=0; pthread_mutex_lock(&ajSharedMutex);
  memset(ajSharedSent,0,sizeof ajSharedSent);
  // Every feature named by the map, in map order, each opened once.
  if (ajSharedCurrent()) for (size_t first=0;first<AJ_SHARED_N;first++) {
    int opened=0, repeated=0;
    const char *feature=ajSharedMap[first].feature;
    for (size_t k=0;k<first && !repeated;k++) repeated=!strcmp(ajSharedMap[k].feature,feature);
    if (repeated) continue;
    for (size_t i=first;i<AJ_SHARED_N;i++) {
      if (strcmp(feature,ajSharedMap[i].feature)) continue;
      AjSharedField *s=&ajSharedState[i]; int remote=ajSharedRemote(base,i);
      if (!s->pending || remote==-2 || remote==s->desired || (remote>=0 && remote!=s->base)) continue;
      if (!opened) { jsw_chave(&patch,feature); jsw_obj_ini(&patch); opened=1; }
      jsw_chave(&patch,ajSharedMap[i].key); jsw_obj_ini(&patch);
      jsw_cs(&patch,"type","boolean"); jsw_chave(&patch,"value");
      jsw_bruto(&patch,s->desired ? "true" : "false"); jsw_obj_fim(&patch); n++;
      ajSharedSent[i]=1;
    }
    if (opened) jsw_obj_fim(&patch);
  }
  { char nomes[512]; unsigned char enviado[AJ_SHARED_N];
    for (size_t i=0;i<AJ_SHARED_N;i++) enviado[i]=(unsigned char)ajSharedSent[i];
    pthread_mutex_unlock(&ajSharedMutex);
    if (n) { ajSharedNomes(nomes,sizeof nomes,enviado,1); printf("[ajustes-conta] enviando %d campo(s): %s\n",n,nomes); } }
  jsw_obj_fim(&patch); jsw_obj_fim(&patch);
  const char *p=jsw_texto_final(&patch);
  if (n && p) *out=ajJsonMerge(base,base+strlen(base),p,p+strlen(p));
  jsw_livre(&patch);
  char *typed=NULL;int extra=aj6Merge(*out?*out:base,&typed);if(typed){free(*out);*out=typed;n+=extra;}
  return *out ? n : 0;
}
void ajustes_compartilhados_confirmar(const char *sent) {
  if (!ajustes_compartilhados_validar(sent)) return;
  aj6Ack(sent);
  pthread_mutex_lock(&ajSharedMutex);
  if (ajSharedCurrent()) for (size_t i=0;i<AJ_SHARED_N;i++) {
    if (!ajSharedSent[i]) continue;
    int v=ajSharedRemote(sent,i); if (v<0) continue;
    AjSharedField *s=&ajSharedState[i];
    // A later local edit survives acknowledgement of an earlier request.
    s->base=v;
    s->pending=s->desired!=v;
  }
  ajSharedSave(); pthread_mutex_unlock(&ajSharedMutex);
}

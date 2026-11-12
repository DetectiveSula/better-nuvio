// Regular NuvioTV payloads through the actual production bridge (ajustes_shared.inc).
// The documents follow ProfileSettingsSyncService.exportSettingsBlob /
// encodePreferenceValue at the pinned commit: version 1, the 13 synced features,
// typed {type,value} entries, provider API keys excluded. No HTTP, no account.
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "js.h"
#include "jsw.h"
#include "shared-test-enum.inc"
#include "shared-test-json.inc"
static int valor[AJ_N], edPerfil, profile=1;
static char owner[80]="account-official", root[512], contaTmdbLing[80],contaLegLing[80];
static void idiomaResolver(int n) { (void)n; }
static int perfis_ativo(void) { return profile; }
static const char *sessao_usuario(void) { return owner; }
static char *dados_ler(const char *name) {
  char path[700]; snprintf(path,sizeof path,"%s/%s",root,name);
  FILE *f=fopen(path,"r"); if (!f) return NULL;
  fseek(f,0,SEEK_END); long n=ftell(f); rewind(f);
  char *out=calloc((size_t)n+1,1); assert(out); assert(fread(out,1,(size_t)n,f)==(size_t)n); fclose(f); return out;
}
static int dados_gravar(const char *name,const char *text) {
  char path[700]; snprintf(path,sizeof path,"%s/%s",root,name);
  FILE *f=fopen(path,"w"); assert(f); fputs(text,f); return fclose(f)==0;
}
static void ajustesHashBytes(unsigned long long *h,const void *p,size_t n) {
  const unsigned char *s=p; while(n--) { *h^=*s++; *h*=1099511628211ULL; }
}
static void nop(const char *p) {(void)p;}
#define ling_conta_legenda nop
#define ling_conta_legenda2 nop
#define ling_conta_audio nop
#define ling_conta_audio2 nop
#define fonteregra_do_blob nop
#define selospacote_conta_do_blob nop
static int seguir=-2;
void fil_definir_seguir_addons(int ligado) { seguir=ligado; }
void ajustes_compartilhados_observar(void);
static int gravar(void) { ajustes_compartilhados_observar(); return 1; }
#include "fixtures/settings-restoration/stage-j6-isolated.h"
#include "ajustes_shared.inc"

// Regular Nuvio's export after toggling "Resolve playable links" on, with
// "Follow addons order" on and the hero off. Every family is present; values
// exercise each encoder type, escapes, braces inside strings and JSON-in-string.
static char *officialBlob(const char *debrid, const char *follow, const char *hero) {
  char *b=malloc(16384); assert(b);
  snprintf(b,16384,
    "{\"version\":1,\"features\":{"
    "\"theme_settings\":{\"selected_theme\":{\"type\":\"string\",\"value\":\"OCEAN\"},\"amoled_mode\":{\"type\":\"boolean\",\"value\":false}},"
    "\"layout_settings\":{\"selected_layout\":{\"type\":\"string\",\"value\":\"MODERN\"},"
      "\"hero_section_enabled\":{\"type\":\"boolean\",\"value\":%s},"
      "\"follow_addons_order\":{\"type\":\"boolean\",\"value\":%s},"
      "\"poster_card_width_dp\":{\"type\":\"int\",\"value\":126},"
      "\"focused_poster_backdrop_expand_delay_seconds\":{\"type\":\"int\",\"value\":3},"
      "\"custom_poster_enabled_screens\":{\"type\":\"string_set\",\"value\":[\"home\",\"search\"]}},"
    "\"experience_mode\":{\"mode\":{\"type\":\"string\",\"value\":\"ADVANCED\"}},"
    "\"player_settings\":{\"subtitle_size\":{\"type\":\"float\",\"value\":1.15},"
      "\"subtitle_vertical_offset\":{\"type\":\"float\",\"value\":1.0E-4},"
      "\"skip_intro_enabled\":{\"type\":\"boolean\",\"value\":true},"
      "\"subtitle_preferred_language\":{\"type\":\"string\",\"value\":\"pt-BR\"},"
      "\"stream_auto_play_regex\":{\"type\":\"string\",\"value\":\"(?i)2160p|\\\\bremux\\\\b\"},"
      "\"pause_overlay_enabled\":{\"type\":\"boolean\",\"value\":true},"
      "\"last_seek_ms\":{\"type\":\"long\",\"value\":9007199254740993}},"
    "\"stream_badge_settings\":{\"stream_badge_rules\":{\"type\":\"string\",\"value\":\"[{\\\"id\\\":\\\"4k\\\",\\\"match\\\":\\\"2160p\\\"}]\"}},"
    "\"plugin_settings\":{\"group_streams_by_repository\":{\"type\":\"boolean\",\"value\":false}},"
    "\"trailer_settings\":{\"trailer_autoplay\":{\"type\":\"boolean\",\"value\":true},\"trailer_volume\":{\"type\":\"double\",\"value\":0.35}},"
    "\"tmdb_settings\":{\"tmdb_enabled\":{\"type\":\"boolean\",\"value\":true},\"tmdb_language\":{\"type\":\"string\",\"value\":\"pt-BR\"}},"
    "\"mdblist_settings\":{\"mdblist_enabled\":{\"type\":\"boolean\",\"value\":false}},"
    "\"trakt_settings\":{\"trakt_scrobble\":{\"type\":\"boolean\",\"value\":true}},"
    "\"debrid_settings\":{\"debrid_enabled\":{\"type\":\"boolean\",\"value\":%s},"
      "\"cloud_library_enabled\":{\"type\":\"boolean\",\"value\":true},"
      "\"preferred_resolver_provider_id\":{\"type\":\"string\",\"value\":\"torbox\"},"
      "\"instant_playback_preparation_limit\":{\"type\":\"int\",\"value\":2},"
      "\"debrid_stream_name_template\":{\"type\":\"string\",\"value\":\"{stream.resolution} {service.shortName}\"},"
      "\"debrid_stream_description_template\":{\"type\":\"string\",\"value\":\"{stream.title}\\n\\u2022 {stream.size}\"},"
      "\"stream_preferences\":{\"type\":\"string\",\"value\":\"{\\\"sortCriteria\\\":[{\\\"key\\\":\\\"RESOLUTION\\\",\\\"direction\\\":\\\"DESC\\\"}]}\"}},"
    "\"animeskip_settings\":{\"animeskip_enabled\":{\"type\":\"boolean\",\"value\":false}},"
    "\"track_preference\":{\"remember_tracks\":{\"type\":\"boolean\",\"value\":true}}"
    "}}", hero, follow, debrid);
  return b;
}
static char *envelope(const char *blob) {
  size_t n=strlen(blob)+160; char *e=malloc(n); assert(e);
  snprintf(e,n,"[{\"profile_id\":1,\"settings_json\":%s,\"updated_at\":\"2026-11-04T12:00:00.123456+00:00\"}]",blob);
  return e;
}
static char logPath[700];
static char *captured(void (*run)(const char *), const char *arg) {
  fflush(stdout); int saved=dup(1); FILE *f=fopen(logPath,"w"); assert(f);
  dup2(fileno(f),1); run(arg); fflush(stdout); dup2(saved,1); close(saved); fclose(f);
  return dados_ler("bridge.log");
}
static void apply(const char *json) { ajustes_compartilhados_aplicar(json); }
static void edit(int op,int v) {valor[op]=v;gravar();}

int main(int argc,char **argv) {
  assert(argc==2); snprintf(root,sizeof root,"%s",argv[1]);
  snprintf(logPath,sizeof logPath,"%s/bridge.log",root);
  valor[AJ_RESOLVE_LINKS]=1; valor[AJ_HERO]=0;     // Native defaults: Resolve Off, hero On
  ajustes_compartilhados_contexto();
  assert(seguir==0 && !ajustes_compartilhados_pendentes());

  // 1. Regular Nuvio turned Resolve Playable Links on: the real envelope is accepted.
  char *on=officialBlob("true","true","false"), *env=envelope(on), *blob=NULL;
  assert(ajustes_compartilhados_resposta(env,&blob) && blob && !strcmp(ajustes_compartilhados_motivo(),""));
  char *log=captured(apply,blob);
  assert(valor[AJ_RESOLVE_LINKS]==0 && valor[AJ_HERO]==1 && valor[AJ_PAUSA_OVERLAY]==0);
  assert(seguir==1);
  assert(strstr(log,"[ajustes-conta] perfil 1:") && strstr(log,"debrid_enabled") &&
         strstr(log,"hero_section_enabled") && strstr(log,"follow_addons_order=true"));
  // Privacy: names and counts only, never owner, values or other features' contents.
  assert(!strstr(log,"account-official") && !strstr(log,"torbox") && !strstr(log,"{stream") &&
         !strstr(log,"pt-BR") && !strstr(log,"remux") && !strstr(log,"\"value\""));
  free(log); free(env);
  puts("PASS: regular Nuvio's full 13-feature export is accepted; Resolve Playable Links, hero and follow order apply");

  // 2. Better Nuvio turns it off: only debrid_enabled changes in a fresh copy of
  //    the account document; templates, other features and absent keys survive.
  edit(AJ_RESOLVE_LINKS,1);
  assert(ajustes_compartilhados_pendentes()==1);
  char *out=NULL; assert(ajustes_compartilhados_mesclar(blob,&out)==1 && out);
  assert(ajustes_compartilhados_validar(out));
  assert(strstr(out,"\"debrid_enabled\":{\"type\":\"boolean\",\"value\":false}"));
  assert(strstr(out,"{stream.resolution} {service.shortName}") && strstr(out,"\\u2022") &&
         strstr(out,"\"selected_theme\"") && strstr(out,"9007199254740993") && strstr(out,"1.0E-4"));
  assert(!strstr(out,"api_key"));
  { const char *v,*f,*a=out,*b=out+strlen(out);
    const char *path[]={"features","debrid_settings","preferred_resolver_provider_id"};
    for (int k=0;k<3;k++) { assert(ajJsonGet(a,b,path[k],&v,&f)); a=v; b=f; } }
  ajustes_compartilhados_confirmar(out); assert(!ajustes_compartilhados_pendentes());
  free(blob); blob=out;
  puts("PASS: a Better Nuvio change uploads only debrid_enabled, preserving the rest of the regular Nuvio document");

  // 3. Regular Nuvio switches it back on afterwards: remote wins over the
  //    acknowledged baseline, without an echo upload.
  char *again=officialBlob("true","true","false");
  ajustes_compartilhados_aplicar(again); assert(valor[AJ_RESOLVE_LINKS]==0 && !ajustes_compartilhados_pendentes());
  out=NULL; assert(!ajustes_compartilhados_mesclar(again,&out) && !out);
  // Missing debrid_settings (older regular Nuvio export) leaves the choice alone.
  ajustes_compartilhados_aplicar("{\"version\":1,\"features\":{\"layout_settings\":{}}}");
  assert(valor[AJ_RESOLVE_LINKS]==0 && seguir==1);
  puts("PASS: later regular Nuvio changes win; absent fields and follow order are preserved");

  // 4. Follow order is profile-scoped and durable; the inactive editor never copies it.
  ajustes_compartilhados_esquecer(); assert(seguir==0);
  ajustes_compartilhados_contexto(); assert(seguir==1);
  int before[AJ_N],after[AJ_N]; memcpy(before,valor,sizeof before); memcpy(after,valor,sizeof after);
  after[AJ_HERO]=!before[AJ_HERO];
  ajustes_compartilhados_edicao(3,before,after);
  profile=3; ajustes_compartilhados_contexto(); assert(seguir==0);
  profile=2; ajustes_compartilhados_contexto(); assert(seguir==0);
  ajustes_compartilhados_aplicar(officialBlob("false","false","true")); assert(seguir==0);
  profile=1; ajustes_compartilhados_contexto(); assert(seguir==1);
  puts("PASS: follow_addons_order is restored per account/profile after restart and never leaks between profiles");

  // 5. Strict rejection names its reason and keeps every local value.
  int keep=valor[AJ_RESOLVE_LINKS];
  struct { const char *doc, *motivo; } bad[]={
    {"{\"version\":1,\"features\":{},\"features\":{}}","duplicate JSON key"},
    {"{\"version\":1,\"features\":{\"layout_settings\":{\"x\":01}}}","malformed JSON"},
    {"{\"version\":2,\"features\":{}}","unsupported settings version"},
    {"[1]","settings_json is not an object"}};
  for (size_t i=0;i<sizeof bad/sizeof *bad;i++) {
    log=captured(apply,bad[i].doc);
    assert(strstr(ajustes_compartilhados_motivo(),bad[i].motivo) && strstr(log,"documento recusado"));
    assert(valor[AJ_RESOLVE_LINKS]==keep); free(log);
  }
  const char *badEnv[][2]={{"[{\"settings_json\":{}},{\"settings_json\":{}}]","more than one row"},
                           {"[{\"profile_id\":1}]","no settings_json"},
                           {"[{\"settings_json\":{\"a\":1,\"a\":2}}]","duplicate JSON key"},
                           {"[{\"settings_json\":{}}","malformed or truncated"}};
  for (size_t i=0;i<4;i++) {
    char *x=NULL; assert(!ajustes_compartilhados_resposta(badEnv[i][0],&x) && !x);
    assert(strstr(ajustes_compartilhados_motivo(),badEnv[i][1]));
  }
  char *big=malloc(2*1024*1024+64); assert(big); memset(big,' ',2*1024*1024+32); memcpy(big,"{}",2); big[2*1024*1024+32]=0;
  assert(!ajustes_compartilhados_validar(big) && strstr(ajustes_compartilhados_motivo(),"2 MB")); free(big);
  puts("PASS: duplicate, malformed, unsupported, multi-row and oversized documents are refused with a logged reason");
  free(on); free(again); free(blob);
  return 0;
}

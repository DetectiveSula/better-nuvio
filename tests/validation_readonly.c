// Network-free transport boundary tests: denied calls must not reach rede.
#include "nuvem.h"
#include "rede.h"
#include "validation_readonly.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static int posts, read_calls, patches;
const char *rede_ultimo_erro(void) { return ""; }
char *rede_postar_st(const char *u,int s,const char *const *h,const char *b,int *st) {
  (void)u;(void)s;(void)h;(void)b;posts++;if(st)*st=200;return NULL;
}
char *rede_baixar_st(const char *u,int s,const char *const *h,int *st) {
  (void)u;(void)s;(void)h;read_calls++;if(st)*st=200;return NULL;
}
int rede_pedir(const RedePedido *p,RedeResposta *r) {
  (void)p; patches++;r->status=204;return 1;
}
void rede_resposta_limpar(RedeResposta *r) {(void)r;}
int main(void) {
  int st=0;
  assert(nuvem_configurar(NULL));
  const char *reads[]={"sync_pull_profiles","sync_pull_profile_locks",
    "sync_pull_profile_settings_blob","sync_pull_home_catalog_settings",
    "sync_pull_collections","sync_pull_library","sync_pull_watched_items"};
  for(unsigned i=0;i<sizeof reads/sizeof reads[0];i++) {
    nuvem_rpc_com(reads[i],"{}","mock",&st);assert(st==200);
  }
  int before=posts;
  const char *blocked[]={"/rest/v1/rpc/sync_push_profile_settings_blob",
    "/rest/v1/rpc/sync_push_plugins","/rest/v1/rpc/sync_push_provider_credentials",
    "/rest/v1/rpc/sync_pull_provider_credentials", // no provider credential loading
    "/auth/v1/signup",
    "/functions/v1/unknown","/rest/v1/rpc/sync_pull_profiles?x=1",
    "rest/v1/rpc/sync_pull_profiles","/rest/v1/rpc/sync_pull_profiles/",
    "/rest/v1/rpc/unknown_read"};
  for(unsigned i=0;i<sizeof blocked/sizeof blocked[0];i++) {
    st=0;nuvem_post(blocked[i],"{}","mock",&st);
    assert(st==(NV_VALIDATION_READONLY_ENABLED?NV_VALIDATION_DENIED_STATUS:200));
  }
  assert(posts==before+(NV_VALIDATION_READONLY_ENABLED?0:10));
  const char *auth[] = {"/auth/v1/token?grant_type=password",
    "/auth/v1/token?grant_type=refresh_token", "/auth/v1/token?grant_type=anonymous",
    "/rest/v1/rpc/start_tv_login_session", "/rest/v1/rpc/poll_tv_login_session",
    "/functions/v1/tv-logins-exchange"};
  for(unsigned i=0;i<sizeof auth/sizeof auth[0];i++) {
    int was=posts;
    st=0;nuvem_post(auth[i],"{}","mock",&st);
    int allowed=!NV_VALIDATION_READONLY_ENABLED || NV_VALIDATION_LOGIN_ENABLED;
    assert(st==(allowed?200:NV_VALIDATION_DENIED_STATUS));
    assert(posts==was+allowed);
  }
  const char *anonymous="{\"data\":{\"tv_client\":\"webos\"}}";
  int was=posts;
  nuvem_post("/auth/v1/signup",anonymous,"mock",&st);
  int allowed=!NV_VALIDATION_READONLY_ENABLED || NV_VALIDATION_LOGIN_ENABLED;
  assert(st==(allowed?200:NV_VALIDATION_DENIED_STATUS));assert(posts==was+allowed);
  const char *denied_auth[]={"/auth/v1/token?grant_type=password&x=1",
    "/auth/v1/user", "/auth/v1/admin/users", "/auth/v1/recover",
    "/auth/v1/token?grant_type=client_credentials", "/functions/v1/tv-logins-exchange/",
    "/rest/v1/rpc/start_tv_login_session?x=1", "/rest/v1/rpc/finish_tv_login_session"};
  for(unsigned i=0;i<sizeof denied_auth/sizeof denied_auth[0];i++) {
    was=posts;nuvem_post(denied_auth[i],"{}","mock",&st);
    assert(st==(NV_VALIDATION_READONLY_ENABLED?NV_VALIDATION_DENIED_STATUS:200));
    assert(posts==was+!NV_VALIDATION_READONLY_ENABLED);
  }
  before=posts;
  nuvem_tabela("addons","mock=eq.mock","mock",&st);assert(st==200);
  nuvem_tabela("plugins","mock=eq.mock","mock",&st);assert(st==200);
  int got=read_calls;
  nuvem_tabela("rpc/unknown","","mock",&st);
  assert(read_calls==got+(NV_VALIDATION_READONLY_ENABLED?0:1));
  nuvem_addon_alternar("mock=eq.mock",1,"mock",1000,&st);
  assert(patches==(NV_VALIDATION_READONLY_ENABLED?0:1));
  assert(st==(NV_VALIDATION_READONLY_ENABLED?NV_VALIDATION_DENIED_STATUS:204));
  if(NV_VALIDATION_READONLY_ENABLED) {
    assert(!nv_validation_account_read("GET","/rest/v1/rpc/sync_pull_profiles"));
    assert(!nv_validation_account_read(NULL,NULL));
    nuvem_post("/auth/v1/logout","{}","mock",NULL);assert(posts==before);
  }
  if(NV_VALIDATION_READONLY_ENABLED) {
    assert(nv_validation_http_method("POST","https://mock.invalid/rest/v1/rpc/sync_pull_profiles"));
    assert(!nv_validation_http_method("POST","https://mock.invalid.evil/rest/v1/rpc/sync_pull_profiles"));
    assert(!nv_validation_http_method("POST","https://provider.invalid/create"));
    assert(!nv_validation_http_method("DELETE","https://provider.invalid/item"));
    assert(!nv_validation_http_method("PUT","https://provider.invalid/item"));
    assert(!nv_validation_http_method("PATCH","https://provider.invalid/item"));
    assert(nv_validation_http_method("GET","https://mock.invalid/rest/v1/addons"));
  }
  if(NV_VALIDATION_READONLY_ENABLED) {
    assert(!nv_validation_account_read("POST", "/auth/v1/token?grant_type=password"));
    assert(nv_validation_http_method("POST","https://mock.invalid/auth/v1/token?grant_type=password")==NV_VALIDATION_LOGIN_ENABLED);
    assert(!nv_validation_http_method("POST","https://mock.invalid/auth/v1/signup"));
    assert(!nv_validation_http_request("POST","https://mock.invalid/auth/v1/signup","{}",2));
    assert(!nv_validation_http_request("POST","https://mock.invalid/auth/v1/signup",anonymous,strlen(anonymous)-1));
    assert(!nv_validation_http_request("POST","https://mock.invalid/auth/v1/signup",anonymous,strlen(anonymous)+1));
    assert(nv_validation_http_request("POST","https://mock.invalid/auth/v1/signup",anonymous,strlen(anonymous))==NV_VALIDATION_LOGIN_ENABLED);
    assert(!nv_validation_http_request("POST","https://mock.invalid.evil/auth/v1/signup",anonymous,strlen(anonymous)));
    assert(!nv_validation_http_request("POST","https://mock.invalid/auth/v1/signup?x=1",anonymous,strlen(anonymous)));
    assert(!nv_validation_http_request("PATCH","https://mock.invalid/auth/v1/token?grant_type=password","{}",2));
    assert(!nv_validation_http_method("POST","https://mock.invalid/rest/v1/rpc/sync_push_profile_settings_blob"));
    assert(!nv_validation_http_method("POST","https://mock.invalid/rest/v1/rpc/sync_push_home_catalog_settings"));
    assert(!nv_validation_http_method("POST","https://mock.invalid/rest/v1/rpc/sync_push_watched_items"));
    assert(!nv_validation_http_method("POST","https://mock.invalid/rest/v1/rpc/sync_push_library"));
  }
  if(NV_VALIDATION_LOGIN_ENABLED) puts("PASS: login and refresh endpoints permitted; signup body constrained; settings/progress/provider writes remain blocked");
  puts(NV_VALIDATION_READONLY_ENABLED?"PASS: read-only account transport denies writes before network":"PASS: ordinary account transport unchanged");
}

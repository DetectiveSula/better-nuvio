// Compile-time dedicated-device preflight guard. No runtime activation bypass.
// The strict preflight build blocks auth as well as writes. A separate debug-only
// login opt-in permits only existing sign-in/session endpoints; settings writes
// and provider mutations remain blocked. Ordinary builds retain their policy.
#ifndef NV_VALIDATION_READONLY_H
#define NV_VALIDATION_READONLY_H
#include <string.h>
// Local temporary refusal: never impersonate an auth rejection that clears tokens.
#define NV_VALIDATION_DENIED_STATUS 503
#ifdef NV_VALIDATION_READONLY
#if defined(NDEBUG)
#error Read-only validation must not be enabled in a release build
#endif
#define NV_VALIDATION_READONLY_ENABLED 1
#else
#define NV_VALIDATION_READONLY_ENABLED 0
#endif

#ifdef NV_VALIDATION_LOGIN
#ifndef NV_VALIDATION_READONLY
#error Validation login requires read-only validation
#endif
#define NV_VALIDATION_LOGIN_ENABLED 1
#else
#define NV_VALIDATION_LOGIN_ENABLED 0
#endif

// Source-verified sessao.c endpoints only. Anonymous signup is the QR bootstrap,
// not account creation with an email/password: permit only its exact fixed body.
static inline int nv_validation_login_request(const char *method, const char *path,
                                               const void *body, size_t size) {
  static const char anonymous[] = "{\"data\":{\"tv_client\":\"webos\"}}";
  static const char *const paths[] = {
    "/auth/v1/token?grant_type=password",
    "/auth/v1/token?grant_type=refresh_token",
    "/auth/v1/token?grant_type=anonymous",
    "/rest/v1/rpc/start_tv_login_session",
    "/rest/v1/rpc/poll_tv_login_session",
    "/functions/v1/tv-logins-exchange"
  };
  if (!NV_VALIDATION_LOGIN_ENABLED || !method || !path || strcmp(method, "POST")) return 0;
  if (!strcmp(path, "/auth/v1/signup"))
    return body && size == sizeof anonymous-1 && !memcmp(body, anonymous, size);
  for (unsigned i = 0; i < sizeof paths/sizeof paths[0]; i++)
    if (!strcmp(path, paths[i])) return 1;
  return 0;
}

static inline int nv_validation_account_read(const char *method, const char *path) {
  if (!NV_VALIDATION_READONLY_ENABLED) return 1;
  // Exact source-verified RPC names, never a prefix/wildcard or arbitrary POST.
  static const char *const reads[] = {
    "/rest/v1/rpc/sync_pull_profiles",
    "/rest/v1/rpc/sync_pull_profile_locks",
    "/rest/v1/rpc/sync_pull_profile_settings_blob",
    "/rest/v1/rpc/sync_pull_home_catalog_settings",
    "/rest/v1/rpc/sync_pull_collections",
    "/rest/v1/rpc/sync_pull_library",
    "/rest/v1/rpc/sync_pull_watched_items"
  };
  if (!method || !path || strcmp(method, "POST")) return 0;
  for (unsigned i = 0; i < sizeof reads / sizeof reads[0]; i++)
    if (!strcmp(path, reads[i])) return 1;
  return 0;
}

static inline int nv_validation_account_request(const char *method, const char *path,
                                                 const void *body, size_t size) {
  return nv_validation_account_read(method, path) ||
         nv_validation_login_request(method, path, body, size);
}

static inline int nv_validation_table_read(const char *table) {
  if (!NV_VALIDATION_READONLY_ENABLED) return 1;
  return table && (!strcmp(table, "addons") || !strcmp(table, "plugins"));
}
// Extra defence for native HTTP mutation paths, including third-party workers.
// Permit only read RPCs plus the separate login opt-in on the configured
// account origin; provider and settings mutations remain refused.
static inline int nv_validation_http_request(const char *method, const char *url,
                                               const void *body, size_t size) {
  if (!NV_VALIDATION_READONLY_ENABLED) return 1;
  if (!method || !strcmp(method, "GET") || !strcmp(method, "HEAD")) return 1;
#ifdef NV_SUPABASE_URL
  const char *base = NV_SUPABASE_URL;
  size_t n = strlen(base);
  while (n && base[n-1] == '/') n--;
  return n && url && !strncmp(url, base, n) && url[n] == '/' &&
    nv_validation_account_request(method, url+n, body, size);
#else
  (void)url; (void)body; (void)size;
  return 0;
#endif
}
static inline int nv_validation_http_method(const char *method, const char *url) {
  return nv_validation_http_request(method, url, NULL, 0);
}

#endif

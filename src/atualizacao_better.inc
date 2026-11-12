// Android strictdebug channel only. Keep release/asset objects bounded: a digest
// from another asset, or a URL from another repository, must never authorise it.
#include "js.h"
#include <ctype.h>

static int at_better_bool(const char *obj, const char *key, int wanted) {
  const char *p = obj;
  if (!p || *p++ != '{') return 0;
  while (*p) {
    char name[80];
    while (isspace((unsigned char)*p) || *p == ',') p++;
    if (*p != '"' || !js_cadeia(p, name, sizeof name)) return 0;
    for (p++; *p && *p != '"'; p++) if (*p == '\\' && p[1]) p++;
    if (!*p++) return 0;
    while (isspace((unsigned char)*p)) p++;
    if (*p++ != ':') return 0;
    while (isspace((unsigned char)*p)) p++;
    if (!strcmp(name, key)) {
      const char *v = wanted ? "true" : "false";
      size_t n = strlen(v);
      return !strncmp(p, v, n) && (p[n] == ',' || p[n] == '}' || isspace((unsigned char)p[n]));
    }
    if (*p == '{' || *p == '[') p = js_fim(p);
    else if (*p == '"') {
      for (p++; *p && *p != '"'; p++) if (*p == '\\' && p[1]) p++;
      if (*p) p++;
    } else while (*p && *p != ',' && *p != '}') p++;
  }
  return 0;
}

// Only the documented preview tag grammar is eligible. Base versions must
// increase over the installed version; equal-version recuts are not upgrades.
static int at_better_tag(const char *tag, char *version, size_t size, char *date) {
  unsigned a, b, c, serial; char day[9]; int end = 0;
  if (!tag || sscanf(tag, "v%u.%u.%u-better-nuvio-preview.%8[0-9].%u%n",
                     &a, &b, &c, day, &serial, &end) != 5 || !end || tag[end] ||
      strlen(day) != 8 || a > 999 || b > 99 || c > 99 || !serial) return 0;
  char canonical[96];
  snprintf(canonical, sizeof canonical, "v%u.%u.%u-better-nuvio-preview.%s.%u", a,b,c,day,serial);
  if (strcmp(canonical, tag)) return 0;
  snprintf(version, size, "%u.%u.%u", a,b,c);
  memcpy(date, day, 9);
  return 1;
}

static int at_better_apk(const char *release, char *url, size_t size,
                         char *hash, size_t hashSize) {
  char tag[96], version[32], date[9], expected[180], prefix[240];
  url[0] = 0; if (hashSize) hash[0] = 0;
  if (!js_texto_raiz(release, "tag_name", tag, sizeof tag) ||
      !at_better_tag(tag, version, sizeof version, date)) return 0;
#ifdef NV_APK_LITE
#if UINTPTR_MAX == UINT32_MAX
  const char *variant = "strictdebug-lite-armv7";
#else
  const char *variant = "strictdebug-lite-arm64";
#endif
#else
  const char *variant = "strictdebug";
#endif
  snprintf(expected, sizeof expected, "Better-Nuvio-%s-%s-preview-%s.apk", version, variant, date);
  snprintf(prefix, sizeof prefix, "https://github.com/DetectiveSula/better-nuvio/releases/download/%s/", tag);
  const char *p = js_array(release, NULL, "assets");
  for (; p && *p == '{'; p = js_prox(js_fim(p))) {
    const char *end = js_fim(p);
    char name[180], candidate[512], digest[80];
    if (!js_texto_raiz_em(p,end,"name",name,sizeof name) || strcmp(name,expected) ||
        !js_texto_raiz_em(p,end,"browser_download_url",candidate,sizeof candidate) ||
        strncmp(candidate,prefix,strlen(prefix)) || strcmp(candidate+strlen(prefix),expected) ||
        !js_texto_raiz_em(p,end,"digest",digest,sizeof digest) ||
        strncmp(digest,"sha256:",7) || strlen(digest) != 71) continue;
    int valid = 1;
    for (int i = 7; i < 71; i++) if (!isxdigit((unsigned char)digest[i])) valid = 0;
    if (!valid || strlen(candidate) >= size || hashSize < 65) continue;
    snprintf(url,size,"%s",candidate); snprintf(hash,hashSize,"%s",digest+7);
    return 1;
  }
  return 0;
}

static char *at_better_release(const char *list) {
  char *chosen = NULL;
  char bestVersion[32] = "", bestTag[96] = "";
  if (!list || !js_array_valido(list)) return NULL;
  const char *p = js_raiz_array(list);
  for (; p && *p == '{'; p = js_prox(js_fim(p))) {
    const char *end = js_fim(p);
    size_t n = (size_t)(end-p);
    if (n > 512*1024) continue;
    char *candidate = malloc(n+1);
    if (!candidate) { free(chosen); return NULL; }
    memcpy(candidate,p,n); candidate[n] = 0;
    char tag[96], version[32], date[9], url[512], hash[80];
    int valid = at_better_bool(candidate,"draft",0) && at_better_bool(candidate,"prerelease",1) &&
      js_texto_raiz(candidate,"tag_name",tag,sizeof tag) &&
      at_better_tag(tag,version,sizeof version,date) &&
      at_better_apk(candidate,url,sizeof url,hash,sizeof hash);
    // No dependence on GitHub's creation order; choose the highest base version.
    // Within one base version the canonical date and numeric serial decide.
    int later = 0;
    if (valid && chosen && !strcmp(version,bestVersion)) {
      const char *x = strrchr(tag,'.'), *y = strrchr(bestTag,'.');
      char oldDate[9], ignored[32]; at_better_tag(bestTag,ignored,sizeof ignored,oldDate);
      later = strcmp(date,oldDate) > 0 || (!strcmp(date,oldDate) && strtoul(x+1,NULL,10)>strtoul(y+1,NULL,10));
    }
    if (valid && (!chosen || maisNova(version,bestVersion) || later)) {
      free(chosen); chosen = candidate;
      snprintf(bestVersion,sizeof bestVersion,"%s",version); snprintf(bestTag,sizeof bestTag,"%s",tag);
    } else free(candidate);
  }
  return chosen;
}

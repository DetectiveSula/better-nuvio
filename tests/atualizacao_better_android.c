#define NV_ANDROID 1
#define NV_VERSAO "2.0.8"
#include <stdint.h>
#ifdef TEST_ARMV7
#undef UINTPTR_MAX
#define UINTPTR_MAX UINT32_MAX
#endif
#include "../src/atualizacao.c"
#include <assert.h>

#define DIGEST "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
#define TAG "v2.0.9-better-nuvio-preview.20261109.1"
static const char *reply;
static int calls;
char *rede_baixar(const char *url, int timeout) {
  assert(!strcmp(url,AT_URL)); assert(timeout == 12); calls++;
  return reply ? strdup(reply) : NULL;
}
int android_instalar_apk(const char *p) { (void)p; assert(!"test must not install"); return 0; }
int android_preparar_apk(const char *p) { (void)p; assert(!"test must not prepare an install"); return 0; }

static const char *variant(void) {
#ifdef NV_APK_LITE
#ifdef TEST_ARMV7
  return "strictdebug-lite-armv7";
#else
  return "strictdebug-lite-arm64";
#endif
#else
  return "strictdebug";
#endif
}

static char *release(const char *tag, int draft, const char *repository, const char *digest) {
  char version[32], date[9]; assert(at_better_tag(tag,version,sizeof version,date));
  char name[200]; snprintf(name,sizeof name,"Better-Nuvio-%s-%s-preview-%s.apk",version,variant(),date);
  char *r = malloc(4096); assert(r);
  snprintf(r,4096,"{\"tag_name\":\"%s\",\"draft\":%s,\"prerelease\":true,\"body\":\"## Fixed\\n- Test\","
    "\"assets\":[{\"name\":\"%s\",\"browser_download_url\":\"https://github.com/%s/releases/download/%s/%s\","
    "\"digest\":\"sha256:%s\"}]}",tag,draft?"true":"false",name,repository,tag,name,digest);
  return r;
}
static void query(const char *json) {
  reply=json; tagNova[0]=apkUrl[0]=apkHash[0]=apkVer[0]=0;
  fioConsulta(NULL);
}
int main(void) {
  assert(!strcmp(AT_URL,"https://api.github.com/repos/DetectiveSula/better-nuvio/releases?per_page=30"));
  char *good=release(TAG,0,"DetectiveSula/better-nuvio",DIGEST);
  char url[512],hash[80];
  assert(acharApk(good,url,sizeof url,hash,sizeof hash)); assert(!strcmp(hash,DIGEST));
  char *wrong=release(TAG,0,"iqui27/nuvio-native-legacy",DIGEST);
  assert(!acharApk(wrong,url,sizeof url,hash,sizeof hash)); assert(!url[0]&&!hash[0]);
  char *bad=release(TAG,0,"DetectiveSula/better-nuvio","abcd");
  assert(!acharApk(bad,url,sizeof url,hash,sizeof hash));
  char *draft=release("v9.0.0-better-nuvio-preview.20261109.1",1,"DetectiveSula/better-nuvio",DIGEST);
  char *newer=release("v2.1.0-better-nuvio-preview.20261110.1",0,"DetectiveSula/better-nuvio",DIGEST);
  char list[20000]; snprintf(list,sizeof list,"[%s,%s,%s,%s]",draft,newer,wrong,good);
  char *selected=at_better_release(list); assert(selected&&strstr(selected,"v2.1.0-")); free(selected);
  char *serial=release("v2.0.9-better-nuvio-preview.20261109.10",0,"DetectiveSula/better-nuvio",DIGEST);
  snprintf(list,sizeof list,"[%s,%s]",good,serial); selected=at_better_release(list);
  assert(selected&&strstr(selected,"20261109.10"));free(selected);
  assert(!at_better_release("[]")); assert(!at_better_release("[{\"tag_name\":\"invalid\"}]"));
  assert(!at_better_release("[{}"));
  assert(!at_better_bool("{\"assets\":[{\"draft\":false}],\"draft\":true}","draft",0));
  assert(at_better_bool("{\"assets\":[{\"draft\":true}],\"draft\":false}","draft",0));
  char v[32],date[9];
  assert(!at_better_tag("v2.0.9-better-nuvio-preview.20261109.1/evil",v,sizeof v,date));
  assert(!at_better_tag("v02.0.9-better-nuvio-preview.20261109.1",v,sizeof v,date));
  assert(!at_better_tag("v2.0.9-better-nuvio-preview.20261109.0",v,sizeof v,date));
  // A hash on another asset never authorises the correct APK without its own digest.
  char *missing=strdup(good); assert(missing);
  strstr(missing,"\"digest\"")[1]='x';
  char noDigest[6000];
  snprintf(noDigest,sizeof noDigest,"{\"tag_name\":\"" TAG "\",\"assets\":[{\"name\":\"other\",\"digest\":\"sha256:" DIGEST "\"},%s",
    strstr(missing,"\"assets\":[")+strlen("\"assets\":["));
  assert(!acharApk(noDigest,url,sizeof url,hash,sizeof hash));
  free(missing);
  char hex[65]; at_sha256_hex((const unsigned char *)"abc",3,hex);
  assert(!strcmp(hex,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
  assert(strcmp(hex,DIGEST)); // A corrupt/mismatched payload cannot match the expected digest.
  mtx=SDL_CreateMutex(); assert(mtx);
  snprintf(list,sizeof list,"[%s]",good); query(list);
  assert(busca==ATUALIZACAO_BUSCA_NOVA && !strcmp(tagNova,&TAG[1]));
  assert(!strcmp(apkVer,&TAG[1]) && strlen(apkVer)>31 && !strcmp(apkHash,DIGEST));
  assert(podeAtualizarApk());
  char *same=release("v2.0.8-better-nuvio-preview.20261109.2",0,"DetectiveSula/better-nuvio",DIGEST);
  snprintf(list,sizeof list,"[%s]",same);query(list);
  assert(busca==ATUALIZACAO_BUSCA_EM_DIA && !apkUrl[0]);
  query("[]"); assert(busca==ATUALIZACAO_BUSCA_EM_DIA);
  query("[{}"); assert(busca==ATUALIZACAO_BUSCA_ERRO);
  query(NULL); assert(busca==ATUALIZACAO_BUSCA_ERRO);
  assert(calls==5); SDL_DestroyMutex(mtx);
  free(good);free(wrong);free(bad);free(draft);free(newer);free(serial);free(same);
  puts("PASS: Better Nuvio strictdebug feed, bounded matching, digest isolation, full tags, mixed/draft/invalid releases, ordering, same-version refusal and actual query; no installs");
}

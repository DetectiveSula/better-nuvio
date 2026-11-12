#include "resolver_policy.h"
#include "ajustes.h"
#include "debrid.h"
#include <stdio.h>
#include <time.h>
static long ms(const struct timespec *a,const struct timespec *b){return (long)((b->tv_sec-a->tv_sec)*1000+(b->tv_nsec-a->tv_nsec)/1000000);}
int resolver_politica_mascara(void) {
  int mask=0;
  // The pinned official visible resolver list is TorBox/Premiumize. Native RD
  // and AD have no safe cache-only torrent creation contract: retain manual use.
  if(debrid_servico_disponivel(DEBRID_TB))mask|=1<<DEBRID_TB;
  if(debrid_servico_disponivel(DEBRID_PM))mask|=1<<DEBRID_PM;
  return mask;
}
int resolver_politica_opcao(int v) {
  int mask=resolver_politica_mascara();
  return v==0 ? mask!=0 : v==1 ? (mask&(1<<DEBRID_TB))!=0 : v==2 ? (mask&(1<<DEBRID_PM))!=0 : 0;
}
int resolver_politica_provedor(int preferred) {
  int mask=resolver_politica_mascara();
  if(preferred==1 && (mask&(1<<DEBRID_TB)))return DEBRID_TB;
  if(preferred==2 && (mask&(1<<DEBRID_PM)))return DEBRID_PM;
  return mask&(1<<DEBRID_TB)?DEBRID_TB:mask&(1<<DEBRID_PM)?DEBRID_PM:-1;
}
int resolver_politica_limite(int v) { return v<0?0:v>5?5:v; }
int resolver_politica_resolver(int requested,const char *hash,int file,char *url,unsigned size,int *outside) {
  if(url && size)url[0]=0;
  if(outside)*outside=0;
  if(!url||!size||!ajustes_resolver_ativo())return 0;
  int provider=ajustes_resolver_provedor();
  if(provider<0 || (requested>=0 && requested!=provider))return 0;
  // Read-only availability check first. Unknown must fail closed, not start a download.
  // One line per check (path, result, ms; never the hash), so a slow
  // "Checking the source" shows where the time went.
  struct timespec t0,t1;clock_gettime(CLOCK_MONOTONIC,&t0);
  int cache=debrid_cache_consultar(provider,hash);
  if(cache!=1) {
    if(outside && cache==0)*outside=1;
    clock_gettime(CLOCK_MONOTONIC,&t1);
    printf("[resolver] %s: %s, %ld ms\n",provider==DEBRID_TB?"TorBox":"Premiumize",
           cache==0?"not cached on the service":"cache state unknown",ms(&t0,&t1));
    return 0;
  }
  if(!ajustes_resolver_ativo()||provider!=ajustes_resolver_provedor())return 0;
  int add=provider==DEBRID_TB && ajustes_torbox_adicionar_cache();
  int ok=add
    ? debrid_torbox_resolver_compativel(hash,file,url,size,outside)
    : debrid_resolver_existente(provider,hash,file,url,size,outside);
  clock_gettime(CLOCK_MONOTONIC,&t1);
  printf("[resolver] %s: cached; %s -> %s, %ld ms\n",provider==DEBRID_TB?"TorBox":"Premiumize",
         add?"add-if-cached":"existing account item",ok?"link":"no link (not in the account)",ms(&t0,&t1));
  if(!ajustes_resolver_ativo()||provider!=ajustes_resolver_provedor()){url[0]=0;return 0;}
  if(!ok)url[0]=0;
  return ok;
}

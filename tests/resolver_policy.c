// Entirely mocked providers: no HTTP, credentials or account operations.
#include "resolver_policy.h"
#include "debrid.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int configured,enabled,preferred,state=1,checks,resolutions,change;
int debrid_servico_disponivel(int p){return (configured&(1<<p))!=0;}
int ajustes_torbox_adicionar_cache(void){return 0;}
int debrid_torbox_resolver_compativel(const char*h,int f,char*u,unsigned n,int*o){(void)h;(void)f;(void)u;(void)n;(void)o;assert(0);return 0;}
int ajustes_resolver_ativo(void){return enabled && resolver_politica_mascara()!=0;}
int ajustes_resolver_provedor(void){return resolver_politica_provedor(preferred);}
int debrid_cache_consultar(int p,const char*h){(void)h;assert(p==ajustes_resolver_provedor());checks++;if(change==1)enabled=0;return state;}
int debrid_resolver_existente(int p,const char*h,int f,char*u,unsigned n,int*o){(void)h;(void)f;(void)o;assert(p==ajustes_resolver_provedor());resolutions++;snprintf(u,n,"https://fixture.invalid/cached");if(change==2)enabled=0;return 1;}
static int resolve(int p,int*out){char u[128]="old";int r=resolver_politica_resolver(p,"0123456789012345678901234567890123456789",0,u,sizeof u,out);assert(r ? u[0]!=0 : u[0]==0);return r;}
int main(void){
 int out;
 for(int i=-10;i<=10;i++)assert(resolver_politica_limite(i)==(i<0?0:i>5?5:i));
 configured=(1<<DEBRID_RD)|(1<<DEBRID_AD);enabled=1;
 assert(!resolver_politica_mascara()&&!resolver_politica_opcao(0));assert(!resolve(-1,&out)&&!checks&&!resolutions);
 configured|=1<<DEBRID_TB;enabled=0;assert(!resolve(-1,&out)&&!checks);
 enabled=1;assert(ajustes_resolver_provedor()==DEBRID_TB);assert(resolver_politica_opcao(1)&&!resolver_politica_opcao(2));
 preferred=2;assert(ajustes_resolver_provedor()==DEBRID_TB&&preferred==2);
 configured|=1<<DEBRID_PM;assert(ajustes_resolver_provedor()==DEBRID_PM);
 assert(!resolve(DEBRID_TB,&out)&&!checks);state=-1;assert(!resolve(-1,&out)&&!out&&!resolutions);
 state=0;assert(!resolve(-1,&out)&&out&&!resolutions);
 state=1;assert(resolve(DEBRID_PM,&out)&&!out&&resolutions==1);
 change=1;assert(!resolve(-1,&out)&&resolutions==1);
 enabled=1;change=2;assert(!resolve(-1,&out)&&resolutions==2);
 configured=0;enabled=1;assert(!resolve(-1,&out));
 puts("PASS: resolver Off/no-provider/mismatch/unknown/uncached gates, preference fallback, cached reuse, concurrent disable and 0–5 clamp");
}

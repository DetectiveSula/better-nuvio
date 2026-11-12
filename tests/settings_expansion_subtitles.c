#include "addons_legendas_prazo.c"
static Legenda subs[2];static int subCount,subDone,embeddedCount,chosenEmbedded=-1;
/* Reuse the selector fixture, replacing its stream-track endpoints below. */
int addons_n_legendas(void){return subCount;}
const Legenda *addons_legenda(int i){return i>=0&&i<subCount?&subs[i]:NULL;}
int addons_legendas_prontas(void){return subDone;}
int addons_legendas_copiar(Legenda *out,int cap,unsigned *id,int *generation){(void)id;(void)generation;int n=subCount<cap?subCount:cap;memcpy(out,subs,n*sizeof *out);return n;}
const char *i18n(const char*s){return s;}
int xtream_url_limitada(const char*u){(void)u;return 0;}
int main(void){
 testForcedSubtitles=1;
 snprintf(subs[0].idioma,sizeof subs[0].idioma,"eng");snprintf(subs[0].url,sizeof subs[0].url,"https://fixture.invalid/full-en.srt");
 subCount=1;subDone=1;ling_local_legenda("");testFullSubtitles=1;
 prazo_auto_iniciar();prazo_auto_avancar(10000);assert(prazo_auto_selecionou());
 puts("PASS automatic full English addon subtitles with empty preference and English audio");
 testFullSubtitles=0;prazo_auto_iniciar();prazo_auto_avancar(10000);assert(!prazo_auto_selecionou());
 puts("PASS Off restores legacy empty-preference policy");
 testFullSubtitles=1;ling_local_legenda("none");prazo_auto_iniciar();prazo_auto_avancar(10000);assert(!prazo_auto_selecionou());
 puts("PASS explicit None is never overridden");
 ling_local_legenda("fr");prazo_auto_iniciar();prazo_auto_avancar(10000);assert(!prazo_auto_selecionou());
 puts("PASS saved language is never replaced by app-language fallback");
 ling_local_legenda("");subCount=0;subDone=0;prazo_auto_iniciar();prazo_auto_avancar(1000);assert(legAuto);
 subCount=1;subDone=1;prazo_auto_avancar(4000);assert(prazo_auto_selecionou());
 puts("PASS late addon response still loads automatically");
 testEmbeddedCount=2;strcpy(testEmbedded[0].idioma,"eng");strcpy(testEmbedded[1].idioma,"eng");testEmbedded[0].tipoLeg=LING_LEG_COMUM;testEmbedded[1].tipoLeg=LING_LEG_FORCADA;
 prazo_auto_iniciar();prazo_auto_avancar(10000);assert(!legAuto&&testEmbeddedSelected==0);
 puts("PASS embedded full dialogue selected over forced track with same-language audio");
 LegMem choice={0};strcpy(choice.titulo,"tt123");strcpy(choice.idioma,"none");legmem_guardar(&choice);
 prazo_auto_iniciar();prazo_auto_avancar(10000);assert(!legAuto&&testEmbeddedSelected==-1);
 puts("PASS manual per-title None wins over automatic embedded/addon selection");return 0;
}

#include "profile_editor.h"
#if defined(NV_ANDROID) || defined(NV_PROFILE_EDITOR_TEST)
#include "android.h"
#include "perfis.h"
#include "sessao.h"
#include "nuvem.h"
#include "js.h"
#include "jsw.h"
#include "ajustes.h"
#include "dados.h"
#include "validation_readonly.h"
#include <stdio.h>
#include <string.h>
#ifdef NV_PROFILE_EDITOR_TEST   // host test: android.h only declares these on Android
int android_profile_editor_open(const char *json);
int android_profile_editor_event(char *dst,size_t n);
void android_profile_editor_close(void);
#endif
// The Android panel (ProfileEditor.kt) owns the account calls; this side only
// hands it the profile and applies what the account confirmed. Events:
//   {"request":n,"saved":0}                               closed, nothing saved
//   {"request":n,"saved":1,"name","photo"[,"color"][,"background"]}
//   {"request":n,"deleted":1}
//   {"request":n,"final":0,"pin":0|1}   PIN changed; the panel stays open
static int active, target;
static unsigned serial;
static uint64_t generation;
static char account[128];
// js_texto reports an empty string as missing; "" is a real answer here (no
// photo, standard background), so it is told apart from an absent field.
static int field(const char *e,const char *k,char *d,size_t n){
  char raw[4];
  if(js_texto(e,NULL,k,d,n))return 1;
  if(js_bruto(e,NULL,k,raw,sizeof raw) && !strcmp(raw,"\"\"")){d[0]=0;return 1;}
  return 0;
}
static int byte(float v){ return v<=0.0f?0:v>=1.0f?255:(int)(v*255.0f+0.5f); }
int profile_editor_open(int slot, const char *pin) {
  const ContaPerfil *p=perfis_item(slot);
  char jwt[4096],user[128],accent[8];
  float r,g,b;
  if(active || !p || !sessao_identidade_local_copiar(user,sizeof user,jwt,sizeof jwt))return 0;
  ajustes_acento(&r,&g,&b);snprintf(accent,sizeof accent,"#%02X%02X%02X",byte(r),byte(g),byte(b));
  Jsw w;jsw_iniciar(&w);jsw_obj_ini(&w);
  jsw_ci(&w,"request",++serial);jsw_ci(&w,"profile",p->indice);
  jsw_cs(&w,"name",p->nome);jsw_cs(&w,"photo",p->avatarUrl);jsw_cs(&w,"color",p->corHex);
  jsw_cs(&w,"background",p->fundoUrl);
  jsw_cb(&w,"hasPin",p->temPin);jsw_cb(&w,"primary",p->primario);jsw_ci(&w,"active",perfis_ativo());
  // The PIN just typed to open a locked profile: official Nuvio needs the
  // current PIN to change or remove it. Kept in the panel's memory only.
  if(pin && p->temPin && strlen(pin)==4)jsw_cs(&w,"pin",pin);
  jsw_cs(&w,"accent",accent);jsw_cs(&w,"client",dados_cliente_id());
  jsw_cs(&w,"base",nuvem_url());jsw_cs(&w,"key",nuvem_anon());
  jsw_cs(&w,"token",jwt);jsw_cs(&w,"user",user);
  jsw_cb(&w,"readonly",NV_VALIDATION_READONLY_ENABLED);jsw_obj_fim(&w);
  active=jsw_texto_final(&w) && android_profile_editor_open(jsw_texto_final(&w));
  memset(jwt,0,sizeof jwt);
  if(w.p)memset(w.p,0,w.n);jsw_livre(&w);
  if(active){target=p->indice;generation=perfis_sessao_geracao();snprintf(account,sizeof account,"%s",user);}
  return active;
}
int profile_editor_active(void){return active;}
void profile_editor_close(void){android_profile_editor_close();active=0;serial++;}
void profile_editor_update(void){
  char event[4096],name[64],photo[300],color[16],background[300],user[128],jwt[4096];
  if(active && !perfis_sessao_atual(generation)){profile_editor_close();return;}
  while(android_profile_editor_event(event,sizeof event)){
    if(!active || (unsigned)js_num(event,NULL,"request",0)!=serial)continue;
    if(js_num(event,NULL,"final",1))active=0;
    int saved=(int)js_num(event,NULL,"saved",0),deleted=(int)js_num(event,NULL,"deleted",0);
    int pin=(int)js_num(event,NULL,"pin",-1);
    if(!saved && !deleted && pin<0)continue;
    if(!sessao_identidade_local_copiar(user,sizeof user,jwt,sizeof jwt))continue;
    memset(jwt,0,sizeof jwt);
    if(strcmp(account,user) || !perfis_sessao_travar(generation))continue;
    if(saved && js_texto(event,NULL,"name",name,sizeof name) && field(event,"photo",photo,sizeof photo)){
      perfis_aplicar_edicao(target,name,photo);
      int hasColor=js_texto(event,NULL,"color",color,sizeof color);
      int hasBackground=field(event,"background",background,sizeof background);
      if(hasColor||hasBackground)perfis_aplicar_aparencia(target,hasColor?color:NULL,hasBackground?background:NULL);
    }
    if(deleted)perfis_aplicar_remocao(target);
    if(pin>=0)perfis_aplicar_pin(target,pin);
    perfis_sessao_destravar();
  }
}
#else
int profile_editor_open(int slot,const char *pin){(void)slot;(void)pin;return 0;}
int profile_editor_active(void){return 0;}
void profile_editor_update(void){}
void profile_editor_close(void){}
#endif

#include "postplay_preview.h"
#include "trailerimdb.h"
#include "trailerapple.h"
#include "trailerytb.h"
#include "trailerfonte.h"
#include "extras.h"
#include <string.h>
#include <stdio.h>
#ifdef NV_ANDROID
#include <jni.h>
/* Activity methods marshal work to the UI thread. No NvPlayer entry points. */
static int preview_call(const char *method,const char *signature,const char *url,GfxRect r){
 JNIEnv *env=(JNIEnv*)SDL_AndroidGetJNIEnv();if(!env)return 0;
 jobject act=SDL_AndroidGetActivity();if(!act)return 0;
 jclass cls=(*env)->GetObjectClass(env,act);jmethodID mid=cls?(*env)->GetMethodID(env,cls,method,signature):NULL;
 int result=0;
 if(mid){
  if(url){jstring text=(*env)->NewStringUTF(env,url);if(text){(*env)->CallVoidMethod(env,act,mid,text,r.x,r.y,r.w,r.h);(*env)->DeleteLocalRef(env,text);}}
  else if(!strcmp(signature,"()Z"))result=(*env)->CallBooleanMethod(env,act,mid);
  else if(!strcmp(signature,"()I"))result=(*env)->CallIntMethod(env,act,mid);
  else (*env)->CallVoidMethod(env,act,mid);
 }
 if((*env)->ExceptionCheck(env)){(*env)->ExceptionClear(env);result=0;}
 if(cls)(*env)->DeleteLocalRef(env,cls);(*env)->DeleteLocalRef(env,act);return result;
}
static int backend_supported(void){return preview_call("postPlayPreviewSupported","()Z",NULL,(GfxRect){0});}
static int backend_state(void){return preview_call("postPlayPreviewState","()I",NULL,(GfxRect){0});}
static void backend_close(void){preview_call("postPlayPreviewClose","()V",NULL,(GfxRect){0});}
static void backend_open(const char *url,GfxRect r){preview_call("postPlayPreviewOpen","(Ljava/lang/String;FFFF)V",url,r);}
#else
#ifndef NV_POSTPLAY_PREVIEW_TEST
static int backend_supported(void){return 0;}
static int backend_state(void){return 0;}
static void backend_close(void){}
static void backend_open(const char *url,GfxRect r){(void)url;(void)r;}
#endif
#endif
/* APPLE, THEN YOUTUBE (owner, 26/11/2026: "fix post play recommendations so it
 * has same order of [Apple] then youtube"). Was IMDb only, which answers every
 * app request with a 403 since ~19/11/2026. Same choice as the home hero
 * (trailerfonte_escolher_destaque): the Apple answer is waited for, a source
 * that fails in the player is skipped, and after the wait window a source that
 * never answered counts as having no trailer. */
static char current[64];static Uint32 since,openedAt;static int asked,opened,failed,openedQual;static unsigned falhas;
/* Where the preview player can open: Apple's reduced master is a file in the
 * app's data (file://), YouTube's may be the local server (127.0.0.1). */
static int urlAceita(const char *u){
 return u&&(!strncmp(u,"https://",8)||!strncmp(u,"file:///",8)||!strncmp(u,"http://127.0.0.1:",17));
}
int postplay_preview_supported(void){return backend_supported();}
void postplay_preview_close(void){if(opened)backend_close();current[0]=0;since=openedAt=0;asked=opened=failed=openedQual=0;falhas=0;}
static TrailerDecisao escolher(const char *id,int venceu,const char **u,int *qual){
 TrailerCandidatos c;char cand[4][16];const char *ids[4];int n,i,pronto=1,aj=trailerfonte_ajuste();
 memset(&c,0,sizeof c);
 c.apple=trailerapple_url(id);c.appleRespondeu=trailerapple_respondeu(id)||venceu;
 if(falhas&(1u<<TRF_APPLE)){c.apple=NULL;c.appleRespondeu=1;c.appleFalhou=1;}
 if(trailerfonte_pede_imdb(aj,0)&&!(falhas&(1u<<TRF_IMDB))){c.imdb=trailerimdb_url(id,NULL);c.imdbRespondeu=trailerimdb_respondeu(id)||venceu;}
 else c.imdbRespondeu=1;
 n=extras_hero_trailers_proprios(id,cand,4);
 for(i=0;i<n;i++)ids[i]=cand[i];
 c.youtube=n?trailerytb_primeiro_1080p(ids,n,&pronto):NULL;
 c.youtubeRespondeu=venceu||(n?pronto:extras_hero_trailer_respondeu(id));
 if(falhas&(1u<<TRF_YOUTUBE)){c.youtube=NULL;c.youtubeRespondeu=1;}
 return trailerfonte_escolher_destaque(aj,0,&c,u,qual);
}
void postplay_preview_draw(const PostplayPreviewItem *it,int enabled,unsigned delay,Uint32 now,GfxRect image){
 const char *id=it?it->imdb:NULL;
 if(!enabled||!id||strncmp(id,"tt",2)||strlen(id)>=sizeof current||!backend_supported()) {postplay_preview_close();return;}
 if(strcmp(current,id)){postplay_preview_close();snprintf(current,sizeof current,"%s",id);since=now;}
 if(failed||(Uint32)(now-since)<delay)return;
 if(!asked){
  if(it->titulo&&it->titulo[0])trailerapple_pedir(current,it->titulo,it->meta?it->meta:"",it->serie);
  extras_hero_trailer_pedir(current,it->serie,it->tmdb);
  if(trailerfonte_pede_imdb(trailerfonte_ajuste(),0))trailerimdb_pedir(current);
  asked=1;
 }
 if(!opened){
  const char *url=NULL;int qual=0;
  TrailerDecisao d=escolher(current,(Uint32)(now-since)>delay+12000u,&url,&qual);
  if(d==TRF_ABRE&&url&&*url){
   if(!urlAceita(url)){falhas|=1u<<qual;return;}
   backend_open(url,image);opened=1;openedAt=now;openedQual=qual;
  }
  else if(d!=TRF_ESPERA)failed=1;
  return;
 }
 int state=backend_state();
 if(state<0||((Uint32)(now-openedAt)>12000u&&state!=2)){backend_close();opened=0;falhas|=1u<<openedQual;return;}
 if(state==2)gfx_furo_raio(image,12.0f/image.h);
}

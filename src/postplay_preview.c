#include "postplay_preview.h"
#include "trailerimdb.h"
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
static char current[64];static Uint32 since,openedAt;static int asked,opened,failed;
int postplay_preview_supported(void){return backend_supported();}
void postplay_preview_close(void){if(opened)backend_close();current[0]=0;since=openedAt=0;asked=opened=failed=0;}
void postplay_preview_draw(const char *id,int enabled,unsigned delay,Uint32 now,GfxRect image){
 if(!enabled||!id||strncmp(id,"tt",2)||strlen(id)>=sizeof current||!backend_supported()) {postplay_preview_close();return;}
 if(strcmp(current,id)){postplay_preview_close();snprintf(current,sizeof current,"%s",id);since=now;}
 if(failed||(Uint32)(now-since)<delay)return;
 if(!asked){trailerimdb_pedir(current);asked=1;}
 if(!opened){
  const char *url=trailerimdb_url(current,NULL);
  if(url&&*url){if(strncmp(url,"https://",8)){failed=1;return;}backend_open(url,image);opened=1;openedAt=now;}
  else if(trailerimdb_respondeu(current)||(Uint32)(now-since)>delay+12000u)failed=1;
  return;
 }
 int state=backend_state();
 if(state<0||((Uint32)(now-openedAt)>12000u&&state!=2)){backend_close();opened=0;failed=1;return;}
 if(state==2)gfx_furo_raio(image,12.0f/image.h);
}

#ifndef NV_SUBTITLE_AUTO_POLICY_H
#define NV_SUBTITLE_AUTO_POLICY_H
/* An explicit None or a saved title/language preference always wins. */
static inline const char *nv_subtitle_auto_preference(const char *saved,int enabled,const char *uiLanguage){
 return saved&&*saved?saved:enabled&&uiLanguage?uiLanguage:"";
}
#endif

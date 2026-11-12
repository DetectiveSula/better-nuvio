#ifndef NV_SETTINGS_EXPANSION_POLICY_H
#define NV_SETTINGS_EXPANSION_POLICY_H
#include <stddef.h>
typedef struct { int value; const char *name; } NvSettingsPreset;
static const NvSettingsPreset nvWidthPresets[]={{104,"Compact"},{112,"Dense"},{120,"Standard"},{126,"Balanced"},{134,"Comfort"},{140,"Large"}};
static const NvSettingsPreset nvRadiusPresets[]={{0,"Sharp"},{4,"Subtle"},{8,"Classic"},{12,"Rounded"},{16,"Pill"}};
static const NvSettingsPreset nvEdgePresets[]={{28,"Subtle"},{42,"Balanced"},{56,"Bold"}};
static const NvSettingsPreset nvCoveragePresets[]={{0,"Top only"},{50,"Half"},{100,"Full outline"}};
static inline const char *nv_settings_preset(const NvSettingsPreset *p,size_t n,int value){for(size_t i=0;i<n;i++)if(p[i].value==value)return p[i].name;return NULL;}
static inline unsigned nv_settings_delay_ms(int tenths,int fallback){return (unsigned)(tenths<0||tenths>300?fallback:tenths)*100u;}
static inline int nv_settings_threshold(double position,double duration,int percent){return duration>1 && position>=0 && percent>0 && percent<=100 && position/duration>=(double)percent/100.0;}
static inline int nv_settings_still_due(unsigned completed,int every){return every>0 && completed>=(unsigned)every;}
#endif

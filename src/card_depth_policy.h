#ifndef NV_CARD_DEPTH_POLICY_H
#define NV_CARD_DEPTH_POLICY_H
enum { NV_CARD_POSTERS, NV_CARD_CONTINUE, NV_CARD_EPISODES, NV_CARD_CAST, NV_CARD_TRAILERS, NV_CARD_SURFACES };
static inline int nv_card_depth_enabled(int master,unsigned mask,int surface){return master&&surface>=0&&surface<NV_CARD_SURFACES&&(mask&(1u<<surface));}
static inline float nv_card_depth_strength(int percent){return percent<0?0:percent>100?1:(float)percent/100.0f;}
#endif

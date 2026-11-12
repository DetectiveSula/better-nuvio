#ifndef NV_PROFILE_EDITOR_H
#define NV_PROFILE_EDITOR_H
// `pin`: the PIN just verified to open a locked profile, or NULL.
int profile_editor_open(int slot, const char *pin);
int profile_editor_active(void);
void profile_editor_update(void);
void profile_editor_close(void);
#endif

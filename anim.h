#ifndef _ANIM_H
#define _ANIM_H

anim_t *anim_load(char *filename);
anim_frame_t *anim_frame(anim_t *anim, unsigned int index);

#endif

#ifndef SIM_SOUND_H
#define SIM_SOUND_H

/* Plays the catalog. One call fills the next 1/60 s of samples. */

void sound_open(void);
void sound_render(void);
void sound_close(void);

#endif

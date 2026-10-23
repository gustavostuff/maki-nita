#ifndef SIM_SOUND_H
#define SIM_SOUND_H

#include "board.h"

/* Plays the catalog. sound_bind routes EVENT bytes into it. */

void sound_bind(Board *board);
void sound_open(void);
void sound_render(void);
void sound_close(void);

#endif

#ifndef FW_SOUND_H
#define FW_SOUND_H

#include <stdint.h>

/*
 * One catalog index on EVENT.
 * The console AVR holds the notes. A game only chooses the index.
 * Index 0 is an empty slot. 1 is the laser. 2 is the rock explosion.
 * 3 is the pop.
 */

#define SOUND_POP 3

void sound(uint8_t index);

#endif

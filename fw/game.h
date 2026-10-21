#ifndef FW_GAME_H
#define FW_GAME_H

#include "input.h"

/* A 3x3 block. The stick steps it. The button jumps it. */
void game_reset(void);
void game_tick(const Input *in);

#endif

#ifndef FW_GAME_H
#define FW_GAME_H

#include "input.h"

/* The shooter. The stick flies the ship. The button fires upward. */
void game_reset(void);
void game_tick(const Input *in);

#endif

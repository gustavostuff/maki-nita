#ifndef FW_PLAYER_H
#define FW_PLAYER_H

#include "input.h"

/*
 * The ship. Three columns, three rows:
 *   010
 *   111
 *   111
 * The stick steps it one LED every two ticks.
 * Its bottom row stops on the row above the score.
 */

void player_reset(void);
void player_tick(const Input *in);
void player_draw(void);
int player_nose_x(void);
int player_nose_y(void);
int player_covers(int x, int y);

#endif

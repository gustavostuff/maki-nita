#ifndef ARKALED_H
#define ARKALED_H

#include "input.h"

/*
 * Bricks on rows 2 through 5, a two-row gap above them, and a
 * paddle of four LEDs on the bottom row. The ball waits above the
 * paddle until the paddle moves, then steps up and to one side.
 * collide_step walks that ball. A brick in the way turns off and
 * scores. The bottom row off the paddle ends the round, and so
 * does the last brick. Later steps use arkaled_period.
 * arkaled_seed sets the opening side. Zero becomes 1.
 * arkaled_load places a live ball and leaves the bricks as they are.
 * arkaled_fill, arkaled_wipe, and arkaled_set edit those bricks.
 */

void arkaled_reset(void);
void arkaled_tick(const Input *in);
void arkaled_seed(unsigned n);
void arkaled_load(int pad_x, int x, int y, int vx, int vy);
void arkaled_fill(void);
void arkaled_wipe(void);
void arkaled_set(int x, int y, int on);
int arkaled_on(int x, int y);
int arkaled_count(void);
void arkaled_ball(int *x, int *y, int *vx, int *vy);
int arkaled_period(int ticks);

#endif

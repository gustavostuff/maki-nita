#ifndef ARKALED_H
#define ARKALED_H

#include "input.h"

/*
 * Forty-eight pieces on rows 2 through 5. Each piece is one LED,
 * in columns 2 through 13. Rows 0 and 1 are empty, and so are
 * the two columns on each side. The paddle
 * is four LEDs on the bottom row. The ball waits above the paddle
 * until the paddle moves, then steps up and toward one side.
 * collide_step walks that ball. A piece in the way turns off.
 * The bottom row off the paddle starts a new round, and so does
 * the last piece. Later steps use arkaled_period.
 * arkaled_seed sets the opening side. Zero becomes 1.
 * arkaled_load places a live ball and leaves the pieces as they are.
 * arkaled_fill, arkaled_wipe, and arkaled_set edit those pieces.
 * arkaled_set addresses that one LED.
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

#ifndef PADDLE_H
#define PADDLE_H

#include "input.h"

/*
 * One paddle and one ball. The stick moves the paddle while held.
 * The button does not move it. The paddle steps one LED every two ticks.
 * The ball waits at the center until the paddle moves, then steps
 * toward one of the four corners. The right edge sends the ball back.
 * A ball alongside the paddle reverses its horizontal step and
 * keeps its vertical step. Meeting an end from outside, or leaving
 * past an end, reverses both steps. Later steps use paddle_period.
 * SLOW_BALL in paddle.c holds those later steps at 2 FPS.
 * paddle_seed sets the sequence that picks the opening corner.
 * Zero becomes 1. paddle_load places a live ball for a test.
 */

void paddle_reset(void);
void paddle_tick(const Input *in);
void paddle_seed(unsigned n);
void paddle_load(int pad_y, int x, int y, int vx, int vy);
int paddle_period(int ticks);

#endif

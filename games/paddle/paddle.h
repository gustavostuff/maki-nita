#ifndef PADDLE_H
#define PADDLE_H

#include "input.h"

/*
 * Two paddles and one ball. The stick moves the left paddle while held.
 * The button moves the right paddle while held, up on the first press
 * and down on the next. Releasing the button stops the right paddle.
 * Both step one LED every two ticks.
 * The ball's period is paddle_period(ticks) at 60 Hz.
 */

void paddle_reset(void);
void paddle_tick(const Input *in);
int paddle_period(int ticks);

#endif

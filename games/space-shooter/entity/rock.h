#ifndef FW_ROCK_H
#define FW_ROCK_H

/*
 * Solid 2x2 and 3x3 rocks. They enter above the field and drift down.
 * A new rock falls every 4, 8, 12, or 16 ticks. Over ROCK_RAMP ticks
 * of the round that period shortens to 1. A sideways step, if any,
 * comes every 15 or 30 ticks and is one column left or right.
 * rock_tick returns 1 when a rock meets the ship.
 */

/* 60 Hz ticks. 60 * 60 is one minute. */
#define ROCK_RAMP (60 * 60)

/* Fall period after `age` play ticks. `base` is 4, 8, 12, or 16. */
int rock_fall_period(int base, int age);

void rock_reset(void);
void rock_add(int x, int y, int size, int every, int dx, int xevery);
int rock_tick(void);

#endif

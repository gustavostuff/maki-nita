#ifndef FW_ROCK_H
#define FW_ROCK_H

/*
 * Solid 2x2 and 3x3 rocks. They enter above the field and drift down
 * every 4, 8, 12, or 16 ticks. A sideways step, if any, comes every
 * 15 or 30 ticks and is one column left or right.
 * rock_tick returns 1 when a rock meets the ship.
 */

void rock_reset(void);
void rock_add(int x, int y, int size, int every, int dx, int xevery);
int rock_tick(void);
void rock_draw(void);

#endif

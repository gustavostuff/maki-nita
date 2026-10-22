#ifndef FW_ROCK_H
#define FW_ROCK_H

/*
 * Solid 2x2 and 3x3 rocks. They enter above the field and drift down.
 * Each one steps every 4, 8, 12, or 16 ticks.
 * rock_tick returns 1 when a rock meets the ship.
 */

void rock_reset(void);
void rock_add(int x, int y, int size, int every);
int rock_tick(void);
void rock_draw(void);

#endif

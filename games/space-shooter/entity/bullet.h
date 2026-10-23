#ifndef FW_BULLET_H
#define FW_BULLET_H

/* One LED. It travels up one cell every two ticks. */

void bullet_reset(void);

/* Returns 1 when a bullet is spawned. */
int bullet_fire(int x, int y);
void bullet_tick(void);

/* Removes one bullet that sits inside the rectangle. Returns 1 when that happens. */
int bullet_take_rect(int x, int y, int w, int h);

#endif

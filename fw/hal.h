#ifndef FW_HAL_H
#define FW_HAL_H

/*
 * The only seam between the flush and a host.
 * level 1 is high. hal_step is a short pause, long enough for the
 * MAX7219 setup and pulse minimums on that host.
 */

void hal_din(int level);
void hal_clk(int level);
void hal_load(int level);
void hal_step(void);

#endif

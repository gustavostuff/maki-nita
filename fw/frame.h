#ifndef FW_FRAME_H
#define FW_FRAME_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Thirty-two rows, thirty-two columns, one bit each.
 * frame[0] is the top row. Bit 0 is the leftmost pixel. A set bit is on.
 */

extern uint32_t frame[32];

void pixel(uint8_t x, uint8_t y, bool on);
bool pixel_get(uint8_t x, uint8_t y);
void clear(void);

#endif

#ifndef FW_FRAME_H
#define FW_FRAME_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Sixteen rows, sixteen columns, one bit each.
 * frame[0] is the top row. Bit 0 is the leftmost pixel. A set bit is on.
 */

extern uint16_t frame[16];

void pixel(uint8_t x, uint8_t y, bool on);
bool pixel_get(uint8_t x, uint8_t y);
void clear(void);

#endif

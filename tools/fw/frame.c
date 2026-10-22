#include "frame.h"

uint16_t frame[16];

void pixel(uint8_t x, uint8_t y, bool on)
{
    uint16_t bit;

    if (x > 15 || y > 15)
        return;
    bit = (uint16_t)(1u << x);
    if (on)
        frame[y] |= bit;
    else
        frame[y] &= (uint16_t)~bit;
}

bool pixel_get(uint8_t x, uint8_t y)
{
    if (x > 15 || y > 15)
        return false;
    return (frame[y] & (uint16_t)(1u << x)) != 0;
}

void clear(void)
{
    int i;

    for (i = 0; i < 16; i++)
        frame[i] = 0;
}

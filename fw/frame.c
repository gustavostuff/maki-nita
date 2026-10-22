#include "frame.h"

uint32_t frame[32];

void pixel(uint8_t x, uint8_t y, bool on)
{
    uint32_t bit;

    if (x > 31 || y > 31)
        return;
    bit = 1u << x;
    if (on)
        frame[y] |= bit;
    else
        frame[y] &= ~bit;
}

bool pixel_get(uint8_t x, uint8_t y)
{
    if (x > 31 || y > 31)
        return false;
    return (frame[y] & (1u << x)) != 0;
}

void clear(void)
{
    int i;

    for (i = 0; i < 32; i++)
        frame[i] = 0;
}

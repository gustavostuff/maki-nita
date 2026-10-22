#include "hal.h"

#include <avr/io.h>

void hal_din(int level)
{
    if (level)
        PORTA.OUTSET = PIN4_bm;
    else
        PORTA.OUTCLR = PIN4_bm;
}

void hal_clk(int level)
{
    if (level)
        PORTA.OUTSET = PIN6_bm;
    else
        PORTA.OUTCLR = PIN6_bm;
}

void hal_load(int level)
{
    if (level)
        PORTA.OUTSET = PIN7_bm;
    else
        PORTA.OUTCLR = PIN7_bm;
}

void hal_step(void)
{
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
}

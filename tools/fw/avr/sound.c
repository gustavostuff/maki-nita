#include "sound.h"

#include <avr/interrupt.h>
#include <avr/io.h>

/*
 * PA0 idles high. A timer tick is 100 us. A 0 holds the line low for
 * one tick, a 1 for two, then the line is high for one tick.
 * The byte is least significant bit first. The queue holds eight
 * indexes, which covers a shot and several explosions in one tick.
 */

#define Q 8

static volatile uint8_t queue[Q];
static volatile uint8_t qn;
static volatile uint8_t qr;
static volatile uint8_t qw;
static uint8_t cur;
static uint8_t bit;
static uint8_t low_left;
static uint8_t phase;

static void start_bit(void)
{
    uint8_t one = (uint8_t)((cur >> bit) & 1u);

    low_left = one ? 2 : 1;
    phase = 1;
    PORTA.OUTCLR = PIN0_bm;
}

ISR(TCB0_INT_vect)
{
    TCB0.INTFLAGS = TCB_CAPT_bm;
    if (phase == 1) {
        low_left--;
        if (low_left == 0) {
            PORTA.OUTSET = PIN0_bm;
            phase = 2;
        }
        return;
    }
    if (phase == 2) {
        bit++;
        if (bit < 8) {
            start_bit();
            return;
        }
        phase = 0;
    }
    if (qn == 0)
        return;
    cur = queue[qr];
    qr = (uint8_t)((qr + 1) % Q);
    qn--;
    bit = 0;
    start_bit();
}

void sound(uint8_t index)
{
    uint8_t saved = SREG;

    cli();
    if (qn < Q) {
        queue[qw] = index;
        qw = (uint8_t)((qw + 1) % Q);
        qn++;
    }
    SREG = saved;
}

void sound_init(void)
{
    PORTA.OUTSET = PIN0_bm;
    PORTA.DIRSET = PIN0_bm;
    /* 24 MHz / 2400 = 10 kHz, one tick per 100 us. */
    TCB0.CCMP = 2399;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;
    sei();
}

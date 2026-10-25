#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include "synth.h"
#include "wire.h"

#include <avr/interrupt.h>
#include <avr/io.h>

/*
 * Catalog image for the console AVR.
 * TCB0 runs at 40 kHz. Each tick samples EVENT on PA1.
 * Every other tick writes the mix to DAC0 on PD6, so the sample
 * rate stays SYNTH_RATE.
 */

static void clock_20mhz(void)
{
    _PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, (uint8_t)(0xAu << 2));
}

static void dac_init(void)
{
    /* REFSEL = 2V500. The sample stays off the 5 V rail.
     * OUTEN is bit 6. ENABLE is bit 0.
     * ISC = INPUT_DISABLE keeps the digital buffer off the DAC pin. */
    VREF.DAC0REF = 0x03;
    DAC0.DATA = 512;
    DAC0.CTRLA = (uint8_t)((1u << 6) | 1u);
    PORTD.PIN6CTRL = 0x04;
}

static void timer_init(void)
{
    /* 20 MHz / 500 = 40 kHz. */
    TCB0.CCMP = 499;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;
}

ISR(TCB0_INT_vect)
{
    static uint8_t div;

    TCB0.INTFLAGS = TCB_CAPT_bm;
    wire_sample((PORTA.IN & PIN1_bm) != 0);
    div ^= 1;
    if (div == 0)
        return;
    {
        int index = wire_take();

        if (index >= 0)
            synth_play((uint8_t)index);
        DAC0.DATA = synth_sample();
    }
}

int main(void)
{
    clock_20mhz();
    dac_init();
    wire_reset();
    synth_reset();
    timer_init();
    sei();
    for (;;)
        ;
}

#ifndef F_CPU
#define F_CPU 12000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#include "cart.h"
#include "flush.h"
#include "input.h"
#include "score.h"

void sound_init(void);

/*
 * OSCHFCTRLA reset is 0x0C, which is 4 MHz. FRQSEL sits in bits 5:2.
 * 0x8 there is 12 MHz. The prescaler reset value already leaves it off.
 */
static void clock_12mhz(void)
{
    _PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, (uint8_t)(0x8u << 2));
}

static int closed(uint8_t mask)
{
    return (PORTD.IN & mask) == 0;
}

static void pins_init(void)
{
    PORTA.DIRSET = PIN4_bm | PIN6_bm | PIN7_bm;
    PORTA.OUTCLR = PIN4_bm | PIN6_bm;
    PORTA.OUTSET = PIN7_bm;
    PORTD.PIN1CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN3CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTD.PIN5CTRL = PORT_PULLUPEN_bm;
}

int main(void)
{
    Input in;

    clock_12mhz();
    pins_init();
    input_reset();
    sound_init();
    score_load();
    game_reset();
    drivers_init();
    drivers_flush();

    for (;;) {
        input_tick(closed(PIN1_bm), closed(PIN2_bm), closed(PIN3_bm),
                   closed(PIN4_bm), closed(PIN5_bm), &in);
        game_tick(&in);
        drivers_flush();
        _delay_ms(16);
    }
}

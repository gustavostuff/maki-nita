#ifndef RD_CLOCKS_H
#define RD_CLOCKS_H

/*
 * Target oscillator rates from docs/software.md.
 * The game loop on the host stays 60 Hz. EVENT timing in board.c uses
 * wall-clock 100 us / 200 us pulses and a 40 kHz sample cadence, which
 * match the 20 MHz console AVR timer setup.
 */
#define CART_CPU_HZ 12000000ULL
#define AUDIO_CPU_HZ 20000000ULL
#define TICK_HZ 60ULL
#define TICK_NS (1000000000ULL / TICK_HZ)

#endif

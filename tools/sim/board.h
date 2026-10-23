#ifndef RD_BOARD_H
#define RD_BOARD_H

/*
 * One Maki-Nita board.
 * U1 is the game AVR128DB28. U6 is the audio AVR128DB28.
 * U2 to U5 are the MAX7219 chain:
 * top left, top right, bottom left, bottom right.
 * Four 8x8 matrices make the 16x16 field.
 * PA4 is DIN, PA6 is CLK, PA7 is LOAD.
 * PD1 to PD5 are the switches, active low.
 * EVENT is cart contact B1. U1 pin 22 drives it. U6 pin 23 listens.
 * A pull-up holds EVENT high while the pin is released.
 * Segment and digit pins keep their package numbers. The lit-pixel
 * query is the latched picture, which is what the eye sees while the
 * drivers scan.
 */

#define NDRV 4

#include "chips/avr128db28.h"
#include "chips/max7219.h"

#include <stdint.h>

#define EVENT_Q 8

/* Cart sender on U1 PA0, and the 40 kHz sample clock for U6 PA1. */
typedef struct EventWire {
    uint8_t q[EVENT_Q];
    uint8_t n;
    uint8_t r;
    uint8_t w;
    uint8_t cur;
    uint8_t bit;
    uint8_t low_left;
    uint8_t phase;
    uint64_t next_sample_ns;
    uint64_t next_send_ns;
    void (*on_index)(uint8_t index);
} EventWire;

typedef struct Board {
    Avr128 cpu;
    Avr128 audio;
    Max7219 drv[NDRV];
    Pin contact[5];
    Net vcc;
    Net gnd;
    Net din;
    Net clk;
    Net load;
    Net cascade[NDRV - 1];
    Net dout;
    Net sw[5];
    Net reset;
    Net updi;
    Net event;
    EventWire event_wire;
    uint64_t now_ns;
} Board;

void board_init(Board *board);
void board_eval(Board *board);
void board_advance(Board *board, uint64_t ns);

/* Queues one catalog index. The 100 us tick shifts it out on EVENT. */
void board_send(Board *board, uint8_t index);
void board_on_index(Board *board, void (*fn)(uint8_t index));
void board_drive(Board *board, int avr_pin, Sig level);
void board_set_switches(Board *board, int up, int down, int left, int right, int button);
int board_led_on(const Board *board, int x, int y);
Sig board_sense(const Board *board, int avr_pin);
int board_fault(const Board *board);

#endif

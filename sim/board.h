#ifndef RD_BOARD_H
#define RD_BOARD_H

/*
 * One Maki Nita board.
 * U1 is the AVR128DB28. U2 to U5 are the MAX7219 chain:
 * top left, top right, bottom left, bottom right.
 * Four 8x8 matrices make the 16x16 field.
 * PA4 is DIN, PA6 is CLK, PA7 is LOAD.
 * PD1 to PD5 are the switches, active low.
 * Segment and digit pins keep their package numbers. The lit-pixel
 * query is the latched picture, which is what the eye sees while the
 * drivers scan.
 */

#define NDRV 4

#include "chips/avr128db28.h"
#include "chips/max7219.h"

typedef struct Board {
    Avr128 cpu;
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
    uint64_t now_ns;
} Board;

void board_init(Board *board);
void board_eval(Board *board);
void board_advance(Board *board, uint64_t ns);
void board_drive(Board *board, int avr_pin, Sig level);
void board_set_switches(Board *board, int up, int down, int left, int right, int button);
int board_led_on(const Board *board, int x, int y);
Sig board_sense(const Board *board, int avr_pin);
int board_fault(const Board *board);

#endif

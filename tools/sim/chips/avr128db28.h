#ifndef RD_AVR128DB28_H
#define RD_AVR128DB28_H

/*
 * AVR128DB28-I/SP, 28-pin SPDIP, as a pin model.
 * Package pin numbers follow the datasheet mux table.
 * The model does not execute instructions. Output pins take the level
 * a host drives. Input pins report the settled net.
 */

#include "engine/signal.h"

#define AVR_PINS 28

#define AVR_PA7 1
#define AVR_PC0 2
#define AVR_PC1 3
#define AVR_PC2 4
#define AVR_PC3 5
#define AVR_VDDIO2 6
#define AVR_PD1 7
#define AVR_PD2 8
#define AVR_PD3 9
#define AVR_PD4 10
#define AVR_PD5 11
#define AVR_PD6 12
#define AVR_PD7 13
#define AVR_AVDD 14
#define AVR_GND1 15
#define AVR_PF0 16
#define AVR_PF1 17
#define AVR_PF6 18
#define AVR_UPDI 19
#define AVR_VDD 20
#define AVR_GND2 21
#define AVR_PA0 22
#define AVR_PA1 23
#define AVR_PA2 24
#define AVR_PA3 25
#define AVR_PA4 26
#define AVR_PA5 27
#define AVR_PA6 28

typedef struct Avr128 {
    const char *refdes;
    Pin pin[AVR_PINS];
} Avr128;

void avr_init(Avr128 *avr, const char *refdes);
void avr_drive(Avr128 *avr, int number, Sig level);
Sig avr_sense(const Avr128 *avr, int number);
Pin *avr_pin(Avr128 *avr, int number);

#endif

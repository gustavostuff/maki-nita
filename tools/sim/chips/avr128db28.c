#include "chips/avr128db28.h"

typedef struct AvrPinDef {
    int number;
    const char *name;
    PinDir dir;
    Sig idle; /* driven level for OUT and PWR, ignored for inputs */
} AvrPinDef;

/* Idle levels: LOAD high, DIN low, CLK low. Rails tied to the supplies. */
static const AvrPinDef avr_pins[AVR_PINS] = {
    {AVR_PA7, "PA7", PIN_OUT, SIG_H},
    {AVR_PC0, "PC0", PIN_IN, SIG_Z},
    {AVR_PC1, "PC1", PIN_IN, SIG_Z},
    {AVR_PC2, "PC2", PIN_IN, SIG_Z},
    {AVR_PC3, "PC3", PIN_IN, SIG_Z},
    {AVR_VDDIO2, "VDDIO2", PIN_PWR, SIG_H},
    {AVR_PD1, "PD1", PIN_IN, SIG_Z},
    {AVR_PD2, "PD2", PIN_IN, SIG_Z},
    {AVR_PD3, "PD3", PIN_IN, SIG_Z},
    {AVR_PD4, "PD4", PIN_IN, SIG_Z},
    {AVR_PD5, "PD5", PIN_IN, SIG_Z},
    {AVR_PD6, "PD6", PIN_IN, SIG_Z},
    {AVR_PD7, "PD7", PIN_IN, SIG_Z},
    {AVR_AVDD, "AVDD", PIN_PWR, SIG_H},
    {AVR_GND1, "GND", PIN_PWR, SIG_L},
    {AVR_PF0, "PF0", PIN_IN, SIG_Z},
    {AVR_PF1, "PF1", PIN_IN, SIG_Z},
    {AVR_PF6, "PF6", PIN_IN, SIG_Z},
    {AVR_UPDI, "UPDI", PIN_IN, SIG_Z},
    {AVR_VDD, "VDD", PIN_PWR, SIG_H},
    {AVR_GND2, "GND", PIN_PWR, SIG_L},
    {AVR_PA0, "PA0", PIN_IN, SIG_Z},
    {AVR_PA1, "PA1", PIN_IN, SIG_Z},
    {AVR_PA2, "PA2", PIN_IN, SIG_Z},
    {AVR_PA3, "PA3", PIN_IN, SIG_Z},
    {AVR_PA4, "PA4", PIN_OUT, SIG_L},
    {AVR_PA5, "PA5", PIN_IN, SIG_Z},
    {AVR_PA6, "PA6", PIN_OUT, SIG_L},
};

void avr_init(Avr128 *avr, const char *refdes)
{
    int i;

    avr->refdes = refdes;
    for (i = 0; i < AVR_PINS; i++) {
        const AvrPinDef *def = &avr_pins[i];
        Pin *pin = &avr->pin[def->number - 1];

        pin->name = def->name;
        pin->number = def->number;
        pin->dir = def->dir;
        pin->net = 0;
        if (def->dir == PIN_IN) {
            pin->drive = SIG_Z;
            pin->level = SIG_Z;
        } else {
            pin->drive = def->idle;
            pin->level = def->idle;
        }
    }
}

Pin *avr_pin(Avr128 *avr, int number)
{
    if (number < 1 || number > AVR_PINS)
        return 0;
    return &avr->pin[number - 1];
}

void avr_output(Avr128 *avr, int number, Sig level)
{
    Pin *pin = avr_pin(avr, number);

    if (pin == 0 || pin->dir == PIN_PWR)
        return;
    pin->dir = PIN_OUT;
    pin->drive = level;
    pin->level = level;
}

void avr_drive(Avr128 *avr, int number, Sig level)
{
    Pin *pin = avr_pin(avr, number);

    if (pin == 0 || pin->dir != PIN_OUT)
        return;
    pin->drive = level;
}

Sig avr_sense(const Avr128 *avr, int number)
{
    if (number < 1 || number > AVR_PINS)
        return SIG_X;
    return avr->pin[number - 1].level;
}

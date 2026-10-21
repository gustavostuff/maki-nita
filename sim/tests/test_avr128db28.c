#include "chips/avr128db28.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "fail: %s\n", msg);
        failures++;
    }
}

/* Package numbers are the 28-pin SPDIP datasheet map, not the header macros. */
static const struct {
    int number;
    const char *name;
    PinDir dir;
    Sig idle;
} package[AVR_PINS] = {
    {1, "PA7", PIN_OUT, SIG_H},
    {2, "PC0", PIN_IN, SIG_Z},
    {3, "PC1", PIN_IN, SIG_Z},
    {4, "PC2", PIN_IN, SIG_Z},
    {5, "PC3", PIN_IN, SIG_Z},
    {6, "VDDIO2", PIN_PWR, SIG_H},
    {7, "PD1", PIN_IN, SIG_Z},
    {8, "PD2", PIN_IN, SIG_Z},
    {9, "PD3", PIN_IN, SIG_Z},
    {10, "PD4", PIN_IN, SIG_Z},
    {11, "PD5", PIN_IN, SIG_Z},
    {12, "PD6", PIN_IN, SIG_Z},
    {13, "PD7", PIN_IN, SIG_Z},
    {14, "AVDD", PIN_PWR, SIG_H},
    {15, "GND", PIN_PWR, SIG_L},
    {16, "PF0", PIN_IN, SIG_Z},
    {17, "PF1", PIN_IN, SIG_Z},
    {18, "PF6", PIN_IN, SIG_Z},
    {19, "UPDI", PIN_IN, SIG_Z},
    {20, "VDD", PIN_PWR, SIG_H},
    {21, "GND", PIN_PWR, SIG_L},
    {22, "PA0", PIN_IN, SIG_Z},
    {23, "PA1", PIN_IN, SIG_Z},
    {24, "PA2", PIN_IN, SIG_Z},
    {25, "PA3", PIN_IN, SIG_Z},
    {26, "PA4", PIN_OUT, SIG_L},
    {27, "PA5", PIN_IN, SIG_Z},
    {28, "PA6", PIN_OUT, SIG_L},
};

static void test_package(void)
{
    Avr128 avr;
    int i;

    avr_init(&avr, "U1");
    expect(AVR_PA4 == 26, "PA4 macro is pin 26");
    expect(AVR_PA6 == 28, "PA6 macro is pin 28");
    expect(AVR_PA7 == 1, "PA7 macro is pin 1");
    expect(AVR_PD1 == 7, "PD1 macro is pin 7");
    expect(AVR_PD5 == 11, "PD5 macro is pin 11");
    expect(AVR_PF6 == 18, "RESET macro is pin 18");
    expect(AVR_UPDI == 19, "UPDI macro is pin 19");
    expect(strcmp(avr.refdes, "U1") == 0, "refdes is kept");

    for (i = 0; i < AVR_PINS; i++) {
        Pin *pin = &avr.pin[package[i].number - 1];

        if (pin->number != package[i].number ||
            pin->dir != package[i].dir ||
            strcmp(pin->name, package[i].name) != 0) {
            fprintf(stderr, "fail: pin %d is %s\n", package[i].number, pin->name);
            failures++;
        }
        if (package[i].dir == PIN_IN) {
            if (pin->drive != SIG_Z)
                expect(0, "an input pin does not drive the net");
        } else if (pin->drive != package[i].idle || pin->level != package[i].idle) {
            fprintf(stderr, "fail: pin %d idle level\n", package[i].number);
            failures++;
        }
    }
}

static void test_drive_and_sense(void)
{
    Avr128 avr;
    Net sw;
    Pin contact;

    avr_init(&avr, "U1");
    expect(avr_pin(&avr, 0) == 0, "pin 0 is out of range");
    expect(avr_pin(&avr, 29) == 0, "pin 29 is out of range");
    expect(avr_sense(&avr, 0) == SIG_X, "sensing a missing pin is unknown");

    avr_drive(&avr, AVR_PD1, SIG_L);
    expect(avr.pin[AVR_PD1 - 1].drive == SIG_Z, "an input ignores a drive");
    avr_drive(&avr, AVR_VDD, SIG_L);
    expect(avr.pin[AVR_VDD - 1].drive == SIG_H, "a supply pin ignores a drive");
    avr_drive(&avr, AVR_PA4, SIG_H);
    expect(avr.pin[AVR_PA4 - 1].drive == SIG_H, "PA4 accepts a high");
    avr_drive(&avr, AVR_PA6, SIG_H);
    expect(avr.pin[AVR_PA6 - 1].drive == SIG_H, "PA6 accepts a high");
    avr_drive(&avr, AVR_PA7, SIG_L);
    expect(avr.pin[AVR_PA7 - 1].drive == SIG_L, "PA7 accepts a low");

    net_init(&sw, "UP", SIG_H);
    contact.name = "UP";
    contact.number = 1;
    contact.dir = PIN_OUT;
    contact.drive = SIG_L;
    contact.level = SIG_Z;
    contact.net = 0;
    net_tie(&sw, avr_pin(&avr, AVR_PD1));
    net_tie(&sw, &contact);
    net_settle(&sw);
    expect(avr_sense(&avr, AVR_PD1) == SIG_L, "a closed contact pulls the input low");
    contact.drive = SIG_Z;
    net_settle(&sw);
    expect(avr_sense(&avr, AVR_PD1) == SIG_H, "the pull-up returns the open input high");
}

int main(void)
{
    test_package();
    test_drive_and_sense();
    if (failures) {
        fprintf(stderr, "%d failure%s\n", failures, failures == 1 ? "" : "s");
        return 1;
    }
    printf("avr128db28 ok\n");
    return 0;
}

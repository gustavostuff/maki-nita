#include "chips/max7219.h"

#include <stdio.h>
#include <string.h>

#define HALF_GAP 100u

static int failures;

static void expect(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "fail: %s\n", msg);
        failures++;
    }
}

static const struct {
    int number;
    const char *name;
    PinDir dir;
    Sig idle;
} package[MAX7219_PINS] = {
    {1, "DIN", PIN_IN, SIG_L},
    {2, "DIG0", PIN_OUT, SIG_H},
    {3, "DIG4", PIN_OUT, SIG_H},
    {4, "GND", PIN_PWR, SIG_L},
    {5, "DIG6", PIN_OUT, SIG_H},
    {6, "DIG2", PIN_OUT, SIG_H},
    {7, "DIG3", PIN_OUT, SIG_H},
    {8, "DIG7", PIN_OUT, SIG_H},
    {9, "GND", PIN_PWR, SIG_L},
    {10, "DIG5", PIN_OUT, SIG_H},
    {11, "DIG1", PIN_OUT, SIG_H},
    {12, "LOAD", PIN_IN, SIG_H},
    {13, "CLK", PIN_IN, SIG_L},
    {14, "SEGA", PIN_OUT, SIG_L},
    {15, "SEGF", PIN_OUT, SIG_L},
    {16, "SEGB", PIN_OUT, SIG_L},
    {17, "SEGG", PIN_OUT, SIG_L},
    {18, "ISET", PIN_IN, SIG_Z},
    {19, "V+", PIN_PWR, SIG_H},
    {20, "SEGC", PIN_OUT, SIG_L},
    {21, "SEGE", PIN_OUT, SIG_L},
    {22, "SEGDP", PIN_OUT, SIG_L},
    {23, "SEGD", PIN_OUT, SIG_L},
    {24, "DOUT", PIN_OUT, SIG_L},
};

static void clock_bit(Max7219 *chip, int bit, uint64_t *now)
{
    chip->pin[MAX7219_DIN - 1].level = bit ? SIG_H : SIG_L;
    max7219_edges(chip, *now);
    *now += HALF_GAP;
    chip->pin[MAX7219_CLK - 1].level = SIG_H;
    max7219_edges(chip, *now);
    *now += HALF_GAP;
    chip->pin[MAX7219_CLK - 1].level = SIG_L;
    max7219_edges(chip, *now);
}

static void clock_word(Max7219 *chip, unsigned word, uint64_t *now)
{
    int i;

    for (i = 15; i >= 0; i--)
        clock_bit(chip, (word >> i) & 1, now);
}

static void test_package(void)
{
    Max7219 chip;
    int i;

    max7219_init(&chip, "U2");
    expect(MAX7219_DIN == 1, "DIN is package pin 1");
    expect(MAX7219_CLK == 13, "CLK is package pin 13");
    expect(MAX7219_LOAD == 12, "LOAD is package pin 12");
    expect(MAX7219_DOUT == 24, "DOUT is package pin 24");
    expect(MAX7219_GND1 == 4 && MAX7219_GND2 == 9, "both GND pins are present");
    expect(chip.shutdown == 1, "power-up enters shutdown");
    expect(chip.scan_limit == 0, "power-up scans digit 0 only");
    expect(chip.intensity == 0, "power-up intensity is the minimum");
    expect(chip.decode == 0, "power-up decode is off");
    expect(chip.display_test == 0, "power-up display test is off");
    expect(chip.shift == 0, "power-up shift register is clear");

    for (i = 0; i < MAX7219_PINS; i++) {
        Pin *pin = &chip.pin[package[i].number - 1];

        if (pin->number != package[i].number ||
            pin->dir != package[i].dir ||
            strcmp(pin->name, package[i].name) != 0) {
            fprintf(stderr, "fail: pin %d is %s\n", package[i].number, pin->name);
            failures++;
        }
        if (package[i].dir == PIN_IN) {
            if (pin->drive != SIG_Z)
                expect(0, "an input pin does not drive the net");
        } else if (pin->drive != package[i].idle) {
            fprintf(stderr, "fail: pin %d idle level\n", package[i].number);
            failures++;
        }
    }
    for (i = 0; i < 8; i++)
        expect(chip.digit[i] == 0, "power-up digits are clear");
}

static void test_shift_and_dout(void)
{
    Max7219 chip;
    uint64_t now = 0;

    max7219_init(&chip, "U2");
    clock_word(&chip, 0xA5C3, &now);
    expect(chip.shift == 0xA5C3, "16 clocks leave the word in the shift register");
    expect(chip.pin[MAX7219_DOUT - 1].drive == SIG_H, "DOUT follows D15 on the falling edge");
    expect(chip.fault == MAX7219_OK, "legal 100 ns clock has no timing fault");

    clock_bit(&chip, 0, &now);
    expect(chip.shift == 0x4B86, "the next clock shifts D15 out");
    expect(chip.pin[MAX7219_DOUT - 1].drive == SIG_L, "DOUT falls with the new D15");
}

static void test_latch(void)
{
    Max7219 chip;
    uint64_t now = 0;

    max7219_init(&chip, "U2");
    clock_word(&chip, 0x015A, &now);
    now += 100;
    chip.pin[MAX7219_LOAD - 1].level = SIG_L;
    max7219_edges(&chip, now);
    now += 100;
    chip.pin[MAX7219_LOAD - 1].level = SIG_H;
    max7219_edges(&chip, now);
    expect(chip.digit[0] == 0x5A, "LOAD rising edge writes digit 0");
    expect(chip.fault == MAX7219_OK, "LOAD pulse meets the 50 ns gaps");

    clock_word(&chip, 0x0000, &now);
    now += 100;
    chip.pin[MAX7219_LOAD - 1].level = SIG_L;
    max7219_edges(&chip, now);
    now += 100;
    chip.pin[MAX7219_LOAD - 1].level = SIG_H;
    max7219_edges(&chip, now);
    expect(chip.digit[0] == 0x5A, "no-op leaves the digit register");
}

static void test_short_clock(void)
{
    Max7219 chip;
    uint64_t now = 1000;

    max7219_init(&chip, "U2");
    chip.pin[MAX7219_CLK - 1].level = SIG_H;
    max7219_edges(&chip, now);
    now += 40;
    chip.pin[MAX7219_CLK - 1].level = SIG_L;
    max7219_edges(&chip, now);
    expect(chip.fault == MAX7219_FAULT_TCH, "CLK high of 40 ns is a fault");
}

static void test_code_b_and_scan(void)
{
    Max7219 chip;
    uint64_t on_ns = (MAX7219_SLOT_NS * 31u) / 32u;

    max7219_init(&chip, "U2");
    chip.shutdown = 0;
    chip.wake_ns = 0;
    chip.scan_origin_ns = 5000;
    chip.scan_limit = 7;
    chip.intensity = 15;
    chip.decode = 0x01;
    chip.digit[0] = 0x01;

    expect(max7219_led_on(&chip, 2, 0, 5000), "code B '1' lights segment B");
    expect(max7219_led_on(&chip, 3, 0, 5000), "code B '1' lights segment C");
    expect(!max7219_led_on(&chip, 0, 0, 5000), "code B '1' leaves DP off");
    expect(!max7219_led_on(&chip, 1, 0, 5000), "code B '1' leaves A off");

    chip.decode = 0;
    chip.digit[0] = 0x80;
    expect(max7219_active_digit(&chip, 5000) == 0, "scan starts on digit 0");
    max7219_pins(&chip, 5000);
    expect(chip.pin[MAX7219_DIG0 - 1].drive == SIG_L, "the active digit pin sinks");
    expect(chip.pin[MAX7219_DIG1 - 1].drive == SIG_H, "an idle digit pin sits at V+");
    expect(chip.pin[MAX7219_SEGDP - 1].drive == SIG_H, "a lit segment pin sources");
    expect(chip.pin[MAX7219_SEGA - 1].drive == SIG_L, "an unlit segment pin sits at GND");

    expect(max7219_active_digit(&chip, 5000 + MAX7219_SLOT_NS) == 1,
           "the next slot selects digit 1");
    expect(max7219_active_digit(&chip, 5000 + on_ns) == -1,
           "the 1/32 blank releases the digit");

    chip.shutdown = 1;
    expect(!max7219_led_on(&chip, 0, 0, 5000), "shutdown blanks the latch output");
    expect(chip.digit[0] == 0x80, "shutdown keeps the digit register");
    max7219_pins(&chip, 5000);
    expect(chip.pin[MAX7219_SEGDP - 1].drive == SIG_L, "shutdown pulls segments to GND");
    expect(chip.pin[MAX7219_DIG0 - 1].drive == SIG_H, "shutdown pulls digits to V+");

    chip.display_test = 1;
    expect(max7219_led_on(&chip, 4, 6, 5000), "display test lights every pixel");
}

int main(void)
{
    test_package();
    test_shift_and_dout();
    test_latch();
    test_short_clock();
    test_code_b_and_scan();
    if (failures) {
        fprintf(stderr, "%d failure%s\n", failures, failures == 1 ? "" : "s");
        return 1;
    }
    printf("max7219 ok\n");
    return 0;
}

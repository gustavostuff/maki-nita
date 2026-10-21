#include "flush.h"

#include "frame.h"
#include "hal.h"

static void shift_bit(int bit)
{
    hal_din(bit ? 1 : 0);
    hal_step();
    hal_clk(1);
    hal_step();
    hal_clk(0);
    hal_step();
}

static void send_word(unsigned word)
{
    int i;

    for (i = 15; i >= 0; i--)
        shift_bit((word >> i) & 1);
}

/* w0 is clocked first and lands in the last driver. CLK is left low, DIN low, LOAD high. */
static void send4(unsigned w0, unsigned w1, unsigned w2, unsigned w3)
{
    hal_load(0);
    hal_step();
    send_word(w0);
    send_word(w1);
    send_word(w2);
    send_word(w3);
    hal_din(0);
    hal_step();
    hal_load(1);
    hal_step();
}

static unsigned reg_word(unsigned addr, unsigned data)
{
    return ((addr & 0x0Fu) << 8) | (data & 0xFFu);
}

/* Bit 0 of the row is the left pixel. Bit 7 of the driver byte is the left pixel. */
static unsigned pack_side(unsigned row, int high)
{
    unsigned byte = 0;
    int base = high ? 8 : 0;
    int i;

    for (i = 0; i < 8; i++) {
        if ((row >> (base + i)) & 1u)
            byte |= 1u << (7 - i);
    }
    return byte;
}

static void send_same(unsigned addr, unsigned data)
{
    unsigned word = reg_word(addr, data);

    send4(word, word, word, word);
}

void drivers_init(void)
{
    int d;

    send_same(0x0F, 0x00);
    send_same(0x09, 0x00);
    send_same(0x0A, 0x0F);
    send_same(0x0B, 0x07);
    for (d = 0; d < 8; d++)
        send_same((unsigned)(d + 1), 0x00);
    send_same(0x0C, 0x01);
}

void drivers_flush(void)
{
    int d;

    for (d = 0; d < 8; d++) {
        unsigned top = frame[d];
        unsigned bot = frame[8 + d];
        unsigned addr = (unsigned)(d + 1);

        send4(reg_word(addr, pack_side(bot, 1)),
              reg_word(addr, pack_side(bot, 0)),
              reg_word(addr, pack_side(top, 1)),
              reg_word(addr, pack_side(top, 0)));
    }
}

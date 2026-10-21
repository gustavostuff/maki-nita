#include "board.h"

/*
 * Half-period of the bring-up clock. 100 ns high, 100 ns low.
 * Data changes 100 ns before the rising edge. LOAD stays high for 100 ns.
 * Those gaps sit inside the MAX7219 minimums.
 */
#define HALF_NS 100u

static const unsigned short border[16] = {
    0xFFFF, 0x8001, 0x8001, 0x8001,
    0x8001, 0x8001, 0x8001, 0x8001,
    0x8001, 0x8001, 0x8001, 0x8001,
    0x8001, 0x8001, 0x8001, 0xFFFF,
};

static const int sw_pin[5] = {
    AVR_PD1, AVR_PD2, AVR_PD3, AVR_PD4, AVR_PD5,
};

static const char *sw_name[5] = {
    "UP", "DOWN", "LEFT", "RIGHT", "BUTTON",
};

static void settle_all(Board *board)
{
    int i;

    net_settle(&board->vcc);
    net_settle(&board->gnd);
    net_settle(&board->din);
    net_settle(&board->clk);
    net_settle(&board->load);
    net_settle(&board->dout);
    net_settle(&board->reset);
    net_settle(&board->updi);
    for (i = 0; i < 3; i++)
        net_settle(&board->cascade[i]);
    for (i = 0; i < 5; i++)
        net_settle(&board->sw[i]);
}

void board_eval(Board *board)
{
    int i;

    settle_all(board);
    for (i = 0; i < 4; i++)
        max7219_edges(&board->drv[i], board->now_ns);
    settle_all(board);
    for (i = 0; i < 4; i++)
        max7219_sense(&board->drv[i], board->now_ns);
    for (i = 0; i < 4; i++)
        max7219_pins(&board->drv[i], board->now_ns);
}

void board_advance(Board *board, uint64_t ns)
{
    board->now_ns += ns;
    board_eval(board);
}

void board_drive(Board *board, int avr_pin, Sig level)
{
    avr_drive(&board->cpu, avr_pin, level);
}

Sig board_sense(const Board *board, int avr_pin)
{
    return avr_sense(&board->cpu, avr_pin);
}

static void tie_chip_rails(Board *board, Max7219 *chip)
{
    net_tie(&board->vcc, max7219_pin(chip, MAX7219_VCC));
    net_tie(&board->gnd, max7219_pin(chip, MAX7219_GND1));
    net_tie(&board->gnd, max7219_pin(chip, MAX7219_GND2));
    net_tie(&board->clk, max7219_pin(chip, MAX7219_CLK));
    net_tie(&board->load, max7219_pin(chip, MAX7219_LOAD));
}

void board_init(Board *board)
{
    static const char *refdes[4] = {"U2", "U3", "U4", "U5"};
    int i;

    avr_init(&board->cpu, "U1");
    for (i = 0; i < 4; i++)
        max7219_init(&board->drv[i], refdes[i]);

    for (i = 0; i < 5; i++) {
        board->contact[i].name = sw_name[i];
        board->contact[i].number = i + 1;
        board->contact[i].dir = PIN_OUT;
        board->contact[i].drive = SIG_Z;
        board->contact[i].level = SIG_H;
        board->contact[i].net = 0;
    }

    net_init(&board->vcc, "VCC", SIG_H);
    net_init(&board->gnd, "GND", SIG_L);
    net_init(&board->din, "DIN", SIG_L);
    net_init(&board->clk, "CLK", SIG_L);
    net_init(&board->load, "LOAD", SIG_H);
    net_init(&board->dout, "DOUT", SIG_L);
    net_init(&board->reset, "RESET", SIG_H);
    net_init(&board->updi, "UPDI", SIG_H);
    for (i = 0; i < 3; i++)
        net_init(&board->cascade[i], "CASCADE", SIG_L);
    for (i = 0; i < 5; i++)
        net_init(&board->sw[i], sw_name[i], SIG_H);

    net_tie(&board->vcc, avr_pin(&board->cpu, AVR_VDD));
    net_tie(&board->vcc, avr_pin(&board->cpu, AVR_AVDD));
    net_tie(&board->vcc, avr_pin(&board->cpu, AVR_VDDIO2));
    net_tie(&board->gnd, avr_pin(&board->cpu, AVR_GND1));
    net_tie(&board->gnd, avr_pin(&board->cpu, AVR_GND2));
    net_tie(&board->din, avr_pin(&board->cpu, AVR_PA4));
    net_tie(&board->clk, avr_pin(&board->cpu, AVR_PA6));
    net_tie(&board->load, avr_pin(&board->cpu, AVR_PA7));
    net_tie(&board->reset, avr_pin(&board->cpu, AVR_PF6));
    net_tie(&board->updi, avr_pin(&board->cpu, AVR_UPDI));

    net_tie(&board->din, max7219_pin(&board->drv[0], MAX7219_DIN));
    for (i = 0; i < 4; i++)
        tie_chip_rails(board, &board->drv[i]);
    for (i = 0; i < 3; i++) {
        net_tie(&board->cascade[i], max7219_pin(&board->drv[i], MAX7219_DOUT));
        net_tie(&board->cascade[i], max7219_pin(&board->drv[i + 1], MAX7219_DIN));
    }
    net_tie(&board->dout, max7219_pin(&board->drv[3], MAX7219_DOUT));

    for (i = 0; i < 5; i++) {
        net_tie(&board->sw[i], avr_pin(&board->cpu, sw_pin[i]));
        net_tie(&board->sw[i], &board->contact[i]);
    }

    board->now_ns = 0;
    board_eval(board);
}

static void shift_bit(Board *board, int bit)
{
    board_drive(board, AVR_PA4, bit ? SIG_H : SIG_L);
    board_eval(board);
    board_advance(board, HALF_NS);
    board_drive(board, AVR_PA6, SIG_H);
    board_eval(board);
    board_advance(board, HALF_NS);
    board_drive(board, AVR_PA6, SIG_L);
    board_eval(board);
}

static void send_word(Board *board, unsigned word)
{
    int i;

    for (i = 15; i >= 0; i--)
        shift_bit(board, (word >> i) & 1);
}

/* w0 is clocked first and lands in the last driver. */
static void send4(Board *board, unsigned w0, unsigned w1, unsigned w2, unsigned w3)
{
    board_drive(board, AVR_PA7, SIG_L);
    board_eval(board);
    board_advance(board, HALF_NS);
    send_word(board, w0);
    send_word(board, w1);
    send_word(board, w2);
    send_word(board, w3);
    board_drive(board, AVR_PA7, SIG_H);
    board_eval(board);
    board_advance(board, HALF_NS);
}

static unsigned reg_word(unsigned addr, unsigned data)
{
    return ((addr & 0x0F) << 8) | (data & 0xFF);
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

static void send_same(Board *board, unsigned addr, unsigned data)
{
    unsigned word = reg_word(addr, data);

    send4(board, word, word, word, word);
}

int board_bringup(Board *board)
{
    int d;

    send_same(board, 0x0F, 0x00);
    send_same(board, 0x09, 0x00);
    send_same(board, 0x0A, 0x0F);
    send_same(board, 0x0B, 0x07);
    for (d = 0; d < 8; d++)
        send_same(board, (unsigned)(d + 1), 0x00);
    send_same(board, 0x0C, 0x01);

    for (d = 0; d < 8; d++) {
        unsigned top = border[d];
        unsigned bot = border[8 + d];
        unsigned addr = (unsigned)(d + 1);

        send4(board,
              reg_word(addr, pack_side(bot, 1)),
              reg_word(addr, pack_side(bot, 0)),
              reg_word(addr, pack_side(top, 1)),
              reg_word(addr, pack_side(top, 0)));
    }

    board_advance(board, MAX7219_WAKE_NS);
    return board_fault(board);
}

void board_set_switches(Board *board, int up, int down, int left, int right, int button)
{
    int closed[5];
    int i;

    closed[0] = up;
    closed[1] = down;
    closed[2] = left;
    closed[3] = right;
    closed[4] = button;
    for (i = 0; i < 5; i++)
        board->contact[i].drive = closed[i] ? SIG_L : SIG_Z;
    board_eval(board);
}

int board_led_on(const Board *board, int x, int y)
{
    int block;

    if (x < 0 || x > 15 || y < 0 || y > 15)
        return 0;
    block = (y >= 8 ? 2 : 0) + (x >= 8 ? 1 : 0);
    return max7219_led_on(&board->drv[block], x & 7, y & 7, board->now_ns);
}

int board_fault(const Board *board)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (board->drv[i].fault != MAX7219_OK)
            return board->drv[i].fault;
    }
    return MAX7219_OK;
}

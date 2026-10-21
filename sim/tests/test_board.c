#include "board.h"
#include "flush.h"
#include "frame.h"
#include "game.h"
#include "host.h"
#include "input.h"

#include <stdio.h>

static int failures;

static void expect(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "fail: %s\n", msg);
        failures++;
    }
}

static const unsigned short border[16] = {
    0xFFFF, 0x8001, 0x8001, 0x8001,
    0x8001, 0x8001, 0x8001, 0x8001,
    0x8001, 0x8001, 0x8001, 0x8001,
    0x8001, 0x8001, 0x8001, 0xFFFF,
};

static void draw_border(void)
{
    int x;
    int y;

    clear();
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++)
            pixel((uint8_t)x, (uint8_t)y, (border[y] >> x) & 1u);
    }
}

static void show(Board *board)
{
    sim_bind(board);
    drivers_init();
    drivers_flush();
    board_advance(board, MAX7219_WAKE_NS);
}

static void test_switches(void)
{
    Board board;

    board_init(&board);
    expect(!board_led_on(&board, 0, 0), "power-up leaves the field dark");

    board_set_switches(&board, 1, 0, 0, 0, 1);
    expect(board_sense(&board, AVR_PD1) == SIG_L, "a closed switch pulls the input low");
    expect(board_sense(&board, AVR_PD2) == SIG_H, "an open switch stays high");
    expect(board_sense(&board, AVR_PD5) == SIG_L, "the button is package PD5");
    expect(board_sense(&board, AVR_PF6) == SIG_H, "RESET sits on its pull-up");
    board_set_switches(&board, 0, 0, 0, 0, 0);
    expect(board_sense(&board, AVR_PD1) == SIG_H, "releasing the switch returns the input high");
}

static void test_flush_border(void)
{
    Board board;
    int i;

    board_init(&board);
    draw_border();
    show(&board);
    if (board_fault(&board) != MAX7219_OK) {
        for (i = 0; i < 4; i++) {
            if (board.drv[i].fault != MAX7219_OK)
                fprintf(stderr, "fail: %s timing fault %d\n",
                        board.drv[i].refdes, board.drv[i].fault);
        }
        failures++;
        return;
    }

    expect(board.drv[0].digit[0] == 0xFF, "top-left top row is full");
    expect(board.drv[0].digit[1] == 0x80, "top-left second row keeps the left edge");
    expect(board.drv[1].digit[1] == 0x01, "top-right second row keeps the right edge");
    expect(board.drv[2].digit[1] == 0x80, "bottom-left follows the same edge");
    expect(board.drv[3].digit[1] == 0x01, "the first word reached the last driver");
    expect(board.drv[0].decode == 0, "decode stays off");
    expect(board.drv[1].intensity == 0x0F, "intensity is full scale");
    expect(board.drv[2].scan_limit == 7, "scan limit is all eight digits");
    expect(board.drv[3].shutdown == 0, "the drivers have left shutdown");

    expect(board_led_on(&board, 0, 0), "top-left pixel is on");
    expect(board_led_on(&board, 15, 0), "top-right pixel is on");
    expect(board_led_on(&board, 0, 15), "bottom-left pixel is on");
    expect(board_led_on(&board, 15, 15), "bottom-right pixel is on");
    expect(board_led_on(&board, 0, 1), "left edge stays on");
    expect(!board_led_on(&board, 1, 1), "the inside of the border is off");
    expect(!board_led_on(&board, 8, 1), "the inside of the right block is off");
    expect(board_led_on(&board, 15, 1), "right edge stays on");
}

static void test_square_on_leds(void)
{
    Board board;
    Input in;

    board_init(&board);
    input_reset();
    game_reset();
    show(&board);
    expect(board_fault(&board) == MAX7219_OK, "the square flush stays inside the timings");
    expect(board_led_on(&board, 6, 6), "the middle of the block is lit");
    expect(!board_led_on(&board, 5, 6), "the cell left of the block is dark");
    expect(board_led_on(&board, 8, 8), "the bottom-right of the block is lit");

    board_set_switches(&board, 0, 0, 0, 1, 0);
    input_tick(0, 0, 0, 1, 0, &in);
    game_tick(&in);
    drivers_flush();
    expect(board_led_on(&board, 6, 6), "debounce holds the block for the first tick");

    input_tick(0, 0, 0, 1, 0, &in);
    game_tick(&in);
    drivers_flush();
    expect(!board_led_on(&board, 6, 6), "the old left column turns off");
    expect(board_led_on(&board, 9, 6), "the new right column turns on");
}

int main(void)
{
    test_switches();
    test_flush_border();
    test_square_on_leds();
    if (failures) {
        fprintf(stderr, "%d failure%s\n", failures, failures == 1 ? "" : "s");
        return 1;
    }
    printf("board ok\n");
    return 0;
}

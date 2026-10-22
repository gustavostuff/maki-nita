#include "host.h"

#include "hal.h"

static Board *board;

void sim_bind(Board *next)
{
    board = next;
}

void hal_din(int level)
{
    board_drive(board, AVR_PA4, level ? SIG_H : SIG_L);
    board_eval(board);
}

void hal_clk(int level)
{
    board_drive(board, AVR_PA6, level ? SIG_H : SIG_L);
    board_eval(board);
}

void hal_load(int level)
{
    board_drive(board, AVR_PA7, level ? SIG_H : SIG_L);
    board_eval(board);
}

void hal_step(void)
{
    board_advance(board, 100);
}

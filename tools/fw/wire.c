#include "wire.h"

#include <stdint.h>

/*
 * A low run of 3 to 5 samples is a 0. A low run of 6 or more is a 1.
 * At 40 kHz those bins sit on the 100 us and 200 us pulses, with a
 * sample of slack for where the edge falls.
 */

#define Q 4

static uint8_t queue[Q];
static uint8_t qn;
static uint8_t qr;
static uint8_t qw;
static uint8_t acc;
static uint8_t nbit;
static uint8_t low_n;

void wire_reset(void)
{
    qn = 0;
    qr = 0;
    qw = 0;
    acc = 0;
    nbit = 0;
    low_n = 0;
}

static void push(uint8_t byte)
{
    if (qn >= Q)
        return;
    queue[qw] = byte;
    qw = (uint8_t)((qw + 1) % Q);
    qn++;
}

void wire_sample(int high)
{
    uint8_t bit;

    if (!high) {
        if (low_n < 255)
            low_n++;
        return;
    }
    if (low_n == 0)
        return;
    if (low_n < 3) {
        acc = 0;
        nbit = 0;
        low_n = 0;
        return;
    }
    bit = low_n >= 6 ? 1 : 0;
    acc = (uint8_t)(acc | (bit << nbit));
    nbit++;
    low_n = 0;
    if (nbit < 8)
        return;
    push(acc);
    acc = 0;
    nbit = 0;
}

int wire_take(void)
{
    int byte;

    if (qn == 0)
        return -1;
    byte = queue[qr];
    qr = (uint8_t)((qr + 1) % Q);
    qn--;
    return byte;
}

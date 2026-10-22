#include "persist.h"

/* Tests keep the word in RAM. An empty image reads as erased. */

static uint16_t word = 0xFFFF;

uint16_t persist_read(void)
{
    return word;
}

void persist_write(uint16_t value)
{
    word = value;
}

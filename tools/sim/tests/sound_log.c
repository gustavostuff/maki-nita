#include "sound_log.h"

#define LOG 16

static uint8_t log[LOG];
static int n;

void sound_log_reset(void)
{
    n = 0;
}

int sound_log_count(void)
{
    return n;
}

uint8_t sound_log_at(int i)
{
    if (i < 0 || i >= n)
        return 0;
    return log[i];
}

void sound(uint8_t index)
{
    if (n < LOG)
        log[n++] = index;
}

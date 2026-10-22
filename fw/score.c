#include "score.h"

#include "entity/base_entity.h"
#include "frame.h"

#define SCORE_MAX 256
#define SCORE_BITS 9

static int score;

void score_reset(void)
{
    score = 0;
}

void score_add(int points)
{
    if (points <= 0 || score >= SCORE_MAX)
        return;
    score += points;
    if (score > SCORE_MAX)
        score = SCORE_MAX;
}

void score_draw(void)
{
    int bit;
    int value = score;
    int x;

    for (x = 0; x < FIELD; x++)
        pixel((uint8_t)x, (uint8_t)(FIELD - 1), false);
    for (bit = 0; bit < SCORE_BITS; bit++) {
        if ((value & 1) != 0)
            pixel((uint8_t)(FIELD - 1 - bit), (uint8_t)(FIELD - 1), true);
        value >>= 1;
    }
}

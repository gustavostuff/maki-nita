#include "game.h"

#include "frame.h"

#define ORIGIN_MAX 13
#define STEP_GAP 8

static int x;
static int y;
static int gap;
static uint16_t rng;

static uint16_t next_rand(void)
{
    rng = (uint16_t)(rng * 25173u + 13849u);
    return rng;
}

static void draw(void)
{
    int dx;
    int dy;

    clear();
    for (dy = 0; dy < 3; dy++) {
        for (dx = 0; dx < 3; dx++)
            pixel((uint8_t)(x + dx), (uint8_t)(y + dy), true);
    }
}

static void jump(void)
{
    int here = x + y * 14;
    int pick = (int)(next_rand() % 195u);

    if (pick >= here)
        pick++;
    x = pick % 14;
    y = pick / 14;
}

static void step(int dx, int dy)
{
    int nx = x + dx;
    int ny = y + dy;

    if (nx < 0)
        nx = 0;
    if (ny < 0)
        ny = 0;
    if (nx > ORIGIN_MAX)
        nx = ORIGIN_MAX;
    if (ny > ORIGIN_MAX)
        ny = ORIGIN_MAX;
    x = nx;
    y = ny;
}

void game_reset(void)
{
    x = 6;
    y = 6;
    gap = 0;
    rng = 1;
    draw();
}

void game_tick(const Input *in)
{
    int dx = 0;
    int dy = 0;

    if (in->button_pressed) {
        jump();
        gap = STEP_GAP;
    } else {
        if (in->left && !in->right)
            dx = -1;
        if (in->right && !in->left)
            dx = 1;
        if (in->up && !in->down)
            dy = -1;
        if (in->down && !in->up)
            dy = 1;
        if (dx == 0 && dy == 0)
            gap = 0;
        else if (gap == 0) {
            step(dx, dy);
            gap = STEP_GAP;
        } else
            gap--;
    }
    draw();
}

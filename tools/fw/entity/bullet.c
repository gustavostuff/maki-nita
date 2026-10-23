#include "bullet.h"

#include "base_entity.h"

#define BULLET_MAX 4
#define BULLET_EVERY 2

static const uint8_t dot_row[1] = {SHAPE_ROW1(1)};
static const Shape dot = {dot_row, 1, 1};

static BaseEntity shot[BULLET_MAX];

void bullet_reset(void)
{
    int i;

    for (i = 0; i < BULLET_MAX; i++)
        shot[i].live = 0;
}

int bullet_fire(int x, int y)
{
    int i;

    if (y < 0 || y >= FIELD || x < 0 || x >= FIELD)
        return 0;
    for (i = 0; i < BULLET_MAX; i++) {
        if (shot[i].live)
            continue;
        base_take(&shot[i], x, y, BULLET_EVERY);
        return 1;
    }
    return 0;
}

void bullet_tick(void)
{
    int i;

    for (i = 0; i < BULLET_MAX; i++) {
        if (!shot[i].live)
            continue;
        if (!base_due(&shot[i]))
            continue;
        shot[i].y--;
        if (shot[i].y < 0)
            shot[i].live = 0;
    }
}

void bullet_draw(void)
{
    int i;

    for (i = 0; i < BULLET_MAX; i++)
        base_draw(&shot[i], &dot);
}

int bullet_take_rect(int x, int y, int w, int h)
{
    int i;

    for (i = 0; i < BULLET_MAX; i++) {
        if (!base_in_rect(&shot[i], x, y, w, h))
            continue;
        shot[i].live = 0;
        return 1;
    }
    return 0;
}

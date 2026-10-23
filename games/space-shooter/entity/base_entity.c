#include "base_entity.h"

#include "frame.h"

static int bit_on(uint8_t bits, int col, int width)
{
    return (bits >> (width - 1 - col)) & 1;
}

void base_take(BaseEntity *e, int x, int y, int every)
{
    e->x = x;
    e->y = y;
    e->pace = 0;
    e->every = every;
    e->live = 1;
}

int base_due(BaseEntity *e)
{
    e->pace++;
    if (e->pace < e->every)
        return 0;
    e->pace = 0;
    return 1;
}

int base_hold(BaseEntity *e, int moving)
{
    if (!moving) {
        e->pace = 0;
        return 0;
    }
    if (e->pace > 0) {
        e->pace--;
        return 0;
    }
    e->pace = e->every - 1;
    return 1;
}

void base_clamp(BaseEntity *e, int w, int h)
{
    if (e->x < 0)
        e->x = 0;
    if (e->y < 0)
        e->y = 0;
    if (e->x > FIELD - w)
        e->x = FIELD - w;
    if (e->y > FIELD - h)
        e->y = FIELD - h;
}

void base_draw(const BaseEntity *e, const Shape *shape)
{
    int col;
    int row;

    if (!e->live)
        return;
    for (row = 0; row < shape->h; row++) {
        for (col = 0; col < shape->w; col++) {
            int px;
            int py;

            if (!bit_on(shape->row[row], col, shape->w))
                continue;
            px = e->x + col;
            py = e->y + row;
            if (px < 0 || py < 0 || px >= FIELD || py >= FIELD)
                continue;
            pixel((uint8_t)px, (uint8_t)py, true);
        }
    }
}

int base_covers(const BaseEntity *e, const Shape *shape, int x, int y)
{
    int col = x - e->x;
    int row = y - e->y;

    if (!e->live)
        return 0;
    if (col < 0 || row < 0 || col >= shape->w || row >= shape->h)
        return 0;
    return bit_on(shape->row[row], col, shape->w);
}

int base_in_rect(const BaseEntity *e, int x, int y, int w, int h)
{
    if (!e->live)
        return 0;
    if (e->x < x || e->x >= x + w)
        return 0;
    if (e->y < y || e->y >= y + h)
        return 0;
    return 1;
}

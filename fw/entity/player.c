#include "player.h"

#include "base_entity.h"

#define STEP_EVERY 2

static const uint8_t ship_rows[3] = {
    SHAPE_ROW3(0, 1, 0),
    SHAPE_ROW3(1, 1, 1),
    SHAPE_ROW3(1, 1, 1),
};

static const Shape ship_shape = {ship_rows, 3, 3};

static BaseEntity ship;

void player_reset(void)
{
    base_take(&ship, 6, 12, STEP_EVERY);
}

void player_tick(const Input *in)
{
    int dx = 0;
    int dy = 0;

    if (in->left && !in->right)
        dx = -1;
    if (in->right && !in->left)
        dx = 1;
    if (in->up && !in->down)
        dy = -1;
    if (in->down && !in->up)
        dy = 1;

    if (!base_hold(&ship, dx || dy))
        return;
    ship.x += dx;
    ship.y += dy;
    base_clamp(&ship, ship_shape.w, ship_shape.h);
    /* The bottom row is the score. The ship stops when it touches that row. */
    if (ship.y > FIELD - 1 - ship_shape.h)
        ship.y = FIELD - 1 - ship_shape.h;
}

void player_draw(void)
{
    base_draw(&ship, &ship_shape);
}

int player_nose_x(void)
{
    return ship.x + 1;
}

int player_nose_y(void)
{
    return ship.y;
}

int player_covers(int x, int y)
{
    return base_covers(&ship, &ship_shape, x, y);
}

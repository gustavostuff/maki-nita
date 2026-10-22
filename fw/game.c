#include "game.h"

#include "entity/base_entity.h"
#include "entity/bullet.h"
#include "entity/player.h"
#include "entity/rock.h"
#include "frame.h"
#include "score.h"

#define BURST_TICKS 120

static const uint8_t burst_rows[8] = {
    SHAPE_ROW8(1, 1, 0, 0, 0, 0, 1, 1),
    SHAPE_ROW8(1, 1, 1, 0, 0, 1, 1, 1),
    SHAPE_ROW8(0, 1, 1, 1, 1, 1, 1, 0),
    SHAPE_ROW8(0, 0, 1, 1, 1, 1, 0, 0),
    SHAPE_ROW8(0, 0, 1, 1, 1, 1, 0, 0),
    SHAPE_ROW8(0, 1, 1, 1, 1, 1, 1, 0),
    SHAPE_ROW8(1, 1, 1, 0, 0, 1, 1, 1),
    SHAPE_ROW8(1, 1, 0, 0, 0, 0, 1, 1),
};
static const Shape burst_shape = {burst_rows, 8, 8};
static const BaseEntity burst_at = {.x = 4, .y = 4, .live = 1};

static int over;

static void draw(void)
{
    clear();
    rock_draw();
    bullet_draw();
    player_draw();
    score_draw();
}

static void draw_burst(void)
{
    clear();
    base_draw(&burst_at, &burst_shape);
}

void game_reset(void)
{
    over = 0;
    score_reset();
    player_reset();
    bullet_reset();
    rock_reset();
    draw();
}

void game_tick(const Input *in)
{
    if (over > 0) {
        over--;
        if (over == 0)
            game_reset();
        else
            draw_burst();
        return;
    }

    player_tick(in);
    bullet_tick();
    if (in->button_pressed)
        bullet_fire(player_nose_x(), player_nose_y() - 1);
    if (rock_tick()) {
        over = BURST_TICKS;
        draw_burst();
        return;
    }
    draw();
}

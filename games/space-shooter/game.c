#include "game.h"

#include "entity/bullet.h"
#include "entity/player.h"
#include "entity/rock.h"
#include "frame.h"
#include "over.h"
#include "score.h"
#include "sound.h"

static int over;

static void draw(void)
{
    clear();
    rock_draw();
    bullet_draw();
    player_draw();
}

void game_reset(void)
{
    over = 0;
    over_clear();
    score_reset();
    player_reset();
    bullet_reset();
    rock_reset();
    draw();
}

void game_tick(const Input *in)
{
    if (over) {
        if (over_tick(in))
            game_reset();
        return;
    }

    player_tick(in);
    bullet_tick();
    if (in->button_pressed &&
        bullet_fire(player_nose_x(), player_nose_y() - 1))
        sound(1);
    if (rock_tick()) {
        over = 1;
        over_start();
        return;
    }
    draw();
}

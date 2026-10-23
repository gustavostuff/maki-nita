#include "game.h"

#include "entity/bullet.h"
#include "entity/player.h"
#include "entity/rock.h"
#include "frame.h"
#include "score.h"
#include "sound.h"
#include "text.h"

/* Rows of the end picture. Bit 0 is the left pixel.
 * The mouth is the only part that changes. */
static const uint16_t face_top[4] = {0x0810, 0x1248, 0x1008, 0x1008};
static const uint16_t mouth[3][2] = {
    {0x1188, 0x0240},
    {0x13c8, 0x0000},
    {0x1428, 0x03c0},
};

#define OVER_HOLD 60

static int over;
static int over_hold;
static int mark;

static void paint(int y, uint16_t bits)
{
    int x;

    for (x = 0; x < 16; x++) {
        if ((bits & (uint16_t)(1u << x)) != 0)
            pixel((uint8_t)x, (uint8_t)y, true);
    }
}

static void draw(void)
{
    clear();
    rock_draw();
    bullet_draw();
    player_draw();
}

static void draw_points(void)
{
    char buf[5];
    int n = score_get();
    int i = 4;

    buf[4] = '\0';
    if (n < 0)
        n = 0;
    do {
        buf[--i] = (char)('0' + (n % 10));
        n /= 10;
    } while (n > 0 && i > 0);
    text_draw(1, 8, buf + i);
}

static void draw_over(void)
{
    const uint16_t *rows = mouth[mark + 1];
    int i;

    clear();
    for (i = 0; i < 4; i++)
        paint(i, face_top[i]);
    paint(4, rows[0]);
    paint(5, rows[1]);
    draw_points();
}

void game_reset(void)
{
    over = 0;
    over_hold = 0;
    score_reset();
    player_reset();
    bullet_reset();
    rock_reset();
    draw();
}

void game_tick(const Input *in)
{
    if (over) {
        draw_over();
        if (over_hold > 0)
            over_hold--;
        if (over_hold == 0 && in->button_pressed)
            game_reset();
        return;
    }

    player_tick(in);
    bullet_tick();
    if (in->button_pressed &&
        bullet_fire(player_nose_x(), player_nose_y() - 1))
        sound(1);
    if (rock_tick()) {
        mark = score_judge();
        over = 1;
        over_hold = OVER_HOLD;
        draw_over();
        return;
    }
    draw();
}

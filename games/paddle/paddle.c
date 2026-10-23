#include "paddle.h"

#include "frame.h"
#include "over.h"
#include "score.h"
#include "sound.h"

#include <stdint.h>
#if !defined(__AVR__)
#include <time.h>
#endif

#define FIELD 16
#define PAD_H 4
#define PAD_EVERY 2
#define BALL_SLOW 7
#define RAMP (10 * 60)
#define SLOW_BALL 0

static int over;
static int ly;
static int lpace;
static int bx;
static int by;
static int dx;
static int dy;
static int bpace;
static int age;
static int live;
static uint32_t rng;

int paddle_period(int ticks)
{
    int p;

    if (SLOW_BALL)
        return 60 / 2;
    if (ticks < 0)
        ticks = 0;
    p = BALL_SLOW - ticks / RAMP;
    if (p < 1)
        p = 1;
    return p;
}

static void draw(void)
{
    int i;

    clear();
    for (i = 0; i < PAD_H; i++)
        pixel(0, (uint8_t)(ly + i), true);
    pixel((uint8_t)bx, (uint8_t)by, true);
}

void paddle_seed(unsigned n)
{
    rng = n == 0 ? 1u : (uint32_t)n;
}

static void stir(void)
{
    if (rng == 0) {
#if defined(__AVR__)
        rng = 0xA5A5u;
#else
        rng = (uint32_t)time(NULL);
        if (rng == 0)
            rng = 1u;
#endif
    }
    rng = rng * 1664525u + 1013904223u;
}

static void aim(void)
{
    uint32_t bits = rng >> 30;

    dx = (bits & 1u) ? 1 : -1;
    dy = (bits & 2u) ? 1 : -1;
}

static int step_pad(int *y, int *pace, int dir)
{
    int next;

    if (dir == 0) {
        *pace = 0;
        return 0;
    }
    if (*pace > 0) {
        (*pace)--;
        return 0;
    }
    *pace = PAD_EVERY - 1;
    next = *y + dir;
    if (next < 0)
        next = 0;
    if (next > FIELD - PAD_H)
        next = FIELD - PAD_H;
    if (next == *y)
        return 0;
    *y = next;
    return 1;
}

static int on_pad(int y)
{
    return y >= ly && y < ly + PAD_H;
}

static int ball_step(void)
{
    int nx = bx + dx;
    int ny = by + dy;
    int from_on;
    int dest_on;
    int corner;
    int bounced = 0;

    if (ny < 0 || ny >= FIELD) {
        dy = -dy;
        ny = by + dy;
        bounced = 1;
    }
    if (nx >= FIELD) {
        dx = -dx;
        nx = bx + dx;
        bounced = 1;
    }
    if (nx < 1) {
        from_on = on_pad(by);
        dest_on = on_pad(ny);
        if (!from_on && !dest_on) {
            if (bounced)
                sound(SOUND_POP);
            return 1;
        }
        /* An end counts when the ball arrives from outside or leaves past it. */
        corner = (from_on && !dest_on && (by == ly || by == ly + PAD_H - 1))
            || (!from_on && dest_on && (ny == ly || ny == ly + PAD_H - 1));
        dx = -dx;
        nx = bx + dx;
        if (corner) {
            dy = -dy;
            ny = by + dy;
            if (ny < 0 || ny >= FIELD) {
                dy = -dy;
                ny = by + dy;
            }
        }
        score_add(1);
        bounced = 1;
    }
    bx = nx;
    by = ny;
    if (bounced)
        sound(SOUND_POP);
    return 0;
}

void paddle_reset(void)
{
    over = 0;
    over_clear();
    score_reset();
    ly = (FIELD - PAD_H) / 2;
    lpace = 0;
    bx = FIELD / 2;
    by = FIELD / 2;
    dx = 0;
    dy = 0;
    bpace = 0;
    age = 0;
    live = 0;
    draw();
}

void paddle_load(int pad_y, int x, int y, int vx, int vy)
{
    paddle_reset();
    ly = pad_y;
    bx = x;
    by = y;
    dx = vx;
    dy = vy;
    live = 1;
    bpace = paddle_period(0) - 1;
    if (bpace < 0)
        bpace = 0;
    draw();
}

void paddle_tick(const Input *in)
{
    int ldir = 0;
    int moved;

    if (over) {
        if (over_tick(in))
            paddle_reset();
        return;
    }

    stir();
    if (in->up && !in->down)
        ldir = -1;
    if (in->down && !in->up)
        ldir = 1;
    moved = step_pad(&ly, &lpace, ldir);

    if (!live) {
        if (!moved) {
            draw();
            return;
        }
        live = 1;
        aim();
        if (ball_step()) {
            over = 1;
            over_start();
            return;
        }
    } else {
        age++;
        bpace++;
        if (bpace >= paddle_period(age)) {
            bpace = 0;
            if (ball_step()) {
                over = 1;
                over_start();
                return;
            }
        }
    }
    draw();
}

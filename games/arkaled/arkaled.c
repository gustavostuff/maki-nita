#include "arkaled.h"

#include "collide.h"
#include "frame.h"
#include "over.h"
#include "score.h"
#include "sound.h"

#include <stdint.h>
#if !defined(__AVR__)
#include <time.h>
#endif

#define FIELD 16
#define PAD_W 4
#define PAD_EVERY 2
#define BRICK_Y 2
#define BRICK_H 4
#define BALL_SLOW 7
#define RAMP (10 * 60)

static int over;
static int px;
static int ppace;
static int bx;
static int by;
static int dx;
static int dy;
static int bpace;
static int age;
static int live;
static int remain;
static uint16_t bricks[BRICK_H];
static uint32_t rng;

int arkaled_period(int ticks)
{
    int p;

    if (ticks < 0)
        ticks = 0;
    p = BALL_SLOW - ticks / RAMP;
    if (p < 1)
        p = 1;
    return p;
}

static int brick_at(int x, int y)
{
    if (x < 0 || x >= FIELD || y < BRICK_Y || y >= BRICK_Y + BRICK_H)
        return 0;
    return (bricks[y - BRICK_Y] & (uint16_t)(1u << x)) != 0;
}

int arkaled_on(int x, int y)
{
    return brick_at(x, y);
}

int arkaled_count(void)
{
    return remain;
}

void arkaled_fill(void)
{
    int i;

    for (i = 0; i < BRICK_H; i++)
        bricks[i] = 0xFFFFu;
    remain = FIELD * BRICK_H;
}

void arkaled_wipe(void)
{
    int i;

    for (i = 0; i < BRICK_H; i++)
        bricks[i] = 0;
    remain = 0;
}

void arkaled_set(int x, int y, int on)
{
    uint16_t bit;

    if (x < 0 || x >= FIELD || y < BRICK_Y || y >= BRICK_Y + BRICK_H)
        return;
    bit = (uint16_t)(1u << x);
    if (on) {
        if (bricks[y - BRICK_Y] & bit)
            return;
        bricks[y - BRICK_Y] |= bit;
        remain++;
        return;
    }
    if ((bricks[y - BRICK_Y] & bit) == 0)
        return;
    bricks[y - BRICK_Y] = (uint16_t)(bricks[y - BRICK_Y] & (uint16_t)~bit);
    remain--;
}

static int clear_brick(int x, int y)
{
    if (!brick_at(x, y))
        return 0;
    arkaled_set(x, y, 0);
    return 1;
}

void arkaled_ball(int *x, int *y, int *vx, int *vy)
{
    *x = bx;
    *y = by;
    *vx = dx;
    *vy = dy;
}

static void draw(void)
{
    int x;
    int y;

    clear();
    for (y = 0; y < BRICK_H; y++) {
        uint16_t bits = bricks[y];

        for (x = 0; x < FIELD; x++) {
            if (bits & (uint16_t)(1u << x))
                pixel((uint8_t)x, (uint8_t)(BRICK_Y + y), true);
        }
    }
    for (x = 0; x < PAD_W; x++)
        pixel((uint8_t)(px + x), (uint8_t)(FIELD - 1), true);
    if (bx >= 0 && bx < FIELD && by >= 0 && by < FIELD)
        pixel((uint8_t)bx, (uint8_t)by, true);
}

void arkaled_seed(unsigned n)
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
    dx = ((rng >> 30) & 1u) ? 1 : -1;
    dy = -1;
}

static int step_pad(int dir)
{
    int next;

    if (dir == 0) {
        ppace = 0;
        return 0;
    }
    if (ppace > 0) {
        ppace--;
        return 0;
    }
    ppace = PAD_EVERY - 1;
    next = px + dir;
    if (next < 0)
        next = 0;
    if (next > FIELD - PAD_W)
        next = FIELD - PAD_W;
    if (next == px)
        return 0;
    px = next;
    return 1;
}

static int blocked(int x, int y, void *ctx)
{
    (void)ctx;
    if (x < 0 || x >= FIELD || y < 0)
        return 1;
    if (y >= FIELD)
        return 0;
    if (y == FIELD - 1)
        return x >= px && x < px + PAD_W;
    return brick_at(x, y);
}

static int ball_step(void)
{
    CollideHit hit[2];
    int n;
    int i;
    int broke = 0;

    n = collide_step(&bx, &by, &dx, &dy, blocked, NULL, hit, 2);
    for (i = 0; i < n; i++) {
        if (!clear_brick(hit[i].x, hit[i].y))
            continue;
        score_add(1);
        broke = 1;
    }
    if (n > 0)
        sound(SOUND_POP);
    /* The bottom row is the paddle. An open cell there is a miss. */
    if (by >= FIELD - 1)
        return 1;
    if (broke && remain == 0)
        return 1;
    return 0;
}

static void ready(int fill)
{
    over = 0;
    over_clear();
    score_reset();
    px = (FIELD - PAD_W) / 2;
    ppace = 0;
    bx = FIELD / 2;
    by = FIELD - 2;
    dx = 0;
    dy = 0;
    bpace = 0;
    age = 0;
    live = 0;
    if (fill)
        arkaled_fill();
}

void arkaled_reset(void)
{
    ready(1);
    draw();
}

void arkaled_load(int pad_x, int x, int y, int vx, int vy)
{
    ready(0);
    px = pad_x;
    bx = x;
    by = y;
    dx = vx;
    dy = vy;
    live = 1;
    bpace = arkaled_period(0) - 1;
    if (bpace < 0)
        bpace = 0;
    draw();
}

void arkaled_tick(const Input *in)
{
    int dir = 0;
    int moved;

    if (over) {
        if (over_tick(in))
            arkaled_reset();
        return;
    }

    stir();
    if (in->left && !in->right)
        dir = -1;
    if (in->right && !in->left)
        dir = 1;
    moved = step_pad(dir);

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
        if (bpace >= arkaled_period(age)) {
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

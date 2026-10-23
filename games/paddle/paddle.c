#include "paddle.h"

#include "frame.h"
#include "over.h"
#include "score.h"

#define FIELD 16
#define PAD_H 4
#define PAD_EVERY 2
#define BALL_SLOW 7
#define RAMP (10 * 60)

static int over;
static int ly;
static int ry;
static int lpace;
static int rpace;
static int rdir;
static int rnext;
static int bx;
static int by;
static int dx;
static int dy;
static int bpace;
static int age;

int paddle_period(int ticks)
{
    int p;

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
    for (i = 0; i < PAD_H; i++) {
        pixel(0, (uint8_t)(ly + i), true);
        pixel((uint8_t)(FIELD - 1), (uint8_t)(ry + i), true);
    }
    pixel((uint8_t)bx, (uint8_t)by, true);
}

static int step_pad(int *y, int *pace, int dir)
{
    if (dir == 0) {
        *pace = 0;
        return 0;
    }
    if (*pace > 0) {
        (*pace)--;
        return 0;
    }
    *pace = PAD_EVERY - 1;
    *y += dir;
    if (*y < 0)
        *y = 0;
    if (*y > FIELD - PAD_H)
        *y = FIELD - PAD_H;
    return 1;
}

static int ball_step(void)
{
    int nx = bx + dx;
    int ny = by + dy;
    int top;

    if (ny < 0 || ny >= FIELD) {
        dy = -dy;
        ny = by + dy;
    }
    if (nx < 1 || nx > FIELD - 2) {
        top = nx < 1 ? ly : ry;
        if (ny < top || ny >= top + PAD_H)
            return 1;
        dx = -dx;
        nx = bx;
        score_add(1);
    }
    bx = nx;
    by = ny;
    return 0;
}

void paddle_reset(void)
{
    over = 0;
    over_clear();
    score_reset();
    ly = (FIELD - PAD_H) / 2;
    ry = ly;
    lpace = 0;
    rpace = 0;
    rdir = 0;
    rnext = -1;
    bx = FIELD / 2;
    by = FIELD / 2;
    dx = -1;
    dy = 1;
    bpace = 0;
    age = 0;
    draw();
}

void paddle_tick(const Input *in)
{
    int ldir = 0;

    if (over) {
        if (over_tick(in))
            paddle_reset();
        return;
    }

    if (in->button_pressed) {
        rdir = rnext;
        rnext = -rnext;
        rpace = 0;
    }
    if (!in->button)
        rdir = 0;
    if (in->up && !in->down)
        ldir = -1;
    if (in->down && !in->up)
        ldir = 1;
    step_pad(&ly, &lpace, ldir);
    step_pad(&ry, &rpace, rdir);

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
    draw();
}

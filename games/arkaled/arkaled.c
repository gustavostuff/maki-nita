#include "arkaled.h"

#include "collide.h"
#include "entity.h"
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
#define BRICK_X 2
#define BRICK_Y 2
#define BRICK_COLS 6
#define BRICK_ROWS 4
#define BALL_SLOW 7
#define RAMP (10 * 60)

static const Shape brick_shape = SHAPE1(0, 0, "11");
static const Shape pad_shape = SHAPE1(0, 0, "1111");
static const Shape dot = SHAPE1(0, 0, "1");

static int over;
static int pad_id = -1;
static int ball_id = -1;
static int piece[BRICK_ROWS][BRICK_COLS];
static int ppace;
static int age;
static int live;
static int remain;
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

static int piece_at(int x, int y, int *col, int *row)
{
    if (y < BRICK_Y || y >= BRICK_Y + BRICK_ROWS)
        return 0;
    if (x < BRICK_X || x >= BRICK_X + BRICK_COLS * 2)
        return 0;
    *row = y - BRICK_Y;
    *col = (x - BRICK_X) / 2;
    return 1;
}

static int piece_live(int col, int row)
{
    return entity_get(piece[row][col]) != NULL;
}

int arkaled_on(int x, int y)
{
    int col;
    int row;

    if (!piece_at(x, y, &col, &row))
        return 0;
    return piece_live(col, row);
}

int arkaled_count(void)
{
    return remain;
}

static int spawn_piece(int col, int row)
{
    int x = BRICK_X + col * 2;
    int y = BRICK_Y + row;

    piece[row][col] = entity_spawn(&brick_shape, x, y, 0, 0, 1,
                                   ENTITY_INACTIVE | ENTITY_CAN_BE_COLLIDED);
    return piece[row][col] >= 0;
}

void arkaled_fill(void)
{
    int col;
    int row;

    remain = 0;
    for (row = 0; row < BRICK_ROWS; row++) {
        for (col = 0; col < BRICK_COLS; col++) {
            if (!piece_live(col, row) && !spawn_piece(col, row))
                continue;
            remain++;
        }
    }
}

void arkaled_wipe(void)
{
    int col;
    int row;

    for (row = 0; row < BRICK_ROWS; row++) {
        for (col = 0; col < BRICK_COLS; col++) {
            entity_kill(piece[row][col]);
            piece[row][col] = -1;
        }
    }
    remain = 0;
}

void arkaled_set(int x, int y, int on)
{
    int col;
    int row;

    if (!piece_at(x, y, &col, &row))
        return;
    if (on) {
        if (piece_live(col, row))
            return;
        if (!spawn_piece(col, row))
            return;
        remain++;
        return;
    }
    if (!piece_live(col, row))
        return;
    entity_kill(piece[row][col]);
    piece[row][col] = -1;
    remain--;
}

static int clear_brick(int x, int y)
{
    int col;
    int row;

    if (!piece_at(x, y, &col, &row) || !piece_live(col, row))
        return 0;
    entity_kill(piece[row][col]);
    piece[row][col] = -1;
    remain--;
    return 1;
}

void arkaled_ball(int *x, int *y, int *vx, int *vy)
{
    Entity *ball = entity_get(ball_id);

    if (ball == NULL) {
        *x = 0;
        *y = 0;
        *vx = 0;
        *vy = 0;
        return;
    }
    *x = ball->x;
    *y = ball->y;
    *vx = ball->vx;
    *vy = ball->vy;
}

static void draw(void)
{
    world_draw();
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

static void aim(Entity *ball)
{
    ball->vx = ((rng >> 30) & 1u) ? 1 : -1;
    ball->vy = -1;
}

static int step_pad(Entity *pad, int dir)
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
    next = pad->x + dir;
    if (next < 0)
        next = 0;
    if (next > FIELD - PAD_W)
        next = FIELD - PAD_W;
    if (next == pad->x)
        return 0;
    pad->x = next;
    return 1;
}

static int blocked(int x, int y, void *ctx)
{
    Entity *pad = ctx;

    if (x < 0 || x >= FIELD || y < 0)
        return 1;
    if (y >= FIELD)
        return 0;
    if (y == FIELD - 1)
        return x >= pad->x && x < pad->x + PAD_W;
    return arkaled_on(x, y);
}

static int ball_step(Entity *pad, Entity *ball)
{
    CollideHit hit[2];
    int n;
    int i;
    int broke = 0;
    int x = ball->x;
    int y = ball->y;
    int vx = ball->vx;
    int vy = ball->vy;

    n = collide_step(&x, &y, &vx, &vy, blocked, pad, hit, 2);
    ball->x = x;
    ball->y = y;
    ball->vx = vx;
    ball->vy = vy;
    for (i = 0; i < n; i++) {
        if (!clear_brick(hit[i].x, hit[i].y))
            continue;
        score_add(1);
        broke = 1;
    }
    if (n > 0)
        sound(SOUND_POP);
    /* The bottom row is the paddle. An open cell there is a miss. */
    if (y >= FIELD - 1)
        return 1;
    if (broke && remain == 0)
        return 1;
    return 0;
}

static void spawn_actors(int pad_x, int x, int y)
{
    if (pad_id >= 0)
        entity_kill(pad_id);
    if (ball_id >= 0)
        entity_kill(ball_id);
    pad_id = entity_spawn(&pad_shape, pad_x, FIELD - 1, 0, 0, PAD_EVERY,
                          ENTITY_INACTIVE | ENTITY_CAN_BE_COLLIDED);
    ball_id = entity_spawn(&dot, x, y, 0, 0, BALL_SLOW,
                           ENTITY_INACTIVE | ENTITY_CAN_COLLIDE);
}

static void blank_pieces(void)
{
    int col;
    int row;

    for (row = 0; row < BRICK_ROWS; row++) {
        for (col = 0; col < BRICK_COLS; col++)
            piece[row][col] = -1;
    }
    remain = 0;
}

static void ready(int fill)
{
    Entity *ball;

    over = 0;
    over_clear();
    score_reset();
    ppace = 0;
    age = 0;
    live = 0;
    spawn_actors((FIELD - PAD_W) / 2, FIELD / 2, FIELD - 2);
    ball = entity_get(ball_id);
    if (ball != NULL) {
        ball->vx = 0;
        ball->vy = 0;
        ball->pace = 0;
    }
    if (fill)
        arkaled_fill();
}

void arkaled_reset(void)
{
    world_reset();
    pad_id = -1;
    ball_id = -1;
    blank_pieces();
    ready(1);
    draw();
}

void arkaled_load(int pad_x, int x, int y, int vx, int vy)
{
    Entity *ball;

    ready(0);
    entity_get(pad_id)->x = pad_x;
    ball = entity_get(ball_id);
    ball->x = x;
    ball->y = y;
    ball->vx = vx;
    ball->vy = vy;
    live = 1;
    ball->pace = arkaled_period(0) - 1;
    if (ball->pace < 0)
        ball->pace = 0;
    draw();
}

void arkaled_tick(const Input *in)
{
    Entity *pad = entity_get(pad_id);
    Entity *ball = entity_get(ball_id);
    int dir = 0;
    int moved;

    if (over) {
        if (over_tick(in))
            arkaled_reset();
        return;
    }
    if (pad == NULL || ball == NULL)
        return;

    stir();
    if (in->left && !in->right)
        dir = -1;
    if (in->right && !in->left)
        dir = 1;
    moved = step_pad(pad, dir);

    if (!live) {
        if (!moved) {
            draw();
            return;
        }
        live = 1;
        aim(ball);
        if (ball_step(pad, ball)) {
            over = 1;
            over_start();
            return;
        }
    } else {
        age++;
        ball->every = arkaled_period(age);
        ball->pace++;
        if (ball->pace >= ball->every) {
            ball->pace = 0;
            if (ball_step(pad, ball)) {
                over = 1;
                over_start();
                return;
            }
        }
    }
    draw();
}

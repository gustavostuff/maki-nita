#include "paddle.h"

#include "entity.h"
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
#define SLOW_BALL 1

static const Shape pad_shape = SHAPE4(0, 0, "1", "1", "1", "1");
static const Shape dot = SHAPE1(0, 0, "1");

static int over;
static int pad_id = -1;
static int ball_id = -1;
static int lpace;
static int age;
static int live;
static uint32_t rng;

int paddle_period(int ticks) {
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

static void draw(void) { world_draw(); }

void paddle_seed(unsigned n) { rng = n == 0 ? 1u : (uint32_t)n; }

static void stir(void) {
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

static void aim(Entity *ball) {
  uint32_t bits = rng >> 30;

  ball->vx = (bits & 1u) ? 1 : -1;
  ball->vy = (bits & 2u) ? 1 : -1;
}

static int step_pad(Entity *pad, int dir) {
  int next;

  if (dir == 0) {
    lpace = 0;
    return 0;
  }
  if (lpace > 0) {
    lpace--;
    return 0;
  }
  lpace = PAD_EVERY - 1;
  next = pad->y + dir;
  if (next < 0)
    next = 0;
  if (next > FIELD - PAD_H)
    next = FIELD - PAD_H;
  if (next == pad->y)
    return 0;
  pad->y = next;
  return 1;
}

static int on_pad(const Entity *pad, int y) {
  return y >= pad->y && y < pad->y + PAD_H;
}

static int ball_step(Entity *pad, Entity *ball) {
  int bx = ball->x;
  int by = ball->y;
  int dx = ball->vx;
  int dy = ball->vy;
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
    from_on = on_pad(pad, by);
    dest_on = on_pad(pad, ny);
    if (!from_on && !dest_on) {
      if (bounced)
        sound(SOUND_POP);
      ball->vx = dx;
      ball->vy = dy;
      return 1;
    }
    /* An end counts when the ball arrives from outside or leaves past it. */
    corner =
        (from_on && !dest_on && (by == pad->y || by == pad->y + PAD_H - 1)) ||
        (!from_on && dest_on && (ny == pad->y || ny == pad->y + PAD_H - 1));
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
  ball->x = nx;
  ball->y = ny;
  ball->vx = dx;
  ball->vy = dy;
  if (bounced)
    sound(SOUND_POP);
  return 0;
}

static void spawn_actors(int pad_y, int x, int y) {
  world_reset();
  pad_id = entity_spawn(&pad_shape, 0, pad_y, 0, 0, PAD_EVERY, ENTITY_INACTIVE);
  ball_id = entity_spawn(&dot, x, y, 0, 0, BALL_SLOW,
                         ENTITY_INACTIVE | ENTITY_CAN_COLLIDE);
}

void paddle_reset(void) {
  over = 0;
  over_clear();
  score_reset();
  lpace = 0;
  age = 0;
  live = 0;
  spawn_actors((FIELD - PAD_H) / 2, FIELD / 2, FIELD / 2);
  draw();
}

void paddle_load(int pad_y, int x, int y, int vx, int vy) {
  Entity *ball;

  paddle_reset();
  entity_get(pad_id)->y = pad_y;
  ball = entity_get(ball_id);
  ball->x = x;
  ball->y = y;
  ball->vx = vx;
  ball->vy = vy;
  live = 1;
  ball->pace = paddle_period(0) - 1;
  if (ball->pace < 0)
    ball->pace = 0;
  draw();
}

void paddle_tick(const Input *in) {
  Entity *pad = entity_get(pad_id);
  Entity *ball = entity_get(ball_id);
  int ldir = 0;
  int moved;

  if (over) {
    if (over_tick(in))
      paddle_reset();
    return;
  }
  if (pad == NULL || ball == NULL)
    return;

  stir();
  if (in->up && !in->down)
    ldir = -1;
  if (in->down && !in->up)
    ldir = 1;
  moved = step_pad(pad, ldir);

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
    ball->every = paddle_period(age);
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

#include "rock.h"

#include "base_entity.h"
#include "bullet.h"
#include "player.h"

#include <stdint.h>

#define ROCK_MAX 4
#define SPAWN_EVERY 36

static const int period[4] = {4, 8, 12, 16};

static const uint8_t solid2_rows[2] = {
    SHAPE_ROW2(1, 1),
    SHAPE_ROW2(1, 1),
};
static const uint8_t solid3_rows[3] = {
    SHAPE_ROW3(1, 1, 1),
    SHAPE_ROW3(1, 1, 1),
    SHAPE_ROW3(1, 1, 1),
};
static const Shape solid2 = {solid2_rows, 2, 2};
static const Shape solid3 = {solid3_rows, 3, 3};

typedef struct Rock {
    BaseEntity e;
    int size;
} Rock;

static Rock rock[ROCK_MAX];
static int spawn_wait;
static uint16_t rng;

static const Shape *shape_of(const Rock *r)
{
    return r->size == 2 ? &solid2 : &solid3;
}

static uint16_t next_rand(void)
{
    rng = (uint16_t)(rng * 25173u + 13849u);
    return rng;
}

void rock_reset(void)
{
    int i;

    for (i = 0; i < ROCK_MAX; i++)
        rock[i].e.live = 0;
    spawn_wait = 12;
    rng = 1;
}

void rock_add(int x, int y, int size, int every)
{
    int i;

    if (size != 2 && size != 3)
        return;
    if (every != 4 && every != 8 && every != 12 && every != 16)
        return;
    if (x < 0 || x > FIELD - size)
        return;
    for (i = 0; i < ROCK_MAX; i++) {
        if (rock[i].e.live)
            continue;
        base_take(&rock[i].e, x, y, every);
        rock[i].size = size;
        return;
    }
}

static int hits_player(const Rock *r)
{
    const Shape *shape = shape_of(r);
    int dx;
    int dy;

    for (dy = 0; dy < shape->h; dy++) {
        for (dx = 0; dx < shape->w; dx++) {
            if (player_covers(r->e.x + dx, r->e.y + dy))
                return 1;
        }
    }
    return 0;
}

static void spawn(void)
{
    uint16_t r = next_rand();
    int size = (r & 1u) ? 2 : 3;
    /* The low bits of this generator count, so the pace comes from the top. */
    int every = period[(r >> 13) & 3u];
    int x = (int)((r >> 3) % (uint16_t)(FIELD - size));

    rock_add(x, -size, size, every);
}

int rock_tick(void)
{
    int i;
    int crashed = 0;

    spawn_wait--;
    if (spawn_wait <= 0) {
        spawn();
        spawn_wait = SPAWN_EVERY;
    }

    for (i = 0; i < ROCK_MAX; i++) {
        Rock *r = &rock[i];

        if (!r->e.live)
            continue;
        if (base_due(&r->e))
            r->e.y++;
        if (r->e.y >= FIELD) {
            r->e.live = 0;
            continue;
        }
        if (bullet_take_rect(r->e.x, r->e.y, r->size, r->size)) {
            r->e.live = 0;
            continue;
        }
        if (hits_player(r))
            crashed = 1;
    }
    return crashed;
}

void rock_draw(void)
{
    int i;

    for (i = 0; i < ROCK_MAX; i++)
        base_draw(&rock[i].e, shape_of(&rock[i]));
}

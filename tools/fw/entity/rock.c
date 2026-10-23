#include "rock.h"

#include "base_entity.h"
#include "bullet.h"
#include "player.h"
#include "score.h"
#include "sound.h"

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
    int fall;
    int dx;
    int xevery;
    int xpace;
} Rock;

static Rock rock[ROCK_MAX];
static int spawn_wait;
static int age;
static uint16_t rng;

int rock_fall_period(int base, int age_ticks)
{
    int drop;

    if (base <= 1)
        return 1;
    if (age_ticks < 0)
        age_ticks = 0;
    if (age_ticks > ROCK_RAMP)
        age_ticks = ROCK_RAMP;
    drop = (base - 1) * age_ticks / ROCK_RAMP;
    return base - drop;
}

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
    age = 0;
    rng = 1;
}

void rock_add(int x, int y, int size, int every, int dx, int xevery)
{
    int i;

    if (size != 2 && size != 3)
        return;
    if (every != 4 && every != 8 && every != 12 && every != 16)
        return;
    if (dx < -1 || dx > 1)
        return;
    if (xevery != 15 && xevery != 30)
        return;
    if (x < 0 || x > FIELD - size)
        return;
    for (i = 0; i < ROCK_MAX; i++) {
        if (rock[i].e.live)
            continue;
        base_take(&rock[i].e, x, y, every);
        rock[i].size = size;
        rock[i].fall = every;
        rock[i].dx = dx;
        rock[i].xevery = xevery;
        rock[i].xpace = 0;
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
    /* Low bits of this generator count. The fall period stays in bits
     * 13 and 14. The sideways step uses the bits just under those. */
    int every = period[(r >> 13) & 3u];
    int xevery = (r & 0x8000u) ? 15 : 30;
    int dx = (int)(((r >> 10) & 7u) % 3u) - 1;
    int x = (int)((r >> 3) % (uint16_t)(FIELD - size));

    rock_add(x, -size, size, every, dx, xevery);
}

int rock_tick(void)
{
    int i;
    int crashed = 0;

    age++;
    spawn_wait--;
    if (spawn_wait <= 0) {
        spawn();
        spawn_wait = SPAWN_EVERY;
    }

    for (i = 0; i < ROCK_MAX; i++) {
        Rock *r = &rock[i];

        if (!r->e.live)
            continue;
        r->e.every = rock_fall_period(r->fall, age);
        if (base_due(&r->e))
            r->e.y++;
        if (r->dx != 0) {
            r->xpace++;
            if (r->xpace >= r->xevery) {
                int nx = r->e.x + r->dx;

                r->xpace = 0;
                if (nx < 0)
                    nx = 0;
                if (nx > FIELD - r->size)
                    nx = FIELD - r->size;
                r->e.x = nx;
            }
        }
        if (r->e.y >= FIELD) {
            r->e.live = 0;
            continue;
        }
        if (bullet_take_rect(r->e.x, r->e.y, r->size, r->size)) {
            r->e.live = 0;
            score_add(1);
            sound(2);
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

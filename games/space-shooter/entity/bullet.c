#include "bullet.h"

#include "entity.h"

#define FIELD 16
#define BULLET_MAX 8
#define BULLET_EVERY 2

#if BULLET_MAX + 32 + 1 > ENTITY_MAX
#error the shooter does not fit in the entity table
#endif

static const Shape dot = SHAPE1(0, 0, "1");

static int shot[BULLET_MAX] = {-1, -1, -1, -1, -1, -1, -1, -1};

void bullet_reset(void)
{
    int i;

    for (i = 0; i < BULLET_MAX; i++) {
        entity_kill(shot[i]);
        shot[i] = -1;
    }
}

int bullet_fire(int x, int y)
{
    int i;

    if (y < 0 || y >= FIELD || x < 0 || x >= FIELD)
        return 0;
    for (i = 0; i < BULLET_MAX; i++) {
        if (shot[i] >= 0 && entity_get(shot[i]) != NULL)
            continue;
        shot[i] = entity_spawn(&dot, x, y, 0, -1, BULLET_EVERY, 0);
        return shot[i] >= 0;
    }
    return 0;
}

void bullet_tick(void)
{
    int i;

    for (i = 0; i < BULLET_MAX; i++) {
        Entity *e;

        if (shot[i] < 0)
            continue;
        entity_step(shot[i]);
        e = entity_get(shot[i]);
        if (e == NULL || e->y < 0) {
            entity_kill(shot[i]);
            shot[i] = -1;
        }
    }
}

int bullet_take_rect(int x, int y, int w, int h)
{
    int i;

    for (i = 0; i < BULLET_MAX; i++) {
        Entity *e = entity_get(shot[i]);

        if (e == NULL)
            continue;
        if (e->x < x || e->x >= x + w || e->y < y || e->y >= y + h)
            continue;
        entity_kill(shot[i]);
        shot[i] = -1;
        return 1;
    }
    return 0;
}

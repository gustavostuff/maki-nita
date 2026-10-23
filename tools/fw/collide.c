#include "collide.h"

#include <stddef.h>

static void note(CollideHit *hit, int cap, int *n, int x, int y)
{
    if (hit != NULL && *n < cap) {
        hit[*n].x = x;
        hit[*n].y = y;
    }
    (*n)++;
}

int collide_step(int *x, int *y, int *dx, int *dy,
                 collide_solid_fn solid, void *ctx,
                 CollideHit *hit, int cap)
{
    int sx = *x;
    int sy = *y;
    int vx = *dx;
    int vy = *dy;
    int ndx = vx;
    int ndy = vy;
    int block_x = 0;
    int block_y = 0;
    int block_xy = 0;
    int n = 0;

    if (cap < 0)
        cap = 0;
    if (vx != 0)
        block_x = solid(sx + vx, sy, ctx) ? 1 : 0;
    if (vy != 0)
        block_y = solid(sx, sy + vy, ctx) ? 1 : 0;
    if (vx != 0 && vy != 0)
        block_xy = solid(sx + vx, sy + vy, ctx) ? 1 : 0;

    if (block_x)
        note(hit, cap, &n, sx + vx, sy);
    if (block_y)
        note(hit, cap, &n, sx, sy + vy);
    if (!block_x && !block_y && block_xy)
        note(hit, cap, &n, sx + vx, sy + vy);

    if (block_x)
        ndx = -vx;
    if (block_y)
        ndy = -vy;
    if (!block_x && !block_y && block_xy) {
        ndx = -vx;
        ndy = -vy;
    }

    if (ndx != 0 && !solid(sx + ndx, sy, ctx))
        *x = sx + ndx;
    if (ndy != 0 && !solid(*x, sy + ndy, ctx))
        *y = sy + ndy;
    if (solid(*x, *y, ctx)) {
        *x = sx;
        *y = sy;
    }
    *dx = ndx;
    *dy = ndy;
    return n;
}

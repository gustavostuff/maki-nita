#ifndef FW_COLLIDE_H
#define FW_COLLIDE_H

/*
 * One cell steps by dx and dy. Each of those is -1, 0, or 1.
 * solid returns 1 when that cell blocks the step.
 * The step reads at most three cells: the horizontal neighbor,
 * the vertical neighbor, and the diagonal cell.
 * A blocking neighbor reverses that axis. When both neighbors
 * block, or only the diagonal cell blocks, both axes reverse.
 * The horizontal part lands before the vertical part, and only
 * when that landing cell is open. The ball stays out of a
 * blocking cell.
 * hit receives each blocking cell the step touched, up to cap.
 * The return value is how many cells were touched.
 */

typedef int (*collide_solid_fn)(int x, int y, void *ctx);

typedef struct CollideHit {
    int x;
    int y;
} CollideHit;

int collide_step(int *x, int *y, int *dx, int *dy,
                 collide_solid_fn solid, void *ctx,
                 CollideHit *hit, int cap);

#endif

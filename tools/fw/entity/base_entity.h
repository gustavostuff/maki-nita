#ifndef FW_BASE_ENTITY_H
#define FW_BASE_ENTITY_H

#include <stdint.h>

/*
 * A cell on the field, plus the timer that says when it may step.
 * The picture is a shared Shape. An instance does not store pixels.
 *
 * pace counts toward every. base_due steps free-running objects.
 * base_hold steps an object while a control is held, and a release
 * starts the next press on the following tick.
 */

#define FIELD 16

#define SHAPE_ROW1(a) ((uint8_t)(a))
#define SHAPE_ROW2(a, b) ((uint8_t)(((a) << 1) | (b)))
#define SHAPE_ROW3(a, b, c) ((uint8_t)(((a) << 2) | ((b) << 1) | (c)))
#define SHAPE_ROW8(a, b, c, d, e, f, g, h) \
    ((uint8_t)(((a) << 7) | ((b) << 6) | ((c) << 5) | ((d) << 4) | \
               ((e) << 3) | ((f) << 2) | ((g) << 1) | (h)))

typedef struct BaseEntity {
    int x;
    int y;
    int pace;
    int every;
    int live;
} BaseEntity;

typedef struct Shape {
    const uint8_t *row;
    int w;
    int h;
} Shape;

void base_take(BaseEntity *e, int x, int y, int every);
int base_due(BaseEntity *e);
int base_hold(BaseEntity *e, int moving);
void base_clamp(BaseEntity *e, int w, int h);
void base_draw(const BaseEntity *e, const Shape *shape);
int base_covers(const BaseEntity *e, const Shape *shape, int x, int y);
int base_in_rect(const BaseEntity *e, int x, int y, int w, int h);

#endif

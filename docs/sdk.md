# Maki-Nita SDK

The entity world lives in `tools/fw/entity.h` and `tools/fw/entity.c`. Every cart links that file. The software doc covers the 60 Hz tick, the framebuffer, the controls, the sound catalog, and the three carts.

## Slots

A cart keeps up to 64 entities but it might use may use fewer. The software doc lists each cart's own limit.

```c
int entity_spawn(const Shape *shape, int x, int y, int vx, int vy,
                 int every, unsigned flags);
void entity_kill(int id);
Entity *entity_get(int id);
void world_reset(void);
```

`entity_spawn` returns a slot index. It returns -1 when every slot is live, or when the shape is missing, wider or taller than 4, or has its origin outside the box. `entity_kill` frees one slot. An index outside 0..63 is ignored. `entity_get` returns the entity, and returns null when the slot is free or the index is out of range. `world_reset` frees every slot.

## Coordinates

The screen is columns 0..15 and rows 0..15. A one-screen margin around it makes the live area -8..23 on each axis. That 32 by 32 grid is 128 bytes. `world_draw` clears the grid, stamps every living entity, and copies the center window into `frame`.

`x` and `y` are the origin, one cell of the shape box. The cell may be lit or dark. Pixels may hang outside the grid while the origin is inside. Those pixels are clipped.

`entity_hidden` is true when the entity is live and no lit pixel lies on the screen. A freed slot is empty, so `entity_hidden` is false for it.

## Motion

```c
int entity_step(int id);
void world_tick(void);
```

`vx` and `vy` are the step, and each one is -1, 0, or 1. `every` is how many 60 Hz ticks pass between steps. `pace` counts toward `every`. Spawn sets `pace` to 0. A game may write `every` or `pace` as a rally goes on.

`entity_step` advances one entity. When `pace` reaches `every`, the entity moves by `vx` and `vy` together and `pace` returns to 0. The call returns 1 when the entity moved. `world_tick` is that step for every live entity.

`ENTITY_INACTIVE` skips the step and leaves `pace` and the stored speed alone.

After a step, an origin outside -8..23 frees the slot. A move that frees the slot still counts as a step. The next `entity_get` for that index returns null.

## Shapes

A shape is one 16-bit word in flash. `SHAPE1` through `SHAPE4` pack rows written as strings of `0` and `1`. Width and height come from those strings and run from 1 to 4. An instance does not copy the pixels. The origin is `(ox, oy)` inside the box. The ship nose is cell `(1, 0)`.

```c
static const Shape ship = SHAPE3(1, 0,
    "010",
    "111",
    "111");
```

## Collision

```c
int entity_covers(int id, int x, int y);
int entity_overlaps(int a, int b);
void world_pairs(void (*fn)(int a, int b, void *user), void *user);
```

`ENTITY_CAN_COLLIDE` and `ENTITY_CAN_BE_COLLIDED` mark the two sides of a pair. `world_pairs` calls `fn` once for each pair where the first entity can collide, the second can be collided with, and any lit pixels share a cell. The same two entities produce a second call when both flags are set on both. `entity_covers` tests one world cell. `entity_overlaps` tests two entities, and rejects the pair when the boxes miss.

The report leaves the outcome to the game. Bounce, score, and removal stay in the cart.

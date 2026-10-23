#include "arkaled.h"
#include "collide.h"
#include "frame.h"
#include "input.h"
#include "score.h"
#include "sound.h"
#include "tests/report.h"
#include "tests/sound_log.h"

#include <stddef.h>

struct Grid {
    const char **row;
    int w;
    int h;
};

static void expect(int cond, const char *msg)
{
    report_check(cond, msg);
}

static void feed(int up, int down, int left, int right, int button)
{
    Input in;

    input_tick(up, down, left, right, button, &in);
    arkaled_tick(&in);
}

static void at(int x, int y, int vx, int vy, const char *msg)
{
    int ax;
    int ay;
    int dx;
    int dy;

    arkaled_ball(&ax, &ay, &dx, &dy);
    expect(ax == x && ay == y && dx == vx && dy == vy, msg);
}

static int pump(int moves)
{
    int i;
    int x;
    int y;
    int vx;
    int vy;
    int px;
    int py;
    int seen = 0;

    arkaled_ball(&px, &py, &vx, &vy);
    for (i = 0; i < moves * 40 && seen < moves; i++) {
        feed(0, 0, 0, 0, 0);
        arkaled_ball(&x, &y, &vx, &vy);
        if (x != px || y != py) {
            seen++;
            px = x;
            py = y;
        }
    }
    return seen;
}

static int row_count(int y)
{
    int x;
    int n = 0;

    for (x = 0; x < 16; x++) {
        if (pixel_get((uint8_t)x, (uint8_t)y))
            n++;
    }
    return n;
}

static int pad_x(void)
{
    int x;
    int n = 0;
    int first = -1;

    for (x = 0; x < 16; x++) {
        if (!pixel_get((uint8_t)x, 15))
            continue;
        if (first < 0)
            first = x;
        n++;
    }
    if (n != 4)
        return -1;
    return first;
}

static int ended(void)
{
    return pixel_get(4, 0) && pixel_get(11, 0);
}

static int flat_mouth(void)
{
    return pixel_get(6, 4) && pixel_get(9, 4) && !pixel_get(5, 4) && !pixel_get(6, 5);
}

static int grid_solid(int x, int y, void *ctx)
{
    struct Grid *g = ctx;

    if (x < 0 || y < 0 || x >= g->w || y >= g->h)
        return 1;
    return g->row[y][x] == '#';
}

static int open_solid(int x, int y, void *ctx)
{
    (void)x;
    (void)y;
    (void)ctx;
    return 0;
}

static int checker_solid(int x, int y, void *ctx)
{
    (void)ctx;
    if (x < 0 || y < 0 || x >= 8 || y >= 8)
        return 1;
    return ((x + y) & 1) == 0;
}

static void test_step(void)
{
    static const char *both[] = {".#.", "#..", "..."};
    static const char *diag[] = {"...", "...", "..#"};
    static const char *tunnel[] = {"#.#", "#.#", "#.#"};
    static const char *hole[] = {".....", "##.##", "....."};
    static const char *cave[] = {
        "######",
        "#....#",
        "#.##.#",
        "#....#",
        "######",
    };
    struct Grid g;
    CollideHit hit[2];
    int x;
    int y;
    int dx;
    int dy;
    int n;
    int i;
    int bad;
    int seen_n;
    int seen[32];

    x = 1;
    y = 1;
    dx = 1;
    dy = 0;
    n = collide_step(&x, &y, &dx, &dy, open_solid, NULL, hit, 2);
    expect(n == 0 && x == 2 && y == 1 && dx == 1 && dy == 0, "an open cell takes the step");

    g.row = both;
    g.w = 3;
    g.h = 3;
    x = 1;
    y = 1;
    dx = -1;
    dy = -1;
    n = collide_step(&x, &y, &dx, &dy, grid_solid, &g, hit, 2);
    expect(n == 2 && hit[0].x == 0 && hit[0].y == 1 && hit[1].x == 1 && hit[1].y == 0,
           "two neighbors are both hits");
    expect(x == 2 && y == 2 && dx == 1 && dy == 1, "two neighbors reverse both steps");

    g.row = diag;
    x = 1;
    y = 1;
    dx = 1;
    dy = 1;
    n = collide_step(&x, &y, &dx, &dy, grid_solid, &g, hit, 2);
    expect(n == 1 && hit[0].x == 2 && hit[0].y == 2, "a diagonal cell is one hit");
    expect(x == 0 && y == 0 && dx == -1 && dy == -1, "a diagonal cell reverses both steps");

    g.row = tunnel;
    g.w = 3;
    g.h = 3;
    x = 1;
    y = 1;
    dx = 1;
    dy = -1;
    bad = 0;
    for (i = 0; i < 6; i++) {
        collide_step(&x, &y, &dx, &dy, grid_solid, &g, NULL, 0);
        if (x != 1 || grid_solid(x, y, &g))
            bad = 1;
    }
    expect(!bad && y == 1 && dx == 1 && dy == 1, "a one-cell tunnel keeps the ball off the walls");

    g.row = hole;
    g.w = 5;
    g.h = 3;
    x = 2;
    y = 2;
    dx = 0;
    dy = -1;
    collide_step(&x, &y, &dx, &dy, grid_solid, &g, NULL, 0);
    expect(x == 2 && y == 1 && dx == 0 && dy == -1, "a one-cell hole lets the ball through");
    collide_step(&x, &y, &dx, &dy, grid_solid, &g, NULL, 0);
    expect(x == 2 && y == 0 && dx == 0 && dy == -1, "the ball comes out on the far side of the hole");

    g.row = cave;
    g.w = 6;
    g.h = 5;
    x = 1;
    y = 1;
    dx = 1;
    dy = 1;
    bad = 0;
    seen_n = 0;
    for (i = 0; i < 60; i++) {
        int px = x;
        int py = y;
        int k;
        int ddx;
        int ddy;

        collide_step(&x, &y, &dx, &dy, grid_solid, &g, NULL, 0);
        ddx = x - px;
        ddy = y - py;
        if (ddx < 0)
            ddx = -ddx;
        if (ddy < 0)
            ddy = -ddy;
        if (grid_solid(x, y, &g) || ddx > 1 || ddy > 1)
            bad = 1;
        for (k = 0; k < seen_n; k++) {
            if (seen[k] == y * 6 + x)
                break;
        }
        if (k == seen_n && seen_n < 32)
            seen[seen_n++] = y * 6 + x;
    }
    expect(!bad && seen_n >= 4 && x == 4 && y == 2 && dx == 1 && dy == 1,
           "a closed cave keeps the ball in open cells");

    x = 1;
    y = 0;
    dx = 1;
    dy = 1;
    bad = 0;
    for (i = 0; i < 40; i++) {
        collide_step(&x, &y, &dx, &dy, checker_solid, NULL, NULL, 0);
        if (checker_solid(x, y, NULL))
            bad = 1;
    }
    expect(!bad && x == 1 && y == 0 && dx == 1 && dy == 1,
           "a checkerboard never lets the ball enter a cell");
}

static void test_period(void)
{
    expect(arkaled_period(0) == 7 && arkaled_period(599) == 7,
           "the opening pace waits 7 ticks");
    expect(arkaled_period(600) == 6, "ten seconds trims one tick from the wait");
    expect(arkaled_period(3000) == 2 && arkaled_period(3600) == 1,
           "the wait stops shrinking at one tick");
}

static void test_open(void)
{
    int x;
    int y;
    int vx;
    int vy;

    input_reset();
    arkaled_reset();
    expect(arkaled_count() == 64, "the block starts with 64 bricks");
    expect(row_count(0) == 0 && row_count(1) == 0, "two rows above the block stay empty");
    expect(row_count(2) == 16 && row_count(3) == 16 && row_count(4) == 16 && row_count(5) == 16,
           "rows 2 through 5 start full");
    expect(row_count(6) == 0 && row_count(13) == 0, "the playfield under the block starts empty");
    expect(pad_x() == 6, "the paddle is 4 LEDs on the bottom row");
    expect(row_count(14) == 1, "the ball is the only LED on its row");
    arkaled_ball(&x, &y, &vx, &vy);
    expect(x == 8 && y == 14 && vx == 0 && vy == 0, "the ball waits above the paddle");
    expect(!pixel_get(1, 8), "the score stays off the field");
    for (x = 0; x < 20; x++)
        feed(0, 0, 0, 0, 0);
    at(8, 14, 0, 0, "the ball waits until the paddle moves");
    expect(!ended(), "a waiting ball does not end the round");
    arkaled_set(0, 0, 1);
    arkaled_set(4, 1, 1);
    arkaled_set(2, 6, 1);
    feed(0, 0, 0, 0, 0);
    expect(arkaled_count() == 64 && row_count(0) == 0 && row_count(1) == 0,
           "bricks stay inside the 16 by 4 block");
}

static void serve(unsigned seed, int left)
{
    input_reset();
    arkaled_seed(seed);
    arkaled_reset();
    if (left) {
        feed(0, 0, 1, 0, 0);
        feed(0, 0, 1, 0, 0);
    } else {
        feed(0, 0, 0, 1, 0);
        feed(0, 0, 0, 1, 0);
    }
}

static void test_serve(void)
{
    int i;

    serve(8, 1);
    at(7, 13, -1, -1, "the first paddle step sends the ball up and left");
    expect(pad_x() == 5, "that step moves the paddle left");
    for (i = 0; i < 6; i++)
        feed(0, 0, 0, 0, 0);
    at(7, 13, -1, -1, "the ball waits 7 ticks between steps");
    feed(0, 0, 0, 0, 0);
    at(6, 12, -1, -1, "the next tick steps the ball");

    serve(1, 0);
    at(9, 13, 1, -1, "the other opening sends the ball up and right");
    expect(pad_x() == 7, "that step moves the paddle right");
}

static void test_move(void)
{
    input_reset();
    arkaled_reset();
    feed(0, 0, 0, 1, 0);
    expect(pad_x() == 6, "one tick does not pass debounce");
    feed(0, 0, 0, 1, 0);
    expect(pad_x() == 7, "the paddle steps right");
    feed(0, 0, 0, 1, 0);
    expect(pad_x() == 7, "the paddle waits one tick");
    feed(0, 0, 0, 1, 0);
    expect(pad_x() == 8, "the next tick steps right again");

    input_reset();
    arkaled_reset();
    feed(1, 0, 0, 0, 0);
    feed(1, 0, 0, 0, 0);
    expect(pad_x() == 6, "up and down do not move the paddle");
    at(8, 14, 0, 0, "up and down do not serve");

    input_reset();
    arkaled_reset();
    feed(0, 0, 1, 1, 0);
    feed(0, 0, 1, 1, 0);
    expect(pad_x() == 6 && !ended(), "opposite directions cancel");
    at(8, 14, 0, 0, "a cancelled step does not serve");

    input_reset();
    arkaled_wipe();
    arkaled_load(12, 8, 10, 0, 0);
    feed(0, 0, 0, 1, 0);
    feed(0, 0, 0, 1, 0);
    expect(pad_x() == 12, "the paddle stops on the right edge");
    input_reset();
    arkaled_load(0, 8, 10, 0, 0);
    feed(0, 0, 1, 0, 0);
    feed(0, 0, 1, 0, 0);
    expect(pad_x() == 0, "the paddle stops on the left edge");
}

static void test_button(void)
{
    input_reset();
    arkaled_seed(1);
    arkaled_reset();
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(pad_x() == 6, "the button does not move the paddle");
    at(8, 14, 0, 0, "the button does not serve");
}

static void test_bricks(void)
{
    int x;

    input_reset();
    arkaled_wipe();
    arkaled_set(0, 2, 1);
    arkaled_set(8, 5, 1);
    arkaled_load(6, 8, 6, 0, -1);
    feed(0, 0, 0, 0, 0);
    at(8, 7, 0, 1, "a brick overhead sends the ball back down");
    expect(score_get() == 1 && !arkaled_on(8, 5) && arkaled_on(0, 2) && !ended(),
           "that brick turns off and scores");

    arkaled_wipe();
    arkaled_set(0, 2, 1);
    arkaled_set(4, 4, 1);
    arkaled_set(5, 3, 1);
    arkaled_load(6, 5, 4, -1, -1);
    feed(0, 0, 0, 0, 0);
    at(6, 5, 1, 1, "a corner of two bricks reverses both steps");
    expect(score_get() == 2 && !arkaled_on(4, 4) && !arkaled_on(5, 3) && !ended(),
           "both bricks of that corner turn off");

    arkaled_wipe();
    arkaled_set(0, 2, 1);
    arkaled_set(8, 5, 1);
    arkaled_load(6, 7, 6, 1, -1);
    feed(0, 0, 0, 0, 0);
    at(6, 7, -1, 1, "a lone diagonal brick reverses both steps");
    expect(score_get() == 1 && !arkaled_on(8, 5), "the diagonal brick turns off");

    arkaled_wipe();
    for (x = 0; x < 16; x++)
        arkaled_set(x, 5, 1);
    arkaled_load(6, 8, 6, 1, -1);
    feed(0, 0, 0, 0, 0);
    at(9, 7, 1, 1, "a ceiling keeps the horizontal step");
    expect(score_get() == 1 && !arkaled_on(8, 5) && arkaled_on(9, 5),
           "the brick overhead turns off and the next one stays");

    arkaled_reset();
    arkaled_load(6, 4, 0, 1, -1);
    feed(0, 0, 0, 0, 0);
    at(5, 1, 1, 1, "the top edge sends the ball back down");
    expect(score_get() == 0, "the top edge scores nothing");

    arkaled_load(6, 0, 8, -1, 1);
    feed(0, 0, 0, 0, 0);
    at(1, 9, 1, 1, "the left edge sends the ball back");

    arkaled_load(6, 15, 8, 1, -1);
    feed(0, 0, 0, 0, 0);
    at(14, 7, -1, -1, "the right edge sends the ball back");

    arkaled_wipe();
    for (x = 0; x < 16; x++)
        arkaled_set(x, 2, 1);
    arkaled_set(8, 2, 0);
    arkaled_load(6, 8, 4, 0, -1);
    expect(pump(5) == 5, "the ball reaches the gap above the block");
    at(8, 1, 0, 1, "a hole lets the ball into the two open rows");
    expect(score_get() == 0 && !arkaled_on(8, 2) && arkaled_on(7, 2) && arkaled_on(9, 2),
           "the hole stays open and the bricks beside it stay");

    serve(8, 1);
    expect(pump(8) == 8, "the left opening reaches the block");
    at(1, 7, 1, 1, "the left wall and the block reverse both steps");
    expect(score_get() == 1 && !arkaled_on(0, 5) && arkaled_on(1, 5) && arkaled_on(0, 4),
           "that corner turns off one brick");
    expect(!ended(), "the rally continues");

    serve(1, 0);
    expect(pump(8) == 8, "the right opening reaches the block");
    at(13, 7, -1, 1, "the right side of the block keeps the ball moving down");
    expect(score_get() == 1 && !arkaled_on(14, 5) && arkaled_on(15, 5),
           "the brick under the bounce turns off");
}

static void test_paddle(void)
{
    input_reset();
    arkaled_reset();
    arkaled_load(6, 8, 14, 1, 1);
    feed(0, 0, 0, 0, 0);
    at(9, 13, 1, -1, "the body of the paddle keeps the horizontal step");
    expect(score_get() == 0 && pad_x() == 6 && !ended(), "the paddle scores nothing");

    arkaled_load(6, 9, 14, 1, 1);
    feed(0, 0, 0, 0, 0);
    at(10, 13, 1, -1, "the LED at the end of the paddle keeps the horizontal step");

    arkaled_load(6, 10, 14, -1, 1);
    feed(0, 0, 0, 0, 0);
    at(11, 13, 1, -1, "meeting the end from outside reverses both steps");
    expect(score_get() == 0 && !ended(), "that corner scores nothing");

    arkaled_load(6, 8, 12, 0, -1);
    feed(0, 0, 0, 0, 0);
    at(8, 11, 0, -1, "a ball moving away from the paddle does not bounce");
}

static void test_miss(void)
{
    score_clear_saved();
    input_reset();
    arkaled_wipe();
    arkaled_load(6, 0, 14, 0, 1);
    feed(0, 0, 0, 0, 0);
    expect(ended(), "a ball past the paddle shows the face");
    expect(score_get() == 0, "a miss scores nothing");
    expect(flat_mouth(), "a zero round draws the straight mouth");
    expect(pixel_get(1, 8) && pixel_get(2, 8) && pixel_get(3, 8) && !pixel_get(4, 8),
           "the end picture shows a zero");
    expect(!pixel_get(7, 13), "the ship is not part of this cart");
}

static void test_pop(void)
{
    sound_log_reset();
    input_reset();
    arkaled_wipe();
    arkaled_load(6, 8, 10, 1, 0);
    feed(0, 0, 0, 0, 0);
    expect(sound_log_count() == 0, "an open step stays quiet");

    sound_log_reset();
    arkaled_set(0, 2, 1);
    arkaled_set(8, 5, 1);
    arkaled_load(6, 8, 6, 0, -1);
    feed(0, 0, 0, 0, 0);
    expect(sound_log_count() == 1 && sound_log_at(0) == SOUND_POP,
           "a brick sends the pop");

    sound_log_reset();
    arkaled_wipe();
    arkaled_set(0, 2, 1);
    arkaled_set(4, 4, 1);
    arkaled_set(5, 3, 1);
    arkaled_load(6, 5, 4, -1, -1);
    feed(0, 0, 0, 0, 0);
    expect(sound_log_count() == 1 && sound_log_at(0) == SOUND_POP,
           "two bricks in one step send one pop");

    sound_log_reset();
    arkaled_load(6, 0, 8, -1, 0);
    feed(0, 0, 0, 0, 0);
    expect(sound_log_count() == 1 && sound_log_at(0) == SOUND_POP,
           "the left edge sends the pop");

    sound_log_reset();
    arkaled_load(6, 8, 14, 0, 1);
    feed(0, 0, 0, 0, 0);
    expect(sound_log_count() == 1 && sound_log_at(0) == SOUND_POP && score_get() == 0,
           "the paddle sends the pop");

    sound_log_reset();
    score_clear_saved();
    arkaled_wipe();
    arkaled_load(6, 0, 14, 0, 1);
    feed(0, 0, 0, 0, 0);
    expect(ended() && sound_log_count() == 0, "a miss stays quiet");

    sound_log_reset();
    arkaled_wipe();
    arkaled_set(8, 5, 1);
    arkaled_load(6, 8, 6, 0, -1);
    feed(0, 0, 0, 0, 0);
    expect(ended() && sound_log_count() == 1 && sound_log_at(0) == SOUND_POP,
           "the last brick still sends the pop");
}

static void test_clear(void)
{
    input_reset();
    arkaled_wipe();
    arkaled_set(8, 5, 1);
    arkaled_load(6, 8, 6, 0, -1);
    feed(0, 0, 0, 0, 0);
    expect(ended() && score_get() == 1 && arkaled_count() == 0,
           "the last brick ends the round");
}

static void test_cave(void)
{
    int i;
    int x;
    int y;
    int vx;
    int vy;
    int px;
    int py;
    int moves = 0;
    int clean = 1;

    input_reset();
    arkaled_wipe();
    for (x = 3; x <= 10; x++) {
        arkaled_set(x, 2, 1);
        arkaled_set(x, 5, 1);
    }
    arkaled_set(3, 3, 1);
    arkaled_set(10, 3, 1);
    arkaled_set(3, 4, 1);
    arkaled_set(10, 4, 1);
    arkaled_load(6, 6, 4, 1, -1);
    for (i = 0; i < 400 && !ended(); i++) {
        int ddx;
        int ddy;

        arkaled_ball(&px, &py, &vx, &vy);
        feed(0, 0, 0, 0, 0);
        if (ended())
            break;
        arkaled_ball(&x, &y, &vx, &vy);
        ddx = x - px;
        ddy = y - py;
        if (ddx < 0)
            ddx = -ddx;
        if (ddy < 0)
            ddy = -ddy;
        if (arkaled_on(x, y) || x < 0 || x >= 16 || y < 0 || y >= 15 || ddx > 1 || ddy > 1)
            clean = 0;
        if (x != px || y != py)
            moves++;
    }
    expect(clean && moves >= 8 && ended(), "the ball crosses the cave and then misses");
    expect(score_get() == 7 && arkaled_count() == 13, "the cave walls it touched turn off");
}

int main(void)
{
    report_begin("arkaled");
    test_step();
    test_period();
    test_open();
    test_serve();
    test_move();
    test_button();
    test_bricks();
    test_paddle();
    test_miss();
    test_clear();
    test_pop();
    test_cave();
    return report_end();
}

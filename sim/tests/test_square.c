#include "frame.h"
#include "game.h"
#include "input.h"

#include <stdio.h>

static int failures;

static void expect(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "fail: %s\n", msg);
        failures++;
    }
}

static int find_origin(int *ox, int *oy)
{
    int x;
    int y;
    int n = 0;
    int minx = 16;
    int miny = 16;
    int maxx = -1;
    int maxy = -1;

    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            if (!pixel_get((uint8_t)x, (uint8_t)y))
                continue;
            n++;
            if (x < minx)
                minx = x;
            if (y < miny)
                miny = y;
            if (x > maxx)
                maxx = x;
            if (y > maxy)
                maxy = y;
        }
    }
    *ox = minx;
    *oy = miny;
    return n == 9 && maxx - minx == 2 && maxy - miny == 2;
}

static void feed(int up, int down, int left, int right, int button)
{
    Input in;

    input_tick(up, down, left, right, button, &in);
    game_tick(&in);
}

static void tap(int up, int down, int left, int right, int button)
{
    feed(up, down, left, right, button);
    feed(up, down, left, right, button);
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
}

static void test_frame(void)
{
    clear();
    pixel(0, 0, true);
    expect(pixel_get(0, 0), "bit 0 is the left pixel");
    expect((frame[0] & 1u) != 0, "the top row stores that bit");
    pixel(0, 0, false);
    expect(!pixel_get(0, 0), "a pixel can turn off");
    pixel(16, 0, true);
    expect(frame[0] == 0, "a column past 15 is ignored");
}

static void test_square(void)
{
    int ox = 0;
    int oy = 0;
    int i;

    input_reset();
    game_reset();
    expect(find_origin(&ox, &oy), "the picture is one solid 3x3");
    expect(ox == 6 && oy == 6, "the block starts near the middle");

    feed(0, 0, 0, 1, 0);
    expect(find_origin(&ox, &oy) && ox == 6 && oy == 6, "one tick does not pass debounce");
    feed(0, 0, 0, 1, 0);
    expect(find_origin(&ox, &oy) && ox == 7 && oy == 6, "the stick steps one cell to the right");

    input_reset();
    game_reset();
    tap(1, 0, 0, 1, 0);
    expect(find_origin(&ox, &oy) && ox == 7 && oy == 5, "a diagonal steps both axes");

    input_reset();
    game_reset();
    tap(0, 0, 1, 1, 0);
    expect(find_origin(&ox, &oy) && ox == 6 && oy == 6, "opposite switches cancel that axis");

    input_reset();
    game_reset();
    for (i = 0; i < 8; i++)
        tap(0, 0, 1, 0, 0);
    expect(find_origin(&ox, &oy) && ox == 0 && oy == 6, "the block stops at the left edge");

    input_reset();
    game_reset();
    tap(0, 0, 0, 0, 1);
    expect(find_origin(&ox, &oy), "a jump is still one solid 3x3");
    expect(!(ox == 6 && oy == 6), "the button leaves the starting cell");
}

int main(void)
{
    test_frame();
    test_square();
    if (failures) {
        fprintf(stderr, "%d failure%s\n", failures, failures == 1 ? "" : "s");
        return 1;
    }
    printf("square ok\n");
    return 0;
}

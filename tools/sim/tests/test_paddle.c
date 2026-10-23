#include "frame.h"
#include "input.h"
#include "paddle.h"
#include "score.h"

#include <stdio.h>

static int failures;

static void expect(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "fail: %s\n", msg);
        failures++;
    }
}

static void feed(int up, int down, int left, int right, int button)
{
    Input in;

    input_tick(up, down, left, right, button, &in);
    paddle_tick(&in);
}

static int col_top(int x)
{
    int y;

    for (y = 0; y < 16; y++) {
        if (pixel_get((uint8_t)x, (uint8_t)y))
            return y;
    }
    return -1;
}

static int col_count(int x)
{
    int y;
    int n = 0;

    for (y = 0; y < 16; y++) {
        if (pixel_get((uint8_t)x, (uint8_t)y))
            n++;
    }
    return n;
}

static int find_ball(int *x, int *y)
{
    int ix;
    int iy;
    int n = 0;

    *x = -1;
    *y = -1;
    for (iy = 0; iy < 16; iy++) {
        for (ix = 1; ix <= 14; ix++) {
            if (!pixel_get((uint8_t)ix, (uint8_t)iy))
                continue;
            *x = ix;
            *y = iy;
            n++;
        }
    }
    return n;
}

static int flat_mouth(void)
{
    return pixel_get(6, 4) && pixel_get(9, 4) && !pixel_get(5, 4) && !pixel_get(6, 5);
}

static void test_period(void)
{
    expect(paddle_period(0) == 7 && paddle_period(599) == 7,
           "the ball starts at every 7 ticks");
    expect(paddle_period(600) == 6 && paddle_period(1199) == 6,
           "ten seconds later the ball steps every 6 ticks");
    expect(paddle_period(3600) == 1 && paddle_period(5000) == 1,
           "the ball reaches one step per tick and stays there");
}

static void test_open(void)
{
    int x;
    int y;

    input_reset();
    paddle_reset();
    expect(col_top(0) == 6 && col_count(0) == 4, "the left paddle is 4 LEDs at mid field");
    expect(col_top(15) == 6 && col_count(15) == 4, "the right paddle matches it");
    expect(find_ball(&x, &y) == 1 && x == 8 && y == 8, "the ball starts in the center");
    expect(!pixel_get(1, 8), "the score stays off the field");
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 8 && y == 8, "six ticks leave the ball in place");
    feed(0, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 7 && y == 9, "the seventh tick steps the ball");
}

static void test_left(void)
{
    input_reset();
    paddle_reset();
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 6, "one tick does not pass debounce");
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 5 && col_count(0) == 4, "the left paddle steps up");
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 5, "the paddle waits one tick");
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 4, "the next tick steps up again");
    feed(0, 0, 1, 0, 0);
    feed(0, 0, 1, 0, 0);
    expect(col_top(0) == 4, "left and right do not move the paddle");
}

static void test_right(void)
{
    input_reset();
    paddle_reset();
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    expect(col_top(15) == 6, "the right paddle waits for the button");
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(col_top(15) == 5, "the first press sends it up");
    feed(0, 0, 0, 0, 1);
    expect(col_top(15) == 5, "it waits one tick between steps");
    feed(0, 0, 0, 0, 1);
    expect(col_top(15) == 4, "holding the button keeps it climbing at 30 FPS");
    feed(0, 0, 0, 0, 0);
    expect(col_top(15) == 4, "one tick does not pass the release debounce");
    feed(0, 0, 0, 0, 0);
    expect(col_top(15) == 4, "releasing the button stops it");
    feed(0, 0, 0, 0, 0);
    expect(col_top(15) == 4, "it stays put after the release");
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(col_top(15) == 5, "the next press sends it down");
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    expect(col_top(15) == 5, "releasing again stops it");
}

static void test_return(void)
{
    int i;
    int x;
    int y;

    input_reset();
    paddle_reset();
    for (i = 0; i < 80 && score_get() == 0 && !pixel_get(4, 0); i++)
        feed(0, 1, 0, 0, 0);
    expect(score_get() == 1 && !pixel_get(4, 0), "a paddle sends the ball back and scores");
    expect(find_ball(&x, &y) == 1 && x == 1 && y == 14, "the ball stays on the field");
    expect(!pixel_get(1, 8), "the point stays off the field");
}

static void test_miss(void)
{
    int i;

    score_clear_saved();
    input_reset();
    paddle_reset();
    for (i = 0; i < 200 && !pixel_get(4, 0); i++)
        feed(0, 0, 0, 0, 0);
    expect(pixel_get(4, 0), "a miss shows the face");
    expect(flat_mouth(), "a zero rally draws the straight mouth");
    expect(pixel_get(1, 8) && pixel_get(2, 8) && pixel_get(3, 8) && !pixel_get(4, 8),
           "the end picture shows a zero");
    expect(!pixel_get(7, 13), "the ship is not part of this cart");
}

int main(void)
{
    test_period();
    test_open();
    test_left();
    test_right();
    test_return();
    test_miss();
    if (failures) {
        fprintf(stderr, "%d failure%s\n", failures, failures == 1 ? "" : "s");
        return 1;
    }
    printf("paddle ok\n");
    return 0;
}

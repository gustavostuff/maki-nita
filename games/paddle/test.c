#include "frame.h"
#include "input.h"
#include "paddle.h"
#include "score.h"
#include "tests/report.h"

static void expect(int cond, const char *msg)
{
    report_check(cond, msg);
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
        for (ix = 1; ix <= 15; ix++) {
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
           "the opening pace waits 7 ticks");
    expect(paddle_period(600) == 6, "ten seconds trims one tick from the wait");
    expect(paddle_period(3000) == 2 && paddle_period(3600) == 1,
           "the wait stops shrinking at one tick");
}

static void test_open(void)
{
    int x;
    int y;

    input_reset();
    paddle_reset();
    expect(col_top(0) == 6 && col_count(0) == 4, "the paddle is 4 LEDs at mid field");
    expect(col_count(15) == 0, "the right column starts empty");
    expect(find_ball(&x, &y) == 1 && x == 8 && y == 8, "the ball starts in the center");
    expect(!pixel_get(1, 8), "the score stays off the field");
    for (x = 0; x < 20; x++)
        feed(0, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 8 && y == 8, "the ball waits until a paddle moves");
    expect(!pixel_get(4, 0), "a waiting ball does not end the round");
}

static void test_serve(void)
{
    static const unsigned seeds[] = { 8, 1, 3, 6 };
    static const int wantx[] = { 7, 9, 7, 9 };
    static const int wanty[] = { 7, 7, 9, 9 };
    int i;
    int x;
    int y;

    for (i = 0; i < 4; i++) {
        input_reset();
        paddle_seed(seeds[i]);
        paddle_reset();
        feed(1, 0, 0, 0, 0);
        feed(1, 0, 0, 0, 0);
        expect(find_ball(&x, &y) == 1 && x == wantx[i] && y == wanty[i],
               "the first paddle step sends the ball toward a corner");
    }
    input_reset();
    paddle_seed(8);
    paddle_reset();
    feed(1, 0, 0, 0, 0);
    feed(1, 0, 0, 0, 0);
    for (i = 0; i < 6; i++)
        feed(1, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 7 && y == 7, "the ball waits 7 ticks between steps");
    feed(1, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 6 && y == 6, "the next tick steps the ball");
}

static void test_left(void)
{
    input_reset();
    paddle_reset();
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 6, "one tick does not pass debounce");
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 5 && col_count(0) == 4, "the paddle steps up");
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 5, "the paddle waits one tick");
    feed(1, 0, 0, 0, 0);
    expect(col_top(0) == 4, "the next tick steps up again");
    feed(0, 0, 1, 0, 0);
    feed(0, 0, 1, 0, 0);
    expect(col_top(0) == 4, "left and right do not move the paddle");
}

static void test_button(void)
{
    int x;
    int y;

    input_reset();
    paddle_seed(1);
    paddle_reset();
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(col_top(0) == 6, "the button does not move the paddle");
    expect(find_ball(&x, &y) == 1 && x == 8 && y == 8, "the button does not serve");
}

static void test_wall(void)
{
    int i;
    int x;
    int y;

    input_reset();
    paddle_seed(1);
    paddle_reset();
    for (i = 0; i < 44; i++)
        feed(1, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 15 && y == 1, "the ball reaches the right edge");
    expect(score_get() == 0 && !pixel_get(4, 0), "the wall is not a miss");
    for (i = 0; i < 7; i++)
        feed(1, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 14 && y == 0, "the right edge sends the ball back");
}

static void test_corner(void)
{
    int i;
    int x;
    int y;

    input_reset();
    paddle_seed(8);
    paddle_reset();
    for (i = 0; i < 51; i++)
        feed(1, 0, 0, 0, 0);
    expect(score_get() == 1 && !pixel_get(4, 0), "the end of the paddle still scores");
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 0,
           "a ball already on the paddle keeps its vertical step");
}

static void test_bounce(void)
{
    int x;
    int y;

    input_reset();
    paddle_load(6, 1, 7, -1, 1);
    feed(0, 0, 0, 0, 0);
    expect(score_get() == 1 && !pixel_get(4, 0), "the body of the paddle scores");
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 8,
           "the body sends the ball back and keeps it moving down");

    input_reset();
    paddle_load(6, 1, 8, -1, -1);
    feed(0, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 7,
           "the body sends the ball back and keeps it moving up");

    input_reset();
    paddle_load(6, 1, 8, -1, 1);
    feed(0, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 9,
           "the LED beside an end keeps the vertical step");

    input_reset();
    paddle_load(6, 1, 9, -1, 1);
    feed(0, 0, 0, 0, 0);
    expect(score_get() == 1 && !pixel_get(4, 0), "leaving the bottom end still scores");
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 8,
           "leaving an end reverses both steps");

    input_reset();
    paddle_load(6, 1, 6, -1, -1);
    feed(0, 0, 0, 0, 0);
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 7,
           "leaving the top end reverses both steps");

    input_reset();
    paddle_load(6, 1, 5, -1, 1);
    feed(0, 0, 0, 0, 0);
    expect(score_get() == 1 && !pixel_get(4, 0), "meeting the top end from outside scores");
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 4,
           "meeting an end from outside reverses both steps");

    input_reset();
    paddle_load(6, 1, 1, 1, 1);
    feed(0, 0, 0, 0, 0);
    expect(score_get() == 0 && find_ball(&x, &y) == 1 && x == 2 && y == 2,
           "a ball moving away from the paddle does not bounce");

    score_clear_saved();
    input_reset();
    paddle_load(6, 1, 3, -1, -1);
    feed(0, 0, 0, 0, 0);
    expect(pixel_get(4, 0) && score_get() == 0, "a ball past the paddle is a miss");
}

static void test_return(void)
{
    int i;
    int x;
    int y;

    input_reset();
    paddle_seed(3);
    paddle_reset();
    for (i = 0; i < 300 && score_get() == 0 && !pixel_get(4, 0); i++)
        feed(0, 1, 0, 0, 0);
    expect(score_get() == 1 && !pixel_get(4, 0), "a paddle sends the ball back and scores");
    expect(find_ball(&x, &y) == 1 && x == 2 && y == 14, "the ball leaves the paddle");
    expect(!pixel_get(1, 8), "the point stays off the field");
}

static void test_miss(void)
{
    int i;

    score_clear_saved();
    input_reset();
    paddle_seed(3);
    paddle_reset();
    for (i = 0; i < 300 && !pixel_get(4, 0); i++)
        feed(1, 0, 0, 0, 0);
    expect(pixel_get(4, 0), "a miss shows the face");
    expect(flat_mouth(), "a zero rally draws the straight mouth");
    expect(pixel_get(1, 8) && pixel_get(2, 8) && pixel_get(3, 8) && !pixel_get(4, 8),
           "the end picture shows a zero");
    expect(!pixel_get(7, 13), "the ship is not part of this cart");
}

int main(void)
{
    report_begin("paddle");
    test_period();
    test_open();
    test_serve();
    test_left();
    test_button();
    test_wall();
    test_corner();
    test_bounce();
    test_return();
    test_miss();
    return report_end();
}

#include "entity/rock.h"
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

static void feed(int up, int down, int left, int right, int button)
{
    Input in;

    input_tick(up, down, left, right, button, &in);
    game_tick(&in);
}

static int ship_x(void)
{
    int x;

    for (x = 0; x < 32; x++) {
        if (pixel_get((uint8_t)x, 30))
            return x;
    }
    return -1;
}

static void test_frame(void)
{
    clear();
    pixel(0, 0, true);
    expect(pixel_get(0, 0), "bit 0 is the left pixel");
    expect((frame[0] & 1u) != 0, "the top row stores that bit");
    pixel(0, 0, false);
    expect(!pixel_get(0, 0), "a pixel can turn off");
    pixel(32, 0, true);
    expect(frame[0] == 0, "a column past 31 is ignored");
}

static void test_ship(void)
{
    int i;

    input_reset();
    game_reset();
    expect(pixel_get(15, 29) && !pixel_get(14, 29) && !pixel_get(16, 29),
           "the nose is the center LED of the top row");
    expect(pixel_get(14, 30) && pixel_get(15, 30) && pixel_get(16, 30),
           "the middle row is full");
    expect(pixel_get(14, 31) && pixel_get(15, 31) && pixel_get(16, 31),
           "the bottom row is full");
    expect(ship_x() == 14, "the ship starts near the middle of the bottom");

    feed(0, 0, 0, 1, 0);
    expect(ship_x() == 14, "one tick does not pass debounce");
    feed(0, 0, 0, 1, 0);
    expect(ship_x() == 15, "the next tick steps one LED");
    feed(0, 0, 0, 1, 0);
    expect(ship_x() == 16, "the ship steps again on the following tick");

    input_reset();
    game_reset();
    feed(1, 0, 0, 1, 0);
    feed(1, 0, 0, 1, 0);
    expect(ship_x() == 15 && pixel_get(16, 28) && !pixel_get(16, 31),
           "a diagonal steps both axes");

    input_reset();
    game_reset();
    feed(0, 0, 1, 1, 0);
    feed(0, 0, 1, 1, 0);
    expect(ship_x() == 14, "opposite switches cancel that axis");

    input_reset();
    game_reset();
    for (i = 0; i < 15; i++)
        feed(0, 0, 1, 0, 0);
    expect(ship_x() == 0, "the ship stops at the left edge");
}

static void test_shot(void)
{
    int i;

    input_reset();
    game_reset();
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(pixel_get(15, 28), "the bullet leaves the nose");
    expect(!pixel_get(15, 27), "the bullet is one LED on the fire tick");
    feed(0, 0, 0, 0, 0);
    expect(pixel_get(15, 28) && !pixel_get(15, 27), "the bullet waits one tick");
    feed(0, 0, 0, 0, 0);
    expect(pixel_get(15, 27) && !pixel_get(15, 28), "the second tick climbs one LED");

    input_reset();
    game_reset();
    rock_add(15, 24, 2, 16, 0, 30);
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(pixel_get(15, 28) && pixel_get(15, 24), "the shot and the rock are both on screen");
    for (i = 0; i < 6; i++)
        feed(0, 0, 0, 0, 0);
    expect(!pixel_get(15, 24) && !pixel_get(16, 25) && !pixel_get(15, 25),
           "the rock is gone after the hit");
    expect(pixel_get(15, 29), "the ship is still there after a shot");
}

static void advance(int n)
{
    int i;

    for (i = 0; i < n; i++)
        feed(0, 0, 0, 0, 0);
}

static void test_speeds(void)
{
    input_reset();
    game_reset();
    rock_add(0, 0, 2, 4, 0, 30);
    rock_add(3, 0, 2, 8, 0, 30);
    rock_add(6, 0, 2, 12, 0, 30);
    rock_add(9, 0, 2, 16, 0, 30);
    advance(4);
    expect(!pixel_get(0, 0) && pixel_get(0, 1), "one rock steps every 4 ticks");
    expect(pixel_get(3, 0) && pixel_get(6, 0) && pixel_get(9, 0),
           "the slower rocks stay put");
    advance(4);
    expect(!pixel_get(3, 0) && pixel_get(3, 1), "one rock steps every 8 ticks");
    expect(pixel_get(6, 0) && pixel_get(9, 0), "the 12 and 16 rocks stay put");
    advance(4);
    expect(!pixel_get(6, 0) && pixel_get(6, 1), "one rock steps every 12 ticks");
    expect(pixel_get(9, 0), "the 16 rock stays put");
    advance(4);
    expect(!pixel_get(9, 0) && pixel_get(9, 1), "one rock steps every 16 ticks");

    input_reset();
    game_reset();
    rock_add(10, 4, 2, 16, 1, 15);
    advance(14);
    expect(pixel_get(10, 4) && pixel_get(11, 4) && !pixel_get(12, 4),
           "sideways drift waits out its period");
    advance(1);
    expect(pixel_get(11, 4) && pixel_get(12, 4) && !pixel_get(10, 4),
           "a rock steps one column every 15 ticks");
}

static void test_both_sizes(void)
{
    input_reset();
    game_reset();
    advance(119);
    expect(pixel_get(11, 2) && pixel_get(12, 2) && pixel_get(11, 3) && pixel_get(12, 3),
           "a spawned 2x2 rock is on screen");
    expect(!pixel_get(10, 2) && !pixel_get(13, 2), "that rock is two columns wide");
    expect(pixel_get(0, 0) && pixel_get(2, 0) && pixel_get(0, 2),
           "a spawned 3x3 rock is on screen");
    expect(!pixel_get(3, 0), "that rock is three columns wide");
}

static void test_crash(void)
{
    int i;

    input_reset();
    game_reset();
    rock_add(14, 29, 2, 16, 0, 30);
    feed(0, 0, 0, 0, 0);
    expect(pixel_get(12, 12) && pixel_get(13, 12) && !pixel_get(14, 12),
           "a crash draws the burst");
    expect(pixel_get(19, 12) && pixel_get(13, 14) && !pixel_get(12, 14),
           "the burst matches the full mark");
    expect(!pixel_get(15, 29) && !pixel_get(0, 0), "the burst replaces the field");

    for (i = 0; i < 119; i++)
        feed(0, 0, 0, 0, 0);
    expect(pixel_get(12, 12) && !pixel_get(15, 29), "the burst holds for two seconds");
    feed(0, 0, 0, 0, 0);
    expect(pixel_get(15, 29) && !pixel_get(14, 29) && !pixel_get(12, 12),
           "the round starts over on the ship");
}

int main(void)
{
    test_frame();
    test_ship();
    test_shot();
    test_speeds();
    test_both_sizes();
    test_crash();
    if (failures) {
        fprintf(stderr, "%d failure%s\n", failures, failures == 1 ? "" : "s");
        return 1;
    }
    printf("game ok\n");
    return 0;
}

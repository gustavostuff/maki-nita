#include "text_test.h"

#include "input.h"
#include "tests/report.h"
#include "text.h"

static void expect(int cond, const char *msg)
{
    report_check(cond, msg);
}

static void feed(int up, int down, int left, int right, int button)
{
    Input in;

    input_tick(up, down, left, right, button, &in);
    text_test_tick(&in);
}

static void test_initial_state(void)
{
    input_reset();
    text_test_reset();

    expect(text_test_scroll_x() == 0, "starts at column 0");
    expect(text_test_scroll_y() == 6, "starts centered vertically at row 6");
    expect(!text_test_is_paused(), "starts unpaused");
}

static void test_scroll_30fps(void)
{
    int i;

    input_reset();
    text_test_reset();

    /* Tick 1: should not advance yet (period is 2 ticks = 30 FPS on 60Hz) */
    feed(0, 0, 0, 0, 0);
    expect(text_test_scroll_x() == 0, "first tick does not step scroll");

    /* Tick 2: steps 1 pixel */
    feed(0, 0, 0, 0, 0);
    expect(text_test_scroll_x() == -1, "second tick steps one pixel left");

    /* Tick 3: does not step */
    feed(0, 0, 0, 0, 0);
    expect(text_test_scroll_x() == -1, "third tick holds position");

    /* Tick 4: steps again */
    feed(0, 0, 0, 0, 0);
    expect(text_test_scroll_x() == -2, "fourth tick steps one pixel left");

    /* Over 60 ticks (1 full second), steps exactly 30 pixels */
    text_test_reset();
    for (i = 0; i < 60; i++)
        feed(0, 0, 0, 0, 0);
    expect(text_test_scroll_x() == -30, "60 ticks at 30 FPS scrolls exactly 30 pixels");
}

static void test_controls(void)
{
    input_reset();
    text_test_reset();

    /* Press button for 2 ticks to pass debounce */
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(text_test_is_paused(), "button press pauses auto-scroll");

    /* Release button and run 2 ticks: scroll should not move while paused */
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0);
    expect(text_test_scroll_x() == 0, "scroll holds position while paused");

    /* Press button again for 2 ticks to resume */
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(!text_test_is_paused(), "second button press resumes scroll");

    /* Test Stick Up / Down */
    int initial_y = text_test_scroll_y();
    feed(1, 0, 0, 0, 0);
    feed(1, 0, 0, 0, 0);
    expect(text_test_scroll_y() == initial_y - 1, "stick up moves row up");
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0); /* release */
    feed(0, 1, 0, 0, 0);
    feed(0, 1, 0, 0, 0); /* press down */
    expect(text_test_scroll_y() == initial_y, "stick down moves row down");

    /* Test Stick Right (reverses scroll direction) */
    text_test_reset();
    feed(0, 0, 0, 1, 0);
    feed(0, 0, 0, 1, 0); /* pass debounce */
    feed(0, 0, 0, 0, 0);
    feed(0, 0, 0, 0, 0); /* step */
    expect(text_test_scroll_x() < 0, "scrolling right wraps smoothly into positive direction");
}

static void test_font_coverage(void)
{
    const char *msg = text_test_message();
    int has_letter[26] = {0};
    int has_digit[10] = {0};
    int has_dot = 0, has_comma = 0, has_colon = 0, has_semi = 0;
    int has_dash = 0, has_plus = 0, has_equals = 0, has_slash = 0;
    int i;

    for (i = 0; msg[i] != '\0'; i++) {
        char c = msg[i];
        if (c >= 'A' && c <= 'Z')
            has_letter[c - 'A'] = 1;
        if (c >= '0' && c <= '9')
            has_digit[c - '0'] = 1;
        if (c == '.') has_dot = 1;
        if (c == ',') has_comma = 1;
        if (c == ':') has_colon = 1;
        if (c == ';') has_semi = 1;
        if (c == '-') has_dash = 1;
        if (c == '+') has_plus = 1;
        if (c == '=') has_equals = 1;
        if (c == '/') has_slash = 1;

        /* Verify every character in the message has a non-NULL glyph */
        expect(text_glyph(c) != 0, "glyph exists for message character");
    }

    for (i = 0; i < 26; i++)
        expect(has_letter[i], "pangram covers all 26 letters A-Z");

    for (i = 0; i < 10; i++)
        expect(has_digit[i], "message covers all 10 digits 0-9");

    expect(has_dot, "message includes dot");
    expect(has_comma, "message includes comma");
    expect(has_colon, "message includes colon");
    expect(has_semi, "message includes semicolon");
    expect(has_dash, "message includes dash");
    expect(has_plus, "message includes plus");
    expect(has_equals, "message includes equals");
    expect(has_slash, "message includes slash");
}

int main(void)
{
    report_begin("text-test");
    test_initial_state();
    test_scroll_30fps();
    test_controls();
    test_font_coverage();
    return report_end();
}

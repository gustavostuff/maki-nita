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
}

static void test_idle_no_scroll(void)
{
    int i;

    input_reset();
    text_test_reset();

    /* 60 ticks without input should not scroll at all */
    for (i = 0; i < 60; i++)
        feed(0, 0, 0, 0, 0);
    expect(text_test_scroll_x() == 0, "text stays stationary when stick is centered");
}

static void test_stick_left_scroll(void)
{
    int i;

    input_reset();
    text_test_reset();

    /* Hold stick left for 60 ticks: steps exactly 15 pixels at 15 FPS (accounting for 2-tick debounce) */
    /* 2 ticks to debounce, then 58 ticks with period 4 -> 14 or 15 steps */
    for (i = 0; i < 60; i++)
        feed(0, 0, 1, 0, 0);

    expect(text_test_scroll_x() <= -14 && text_test_scroll_x() >= -15,
           "holding stick left scrolls left at 15 FPS");
}

static void test_controls(void)
{
    input_reset();
    text_test_reset();

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

    /* Scroll some pixels with stick left, then press button to reset */
    int i;
    for (i = 0; i < 20; i++)
        feed(0, 0, 1, 0, 0);
    expect(text_test_scroll_x() < 0, "scrolled left with stick");

    /* Button press resets to start */
    feed(0, 0, 0, 0, 1);
    feed(0, 0, 0, 0, 1);
    expect(text_test_scroll_x() == 0, "button resets scroll to start");
    expect(text_test_scroll_y() == 6, "button resets row to center");
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
    test_idle_no_scroll();
    test_stick_left_scroll();
    test_controls();
    test_font_coverage();
    return report_end();
}

#include "text_test.h"

#include "frame.h"
#include "text.h"

#include <string.h>

#define SCROLL_PERIOD 2 /* 60Hz / 2 = 30 FPS */

static const char message[] =
    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG - 0123456789 - . , : ; - + = / ";

static int scroll_x;
static int scroll_y;
static int scroll_dir;
static int paused;
static int tick_count;
static int msg_len;
static int msg_width;

void text_test_reset(void)
{
    scroll_x = 0;
    scroll_y = 6; /* Centered vertically on 16-row matrix: rows 6..9 */
    scroll_dir = -1; /* Scroll leftwards (standard reading ticker) */
    paused = 0;
    tick_count = 0;
    msg_len = (int)strlen(message);
    msg_width = msg_len * TEXT_W;
    clear();
    text_draw(scroll_x, scroll_y, message);
}

int text_test_scroll_x(void)
{
    return scroll_x;
}

int text_test_scroll_y(void)
{
    return scroll_y;
}

int text_test_is_paused(void)
{
    return paused;
}

const char *text_test_message(void)
{
    return message;
}

void text_test_tick(const Input *in)
{
    if (msg_width == 0) {
        msg_len = (int)strlen(message);
        msg_width = msg_len * TEXT_W;
    }

    if (in != 0) {
        if (in->button_pressed)
            paused = !paused;

        if (in->up_pressed) {
            if (scroll_y > 0)
                scroll_y--;
        }
        if (in->down_pressed) {
            if (scroll_y < 16 - TEXT_H)
                scroll_y++;
        }
        if (in->left)
            scroll_dir = -1;
        if (in->right)
            scroll_dir = 1;
    }

    if (!paused) {
        tick_count++;
        if ((tick_count % SCROLL_PERIOD) == 0) {
            scroll_x += scroll_dir;

            if (scroll_x <= -msg_width)
                scroll_x += msg_width;
            else if (scroll_x > 0)
                scroll_x -= msg_width;
        }
    }

    clear();
    text_draw(scroll_x, scroll_y, message);
    if (scroll_x + msg_width < 16)
        text_draw(scroll_x + msg_width, scroll_y, message);
    if (scroll_x > 0)
        text_draw(scroll_x - msg_width, scroll_y, message);
}

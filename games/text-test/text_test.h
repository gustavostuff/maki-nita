#ifndef TEXT_TEST_H
#define TEXT_TEST_H

#include "input.h"

/*
 * Text test cart: displays the quick brown fox / lazy dog pangram,
 * all digits, and all punctuation from the 4x4 font set in a 15 FPS
 * horizontal scroll.
 *
 * Controls:
 * - Stick Left: scroll text left at 15 FPS while held
 * - Stick Right: scroll text right at 15 FPS while held
 * - Stick Up: move text row up
 * - Stick Down: move text row down
 * - Button: reset position to start
 */

void text_test_reset(void);
void text_test_tick(const Input *in);
int text_test_scroll_x(void);
int text_test_scroll_y(void);
int text_test_is_paused(void);
const char *text_test_message(void);

#endif

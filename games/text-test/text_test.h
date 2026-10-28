#ifndef TEXT_TEST_H
#define TEXT_TEST_H

#include "input.h"

/*
 * Text test cart: displays the quick brown fox / lazy dog pangram,
 * all digits, and all punctuation from the 4x4 font set in a 30 FPS
 * horizontal scroll.
 *
 * Controls:
 * - Stick Left: scroll left (default reading ticker)
 * - Stick Right: scroll right
 * - Stick Up: move text row up
 * - Stick Down: move text row down
 * - Button: toggle pause / resume
 */

void text_test_reset(void);
void text_test_tick(const Input *in);
int text_test_scroll_x(void);
int text_test_scroll_y(void);
int text_test_is_paused(void);
const char *text_test_message(void);

#endif

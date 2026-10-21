#ifndef FW_INPUT_H
#define FW_INPUT_H

#include <stdbool.h>

/*
 * Five switches, reported independently.
 * A change sticks after the new raw level has lasted two ticks.
 * pressed and released are true for that tick only.
 */

typedef struct Input {
    bool up, down, left, right, button;
    bool up_pressed, down_pressed, left_pressed, right_pressed, button_pressed;
    bool up_released, down_released, left_released, right_released, button_released;
} Input;

void input_reset(void);

/* Raw arguments are 1 when the switch is closed. */
void input_tick(bool up, bool down, bool left, bool right, bool button, Input *out);

#endif

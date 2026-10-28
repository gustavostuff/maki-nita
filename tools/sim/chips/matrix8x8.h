#ifndef RD_MATRIX8X8_H
#define RD_MATRIX8X8_H

/*
 * 1088AS 8x8 LED Matrix module model.
 * 16-pin package (two 1x08 socket strips, 25.4 mm row spacing).
 *
 * Left strip (top to bottom): pads 1 to 8.
 * Right strip (top to bottom): pads 16 to 9.
 *
 * Polarity: Row Cathode / Column Anode (1088AS).
 * An LED at (col, row) illuminates if and only if its column anode pad is driven
 * HIGH (SIG_H) and its row cathode pad is pulled LOW (SIG_L).
 *
 * Physical 1088AS pad assignments:
 *   Left strip:
 *     Pad 1: Row 0 (Cathode)
 *     Pad 2: Col 3 (Anode)
 *     Pad 3: Col 5 (Anode)
 *     Pad 4: Row 3 (Cathode)
 *     Pad 5: Col 0 (Anode)
 *     Pad 6: Row 1 (Cathode)
 *     Pad 7: Col 6 (Anode)
 *     Pad 8: Col 7 (Anode)
 *
 *   Right strip:
 *     Pad 16: Row 2 (Cathode)
 *     Pad 15: Row 5 (Cathode)
 *     Pad 14: Col 4 (Anode)
 *     Pad 13: Row 7 (Cathode)
 *     Pad 12: Col 2 (Anode)
 *     Pad 11: Col 1 (Anode)
 *     Pad 10: Row 6 (Cathode)
 *     Pad  9: Row 4 (Cathode)
 */

#include "engine/signal.h"

#include <stdint.h>

#define MATRIX8X8_PINS 16

/* Physical socket pad numbers for 0-indexed column C0..C7 and row R0..R7 */
extern const int matrix8x8_col_pads[8];
extern const int matrix8x8_row_pads[8];

typedef struct Matrix8x8 {
    const char *refdes;
    Pin pin[MATRIX8X8_PINS];
    /* Accumulated persistence frame over a scan cycle:
     * bit c of frame[r] is 1 if LED (c, r) was forward-biased. */
    uint8_t frame[8];
} Matrix8x8;

void matrix8x8_init(Matrix8x8 *mat, const char *refdes);
Pin *matrix8x8_pin(Matrix8x8 *mat, int pad_number);

/* Returns 1 if LED at (col, row) is currently forward-biased (anode H, cathode L).
 * Returns 0 if reverse-biased, unpowered, floating, or out of range. */
int matrix8x8_led_active(const Matrix8x8 *mat, int col, int row);

/* Updates frame persistence by latching currently active LEDs */
void matrix8x8_sample(Matrix8x8 *mat);

/* Clears frame persistence buffer */
void matrix8x8_clear_frame(Matrix8x8 *mat);

/* Returns 1 if LED at (col, row) is lit in the persistent frame buffer */
int matrix8x8_led_frame(const Matrix8x8 *mat, int col, int row);

/* Returns 1 if applying HIGH to anode_pad and LOW to cathode_pad lights an LED.
 * Sets *out_col and *out_row to the 0-based column and row coordinates.
 * Returns 0 if reverse-biased, invalid pads, or no LED lights. */
int matrix8x8_test_pair(int anode_pad, int cathode_pad, int *out_col, int *out_row);

#endif

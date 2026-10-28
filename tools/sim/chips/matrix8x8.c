#include "chips/matrix8x8.h"

#include <string.h>

/*
 * Column anodes C0..C7 mapping to 1088AS socket pads.
 * C0 is leftmost pixel (bit 7, DP), C7 is rightmost pixel (bit 0, G).
 */
const int matrix8x8_col_pads[8] = {5, 11, 12, 2, 14, 3, 7, 8};

/*
 * Row cathodes R0..R7 mapping to 1088AS socket pads.
 * R0 is top row (DIG0), R7 is bottom row (DIG7).
 */
const int matrix8x8_row_pads[8] = {1, 6, 16, 4, 9, 15, 10, 13};

static const char *pad_names[MATRIX8X8_PINS] = {
    "R0", "C3", "C5", "R3", "C0", "R1", "C6", "C7",
    "R4", "R6", "C1", "C2", "R7", "C4", "R5", "R2",
};

void matrix8x8_init(Matrix8x8 *mat, const char *refdes)
{
    int i;

    mat->refdes = refdes;
    for (i = 0; i < MATRIX8X8_PINS; i++) {
        Pin *p = &mat->pin[i];

        p->name = pad_names[i];
        p->number = i + 1;
        p->dir = PIN_IN;
        p->drive = SIG_Z;
        p->level = SIG_Z;
        p->net = 0;
    }
    matrix8x8_clear_frame(mat);
}

Pin *matrix8x8_pin(Matrix8x8 *mat, int pad_number)
{
    if (pad_number < 1 || pad_number > MATRIX8X8_PINS)
        return 0;
    return &mat->pin[pad_number - 1];
}

int matrix8x8_led_active(const Matrix8x8 *mat, int col, int row)
{
    int anode_pad;
    int cathode_pad;

    if (col < 0 || col > 7 || row < 0 || row > 7)
        return 0;
    anode_pad = matrix8x8_col_pads[col];
    cathode_pad = matrix8x8_row_pads[row];
    return (mat->pin[anode_pad - 1].level == SIG_H &&
            mat->pin[cathode_pad - 1].level == SIG_L);
}

void matrix8x8_sample(Matrix8x8 *mat)
{
    int col;
    int row;

    for (row = 0; row < 8; row++) {
        for (col = 0; col < 8; col++) {
            if (matrix8x8_led_active(mat, col, row))
                mat->frame[row] |= (uint8_t)(1u << col);
        }
    }
}

void matrix8x8_clear_frame(Matrix8x8 *mat)
{
    memset(mat->frame, 0, sizeof mat->frame);
}

int matrix8x8_led_frame(const Matrix8x8 *mat, int col, int row)
{
    if (col < 0 || col > 7 || row < 0 || row > 7)
        return 0;
    return (mat->frame[row] >> col) & 1u;
}

int matrix8x8_test_pair(int anode_pad, int cathode_pad, int *out_col, int *out_row)
{
    int c;
    int r;
    int found_col = -1;
    int found_row = -1;

    if (anode_pad < 1 || anode_pad > MATRIX8X8_PINS ||
        cathode_pad < 1 || cathode_pad > MATRIX8X8_PINS ||
        anode_pad == cathode_pad)
        return 0;

    for (c = 0; c < 8; c++) {
        if (matrix8x8_col_pads[c] == anode_pad) {
            found_col = c;
            break;
        }
    }
    for (r = 0; r < 8; r++) {
        if (matrix8x8_row_pads[r] == cathode_pad) {
            found_row = r;
            break;
        }
    }

    if (found_col >= 0 && found_row >= 0) {
        if (out_col)
            *out_col = found_col;
        if (out_row)
            *out_row = found_row;
        return 1;
    }
    return 0;
}

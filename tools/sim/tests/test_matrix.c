#include "chips/matrix8x8.h"
#include "chips/max7219.h"
#include "engine/signal.h"
#include "tests/report.h"

#include <stdio.h>
#include <string.h>

static void expect(int cond, const char *msg)
{
    report_check(cond, msg);
}

/*
 * Replicates the bench tests performed on actual 1088AS hardware
 * using a +5V source and 220 ohm resistor to ground.
 */
static void test_bench_pairs(void)
{
    int col = -1;
    int row = -1;

    /* Pad 5 is Column 0 (Anode). Testing with all 8 row cathodes: */
    expect(matrix8x8_test_pair(5, 1, &col, &row) && col == 0 && row == 0,
           "Pad 5 (+5V, C0) and Pad 1 (GND, R0) lights LED (0, 0)");
    expect(matrix8x8_test_pair(5, 6, &col, &row) && col == 0 && row == 1,
           "Pad 5 (+5V, C0) and Pad 6 (GND, R1) lights LED (0, 1)");
    expect(matrix8x8_test_pair(5, 16, &col, &row) && col == 0 && row == 2,
           "Pad 5 (+5V, C0) and Pad 16 (GND, R2) lights LED (0, 2)");
    expect(matrix8x8_test_pair(5, 4, &col, &row) && col == 0 && row == 3,
           "Pad 5 (+5V, C0) and Pad 4 (GND, R3) lights LED (0, 3)");
    expect(matrix8x8_test_pair(5, 9, &col, &row) && col == 0 && row == 4,
           "Pad 5 (+5V, C0) and Pad 9 (GND, R4) lights LED (0, 4)");
    expect(matrix8x8_test_pair(5, 15, &col, &row) && col == 0 && row == 5,
           "Pad 5 (+5V, C0) and Pad 15 (GND, R5) lights LED (0, 5)");
    expect(matrix8x8_test_pair(5, 10, &col, &row) && col == 0 && row == 6,
           "Pad 5 (+5V, C0) and Pad 10 (GND, R6) lights LED (0, 6)");
    expect(matrix8x8_test_pair(5, 13, &col, &row) && col == 0 && row == 7,
           "Pad 5 (+5V, C0) and Pad 13 (GND, R7) lights LED (0, 7)");

    /* Diagonal test across all 8 columns and rows: */
    expect(matrix8x8_test_pair(11, 6, &col, &row) && col == 1 && row == 1,
           "Pad 11 (+5V, C1) and Pad 6 (GND, R1) lights LED (1, 1)");
    expect(matrix8x8_test_pair(12, 16, &col, &row) && col == 2 && row == 2,
           "Pad 12 (+5V, C2) and Pad 16 (GND, R2) lights LED (2, 2)");
    expect(matrix8x8_test_pair(2, 4, &col, &row) && col == 3 && row == 3,
           "Pad 2 (+5V, C3) and Pad 4 (GND, R3) lights LED (3, 3)");
    expect(matrix8x8_test_pair(14, 9, &col, &row) && col == 4 && row == 4,
           "Pad 14 (+5V, C4) and Pad 9 (GND, R4) lights LED (4, 4)");
    expect(matrix8x8_test_pair(3, 15, &col, &row) && col == 5 && row == 5,
           "Pad 3 (+5V, C5) and Pad 15 (GND, R5) lights LED (5, 5)");
    expect(matrix8x8_test_pair(7, 10, &col, &row) && col == 6 && row == 6,
           "Pad 7 (+5V, C6) and Pad 10 (GND, R6) lights LED (6, 6)");
    expect(matrix8x8_test_pair(8, 13, &col, &row) && col == 7 && row == 7,
           "Pad 8 (+5V, C7) and Pad 13 (GND, R7) lights LED (7, 7)");
}

static void test_reverse_bias_and_invalid(void)
{
    int col = -1;
    int row = -1;

    /* Inverted polarity (+5V on cathode, GND on anode): */
    expect(!matrix8x8_test_pair(1, 5, &col, &row),
           "Pad 1 (+5V, R0) and Pad 5 (GND, C0) is reverse-biased and dark");
    expect(!matrix8x8_test_pair(4, 2, &col, &row),
           "Pad 4 (+5V, R3) and Pad 2 (GND, C3) is reverse-biased and dark");

    /* Two anodes (+5V and GND on columns): */
    expect(!matrix8x8_test_pair(5, 11, &col, &row),
           "two column anode pads (5 and 11) produce no light");

    /* Two cathodes (+5V and GND on rows): */
    expect(!matrix8x8_test_pair(1, 6, &col, &row),
           "two row cathode pads (1 and 6) produce no light");

    /* Same pad or out of range: */
    expect(!matrix8x8_test_pair(5, 5, &col, &row),
           "same pad for +5V and GND produces no light");
    expect(!matrix8x8_test_pair(0, 5, &col, &row),
           "pad 0 is invalid");
    expect(!matrix8x8_test_pair(5, 17, &col, &row),
           "pad 17 is invalid");
}

static void test_exhaustive_64_leds(void)
{
    Matrix8x8 mat;
    int target_c;
    int target_r;
    int all_ok = 1;

    matrix8x8_init(&mat, "DS1");

    for (target_r = 0; target_r < 8; target_r++) {
        for (target_c = 0; target_c < 8; target_c++) {
            int c;
            int r;
            int anode_pad = matrix8x8_col_pads[target_c];
            int cathode_pad = matrix8x8_row_pads[target_r];

            /* Drive target anode HIGH and target cathode LOW */
            for (c = 0; c < 16; c++)
                mat.pin[c].level = SIG_Z;
            mat.pin[anode_pad - 1].level = SIG_H;
            mat.pin[cathode_pad - 1].level = SIG_L;

            if (!matrix8x8_led_active(&mat, target_c, target_r))
                all_ok = 0;

            /* Check that all other 63 LEDs remain dark */
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    if (c == target_c && r == target_r)
                        continue;
                    if (matrix8x8_led_active(&mat, c, r))
                        all_ok = 0;
                }
            }
        }
    }
    expect(all_ok, "all 64 LEDs light exclusively when their CX and RX are active");
}

static void test_netlist_with_max7219(void)
{
    Max7219 drv;
    Matrix8x8 mat;
    Net col_net[8];
    Net row_net[8];
    static const int seg_pins[8] = {22, 14, 16, 20, 23, 21, 15, 17};
    static const int dig_pins[8] = {2, 11, 6, 7, 3, 10, 5, 8};
    int i;
    int r;
    uint64_t now_ns = 0;

    max7219_init(&drv, "U1");
    matrix8x8_init(&mat, "DS1");

    /* Wire according to the updated netlist: */
    for (i = 0; i < 8; i++) {
        net_init(&col_net[i], "COL", SIG_L);
        net_tie(&col_net[i], max7219_pin(&drv, seg_pins[i]));
        net_tie(&col_net[i], matrix8x8_pin(&mat, matrix8x8_col_pads[i]));

        net_init(&row_net[i], "ROW", SIG_H);
        net_tie(&row_net[i], max7219_pin(&drv, dig_pins[i]));
        net_tie(&row_net[i], matrix8x8_pin(&mat, matrix8x8_row_pads[i]));
    }

    /* Configure MAX7219: wake from shutdown, full scan limit */
    drv.shutdown = 0;
    drv.scan_limit = 7;
    drv.intensity = 15;
    drv.wake_ns = 0;
    drv.scan_origin_ns = 0;

    /*
     * Load an 'X' pattern into the driver registers:
     * Row 0: 0x81 (pixels 0, 7)
     * Row 1: 0x42 (pixels 1, 6)
     * Row 2: 0x24 (pixels 2, 5)
     * Row 3: 0x18 (pixels 3, 4)
     * Row 4: 0x18 (pixels 3, 4)
     * Row 5: 0x24 (pixels 2, 5)
     * Row 6: 0x42 (pixels 1, 6)
     * Row 7: 0x81 (pixels 0, 7)
     */
    drv.digit[0] = 0x81;
    drv.digit[1] = 0x42;
    drv.digit[2] = 0x24;
    drv.digit[3] = 0x18;
    drv.digit[4] = 0x18;
    drv.digit[5] = 0x24;
    drv.digit[6] = 0x42;
    drv.digit[7] = 0x81;

    matrix8x8_clear_frame(&mat);

    /* Step through one full 8-digit scan period (1.25 ms) in 10 us increments */
    for (now_ns = 1000; now_ns <= 8 * MAX7219_SLOT_NS; now_ns += 10000) {
        max7219_pins(&drv, now_ns);
        for (i = 0; i < 8; i++) {
            net_settle(&col_net[i]);
            net_settle(&row_net[i]);
        }
        matrix8x8_sample(&mat);
    }

    /* Verify that the accumulated frame on the matrix matches the pattern */
    for (r = 0; r < 8; r++) {
        char msg[64];
        uint8_t expected_row = 0;

        switch (r) {
        case 0: case 7: expected_row = (1u << 0) | (1u << 7); break;
        case 1: case 6: expected_row = (1u << 1) | (1u << 6); break;
        case 2: case 5: expected_row = (1u << 2) | (1u << 5); break;
        case 3: case 4: expected_row = (1u << 3) | (1u << 4); break;
        }

        snprintf(msg, sizeof msg, "netlist matrix frame matches expected row %d pattern", r);
        expect(mat.frame[r] == expected_row, msg);
    }
}

static void test_old_netlist_fails(void)
{
    /*
     * Under the OLD netlist:
     * Segment DP (pin 22) -> Pad 1 (which is 1088AS Row 0 Cathode)
     * Digit 0 (pin 2) -> Pad 16 (which is 1088AS Row 2 Cathode)
     * Both pads are cathodes, so driving them produces zero forward current.
     */
    Max7219 drv;
    Matrix8x8 mat;
    Net old_col0;
    Net old_row0;

    max7219_init(&drv, "U1");
    matrix8x8_init(&mat, "DS1");

    net_init(&old_col0, "OLD_C0", SIG_L);
    net_tie(&old_col0, max7219_pin(&drv, 22)); /* SEGDP */
    net_tie(&old_col0, matrix8x8_pin(&mat, 1)); /* OLD: Pad 1 */

    net_init(&old_row0, "OLD_R0", SIG_H);
    net_tie(&old_row0, max7219_pin(&drv, 2));   /* DIG0 */
    net_tie(&old_row0, matrix8x8_pin(&mat, 16)); /* OLD: Pad 16 */

    drv.shutdown = 0;
    drv.scan_limit = 0;
    drv.intensity = 15;
    drv.wake_ns = 0;
    drv.scan_origin_ns = 0;
    drv.digit[0] = 0x80; /* Col 0 bit active */

    /* Run during digit 0 slot */
    max7219_pins(&drv, 1000);
    net_settle(&old_col0);
    net_settle(&old_row0);

    /* Under old netlist, LED (0, 0) fails to illuminate */
    expect(!matrix8x8_led_active(&mat, 0, 0),
           "old sequential netlist fails to light LED (0,0) due to cathode-cathode connection");
}

int main(void)
{
    report_begin("matrix");
    test_bench_pairs();
    test_reverse_bias_and_invalid();
    test_exhaustive_64_leds();
    test_netlist_with_max7219();
    test_old_netlist_fails();
    report_end();
    return 0;
}

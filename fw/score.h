#ifndef FW_SCORE_H
#define FW_SCORE_H

/*
 * The bottom row of the field. A binary count of rocks removed by a
 * bullet, drawn on the right. The rightmost LED is the ones bit.
 * The count stops at 256.
 */

void score_reset(void);
void score_add(int points);
void score_draw(void);

#endif

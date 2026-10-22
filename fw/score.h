#ifndef FW_SCORE_H
#define FW_SCORE_H

/*
 * The round's score. It stops at 999.
 * score_judge compares it with the saved score, then stores it.
 * The result is 1 when the score is higher, 0 when the two match,
 * and -1 when the score is lower. A score of 999 against a saved
 * 999 counts as higher.
 */

void score_reset(void);
void score_clear_saved(void);
void score_add(int points);
int score_get(void);
int score_judge(void);

#endif

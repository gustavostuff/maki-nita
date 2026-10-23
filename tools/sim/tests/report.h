#ifndef TEST_REPORT_H
#define TEST_REPORT_H

void report_begin(const char *name);
void report_check(int cond, const char *msg);
int report_end(void);

#endif

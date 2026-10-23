#include "report.h"

#include <stdio.h>
#include <unistd.h>

static int failures;
static int color;
static const char *suite;

static const char *paint(const char *code)
{
    return color ? code : "";
}

void report_begin(const char *name)
{
    suite = name;
    failures = 0;
    color = isatty(STDOUT_FILENO);
    printf("%s\n", name);
}

void report_check(int cond, const char *msg)
{
    if (cond) {
        printf("%s  ok  %s%s\n", paint("\033[32m"), msg, paint("\033[0m"));
        return;
    }
    failures++;
    printf("%s  fail  %s%s\n", paint("\033[31m"), msg, paint("\033[0m"));
}

int report_end(void)
{
    if (failures == 0) {
        printf("%s%s ok%s\n\n", paint("\033[32m"), suite, paint("\033[0m"));
        return 0;
    }
    printf("%s%s: %d failure%s%s\n\n", paint("\033[31m"), suite, failures,
           failures == 1 ? "" : "s", paint("\033[0m"));
    return 1;
}

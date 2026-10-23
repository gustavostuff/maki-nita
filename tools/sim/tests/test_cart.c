#include "../cart.h"
#include "tests/report.h"

#include <stdio.h>
#include <string.h>

static void expect(int cond, const char *msg)
{
    report_check(cond, msg);
}

static void test_id(void)
{
    expect(cart_id(NULL) == NULL, "an empty document has no cart");
    expect(cart_id("") == NULL, "a blank document has no cart");
    expect(cart_id("{}") == NULL, "a missing cart key loads nothing");
    expect(cart_id("{\"cart\": \"chess\"}") == NULL, "an unknown name loads nothing");
    expect(cart_id("{\"cart\":") == NULL, "a broken value loads nothing");
    expect(strcmp(cart_id("{\"cart\": \"paddle\"}"), "paddle") == 0, "paddle is a cart");
    expect(strcmp(cart_id("{\"cart\": \"arkaled\"}"), "arkaled") == 0, "arkaled is a cart");
    expect(strcmp(cart_id("{\n  \"cart\": \"space-shooter\"\n}"), "space-shooter") == 0,
           "space-shooter is a cart");
}

static void test_file(void)
{
    FILE *f;
    const char *path = "tests/cart_sample.json";

    f = fopen(path, "wb");
    expect(f != NULL, "the sample file opens");
    if (f != NULL) {
        fputs("{ \"cart\": \"paddle\" }\n", f);
        fclose(f);
    }
    expect(cart_read_file(path) == 1 && cart_loaded(), "the file seats the paddle cart");
    expect(strcmp(cart_eeprom(), "tools/sim/paddle.eep") == 0,
           "paddle keeps its own high score");
    f = fopen(path, "wb");
    if (f != NULL) {
        fputs("{ \"cart\": \"arkaled\" }\n", f);
        fclose(f);
    }
    expect(cart_read_file(path) == 1 && cart_loaded(), "the file seats the arkaled cart");
    expect(strcmp(cart_eeprom(), "tools/sim/arkaled.eep") == 0,
           "arkaled keeps its own high score");
    f = fopen(path, "wb");
    if (f != NULL) {
        fputs("{ \"cart\": \"nope\" }\n", f);
        fclose(f);
    }
    expect(cart_read_file(path) == 0 && !cart_loaded(), "a bad name leaves the socket empty");
    expect(cart_read_file("tests/no_such_cart.json") == -1, "a missing file is not a cart");
    remove(path);
}

int main(void)
{
    report_begin("cart");
    test_id();
    test_file();
    return report_end();
}

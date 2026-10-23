#include "cart.h"

#include "game.h"
#include "paddle.h"

#include <stdio.h>
#include <string.h>

static int which;

static void skip(const char **p)
{
    while (**p == ' ' || **p == '\t' || **p == '\n' || **p == '\r')
        (*p)++;
}

static int read_string(const char **p, char *out, int cap)
{
    const char *s = *p;
    int n = 0;

    if (*s != '"')
        return 0;
    s++;
    while (*s != '\0' && *s != '"') {
        if (n + 1 < cap)
            out[n++] = *s;
        s++;
    }
    if (*s != '"')
        return 0;
    out[n] = '\0';
    *p = s + 1;
    return 1;
}

static const char *known(const char *name)
{
    if (strcmp(name, "paddle") == 0)
        return "paddle";
    if (strcmp(name, "space-shooter") == 0)
        return "space-shooter";
    return NULL;
}

const char *cart_id(const char *json)
{
    char key[32];
    char value[32];
    const char *p;

    if (json == NULL)
        return NULL;
    p = json;
    while (*p != '\0') {
        skip(&p);
        if (*p != '"') {
            if (*p == '\0')
                break;
            p++;
            continue;
        }
        if (!read_string(&p, key, (int)sizeof key)) {
            p++;
            continue;
        }
        skip(&p);
        if (*p != ':' || strcmp(key, "cart") != 0)
            continue;
        p++;
        skip(&p);
        if (!read_string(&p, value, (int)sizeof value))
            return NULL;
        return known(value);
    }
    return NULL;
}

static void select_name(const char *name)
{
    if (name == NULL)
        which = 0;
    else if (strcmp(name, "paddle") == 0)
        which = 2;
    else if (strcmp(name, "space-shooter") == 0)
        which = 1;
    else
        which = 0;
}

int cart_read_file(const char *path)
{
    FILE *f;
    char buf[512];
    size_t n;

    f = fopen(path, "rb");
    if (f == NULL)
        return -1;
    n = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[n] = '\0';
    select_name(cart_id(buf));
    return which != 0;
}

int cart_loaded(void)
{
    return which != 0;
}

const char *cart_eeprom(void)
{
    if (which == 2)
        return "tools/sim/paddle.eep";
    return NULL;
}

void cart_reset(void)
{
    if (which == 1)
        game_reset();
    else if (which == 2)
        paddle_reset();
}

void cart_tick(const Input *in)
{
    if (which == 1)
        game_tick(in);
    else if (which == 2)
        paddle_tick(in);
}

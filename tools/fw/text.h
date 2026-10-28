#ifndef FW_TEXT_H
#define FW_TEXT_H

/*
 * Shared 4x4 glyphs (3x3 font with 1px right and bottom margin).
 * Digits 0-9, letters A-Z, dot, comma, colon, semicolon, -, +, =, and /.
 * A lowercase letter uses the capital glyph. A space is a blank cell.
 *
 * text_draw paints one line from column x, row y. Each glyph takes
 * the next four columns. Pixels past the field are ignored.
 */

#include <stdint.h>

#define TEXT_W 4
#define TEXT_H 4

const uint8_t *text_glyph(char c);
void text_draw(int x, int y, const char *s);

#endif

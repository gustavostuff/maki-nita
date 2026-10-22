#ifndef FW_TEXT_H
#define FW_TEXT_H

/*
 * Shared 5x5 glyphs. Digits, A to Z, a dot, a dash, and a colon.
 * A lowercase letter uses the capital glyph. A space is a blank cell.
 *
 * text_draw paints one line from column x, row y. Each glyph takes
 * the next five columns. Pixels past the field are ignored.
 */

#define TEXT_W 5
#define TEXT_H 5

void text_draw(int x, int y, const char *s);

#endif

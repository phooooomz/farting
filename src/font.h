#ifndef ECLIPSE_FONT_H
#define ECLIPSE_FONT_H

#include <stdint.h>

typedef struct {
    uint16_t cp;      /* Unicode code point */
    uint8_t  adv;     /* advance width in px (0 for Thai marks) */
    int8_t   x0, y0;  /* bitmap offset from pen origin on the baseline */
    uint8_t  w, h;    /* bitmap size */
    uint8_t  mark;    /* 1 = combining mark, drawn at its base's origin */
    uint16_t off;     /* offset into font_bitmap */
} FontGlyph;

extern const uint8_t   font_bitmap[];
extern const FontGlyph font_glyphs[];
extern const int       font_glyph_count;
extern const int       font_size;

/* Draw a UTF-8 string with its baseline at baseline_y. Thai marks are placed over/under their base. */
void font_draw(int x, int baseline_y, const char *utf8, uint32_t color);
/* Width in pixels of a UTF-8 string (marks add no width). */
int  font_width(const char *utf8);

#endif

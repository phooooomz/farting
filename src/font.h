#ifndef ECLIPSE_FONT_H
#define ECLIPSE_FONT_H

#include <stdint.h>

typedef struct {
    uint16_t cp;      /* Unicode code point */
    uint8_t  adv;     /* advance width in px (0 for Thai marks) */
    int8_t   x0, y0;  /* bitmap offset from pen origin on the baseline */
    uint8_t  w, h;    /* bitmap size */
    uint8_t  mark;    /* 1 = combining mark */
    uint16_t off;     /* offset into font_bitmap */
} FontGlyph;

extern const uint8_t   font_bitmap[];
extern const FontGlyph font_glyphs[];
extern const int       font_glyph_count;
extern const int       font_size;

typedef void (*FontEmitFn)(const FontGlyph *g, int gx, int gy, void *ctx);

/* Walks a UTF-8 string and reports each glyph's pen position. Thai marks are centered over their base. */
void font_layout(const char *utf8, int x, int baseline_y, FontEmitFn emit, void *ctx);
int  font_width(const char *utf8);
const FontGlyph *font_find(uint32_t cp);

#endif

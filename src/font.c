#include "font.h"

/* Decode one UTF-8 code point; returns bytes consumed (>=1). */
static int utf8_next(const char *s, uint32_t *cp)
{
    const unsigned char c = (unsigned char)s[0];
    if (c < 0x80)           { *cp = c; return 1; }
    if ((c & 0xE0) == 0xC0) { *cp = ((c & 0x1F) << 6) | (s[1] & 0x3F); return 2; }
    if ((c & 0xF0) == 0xE0) { *cp = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); return 3; }
    *cp = '?';
    return 1;
}

const FontGlyph *font_find(uint32_t cp)
{
    int lo = 0, hi = font_glyph_count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (font_glyphs[mid].cp == cp) return &font_glyphs[mid];
        if (font_glyphs[mid].cp < cp) lo = mid + 1; else hi = mid - 1;
    }
    return 0;
}

void font_layout(const char *utf8, int x, int baseline_y, FontEmitFn emit, void *ctx)
{
    int pen = x, base_x = x, base_adv = 0;
    while (*utf8) {
        uint32_t cp;
        utf8 += utf8_next(utf8, &cp);
        const FontGlyph *g = font_find(cp);
        if (!g) continue;
        if (g->mark) {
            int mx = base_x + base_adv / 2 - (g->x0 + g->w / 2);
            emit(g, mx, baseline_y, ctx);
        } else {
            emit(g, pen, baseline_y, ctx);
            base_x = pen;
            base_adv = g->adv;
            pen += g->adv;
        }
    }
}

int font_width(const char *utf8)
{
    int w = 0;
    while (*utf8) {
        uint32_t cp;
        utf8 += utf8_next(utf8, &cp);
        const FontGlyph *g = font_find(cp);
        if (g && !g->mark) w += g->adv;
    }
    return w;
}

#include "font.h"
#include "video.h"

/* Decode one UTF-8 code point; returns bytes consumed (>=1). */
static int utf8_next(const char *s, uint32_t *cp)
{
    const unsigned char c = (unsigned char)s[0];
    if (c < 0x80)              { *cp = c; return 1; }
    if ((c & 0xE0) == 0xC0)    { *cp = ((c & 0x1F) << 6) | (s[1] & 0x3F); return 2; }
    if ((c & 0xF0) == 0xE0)    { *cp = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); return 3; }
    *cp = '?';
    return 1;
}

static const FontGlyph *find_glyph(uint32_t cp)
{
    int lo = 0, hi = font_glyph_count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (font_glyphs[mid].cp == cp) return &font_glyphs[mid];
        if (font_glyphs[mid].cp < cp) lo = mid + 1; else hi = mid - 1;
    }
    return 0; /* unknown glyph: caller skips */
}

static void draw_glyph(const FontGlyph *g, int ox, int baseline_y, uint32_t color)
{
    const uint8_t *bmp = font_bitmap + g->off;
    for (int j = 0; j < g->h; j++) {
        for (int i = 0; i < g->w; i++) {
            uint8_t a = bmp[j * g->w + i];
            if (a) video_blend_pixel(ox + g->x0 + i, baseline_y + g->y0 + j, color, a);
        }
    }
}

void font_draw(int x, int baseline_y, const char *utf8, uint32_t color)
{
    int pen = x, base_x = x;
    while (*utf8) {
        uint32_t cp;
        utf8 += utf8_next(utf8, &cp);
        const FontGlyph *g = find_glyph(cp);
        if (!g) continue;
        if (g->mark) {
            draw_glyph(g, base_x, baseline_y, color);   /* stay over the base */
        } else {
            draw_glyph(g, pen, baseline_y, color);
            base_x = pen;
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
        const FontGlyph *g = find_glyph(cp);
        if (g && !g->mark) w += g->adv;
    }
    return w;
}

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <string.h>
#include "gfx.h"
#include "font.h"

#define BUF_W        512
#define FRAME0_OFF   0x00000u
#define FRAME1_OFF   0x88000u        /* 512 * 272 * 4 */
#define DEPTH_OFF    0x110000u       /* 512 * 272 * 2 */

#define ATLAS_SIZE   512
#define CELL         24
#define CELLS_PER_ROW (ATLAS_SIZE / CELL)

static unsigned int __attribute__((aligned(64))) gu_list[262144];
static unsigned int __attribute__((aligned(16))) atlas[ATLAS_SIZE * ATLAS_SIZE];

typedef struct { float u, v; uint32_t color; float x, y, z; } TexVert;
typedef struct { uint32_t color; float x, y, z; } ColVert;

/* Rebuilds the glyph atlas from font_bitmap: white glyphs, alpha = coverage. */
static void build_atlas(void)
{
    memset(atlas, 0, sizeof(atlas));
    int n = font_glyph_count < (CELLS_PER_ROW * CELLS_PER_ROW) ? font_glyph_count : (CELLS_PER_ROW * CELLS_PER_ROW);
    for (int i = 0; i < n; i++) {
        const FontGlyph *g = &font_glyphs[i];
        int cx = (i % CELLS_PER_ROW) * CELL;
        int cy = (i / CELLS_PER_ROW) * CELL;
        int w = g->w > CELL ? CELL : g->w;
        int h = g->h > CELL ? CELL : g->h;
        for (int j = 0; j < h; j++)
            for (int k = 0; k < w; k++) {
                uint8_t a = font_bitmap[g->off + j * g->w + k];
                atlas[(cy + j) * ATLAS_SIZE + cx + k] = ((uint32_t)a << 24) | 0x00FFFFFFu;
            }
    }
    sceKernelDcacheWritebackInvalidateAll();
}

static int cell_of(const FontGlyph *g)
{
    return (int)(g - font_glyphs);   /* atlas cell i holds font_glyphs[i] */
}

void gfx_init(void)
{
    sceGuInit();
    sceGuStart(GU_DIRECT, gu_list);
    sceGuDrawBuffer(GU_PSM_8888, (void *)FRAME0_OFF, BUF_W);
    sceGuDispBuffer(SCREEN_W, SCREEN_H, (void *)FRAME1_OFF, BUF_W);
    sceGuDepthBuffer((void *)DEPTH_OFF, BUF_W);
    sceGuOffset(2048 - (SCREEN_W / 2), 2048 - (SCREEN_H / 2));
    sceGuViewport(2048, 2048, SCREEN_W, SCREEN_H);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCREEN_W, SCREEN_H);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuShadeModel(GU_SMOOTH);
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);

    build_atlas();
    sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexImage(0, ATLAS_SIZE, ATLAS_SIZE, ATLAS_SIZE, atlas);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
}

void gfx_begin_frame(uint32_t clear_color)
{
    sceGuStart(GU_DIRECT, gu_list);
    sceGuClearColor(clear_color);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
}

void gfx_begin_3d(float cam_x, float cam_y, float cam_z, float yaw, float pitch, float dist)
{
    (void)cam_y;
    sceGuEnable(GU_DEPTH_TEST);
    sceGuDisable(GU_TEXTURE_2D);
    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumPerspective(60.0f, (float)SCREEN_W / SCREEN_H, 0.5f, 300.0f);
    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();
    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();
    ScePspFVector3 back = { 0.0f, 0.0f, -dist };
    ScePspFVector3 target = { -cam_x, -cam_y, -cam_z };
    sceGumTranslate(&back);
    sceGumRotateX(pitch);
    sceGumRotateY(yaw);
    sceGumTranslate(&target);
}

void gfx_end_3d(void)
{
    sceGuDisable(GU_DEPTH_TEST);
}

/* Emits one quad (two triangles) into a ColVert buffer, returns vertices written. */
static int quad(ColVert *v, Vec3 a, Vec3 b, Vec3 c, Vec3 d, uint32_t col)
{
    v[0] = (ColVert){ col, a.x, a.y, a.z };
    v[1] = (ColVert){ col, b.x, b.y, b.z };
    v[2] = (ColVert){ col, c.x, c.y, c.z };
    v[3] = (ColVert){ col, a.x, a.y, a.z };
    v[4] = (ColVert){ col, c.x, c.y, c.z };
    v[5] = (ColVert){ col, d.x, d.y, d.z };
    return 6;
}

static uint32_t shade(uint32_t c, int num, int den)
{
    uint32_t r = ((c & 0xFF) * num) / den, g = (((c >> 8) & 0xFF) * num) / den, b = (((c >> 16) & 0xFF) * num) / den;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    return RGBA(r, g, b, 0xFF);
}

static void draw_box_colors(Vec3 p, Vec3 s, uint32_t top, uint32_t side)
{
    float x0 = p.x - s.x / 2, x1 = p.x + s.x / 2;
    float y0 = p.y,            y1 = p.y + s.y;
    float z0 = p.z - s.z / 2, z1 = p.z + s.z / 2;
    ColVert *v = (ColVert *)sceGuGetMemory(36 * sizeof(ColVert));
    int n = 0;
    n += quad(v + n, (Vec3){x0,y1,z0}, (Vec3){x1,y1,z0}, (Vec3){x1,y1,z1}, (Vec3){x0,y1,z1}, top);
    n += quad(v + n, (Vec3){x0,y0,z1}, (Vec3){x1,y0,z1}, (Vec3){x1,y1,z1}, (Vec3){x0,y1,z1}, side);
    n += quad(v + n, (Vec3){x1,y0,z0}, (Vec3){x0,y0,z0}, (Vec3){x0,y1,z0}, (Vec3){x1,y1,z0}, shade(side, 3, 4));
    n += quad(v + n, (Vec3){x0,y0,z0}, (Vec3){x0,y0,z1}, (Vec3){x0,y1,z1}, (Vec3){x0,y1,z0}, shade(side, 3, 4));
    n += quad(v + n, (Vec3){x1,y0,z1}, (Vec3){x1,y0,z0}, (Vec3){x1,y1,z0}, (Vec3){x1,y1,z1}, shade(side, 4, 5));
    sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, n, 0, v);
}

void gfx_draw_box(Vec3 pos, Vec3 size, uint32_t color)
{
    draw_box_colors(pos, size, color, color);
}

void gfx_draw_box_shaded(Vec3 pos, Vec3 size, uint32_t color)
{
    draw_box_colors(pos, size, shade(color, 6, 5), color);
}

void gfx_draw_ground(float cx, float cz, uint32_t c1, uint32_t c2)
{
    const int R = 12;          /* tiles each side */
    const float T = 4.0f;      /* tile size */
    int gx = (int)(cx / T), gz = (int)(cz / T);
    int count = (2 * R) * (2 * R) * 6;
    ColVert *v = (ColVert *)sceGuGetMemory(count * sizeof(ColVert));
    int n = 0;
    for (int i = -R; i < R; i++)
        for (int j = -R; j < R; j++) {
            int tx = gx + i, tz = gz + j;
            float x0 = tx * T, z0 = tz * T;
            uint32_t col = ((tx + tz) & 1) ? c1 : c2;
            n += quad(v + n, (Vec3){x0, 0, z0}, (Vec3){x0 + T, 0, z0}, (Vec3){x0 + T, 0, z0 + T}, (Vec3){x0, 0, z0 + T}, col);
        }
    sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, n, 0, v);
}

void gfx_fill_rect(int x, int y, int w, int h, uint32_t color)
{
    sceGuDisable(GU_TEXTURE_2D);
    sceGuDisable(GU_DEPTH_TEST);
    ColVert *v = (ColVert *)sceGuGetMemory(2 * sizeof(ColVert));
    v[0] = (ColVert){ color, (float)x, (float)y, 0.0f };
    v[1] = (ColVert){ color, (float)(x + w), (float)(y + h), 0.0f };
    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
}

typedef struct { uint32_t color; TexVert *out; int n; } TextCtx;

static void emit_glyph(const FontGlyph *g, int gx, int gy, void *ctx)
{
    TextCtx *t = (TextCtx *)ctx;
    int cell = cell_of(g);
    if (cell < 0 || cell >= font_glyph_count || g->w == 0 || g->h == 0) return;
    int cx = (cell % CELLS_PER_ROW) * CELL, cy = (cell / CELLS_PER_ROW) * CELL;
    int w = g->w > CELL ? CELL : g->w, h = g->h > CELL ? CELL : g->h;
    float x0 = (float)(gx + g->x0), y0 = (float)(gy + g->y0);
    float u0 = (float)cx, v0 = (float)cy;
    float u1 = (float)(cx + w), v1 = (float)(cy + h);
    TexVert *o = t->out + t->n;
    o[0] = (TexVert){ u0, v0, t->color, x0, y0, 0.0f };
    o[1] = (TexVert){ u1, v1, t->color, x0 + w, y0 + h, 0.0f };
    t->n += 2;
}

#define MAX_TEXT_GLYPHS 128

void gfx_text(int x, int baseline_y, const char *utf8, uint32_t color)
{
    TexVert *v = (TexVert *)sceGuGetMemory(MAX_TEXT_GLYPHS * 2 * sizeof(TexVert));
    TextCtx t = { color, v, 0 };
    font_layout(utf8, x, baseline_y, emit_glyph, &t);
    if (t.n == 0) return;
    sceGuDisable(GU_DEPTH_TEST);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuTexScale(1.0f / ATLAS_SIZE, 1.0f / ATLAS_SIZE);
    sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, t.n, 0, v);
    sceGuDisable(GU_TEXTURE_2D);
}

int gfx_text_width(const char *utf8) { return font_width(utf8); }

void gfx_end_frame(void)
{
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}

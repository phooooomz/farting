#ifndef ECLIPSE_GFX_H
#define ECLIPSE_GFX_H

#include <stdint.h>

#define SCREEN_W 480
#define SCREEN_H 272

/* RGBA(r,g,b,a) packs to the PSP GU 8888 order (R in the low byte). */
#define RGBA(r, g, b, a) ((uint32_t)((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((r) & 0xFF)))

typedef struct { float x, y, z; } Vec3;

void gfx_init(void);
void gfx_begin_frame(uint32_t clear_color);
void gfx_begin_3d(float cam_x, float cam_y, float cam_z, float yaw, float pitch, float dist);
void gfx_end_3d(void);
void gfx_draw_ground(float cx, float cz, uint32_t c1, uint32_t c2);
void gfx_draw_box(Vec3 pos, Vec3 size, uint32_t color);
void gfx_draw_box_shaded(Vec3 pos, Vec3 size, uint32_t color);   /* top lighter, sides darker */
void gfx_fill_rect(int x, int y, int w, int h, uint32_t color);  /* 2D, screen space */
void gfx_text(int x, int baseline_y, const char *utf8, uint32_t color);
int  gfx_text_width(const char *utf8);
void gfx_end_frame(void);

#endif

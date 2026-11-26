#include <pspdisplay.h>
#include "video.h"

/* Uncached VRAM mirror: CPU writes are immediately visible to the display controller. */
#define VRAM_UNCACHED_BASE  0x44000000u
#define FRAME_BYTES         ((uint32_t)(LINE_W * SCREEN_H * 4))

static uint32_t *buffers[2];
static int back_index = 0;

void video_init(void)
{
    buffers[0] = (uint32_t *)(VRAM_UNCACHED_BASE);
    buffers[1] = (uint32_t *)(VRAM_UNCACHED_BASE + FRAME_BYTES);

    sceDisplaySetMode(0, SCREEN_W, SCREEN_H);
    sceDisplaySetFrameBuf(buffers[0], LINE_W, PSP_DISPLAY_PIXEL_FORMAT_8888,
                          PSP_DISPLAY_SETBUF_IMMEDIATE);
    back_index = 1;   /* draw into buffer 1 while buffer 0 is shown */
}

void video_fill(uint32_t color)
{
    uint32_t *dst = buffers[back_index];
    for (int y = 0; y < SCREEN_H; y++) {
        uint32_t *row = dst + y * LINE_W;
        for (int x = 0; x < SCREEN_W; x++) {
            row[x] = color;
        }
    }
}

void video_fill_rect(int x, int y, int w, int h, uint32_t color)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_W) w = SCREEN_W - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;
    if (w <= 0 || h <= 0) return;

    uint32_t *dst = buffers[back_index];
    for (int j = y; j < y + h; j++) {
        uint32_t *row = dst + j * LINE_W;
        for (int i = x; i < x + w; i++) {
            row[i] = color;
        }
    }
}

void video_flip(void)
{
    sceDisplaySetFrameBuf(buffers[back_index], LINE_W, PSP_DISPLAY_PIXEL_FORMAT_8888,
                          PSP_DISPLAY_SETBUF_NEXTFRAME);
    back_index ^= 1;
}

void video_wait_vblank(void)
{
    sceDisplayWaitVblankStart();
}

void video_blend_pixel(int x, int y, uint32_t color, uint8_t alpha)
{
    if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return;
    uint32_t *p = &buffers[back_index][y * LINE_W + x];
    uint32_t d = *p;
    uint32_t a = alpha, ia = 255 - a;
    uint32_t r = ((color & 0xFF) * a + (d & 0xFF) * ia) / 255;
    uint32_t g = (((color >> 8) & 0xFF) * a + ((d >> 8) & 0xFF) * ia) / 255;
    uint32_t b = (((color >> 16) & 0xFF) * a + ((d >> 16) & 0xFF) * ia) / 255;
    *p = RGBA(r, g, b, 0xFF);
}

#ifndef ECLIPSE_VIDEO_H
#define ECLIPSE_VIDEO_H

#include <stdint.h>

#define SCREEN_W 480
#define SCREEN_H 272
#define LINE_W   512   /* PSP framebuffer stride (pixels) */

/* Pixel format is PSP_DISPLAY_PIXEL_FORMAT_8888: memory order is R,G,B,A (little endian word = 0xAABBGGRR). */
#define RGBA(r, g, b, a) ((uint32_t)((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((r) & 0xFF)))

void video_init(void);
void video_fill(uint32_t color);
void video_fill_rect(int x, int y, int w, int h, uint32_t color);
void video_flip(void);          /* present the back buffer and swap buffers */
void video_wait_vblank(void);

#endif

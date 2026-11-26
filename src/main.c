/*
 * ECLIPSE: Little Wonders of Aetheria
 * Phase 1: build pipeline + basic loop
 *
 * This file only verifies that:
 *   - the EBOOT builds
 *   - the display initialises at 480x272
 *   - the controller works (D-pad / analog move the marker)
 *   - START exits the game
 *   - the loop is capped at ~30 FPS
 *
 * The marker below is a TEST MARKER, not game content.
 */
#include <pspkernel.h>
#include "system.h"
#include "video.h"
#include "input.h"

PSP_MODULE_INFO("ECLIPSE", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

#define COLOR_SKY     RGBA(0xBF, 0xE6, 0xFF, 0xFF)
#define COLOR_MARKER  RGBA(0xFF, 0xB3, 0xD9, 0xFF)

#define MARKER_SIZE   24
#define MOVE_SPEED    2

int main(void)
{
    system_init();
    video_init();
    input_init();

    int mx = (SCREEN_W - MARKER_SIZE) / 2;
    int my = (SCREEN_H - MARKER_SIZE) / 2;

    InputState in;

    for (;;) {
        input_update(&in);

        if (in.pressed & PSP_CTRL_START) {
            break;
        }

        /* D-pad */
        if (in.buttons & PSP_CTRL_LEFT)  mx -= MOVE_SPEED;
        if (in.buttons & PSP_CTRL_RIGHT) mx += MOVE_SPEED;
        if (in.buttons & PSP_CTRL_UP)    my -= MOVE_SPEED;
        if (in.buttons & PSP_CTRL_DOWN)  my += MOVE_SPEED;

        /* Analog stick (dead zone around the center) */
        int dx = (in.lx - 128) / 32;
        int dy = (in.ly - 128) / 32;
        mx += dx;
        my += dy;

        if (mx < 0) mx = 0;
        if (my < 0) my = 0;
        if (mx > SCREEN_W - MARKER_SIZE) mx = SCREEN_W - MARKER_SIZE;
        if (my > SCREEN_H - MARKER_SIZE) my = SCREEN_H - MARKER_SIZE;

        video_fill(COLOR_SKY);
        video_fill_rect(mx, my, MARKER_SIZE, MARKER_SIZE, COLOR_MARKER);
        video_flip();

        /* 60 Hz display -> two vblanks per frame = 30 FPS target */
        video_wait_vblank();
        video_wait_vblank();
    }

    sceKernelExitGame();
    return 0;
}

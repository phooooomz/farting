/*
 * ECLIPSE: Little Wonders of Aetheria
 * Phase 2: main menu, settings (language switch), Thai text rendering.
 */
#include <pspkernel.h>
#include "system.h"
#include "video.h"
#include "input.h"
#include "font.h"
#include "locale.h"
#include "menu.h"

PSP_MODULE_INFO("ECLIPSE", PSP_MODULE_USER, 0, 2);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

typedef enum { STATE_MENU, STATE_COMING_SOON } GameState;

int main(void)
{
    system_init();
    video_init();
    input_init();
    loc_set(LANG_TH);       /* default language: Thai */
    menu_init();

    GameState state = STATE_MENU;
    InputState in;

    for (;;) {
        input_update(&in);

        if (state == STATE_MENU) {
            MenuAction act = menu_update(&in);
            if (act == MENU_ACT_EXIT) break;
            if (act == MENU_ACT_START) state = STATE_COMING_SOON;
            menu_draw();
        } else {
            if (in.pressed & (PSP_CTRL_CIRCLE | PSP_CTRL_START)) state = STATE_MENU;
            video_fill(RGBA(0xE8, 0xF4, 0xFF, 0xFF));
            int w = font_width(loc_str(MSG_COMING_SOON));
            font_draw((SCREEN_W - w) / 2, SCREEN_H / 2, loc_str(MSG_COMING_SOON), RGBA(0x4A, 0x3B, 0x5C, 0xFF));
        }

        video_flip();
        video_wait_vblank();
        video_wait_vblank();   /* 30 FPS */
    }

    sceKernelExitGame();
    return 0;
}

/*
 * ECLIPSE: Little Wonders of Aetheria
 * Phase 3-15: title menu, 3D world (GU), combat, quests, crafting, pets, save/load.
 */
#include <pspkernel.h>
#include "system.h"
#include "gfx.h"
#include "input.h"
#include "locale.h"
#include "menu.h"
#include "game.h"
#include "gamestate.h"
#include "savefile.h"

PSP_MODULE_INFO("ECLIPSE", PSP_MODULE_USER, 0, 3);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

typedef enum { ST_TITLE, ST_GAME } AppState;

static GameState gs;

static int load_first_slot(void)
{
    for (int i = 0; i < SAVE_SLOTS; i++) {
        if (savefile_read(i, &gs)) return 1;
    }
    return 0;
}

int main(void)
{
    system_init();
    input_init();
    gfx_init();
    loc_set(LANG_TH);
    menu_init();

    AppState state = ST_TITLE;
    int paused = 0;
    InputState in;

    for (;;) {
        input_update(&in);

        if (state == ST_TITLE) {
            MenuAction act = menu_update(&in);
            if (act == MENU_ACT_EXIT) break;
            if (act == MENU_ACT_START) {
                game_new(&gs);
                game_begin(&gs);
                game_menu_reset();
                paused = 0;
                state = ST_GAME;
            } else if (act == MENU_ACT_LOAD) {
                if (load_first_slot()) { game_begin(&gs); game_menu_reset(); paused = 0; state = ST_GAME; }
            }
            menu_draw();
            continue;
        }

        /* ST_GAME */
        if (in.pressed & PSP_CTRL_START) paused = !paused;
        if (!paused && game_menu_wanted()) paused = 1;
        if (paused) game_menu_update(&gs, &in);
        else        game_update(&gs, &in);

        gfx_begin_frame(game_sky_color(&gs));
        game_draw(&gs);
        if (paused) game_menu_draw(&gs);
        gfx_end_frame();
    }

    sceKernelExitGame();
    return 0;
}

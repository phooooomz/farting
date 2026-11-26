#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"

PSP_MODULE_INFO("Fantasy RPG Starter", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main(void) {
    Game game;
    SceCtrlData pad;
    unsigned int previous = 0;
    pspDebugScreenInit();
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    srand((unsigned int)time(NULL));
    game_init(&game);
    game_load(&game);

    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);
        unsigned int pressed = pad.Buttons & ~previous;
        previous = pad.Buttons;

        if (pressed & PSP_CTRL_UP) game_move(&game, 0, -1);
        if (pressed & PSP_CTRL_DOWN) game_move(&game, 0, 1);
        if (pressed & PSP_CTRL_LEFT) game_move(&game, -1, 0);
        if (pressed & PSP_CTRL_RIGHT) game_move(&game, 1, 0);
        if (pressed & PSP_CTRL_CROSS) game_attack(&game);
        if (pressed & PSP_CTRL_TRIANGLE) game_save(&game);
        if (pressed & PSP_CTRL_START) break;

        game_update(&game);
        pspDebugScreenClear();
        game_draw(&game);
        sceKernelDelayThread(80000);
    }
    sceKernelExitGame();
    return 0;
}

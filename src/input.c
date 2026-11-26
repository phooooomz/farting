#include "input.h"

static uint32_t prev_buttons = 0;

void input_init(void)
{
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    prev_buttons = 0;
}

void input_update(InputState *state)
{
    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);

    state->buttons = pad.Buttons;
    state->pressed = pad.Buttons & ~prev_buttons;
    state->lx = pad.Lx;
    state->ly = pad.Ly;

    prev_buttons = pad.Buttons;
}

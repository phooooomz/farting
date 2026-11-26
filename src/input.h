#ifndef ECLIPSE_INPUT_H
#define ECLIPSE_INPUT_H

#include <stdint.h>
#include <pspctrl.h>

typedef struct {
    uint32_t buttons;   /* buttons currently held */
    uint32_t pressed;   /* buttons that went down this frame */
    int      lx;        /* analog stick X, 0..255, center ~128 */
    int      ly;        /* analog stick Y, 0..255, center ~128 */
} InputState;

void input_init(void);
void input_update(InputState *state);

#endif

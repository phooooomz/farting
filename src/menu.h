#ifndef ECLIPSE_MENU_H
#define ECLIPSE_MENU_H

#include "input.h"

typedef enum { MENU_NONE, MENU_ACT_START, MENU_ACT_LOAD, MENU_ACT_EXIT } MenuAction;

void       menu_init(void);
MenuAction menu_update(const InputState *in);
void       menu_draw(void);   /* title screen; owns its own frame */

#endif

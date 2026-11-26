#ifndef ECLIPSE_GAME_H
#define ECLIPSE_GAME_H

#include "gamestate.h"
#include "input.h"

void game_begin(GameState *g);                       /* call when a new game or a load starts */
void game_update(GameState *g, const InputState *in); /* world simulation, not called while paused */
uint32_t game_sky_color(const GameState *g);
void game_draw(const GameState *g);          /* world + HUD; caller owns the frame */
void game_menu_update(GameState *g, const InputState *in);
void game_menu_draw(const GameState *g);     /* pause overlay; caller owns the frame */
void game_menu_reset(void);
int  game_menu_wanted(void);     /* 1 if a world object asked to open the pause menu (clears flag) */

#endif

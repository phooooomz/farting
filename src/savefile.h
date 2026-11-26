#ifndef ECLIPSE_SAVEFILE_H
#define ECLIPSE_SAVEFILE_H
#include "gamestate.h"

int  savefile_exists(int slot);
int  savefile_write(int slot, const GameState *g);   /* 1 on success */
int  savefile_read(int slot, GameState *g);           /* 1 on success, validates checksum */
#endif

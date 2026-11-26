#ifndef ECLIPSE_GAMESTATE_H
#define ECLIPSE_GAMESTATE_H
#include <stdint.h>
#include "stats.h"
#include "items.h"
#include "quest.h"
#include "world.h"
#include "pet.h"

#define SAVE_SLOTS 3

/* Everything that is saved. Only fixed-width fields, so the layout is stable. */
typedef struct {
    PlayerStats  stats;
    Inventory    inv;
    uint16_t     equip[EQ_COUNT];   /* item id per slot, 0 = empty */
    int32_t      gold;
    QuestState   quests[QUEST_COUNT];
    WorldState   world;
    PetState     pets[PET_COUNT];
    uint8_t      current_pet;       /* index, 0xFF = none */
    uint8_t      mounted;
    int32_t      pos_x, pos_z;      /* world units * 16 */
    uint8_t      region;
    uint32_t     play_seconds;
} GameState;

void game_new(GameState *g);
#endif

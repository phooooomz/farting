#include <string.h>
#include "gamestate.h"

void game_new(GameState *g)
{
    memset(g, 0, sizeof(*g));
    stats_init(&g->stats);
    world_init(&g->world);
    g->gold = 50;
    g->current_pet = 0xFF;
    g->pos_x = 0;
    g->pos_z = 0;
    g->region = REG_BLOOMVALE;
    inv_add(&g->inv, 1, 1);    /* ดาบไม้ */
    inv_add(&g->inv, 20, 3);   /* ยาเล็ก */
    g->equip[EQ_WEAPON] = 1;
}

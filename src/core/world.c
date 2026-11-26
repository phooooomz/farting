#include "world.h"

static const ResourceNode seed_nodes[] = {
    { NODE_TREE,     REG_BLOOMVALE,       -20,  30, 10, 60, 0 },
    { NODE_TREE,     REG_BLOOMVALE,        40, -10, 10, 60, 0 },
    { NODE_ROCK,     REG_BLOOMVALE,        10,  50, 11, 90, 0 },
    { NODE_HERB,     REG_BLOOMVALE,       -35, -25, 14, 45, 0 },
    { NODE_MUSHROOM, REG_WHISPERING_WOODS, 60,  70, 14, 60, 0 },
    { NODE_FLOWER,   REG_WHISPERING_WOODS, 80,  10, 15, 60, 0 },
    { NODE_ORE,      REG_CRYSTAL_CAVERNS,  -50, 60, 13, 120, 0 },
    { NODE_ORE,      REG_BLOOMVALE,        75, 60, 12, 120, 0 },
};

void world_init(WorldState *w)
{
    for (int r = 0; r < REGION_COUNT; r++) w->region_unlocked[r] = (r == REG_BLOOMVALE);
    w->node_count = (int)(sizeof(seed_nodes) / sizeof(seed_nodes[0]));
    for (int i = 0; i < w->node_count; i++) w->nodes[i] = seed_nodes[i];
}

uint16_t world_harvest(WorldState *w, int i)
{
    if (i < 0 || i >= w->node_count) return 0;
    ResourceNode *n = &w->nodes[i];
    if (n->timer > 0) return 0;
    n->timer = n->respawn_s;
    return n->item;
}

void world_tick(WorldState *w, int seconds)
{
    for (int i = 0; i < w->node_count; i++) {
        ResourceNode *n = &w->nodes[i];
        if (n->timer > 0) n->timer = (uint16_t)(n->timer > seconds ? n->timer - seconds : 0);
    }
}

int world_unlock_region(WorldState *w, int region)
{
    if (region < 0 || region >= REGION_COUNT) return 0;
    w->region_unlocked[region] = 1;
    return 1;
}

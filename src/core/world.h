#ifndef ECLIPSE_WORLD_H
#define ECLIPSE_WORLD_H
#include <stdint.h>

#define REGION_COUNT 6
#define MAX_NODES 16

typedef enum { REG_BLOOMVALE, REG_WHISPERING_WOODS, REG_SNOWBERRY_VALLEY, REG_CORAL_COAST,
               REG_CRYSTAL_CAVERNS, REG_STARCLOUD_KINGDOM } Region;

typedef enum { NODE_TREE, NODE_ROCK, NODE_ORE, NODE_HERB, NODE_FLOWER, NODE_MUSHROOM } NodeType;

typedef struct {
    uint8_t  type;        /* NodeType */
    uint8_t  region;      /* Region */
    int16_t  x, z;        /* world position */
    uint16_t item;        /* item it yields */
    uint16_t respawn_s;   /* seconds to respawn */
    uint16_t timer;       /* 0 = available, >0 = seconds left */
} ResourceNode;

typedef struct {
    uint8_t region_unlocked[REGION_COUNT];
    ResourceNode nodes[MAX_NODES];
    int node_count;
} WorldState;

void world_init(WorldState *w);
/* Returns item id harvested, or 0 if the node is still respawning. */
uint16_t world_harvest(WorldState *w, int node_index);
/* Advance respawn timers by seconds. */
void world_tick(WorldState *w, int seconds);
int  world_unlock_region(WorldState *w, int region);
#endif

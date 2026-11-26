#ifndef ECLIPSE_CRAFTING_H
#define ECLIPSE_CRAFTING_H
#include <stdint.h>
#include "items.h"

typedef enum { ST_BLACKSMITH, ST_ALCHEMY, ST_WORKSHOP, ST_COOKING, ST_ENCHANT } Station;

typedef struct {
    uint16_t result, result_count;
    uint16_t need_item[4], need_count[4];   /* 0 = unused slot */
    uint16_t gold;
    Station  station;
} Recipe;

extern const Recipe recipes[];
extern const int    recipe_count;

int  recipe_can_craft(const Recipe *r, const Inventory *inv, int gold);
/* Consumes materials and gold, adds the result. Returns 1 on success. */
int  recipe_craft(const Recipe *r, Inventory *inv, int *gold);
#endif

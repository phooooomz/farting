#include "crafting.h"

const Recipe recipes[] = {
    { 1,  1, {10, 11, 0, 0}, {2, 1, 0, 0}, 5,  ST_BLACKSMITH },  /* ดาบไม้ */
    { 2,  1, {10, 0, 0, 0},  {4, 0, 0, 0}, 3,  ST_WORKSHOP },    /* โล่ไม้ */
    { 3,  1, {13, 10, 0, 0}, {5, 2, 0, 0}, 40, ST_BLACKSMITH },  /* ดาบเหล็ก */
    { 4,  1, {10, 0, 0, 0},  {3, 0, 0, 0}, 8,  ST_WORKSHOP },    /* หมวกหนัง */
    { 20, 2, {14, 15, 0, 0}, {1, 1, 0, 0}, 4,  ST_ALCHEMY },     /* ยาเล็ก x2 */
    { 21, 2, {15, 0, 0, 0},  {2, 0, 0, 0}, 4,  ST_ALCHEMY },     /* ยาเวทเล็ก x2 */
    { 22, 1, {14, 16, 0, 0}, {1, 1, 0, 0}, 3,  ST_COOKING },     /* ขนมน่ารัก */
    { 40, 1, {17, 13, 0, 0}, {2, 3, 0, 0}, 120,ST_ENCHANT },     /* แหวนพลังงาน */
};
const int recipe_count = (int)(sizeof(recipes) / sizeof(recipes[0]));

int recipe_can_craft(const Recipe *r, const Inventory *inv, int gold)
{
    if (gold < r->gold) return 0;
    for (int i = 0; i < 4; i++) {
        if (r->need_item[i] && item_count(inv, r->need_item[i]) < r->need_count[i]) return 0;
    }
    return 1;
}

int recipe_craft(const Recipe *r, Inventory *inv, int *gold)
{
    if (!recipe_can_craft(r, inv, *gold)) return 0;
    /* Check room for the result before consuming anything. */
    Inventory test = *inv;
    for (int i = 0; i < 4; i++) if (r->need_item[i]) inv_remove(&test, r->need_item[i], r->need_count[i]);
    if (inv_add(&test, r->result, r->result_count) != r->result_count) return 0;
    *inv = test;
    *gold -= r->gold;
    return 1;
}

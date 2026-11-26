#include "items.h"

/* Item database. Names are stored per language; the UI picks one. */
static const ItemDef db[] = {
    { 1,  "ดาบไม้",       "Wooden Sword",    IT_WEAPON,     RA_COMMON,   10, 1, 4,  0, 0, 0, 0 },
    { 2,  "โล่ไม้",       "Wooden Shield",   IT_ARMOR,      RA_COMMON,   10, 1, 0,  3, 0, 0, 0 },
    { 3,  "ดาบเหล็ก",     "Iron Sword",      IT_WEAPON,     RA_UNCOMMON, 60, 1, 10, 0, 0, 0, 0 },
    { 4,  "หมวกหนัง",     "Leather Cap",     IT_ARMOR,      RA_COMMON,   15, 1, 0,  2, 0, 0, 0 },
    { 10, "ไม้",          "Wood",            IT_MATERIAL,   RA_COMMON,    2, 99, 0, 0, 0, 0, 0 },
    { 11, "หิน",          "Stone",           IT_MATERIAL,   RA_COMMON,    2, 99, 0, 0, 0, 0, 0 },
    { 12, "แร่ทองแดง",    "Copper Ore",      IT_MATERIAL,   RA_COMMON,    5, 99, 0, 0, 0, 0, 0 },
    { 13, "แร่เหล็ก",     "Iron Ore",        IT_MATERIAL,   RA_UNCOMMON,  8, 99, 0, 0, 0, 0, 0 },
    { 14, "เห็ดเวท",      "Magic Mushroom",  IT_MATERIAL,   RA_COMMON,    3, 99, 0, 0, 0, 0, 0 },
    { 15, "ดอกไม้เวท",    "Magic Flower",    IT_MATERIAL,   RA_COMMON,    3, 99, 0, 0, 0, 0, 0 },
    { 16, "เจลลี่สไลม์",   "Slime Jelly",     IT_MATERIAL,   RA_COMMON,    4, 99, 0, 0, 0, 0, 0 },
    { 17, "คริสตัล",      "Crystal Shard",   IT_MATERIAL,   RA_RARE,     25, 99, 0, 0, 0, 0, 0 },
    { 18, "เกล็ดมังกร",    "Dragon Scale",    IT_MATERIAL,   RA_LEGENDARY,200,99, 0, 0, 0, 0, 0 },
    { 20, "ยาเล็ก",       "Small Potion",    IT_CONSUMABLE, RA_COMMON,    8, 20, 0, 0, 30, 0, 0 },
    { 21, "ยาเวทเล็ก",    "Small Mana Potion", IT_CONSUMABLE, RA_COMMON,  8, 20, 0, 0, 0, 25, 0 },
    { 22, "ขนมน่ารัก",    "Cute Treat",      IT_CONSUMABLE, RA_COMMON,    5, 20, 0, 0, 0, 0, 5 },
    { 30, "ของภารกิจ",    "Quest Token",     IT_QUEST,      RA_COMMON,    0, 99, 0, 0, 0, 0, 0 },
    { 40, "แหวนพลังงาน",   "Energy Ring",     IT_ACCESSORY,  RA_RARE,     80, 1, 2,  2, 0, 0, 0 },
};
#define DB_COUNT ((int)(sizeof(db) / sizeof(db[0])))

const ItemDef *item_def(uint16_t id)
{
    for (int i = 0; i < DB_COUNT; i++) if (db[i].id == id) return &db[i];
    return 0;
}

int item_count(const Inventory *inv, uint16_t id)
{
    int n = 0;
    for (int i = 0; i < INV_CAPACITY; i++) if (inv->slots[i].item == id) n += inv->slots[i].count;
    return n;
}

int inv_add(Inventory *inv, uint16_t id, int count)
{
    const ItemDef *d = item_def(id);
    if (!d || count <= 0) return 0;
    int added = 0;
    /* fill existing stacks first */
    for (int i = 0; i < INV_CAPACITY && count > 0; i++) {
        InvSlot *s = &inv->slots[i];
        if (s->item == id && s->count < d->stack_max) {
            int room = d->stack_max - s->count;
            int take = count < room ? count : room;
            s->count += take; count -= take; added += take;
        }
    }
    /* then empty slots */
    for (int i = 0; i < INV_CAPACITY && count > 0; i++) {
        InvSlot *s = &inv->slots[i];
        if (s->item == 0) {
            int take = count < d->stack_max ? count : d->stack_max;
            s->item = id; s->count = (uint16_t)take;
            count -= take; added += take;
        }
    }
    return added;
}

int inv_remove(Inventory *inv, uint16_t id, int count)
{
    int removed = 0;
    for (int i = INV_CAPACITY - 1; i >= 0 && count > 0; i--) {
        InvSlot *s = &inv->slots[i];
        if (s->item != id) continue;
        int take = s->count < count ? s->count : count;
        s->count -= take; count -= take; removed += take;
        if (s->count == 0) s->item = 0;
    }
    return removed;
}

int inv_split(Inventory *inv, int i, int n)
{
    if (i < 0 || i >= INV_CAPACITY) return 0;
    InvSlot *src = &inv->slots[i];
    if (src->item == 0 || n <= 0 || n >= src->count) return 0;
    for (int j = 0; j < INV_CAPACITY; j++) {
        if (inv->slots[j].item == 0) {
            inv->slots[j].item = src->item;
            inv->slots[j].count = (uint16_t)n;
            src->count -= (uint16_t)n;
            return 1;
        }
    }
    return 0;
}

static int type_key(uint16_t item) { const ItemDef *d = item_def(item); return d ? (int)d->type : 99; }

void inv_sort(Inventory *inv)
{
    /* stable insertion sort by type then id; empties go last */
    for (int i = 1; i < INV_CAPACITY; i++) {
        InvSlot cur = inv->slots[i];
        int j = i - 1;
        while (j >= 0) {
            InvSlot *p = &inv->slots[j];
            int pk = p->item ? type_key(p->item) : 1000;
            int ck = cur.item ? type_key(cur.item) : 1000;
            if (pk < ck || (pk == ck && p->item <= cur.item)) break;
            inv->slots[j + 1] = *p;
            j--;
        }
        inv->slots[j + 1] = cur;
    }
}

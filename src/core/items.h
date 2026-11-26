#ifndef ECLIPSE_ITEMS_H
#define ECLIPSE_ITEMS_H
#include <stdint.h>

#define INV_CAPACITY 40

typedef enum { IT_WEAPON, IT_ARMOR, IT_MATERIAL, IT_CONSUMABLE, IT_QUEST, IT_ACCESSORY } ItemType;
typedef enum { RA_COMMON, RA_UNCOMMON, RA_RARE, RA_EPIC, RA_LEGENDARY, RA_MYTHICAL } Rarity;
typedef enum { EQ_WEAPON, EQ_OFFHAND, EQ_HEAD, EQ_CHEST, EQ_GLOVES, EQ_BOOTS, EQ_ACCESSORY, EQ_COUNT } EquipSlot;

typedef struct {
    uint16_t id;
    const char *name_th, *name_en;
    ItemType type;
    Rarity rarity;
    uint16_t value;      /* sell price in gold */
    uint8_t  stack_max;
    int16_t  atk, def;   /* equipment bonus */
    uint8_t  heal_hp, heal_mp, pet_food;
} ItemDef;

typedef struct { uint16_t item; uint16_t count; } InvSlot;
typedef struct { InvSlot slots[INV_CAPACITY]; } Inventory;

const ItemDef *item_def(uint16_t id);
int   item_count(const Inventory *inv, uint16_t id);
/* Returns items actually added (0 if no room). */
int   inv_add(Inventory *inv, uint16_t id, int count);
/* Returns items actually removed. */
int   inv_remove(Inventory *inv, uint16_t id, int count);
/* Moves n items out of slot i into a new free slot. Returns 1 on success. */
int   inv_split(Inventory *inv, int i, int n);
void  inv_sort(Inventory *inv);
#endif

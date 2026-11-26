#include "pet.h"

/* 0 Baby Dragon (mount), 1 Fairy Cat, 2 Cloud Bunny (mount), 3 Mini Fox, 4 Mushroom Spirit, 5 Little Phoenix (mount) */
const PetDef pet_defs[PET_COUNT] = {
    { "ลูกมังกร",        "Baby Dragon",     1, 0 },
    { "แมวนางฟ้า",       "Fairy Cat",       0, 0 },
    { "กระต่ายเมฆ",      "Cloud Bunny",     1, 0 },
    { "จิ้งจอกน้อย",     "Mini Fox",        0, 1 },
    { "วิญญาณเห็ด",       "Mushroom Spirit", 0, 1 },
    { "ฟินิกซ์น้อย",     "Little Phoenix",  1, 0 },
};

int pet_feed(PetState *p, int food_value)
{
    if (!p->owned) return 0;
    p->friendship = (uint16_t)(p->friendship + food_value > 1000 ? 1000 : p->friendship + food_value);
    p->exp = (uint16_t)(p->exp + food_value);
    int need = 20 + p->level * 10;
    if (p->exp >= need && p->level < 20) {
        p->exp = (uint16_t)(p->exp - need);
        p->level++;
        return 1;
    }
    return 0;
}

int pet_can_mount(const PetState *p, int pet_id)
{
    return p->owned && pet_id >= 0 && pet_id < PET_COUNT && pet_defs[pet_id].can_mount && p->friendship >= 300;
}

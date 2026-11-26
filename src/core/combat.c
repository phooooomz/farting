#include "combat.h"

/* Monster ids: 1 Slime, 2 Pudding King (boss), 3 Bunny, 4 Puffshroom, 5 Tiny Drake, 6 Pebble Golem */
const MonsterDef monster_defs[] = {
    { 1, "สไลม์พุดดิ้ง", "Pudding Slime", 1,  30,  5, 1,  12, 4,  0, {16, 0}, {60, 0} },
    { 2, "ราชาพุดดิ้ง",   "Pudding King",  8, 900, 22, 6, 800, 500, 1, {18, 17}, {100, 50} },
    { 3, "กระต่ายปุย",    "Fluffy Bunny",  2,  40,  6, 2,  18, 6,  0, {14, 0}, {40, 0} },
    { 4, "เห็ดปุยฝ้าย",   "Puffshroom",    3,  50,  7, 2,  25, 8,  0, {14, 15}, {50, 30} },
    { 5, "มังกรตัวน้อย",   "Tiny Drake",    6, 160, 14, 5, 90, 40, 0, {18, 17}, {5, 10} },
    { 6, "ก้อนหินโกเลม",   "Pebble Golem",  5, 150, 12, 9, 70, 30, 0, {11, 17}, {80, 15} },
};
const int monster_count = (int)(sizeof(monster_defs) / sizeof(monster_defs[0]));

const MonsterDef *monster_def(uint16_t id)
{
    for (int i = 0; i < monster_count; i++) if (monster_defs[i].id == id) return &monster_defs[i];
    return 0;
}

HitResult combat_hit(int atk, int crit_pct, int def)
{
    HitResult r;
    int base = atk - def / 2;
    if (base < 1) base = 1;
    r.crit = rng_range(1, 100) <= crit_pct;
    r.damage = r.crit ? base * 2 : base;
    return r;
}

int boss_phase(int hp, int max_hp)
{
    if (max_hp <= 0) return 1;
    int pct = hp * 100 / max_hp;
    if (pct > 66) return 1;
    if (pct > 33) return 2;
    return 3;
}

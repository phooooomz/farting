#ifndef ECLIPSE_COMBAT_H
#define ECLIPSE_COMBAT_H
#include <stdint.h>
#include "rng.h"

typedef enum { STATUS_NONE, STATUS_SLEEP, STATUS_SLOW, STATUS_STUN } Status;

typedef struct {
    uint16_t id;
    const char *name_th, *name_en;
    int level, hp, atk, def;
    uint32_t exp;
    uint16_t gold;
    uint8_t  is_boss;
    uint16_t drop_item[2];
    uint8_t  drop_pct[2];
} MonsterDef;

extern const MonsterDef monster_defs[];
extern const int        monster_count;
const MonsterDef *monster_def(uint16_t id);

typedef struct {
    int damage;
    int crit;
} HitResult;

/* Damage = attack - defense/2, minimum 1, with crit doubling. */
HitResult combat_hit(int atk, int crit_pct, int def);
/* Boss phases: 3 phases by HP fraction. Returns 1..3. */
int  boss_phase(int hp, int max_hp);
#endif

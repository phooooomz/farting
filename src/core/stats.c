#include "stats.h"

uint32_t exp_to_next(int level)
{
    if (level >= MAX_LEVEL) return 0;
    return (uint32_t)(50 * level * level + 50 * level);
}

void stats_init(PlayerStats *p)
{
    p->level = 1; p->exp = 0;
    p->str = 5; p->vit = 5; p->agi = 5; p->intel = 5; p->luk = 5;
    p->stat_points = 0; p->skill_points = 0;
    p->hp = stats_max_hp(p);
    p->mp = stats_max_mp(p);
    p->stamina = 100;
}

int stats_max_hp(const PlayerStats *p) { return 80 + p->vit * 10 + p->level * 12; }
int stats_max_mp(const PlayerStats *p) { return 30 + p->intel * 6 + p->level * 4; }
int stats_atk(const PlayerStats *p, int weapon_atk) { return p->str * 2 + p->luk / 2 + weapon_atk; }
int stats_def(const PlayerStats *p, int armor_def) { return p->vit + p->agi / 2 + armor_def; }

int stats_crit_pct(const PlayerStats *p)
{
    int c = p->luk / 2 + p->agi / 4;
    return c > 60 ? 60 : c;
}

int stats_gain_exp(PlayerStats *p, uint32_t amount)
{
    int gained = 0;
    p->exp += (int)amount;
    while (p->level < MAX_LEVEL && (uint32_t)p->exp >= exp_to_next(p->level)) {
        p->exp -= (int)exp_to_next(p->level);
        p->level++;
        p->stat_points += 3;
        p->skill_points += 1;
        p->hp = stats_max_hp(p);
        p->mp = stats_max_mp(p);
        gained++;
    }
    if (p->level >= MAX_LEVEL) p->exp = 0;
    return gained;
}

int stats_spend_point(PlayerStats *p, int which)
{
    if (p->stat_points <= 0) return 0;
    switch (which) {
    case 0: p->str++; break;
    case 1: p->vit++; break;
    case 2: p->agi++; break;
    case 3: p->intel++; break;
    case 4: p->luk++; break;
    default: return 0;
    }
    p->stat_points--;
    return 1;
}

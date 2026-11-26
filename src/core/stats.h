#ifndef ECLIPSE_STATS_H
#define ECLIPSE_STATS_H
#include <stdint.h>

#define MAX_LEVEL 100

typedef struct {
    int level, exp;
    int str, vit, agi, intel, luk;   /* base stats */
    int stat_points, skill_points;
    int hp, mp;                      /* current */
    int stamina;                     /* current, 0..100 */
} PlayerStats;

uint32_t exp_to_next(int level);
void     stats_init(PlayerStats *p);
int      stats_max_hp(const PlayerStats *p);
int      stats_max_mp(const PlayerStats *p);
int      stats_atk(const PlayerStats *p, int weapon_atk);
int      stats_def(const PlayerStats *p, int armor_def);
int      stats_crit_pct(const PlayerStats *p);
/* Adds EXP, levels up as needed. Returns number of levels gained. */
int      stats_gain_exp(PlayerStats *p, uint32_t amount);
/* Spends one stat point on a stat index: 0 STR,1 VIT,2 AGI,3 INT,4 LUK. */
int      stats_spend_point(PlayerStats *p, int which);
#endif

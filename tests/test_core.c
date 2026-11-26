/* Host unit tests for portable game logic (src/core). Build: see tests/run_tests.sh */
#include <stdio.h>
#include <string.h>
#include "stats.h"
#include "items.h"
#include "crafting.h"
#include "quest.h"
#include "combat.h"
#include "world.h"
#include "pet.h"
#include "save.h"
#include "gamestate.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

static void test_stats(void)
{
    PlayerStats p; stats_init(&p);
    CHECK(p.level == 1);
    CHECK(stats_gain_exp(&p, exp_to_next(1)) == 1);     /* exactly one level */
    CHECK(p.level == 2 && p.exp == 0);
    CHECK(p.stat_points == 3 && p.skill_points == 1);
    CHECK(stats_gain_exp(&p, 100000000u) > 0);
    CHECK(p.level == MAX_LEVEL);                        /* capped at 100 */
    int before = p.str;
    CHECK(stats_spend_point(&p, 0) == 1);
    CHECK(p.str == before + 1);
    before = p.str;
    CHECK(stats_spend_point(&p, 9) == 0);               /* invalid index */
    CHECK(p.str == before);
}

static void test_inventory(void)
{
    Inventory inv; memset(&inv, 0, sizeof(inv));
    CHECK(inv_add(&inv, 10, 150) == 150);               /* wood stacks 99 -> 2 slots */
    CHECK(item_count(&inv, 10) == 150);
    CHECK(inv_remove(&inv, 10, 50) == 50);
    CHECK(item_count(&inv, 10) == 100);
    CHECK(inv_split(&inv, 0, 10) == 1);
    CHECK(item_count(&inv, 10) == 100);
    inv_sort(&inv);
    CHECK(item_count(&inv, 10) == 100);
    CHECK(inv_add(&inv, 999, 1) == 0);                  /* unknown item */
}

static void test_crafting(void)
{
    Inventory inv; memset(&inv, 0, sizeof(inv));
    int gold = 100;
    const Recipe *r = &recipes[0];                      /* ดาบไม้: 2 ไม้ + 1 หิน */
    CHECK(!recipe_can_craft(r, &inv, gold));
    inv_add(&inv, 10, 2); inv_add(&inv, 11, 1);
    CHECK(recipe_craft(r, &inv, &gold) == 1);
    CHECK(item_count(&inv, 1) == 1 && gold == 95);
    CHECK(item_count(&inv, 10) == 0 && item_count(&inv, 11) == 0);
}

static void test_quests(void)
{
    QuestState st[QUEST_COUNT]; memset(st, 0, sizeof(st));
    CHECK(!quest_accept(st, 2));                        /* requires quest 1 */
    CHECK(quest_accept(st, 1));
    quest_set_progress(st, 1, 3);
    CHECK(quest_check_complete(st, 1) == 1);
    CHECK(quest_accept(st, 2));
    quest_refresh_gather(st, 10, 3);
    CHECK(quest_check_complete(st, 2) == 0);
    quest_refresh_gather(st, 10, 9);
    CHECK(quest_check_complete(st, 2) == 1);
    CHECK(st[1].status == QS_DONE);
}

static void test_combat(void)
{
    HitResult h = combat_hit(10, 0, 4);
    CHECK(h.damage == 8 && !h.crit);
    h = combat_hit(1, 0, 100);
    CHECK(h.damage == 1);                               /* minimum damage */
    CHECK(boss_phase(900, 900) == 1);
    CHECK(boss_phase(500, 900) == 2);
    CHECK(boss_phase(100, 900) == 3);
    CHECK(monster_def(1) != 0 && monster_def(99) == 0);
}

static void test_world_pet(void)
{
    WorldState w; world_init(&w);
    CHECK(w.region_unlocked[REG_BLOOMVALE] && !w.region_unlocked[REG_STARCLOUD_KINGDOM]);
    CHECK(world_harvest(&w, 0) == 10);
    CHECK(world_harvest(&w, 0) == 0);                   /* still respawning */
    world_tick(&w, 60);
    CHECK(world_harvest(&w, 0) == 10);

    PetState p; memset(&p, 0, sizeof(p));
    CHECK(pet_feed(&p, 10) == 0);                       /* not owned */
    p.owned = 1;
    CHECK(!pet_can_mount(&p, 0));
    p.friendship = 300;
    CHECK(pet_can_mount(&p, 0));
    CHECK(!pet_can_mount(&p, 1));                       /* Fairy Cat cannot mount */
}

static void test_save(void)
{
    GameState a, b; game_new(&a);
    a.gold = 1234;
    a.stats.level = 42;
    uint8_t buf[sizeof(GameState) + 64];
    size_t n = save_pack(&a, buf, sizeof(buf));
    CHECK(n == save_size());
    CHECK(save_unpack(&b, buf, n));
    CHECK(b.gold == 1234 && b.stats.level == 42);
    buf[100] ^= 0xFF;                                   /* corrupt payload */
    CHECK(!save_unpack(&b, buf, n));
    buf[100] ^= 0xFF; buf[0] ^= 0x01;                   /* bad magic */
    CHECK(!save_unpack(&b, buf, n));
}

int main(void)
{
    test_stats();
    test_inventory();
    test_crafting();
    test_quests();
    test_combat();
    test_world_pet();
    test_save();
    if (failures == 0) printf("ALL TESTS PASSED\n");
    else printf("%d FAILED\n", failures);
    return failures ? 1 : 0;
}

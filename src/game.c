#include <math.h>
#include <pspctrl.h>
#include "game.h"
#include "gfx.h"
#include "font.h"
#include "locale.h"
#include "rng.h"
#include "combat.h"
#include "quest.h"
#include "crafting.h"
#include "pet.h"
#include "world.h"
#include "savefile.h"

#define DT          (1.0f / 30.0f)
#define MAX_MOBS    12
#define PICK_RANGE  3.2f
#define NPC_RANGE   3.0f
#define MAX_TOAST_T 2.5f

/* ---- colours ---- */
#define C_GROUND_A RGBA(0xCF, 0xEB, 0xC8, 0xFF)
#define C_GROUND_B RGBA(0xBE, 0xE0, 0xB6, 0xFF)
#define C_PLAYER   RGBA(0xFF, 0x9E, 0xCB, 0xFF)
#define C_HAT      RGBA(0x8E, 0x6F, 0xD8, 0xFF)
#define C_SLIME    RGBA(0x9E, 0xE8, 0xA2, 0xFF)
#define C_BUNNY    RGBA(0xFF, 0xF2, 0xC2, 0xFF)
#define C_BOSS     RGBA(0xFF, 0xD0, 0x7A, 0xFF)
#define C_NPC      RGBA(0x9F, 0xC7, 0xFF, 0xFF)
#define C_TABLE    RGBA(0xC9, 0x9A, 0x6B, 0xFF)
#define C_CAVE     RGBA(0x6B, 0x5C, 0x9A, 0xFF)
#define C_PORTAL   RGBA(0xB8, 0xF0, 0xFF, 0xFF)
#define C_PET      RGBA(0xFF, 0xE0, 0xA8, 0xFF)
#define C_TEXT     RGBA(0x4A, 0x3B, 0x5C, 0xFF)
#define C_HINT     RGBA(0x6B, 0x5C, 0x8A, 0xFF)
#define C_PANEL    RGBA(0xFF, 0xF8, 0xFC, 0xE8)
#define C_SELECT   RGBA(0xFF, 0xC8, 0xE4, 0xFF)
#define C_HP       RGBA(0xFF, 0x7F, 0xA8, 0xFF)
#define C_MP       RGBA(0x7F, 0xB8, 0xFF, 0xFF)
#define C_BAR_BG   RGBA(0xFF, 0xFF, 0xFF, 0xB0)

/* ---- world content (Bloomvale = region 0, Whispering Woods = region 1) ---- */
typedef struct { uint8_t def; int16_t x, z; } Spawn;
static const Spawn spawns_r0[] = { {1, 20, 20}, {1, -30, 10}, {1, 40, -20}, {3, -15, -30}, {3, 50, 40}, {1, -60, -40} };
static const Spawn spawns_r1[] = { {4, 70, 70}, {4, 90, 20}, {5, 100, -40} };
#define BOSS_X 0
#define BOSS_Z (-220)
#define CAVE_X (-120)
#define CAVE_Z (-120)
#define QG_X   (-8)
#define QG_Z   (-8)
#define MR_X   8
#define MR_Z   (-8)
#define TB_X   (-8)
#define TB_Z   8

typedef struct {
    uint8_t def; float x, z, hx, hz;
    int hp, alive;
    float respawn, atk_cd, wander_t, wx, wz;
} Mob;

static Mob mobs[MAX_MOBS];
static int mob_count;
static float px, pz, py, vy;        /* player position (world units), height, vertical speed */
static float yaw = 0.0f, cam_dist = 14.0f;
static float atk_cd, dodge_t, skill_cd, hurt_t;
static int cave_entered;
static StrId toast_id = -1;
static float toast_t;
static int dead_flag;
static int rng_seeded;

/* ---- menu state ---- */
static int menu_tab, menu_cursor, menu_stat_sel;
static int menu_open;

static void toast(StrId id) { toast_id = id; toast_t = MAX_TOAST_T; }

static void spawn_mob(int slot, uint8_t def, float x, float z)
{
    Mob *m = &mobs[slot];
    const MonsterDef *d = monster_def(def);
    m->def = def; m->x = m->hx = x; m->z = m->hz = z;
    m->wx = x; m->wz = z;
    m->hp = d ? d->hp : 10; m->alive = 1; m->respawn = 0; m->atk_cd = 0; m->wander_t = 0;
}

static void load_region_mobs(int region)
{
    mob_count = 0;
    if (region == REG_BLOOMVALE) {
        for (unsigned i = 0; i < sizeof(spawns_r0) / sizeof(spawns_r0[0]) && mob_count < MAX_MOBS; i++, mob_count++)
            spawn_mob(mob_count, spawns_r0[i].def, spawns_r0[i].x, spawns_r0[i].z);
        spawn_mob(mob_count, 2, BOSS_X, BOSS_Z); mob_count++;   /* boss: visible only while quest 6 is active */
    } else if (region == REG_WHISPERING_WOODS) {
        for (unsigned i = 0; i < sizeof(spawns_r1) / sizeof(spawns_r1[0]) && mob_count < MAX_MOBS; i++, mob_count++)
            spawn_mob(mob_count, spawns_r1[i].def, spawns_r1[i].x, spawns_r1[i].z);
    }
}

void game_begin(GameState *g)
{
    if (!rng_seeded) { rng_seed(0x9E3779B9u ^ g->play_seconds); rng_seeded = 1; }
    px = g->pos_x / 16.0f;
    pz = g->pos_z / 16.0f;
    py = 0; vy = 0;
    atk_cd = dodge_t = skill_cd = hurt_t = 0;
    toast_id = -1; toast_t = 0;
    dead_flag = 0;
    menu_open = 0; menu_tab = 0; menu_cursor = 0; menu_stat_sel = 0;
    load_region_mobs(g->region);
    /* keep quest 6 boss in sync with its quest */
    if (g->quests[5].status == QS_ACTIVE) mobs[mob_count - 1].hp = monster_def(2)->hp;
}

void game_menu_reset(void) { menu_open = 0; menu_tab = 0; menu_cursor = 0; menu_stat_sel = 0; }

/* ---- quest / reward helpers ---- */
static void grant_pet(GameState *g, int pet_id)
{
    if (pet_id >= 0 && pet_id < PET_COUNT && !g->pets[pet_id].owned) {
        g->pets[pet_id].owned = 1;
        g->pets[pet_id].level = 1;
        g->pets[pet_id].friendship = 100;
    }
}

static void check_quests(GameState *g)
{
    for (uint16_t id = 1; id <= QUEST_COUNT; id++) {
        if (!quest_check_complete(g->quests, id)) continue;
        const QuestDef *d = quest_def(id);
        if (stats_gain_exp(&g->stats, d->exp)) toast(LEVEL_UP); else toast(QST_COMPLETE);
        g->gold += d->gold;
        if (d->reward_item) inv_add(&g->inv, d->reward_item, d->reward_count);
        if (id == 3) world_unlock_region(&g->world, REG_WHISPERING_WOODS);
        if (id == 2) grant_pet(g, 1);     /* Fairy Cat */
        if (id == 5) grant_pet(g, 0);     /* Baby Dragon */
        if (id == 6) grant_pet(g, 5);     /* Little Phoenix */
    }
}

static void refresh_gather_quests(GameState *g)
{
    for (int i = 0; i < QUEST_COUNT; i++) {
        const QuestDef *d = &quest_defs[i];
        if (d->type == QT_GATHER) quest_refresh_gather(g->quests, d->target, item_count(&g->inv, d->target));
    }
    check_quests(g);
}

static uint16_t equip_atk(const GameState *g)
{
    const ItemDef *d = item_def(g->equip[EQ_WEAPON]);
    return d ? (uint16_t)d->atk : 0;
}

static int equip_def(const GameState *g)
{
    int s = 0;
    for (int i = 0; i < EQ_COUNT; i++) {
        const ItemDef *d = item_def(g->equip[i]);
        if (d) s += d->def;
    }
    return s;
}

static void player_respawn(GameState *g)
{
    px = 0; pz = 0; py = 0; vy = 0;
    g->stats.hp = stats_max_hp(&g->stats) / 2;
    g->stats.mp = stats_max_mp(&g->stats);
    g->region = REG_BLOOMVALE;
    load_region_mobs(g->region);
    toast(PLAYER_DEFEATED);
}

/* ---- world simulation ---- */
static float dist2(float ax, float az, float bx, float bz)
{
    float dx = ax - bx, dz = az - bz;
    return sqrtf(dx * dx + dz * dz);
}

static void kill_mob(GameState *g, Mob *m)
{
    const MonsterDef *d = monster_def(m->def);
    m->alive = 0;
    m->hp = 0;
    m->respawn = (d && d->is_boss) ? 0 : 30.0f;
    if (!d) return;
    stats_gain_exp(&g->stats, d->exp);
    g->gold += d->gold;
    for (int k = 0; k < 2; k++) {
        if (d->drop_item[k] && rng_range(1, 100) <= d->drop_pct[k]) inv_add(&g->inv, d->drop_item[k], 1);
    }
    quest_on_kill(g->quests, m->def);
    if (d->is_boss) quest_set_progress(g->quests, 6, 1);
    toast(DEFEAT_MONSTER);
    check_quests(g);
}

static void update_mobs(GameState *g)
{
    float ppx = px, ppz = pz;
    for (int i = 0; i < mob_count; i++) {
        Mob *m = &mobs[i];
        const MonsterDef *d = monster_def(m->def);
        if (!d) continue;
        if (d->is_boss) {
            /* boss only exists while quest 6 is active and it still has HP */
            m->alive = (g->quests[5].status == QS_ACTIVE && m->hp > 0);
            if (!m->alive) continue;
        }
        if (!m->alive) {
            if (m->respawn > 0) {
                m->respawn -= DT;
                if (m->respawn <= 0) { m->alive = 1; m->hp = d->hp; m->x = m->hx; m->z = m->hz; }
            }
            continue;
        }
        float dd = dist2(m->x, m->z, ppx, ppz);
        m->atk_cd -= DT;
        if (dd < 10.0f && dd > 1.4f) {
            float sp = 2.2f * DT;
            m->x += (ppx - m->x) / dd * sp;
            m->z += (ppz - m->z) / dd * sp;
        } else if (dd <= 1.4f && m->atk_cd <= 0) {
            m->atk_cd = 1.2f;
            int phase = d->is_boss ? boss_phase(m->hp, d->hp) : 1;
            int atk = d->atk * (phase == 1 ? 10 : phase == 2 ? 12 : 15) / 10;
            HitResult h = combat_hit(atk, 0, stats_def(&g->stats, equip_def(g)));
            int dmg = dodge_t > 0 ? 0 : h.damage;
            g->stats.hp -= dmg;
            hurt_t = 0.2f;
            if (g->stats.hp <= 0) { player_respawn(g); return; }
        } else {
            m->wander_t -= DT;
            if (m->wander_t <= 0) {
                m->wander_t = 2.0f;
                m->wx = m->hx + (float)rng_range(-60, 60) / 10.0f;
                m->wz = m->hz + (float)rng_range(-60, 60) / 10.0f;
            }
            float ex = m->wx - m->x, ez = m->wz - m->z;
            float el = sqrtf(ex * ex + ez * ez);
            if (el > 0.1f) { m->x += ex / el * 0.8f * DT; m->z += ez / el * 0.8f * DT; }
        }
        if (dist2(m->x, m->z, m->hx, m->hz) > 25.0f) { m->x = m->hx; m->z = m->hz; }
    }
}

static int nearest_mob(float range)
{
    int best = -1; float bd = range;
    for (int i = 0; i < mob_count; i++) {
        if (!mobs[i].alive) continue;
        float d = dist2(mobs[i].x, mobs[i].z, px, pz);
        if (d < bd) { bd = d; best = i; }
    }
    return best;
}

static int nearest_node(const GameState *g, float range)
{
    int best = -1; float bd = range;
    for (int i = 0; i < g->world.node_count; i++) {
        const ResourceNode *n = &g->world.nodes[i];
        if (n->region != g->region) continue;
        float d = dist2(n->x, n->z, px, pz);
        if (d < bd) { bd = d; best = i; }
    }
    return best;
}

static void do_talk_quest_giver(GameState *g)
{
    for (uint16_t id = 1; id <= QUEST_COUNT; id++) {
        if (g->quests[id - 1].status == QS_ACTIVE && quest_check_complete(g->quests, id)) {
            check_quests(g);
            return;
        }
    }
    /* quest 1 completes on the second talk */
    if (g->quests[0].status == QS_ACTIVE) { quest_set_progress(g->quests, 1, 1); toast(NPC_QUEST_TALK_2); check_quests(g); return; }
    for (uint16_t id = 1; id <= QUEST_COUNT; id++) {
        if (quest_accept(g->quests, id)) {
            if (id == 6) mobs[mob_count - 1].hp = monster_def(2)->hp;
            if (id == 1) quest_set_progress(g->quests, 1, 0);
            toast(QST_ACCEPTED);
            return;
        }
    }
    toast(QST_NO_QUEST);
}

static void do_interact(GameState *g)
{
    float dmerch = dist2(px, pz, MR_X, MR_Z);
    float dqg    = dist2(px, pz, QG_X, QG_Z);
    float dtb    = dist2(px, pz, TB_X, TB_Z);
    float dcave  = dist2(px, pz, CAVE_X, CAVE_Z);

    int mi = nearest_mob(PICK_RANGE);
    if (mi >= 0) {
        Mob *m = &mobs[mi];
        const MonsterDef *d = monster_def(m->def);
        HitResult h = combat_hit(stats_atk(&g->stats, equip_atk(g)), stats_crit_pct(&g->stats), d ? d->def : 0);
        m->hp -= h.damage;
        m->atk_cd = (m->atk_cd > 0.4f) ? m->atk_cd : 0.4f;
        if (m->hp <= 0) kill_mob(g, m);
        atk_cd = 0.4f;
        return;
    }
    if (dqg < NPC_RANGE) { do_talk_quest_giver(g); return; }
    if (dmerch < NPC_RANGE) {
        if (g->gold >= 8 && inv_add(&g->inv, 20, 1) == 1) { g->gold -= 8; toast(SHOP_BOUGHT); }
        else toast(SHOP_NO_GOLD);
        return;
    }
    if (dtb < NPC_RANGE) { menu_open = 1; menu_tab = 3; menu_cursor = 0; return; }
    if (dcave < NPC_RANGE && !cave_entered) {
        cave_entered = 1;
        quest_set_progress(g->quests, 5, 1);
        check_quests(g);
        toast(DUNGEON_ENTER);
        return;
    }
    int ni = nearest_node(g, PICK_RANGE);
    if (ni >= 0) {
        uint16_t item = world_harvest(&g->world, ni);
        if (item) {
            int amount = rng_range(1, 2);
            inv_add(&g->inv, item, amount);
            refresh_gather_quests(g);
            toast(HARVEST_OK);
        } else {
            toast(HARVEST_WAIT);
        }
    }
}

static void portal_check(GameState *g)
{
    if (g->region == REG_BLOOMVALE && px > 180.0f) {
        if (g->world.region_unlocked[REG_WHISPERING_WOODS]) {
            g->region = REG_WHISPERING_WOODS;
            px = -160.0f; pz = 0;
            load_region_mobs(g->region);
        } else {
            px = 170.0f;
            toast(REGION_LOCKED);
        }
    } else if (g->region == REG_WHISPERING_WOODS && px < -180.0f) {
        g->region = REG_BLOOMVALE;
        px = 170.0f; pz = 0;
        load_region_mobs(g->region);
    }
}

void game_update(GameState *g, const InputState *in)
{
    if (atk_cd > 0) atk_cd -= DT;
    if (dodge_t > 0) dodge_t -= DT;
    if (skill_cd > 0) skill_cd -= DT;
    if (hurt_t > 0) hurt_t -= DT;
    if (toast_t > 0) { toast_t -= DT; if (toast_t <= 0) toast_id = -1; }

    /* camera: D-pad left/right rotates, up/down zooms */
    if (in->buttons & PSP_CTRL_LEFT)  yaw -= 0.04f;
    if (in->buttons & PSP_CTRL_RIGHT) yaw += 0.04f;
    if (in->buttons & PSP_CTRL_UP)    cam_dist -= 0.3f;
    if (in->buttons & PSP_CTRL_DOWN)  cam_dist += 0.3f;
    if (cam_dist < 6.0f) cam_dist = 6.0f;
    if (cam_dist > 24.0f) cam_dist = 24.0f;

    /* move relative to camera */
    float ax = (in->lx - 128) / 127.0f, az = (in->ly - 128) / 127.0f;
    if (fabsf(ax) < 0.15f) ax = 0;
    if (fabsf(az) < 0.15f) az = 0;
    float speed = 0.22f;
    if (g->mounted) speed = 0.4f;
    if ((in->buttons & PSP_CTRL_RTRIGGER) && g->stats.stamina > 0 && !g->mounted) {
        speed *= 1.8f;
        g->stats.stamina -= 1;
    } else if (g->stats.stamina < 100 && (g->play_seconds & 1) == 0) {
        g->stats.stamina++;
    }
    if (dodge_t > 0) speed = 0.6f;
    /* stick up (az < 0) = forward; camera-relative. World dir: right=(cos,sin), forward=(-sin,cos). */
    float fwd = -az;
    float cy = cosf(yaw), sy = sinf(yaw);
    px += (ax * cy + fwd * -sy) * speed;
    pz += (ax * sy + fwd * cy) * speed;

    /* jump */
    if ((in->pressed & PSP_CTRL_TRIANGLE) && py <= 0.01f && !dodge_t) vy = 0.45f;
    py += vy;
    vy -= 0.06f;
    if (py < 0) { py = 0; vy = 0; }

    /* dodge */
    if ((in->pressed & PSP_CTRL_SQUARE) && dodge_t <= 0 && g->stats.stamina >= 10) {
        dodge_t = 0.35f; g->stats.stamina -= 10;
    }
    /* attack / interact */
    if ((in->pressed & PSP_CTRL_CROSS) && atk_cd <= 0) do_interact(g);

    /* magic skill: Bubble Blast */
    if ((in->pressed & PSP_CTRL_CIRCLE) && skill_cd <= 0 && g->stats.mp >= 10) {
        g->stats.mp -= 10;
        skill_cd = 1.5f;
        for (int i = 0; i < mob_count; i++) {
            if (!mobs[i].alive) continue;
            if (dist2(mobs[i].x, mobs[i].z, px, pz) > 5.0f) continue;
            const MonsterDef *d = monster_def(mobs[i].def);
            HitResult h = combat_hit(g->stats.intel * 2 + 8, stats_crit_pct(&g->stats), d ? d->def : 0);
            mobs[i].hp -= h.damage;
            if (mobs[i].hp <= 0) kill_mob(g, &mobs[i]);
        }
    }

    /* pet follows; mount toggle */
    if ((in->pressed & PSP_CTRL_SELECT) && g->current_pet < PET_COUNT && pet_can_mount(&g->pets[g->current_pet], g->current_pet)) {
        g->mounted = !g->mounted;
        toast(g->mounted ? PET_MOUNT_ON : PET_MOUNT_OFF);
    }

    update_mobs(g);
    portal_check(g);

    /* clamp to world bounds */
    if (px > 230) px = 230;
    if (px < -230) px = -230;
    if (pz > 230) pz = 230;
    if (pz < -230) pz = -230;

    g->pos_x = (int32_t)(px * 16.0f);
    g->pos_z = (int32_t)(pz * 16.0f);

    static int tick = 0;
    if (++tick >= 30) { tick = 0; g->play_seconds++; world_tick(&g->world, 1); }
}

/* ---- rendering ---- */
uint32_t game_sky_color(const GameState *g)
{
    float t = (float)(g->play_seconds % 240) / 240.0f;          /* 0..1 day cycle */
    float d = 0.5f - 0.5f * cosf(t * 6.2831853f);     /* 0 = midday, 1 = midnight */
    uint32_t r = (uint32_t)(0xE8 - (0xE8 - 0x2A) * d * 0.8f);
    uint32_t gc = (uint32_t)(0xF4 - (0xF4 - 0x2E) * d * 0.8f);
    uint32_t b = (uint32_t)(0xFF - (0xFF - 0x52) * d * 0.8f);
    return RGBA(r, gc, b, 0xFF);
}

void game_draw(const GameState *g)
{
    gfx_begin_3d(px, py, pz, yaw, 0.55f, cam_dist);
    gfx_draw_ground(px, pz, C_GROUND_A, C_GROUND_B);

    /* resource nodes in this region */
    for (int i = 0; i < g->world.node_count; i++) {
        const ResourceNode *n = &g->world.nodes[i];
        if (n->region != g->region) continue;
        uint32_t c = n->timer ? RGBA(0xA8, 0x9C, 0x8E, 0xFF) : RGBA(0x8F, 0xD0, 0x7A, 0xFF);
        Vec3 p = { n->x, 0, n->z };
        Vec3 s = { 1.6f, n->type == NODE_TREE ? 3.0f : 1.2f, 1.6f };
        gfx_draw_box_shaded(p, s, c);
    }

    if (g->region == REG_BLOOMVALE) {
        Vec3 qg = { QG_X, 0, QG_Z }, mr = { MR_X, 0, MR_Z }, tb = { TB_X, 0, TB_Z }, cv = { CAVE_X, 0, CAVE_Z };
        Vec3 bo = { BOSS_X, 0, BOSS_Z };
        gfx_draw_box_shaded(qg, (Vec3){ 1.4f, 2.0f, 1.4f }, C_NPC);
        gfx_draw_box_shaded(mr, (Vec3){ 1.4f, 2.0f, 1.4f }, C_NPC);
        gfx_draw_box_shaded(tb, (Vec3){ 2.2f, 1.0f, 1.4f }, C_TABLE);
        gfx_draw_box_shaded(cv, (Vec3){ 3.5f, 2.5f, 3.5f }, C_CAVE);
        gfx_draw_box_shaded(bo, (Vec3){ 14.0f, 0.4f, 14.0f }, RGBA(0xF4, 0xE0, 0xFF, 0xFF));
    }
    if (g->region == REG_BLOOMVALE) {
        Vec3 portal = { 200, 0, 0 };
        gfx_draw_box_shaded(portal, (Vec3){ 2.0f, 6.0f, 20.0f }, C_PORTAL);
    } else {
        Vec3 portal = { -200, 0, 0 };
        gfx_draw_box_shaded(portal, (Vec3){ 2.0f, 6.0f, 20.0f }, C_PORTAL);
    }

    for (int i = 0; i < mob_count; i++) {
        const Mob *m = &mobs[i];
        if (!m->alive) continue;
        const MonsterDef *d = monster_def(m->def);
        uint32_t c = (d && d->is_boss) ? C_BOSS : (m->def == 3 ? C_BUNNY : (m->def == 4 ? RGBA(0xFF, 0xD6, 0xE0, 0xFF) : C_SLIME));
        float s = d && d->is_boss ? 10.0f : 1.4f;
        Vec3 p = { m->x, 0, m->z };
        gfx_draw_box_shaded(p, (Vec3){ s, s * 0.8f, s }, c);
    }

    /* pet */
    if (g->current_pet < PET_COUNT && g->pets[g->current_pet].owned) {
        Vec3 pp = { px - 1.8f, 0.3f, pz - 1.2f };
        gfx_draw_box_shaded(pp, (Vec3){ 0.9f, 0.9f, 0.9f }, C_PET);
    }

    /* player (mount makes the body longer) */
    Vec3 pp = { px, py, pz };
    float body = g->mounted ? 1.2f : 1.6f;
    if (hurt_t > 0) gfx_draw_box_shaded(pp, (Vec3){ 1.0f, body, 1.0f }, RGBA(0xFF, 0x50, 0x50, 0xFF));
    else            gfx_draw_box_shaded(pp, (Vec3){ 1.0f, body, 1.0f }, C_PLAYER);
    Vec3 hat = { px, py + body, pz };
    gfx_draw_box_shaded(hat, (Vec3){ 1.1f, 0.4f, 1.1f }, C_HAT);

    gfx_end_3d();

    /* HUD */
    gfx_fill_rect(8, 8, 124, 8, C_BAR_BG);
    int hpw = 124 * g->stats.hp / (stats_max_hp(&g->stats) ? stats_max_hp(&g->stats) : 1);
    gfx_fill_rect(8, 8, hpw, 8, C_HP);
    gfx_fill_rect(8, 20, 124, 6, C_BAR_BG);
    int mpw = 124 * g->stats.mp / (stats_max_mp(&g->stats) ? stats_max_mp(&g->stats) : 1);
    gfx_fill_rect(8, 20, mpw, 6, C_MP);
    gfx_text(8, 38, loc_str(STAT_LV), C_TEXT);
    char lv[24] = {0};
    int li = 0; int lvv = g->stats.level;
    if (lvv >= 100) { lv[li++] = '1'; lv[li++] = '0'; lv[li++] = '0'; }
    else { if (lvv >= 10) lv[li++] = (char)('0' + lvv / 10); lv[li++] = (char)('0' + lvv % 10); }
    lv[li] = 0;
    gfx_text(8 + gfx_text_width(loc_str(STAT_LV)) + 4, 38, lv, C_TEXT);
    gfx_text(8, 52, loc_str(HINT_GAME), C_HINT);

    if (toast_id >= 0) {
        int w = gfx_text_width(loc_str(toast_id));
        gfx_fill_rect((SCREEN_W - w) / 2 - 8, 200, w + 16, 22, C_PANEL);
        gfx_text((SCREEN_W - w) / 2, 216, loc_str(toast_id), C_TEXT);
    }
}

/* ---- pause menu ---- */
static const StrId tab_names[6] = { TAB_INV, TAB_CHAR, TAB_QUEST, TAB_CRAFT, TAB_PET, TAB_SAVE };

void game_menu_update(GameState *g, const InputState *in)
{
    if (in->pressed & PSP_CTRL_RTRIGGER) menu_tab = (menu_tab + 1) % 6;
    if (in->pressed & PSP_CTRL_LTRIGGER) menu_tab = (menu_tab + 5) % 6;
    int count = 1;
    switch (menu_tab) {
    case 0: count = 0; for (int i = 0; i < INV_CAPACITY; i++) if (g->inv.slots[i].item) count++; break;
    case 1: count = 5; break;
    case 2: count = QUEST_COUNT; break;
    case 3: count = recipe_count; break;
    case 4: count = PET_COUNT; break;
    case 5: count = SAVE_SLOTS; break;
    }
    if (count < 1) count = 1;
    if (in->pressed & PSP_CTRL_UP)   menu_cursor = (menu_cursor + count - 1) % count;
    if (in->pressed & PSP_CTRL_DOWN) menu_cursor = (menu_cursor + 1) % count;
    if (menu_cursor >= count) menu_cursor = 0;

    switch (menu_tab) {
    case 0: /* bag: X use/equip, SQUARE sort */
        if (in->pressed & PSP_CTRL_SQUARE) { inv_sort(&g->inv); break; }
        if (in->pressed & PSP_CTRL_CROSS) {
            int seen = 0;
            for (int i = 0; i < INV_CAPACITY; i++) {
                if (!g->inv.slots[i].item) continue;
                if (seen++ != menu_cursor) continue;
                uint16_t id = g->inv.slots[i].item;
                const ItemDef *d = item_def(id);
                if (!d) break;
                if (d->type == IT_CONSUMABLE) {
                    if (d->heal_hp) { g->stats.hp += d->heal_hp; if (g->stats.hp > stats_max_hp(&g->stats)) g->stats.hp = stats_max_hp(&g->stats); }
                    if (d->heal_mp) { g->stats.mp += d->heal_mp; if (g->stats.mp > stats_max_mp(&g->stats)) g->stats.mp = stats_max_mp(&g->stats); }
                    if (d->pet_food && g->current_pet < PET_COUNT) {
                        if (pet_feed(&g->pets[g->current_pet], d->pet_food)) toast(LEVEL_UP); else toast(PET_FED);
                    }
                    inv_remove(&g->inv, id, 1);
                } else if (d->type == IT_WEAPON || d->type == IT_ARMOR || d->type == IT_ACCESSORY) {
                    EquipSlot slot = d->type == IT_WEAPON ? EQ_WEAPON : (id == 2 ? EQ_OFFHAND : (d->type == IT_ACCESSORY ? EQ_ACCESSORY : EQ_HEAD));
                    uint16_t old = g->equip[slot];
                    g->equip[slot] = id;
                    inv_remove(&g->inv, id, 1);
                    if (old) inv_add(&g->inv, old, 1);
                    toast(ACT_EQUIP);
                }
                break;
            }
        }
        break;
    case 1: /* character: LEFT/RIGHT choose, X spend */
        if (in->pressed & PSP_CTRL_LEFT)  menu_stat_sel = (menu_stat_sel + 4) % 5;
        if (in->pressed & PSP_CTRL_RIGHT) menu_stat_sel = (menu_stat_sel + 1) % 5;
        if (in->pressed & PSP_CTRL_CROSS) { if (stats_spend_point(&g->stats, menu_stat_sel)) toast(STAT_SPENT); }
        break;
    case 2: break;
    case 3: /* crafting: X craft if near table */
        if (in->pressed & PSP_CTRL_CROSS) {
            if (dist2(px, pz, TB_X, TB_Z) > 6.0f) { toast(CRAFT_NEED_STATION); break; }
            const Recipe *r = &recipes[menu_cursor];
            if (recipe_craft(r, &g->inv, (int *)&g->gold)) { toast(CRAFT_OK); refresh_gather_quests(g); }
            else if (!recipe_can_craft(r, &g->inv, (int)g->gold)) toast(CRAFT_NEED_MATERIALS);
            else toast(CRAFT_FAIL);
        }
        break;
    case 4: /* pets: X select, SQUARE feed a treat, TRIANGLE toggle mount */
        if (in->pressed & PSP_CTRL_CROSS) {
            if (!g->pets[menu_cursor].owned) toast(PET_NONE);
            else { g->current_pet = (uint8_t)menu_cursor; toast(PET_FOLLOW); }
        }
        if ((in->pressed & PSP_CTRL_SQUARE) && g->pets[menu_cursor].owned) {
            if (item_count(&g->inv, 22) > 0) {
                inv_remove(&g->inv, 22, 1);
                if (pet_feed(&g->pets[menu_cursor], 10)) toast(LEVEL_UP); else toast(PET_FED);
            } else {
                toast(PET_LOW_FRIENDSHIP);
            }
        }
        break;
    case 5: /* save: X save, TRIANGLE load */
        if (in->pressed & PSP_CTRL_CROSS) {
            toast(savefile_write(menu_cursor, g) ? SAVE_DONE : LOAD_FAIL);
        }
        if (in->pressed & PSP_CTRL_TRIANGLE) {
            GameState tmp;
            if (savefile_read(menu_cursor, &tmp)) { *g = tmp; game_begin(g); toast(LOAD_DONE); }
            else toast(LOAD_FAIL);
        }
        break;
    }
}

static void draw_row(int y, const char *txt, int selected)
{
    if (selected) gfx_fill_rect(24, y - 14, 432, 18, C_SELECT);
    gfx_text(32, y, txt, C_TEXT);
}

void game_menu_draw(const GameState *g)
{
    /* the world stays visible underneath */
    gfx_fill_rect(16, 14, 448, 244, C_PANEL);
    int tx = 24;
    for (int i = 0; i < 6; i++) {
        uint32_t c = (i == menu_tab) ? C_TEXT : C_HINT;
        if (i == menu_tab) gfx_fill_rect(tx - 4, 16, gfx_text_width(loc_str(tab_names[i])) + 8, 18, C_SELECT);
        gfx_text(tx, 30, loc_str(tab_names[i]), c);
        tx += gfx_text_width(loc_str(tab_names[i])) + 14;
    }
    char line[96];
    switch (menu_tab) {
    case 0: {
        int row = 0, y = 56;
        for (int i = 0; i < INV_CAPACITY; i++) {
            const InvSlot *s = &g->inv.slots[i];
            if (!s->item) continue;
            const ItemDef *d = item_def(s->item);
            if (!d) continue;
            const char *nm = loc_get_lang() == LANG_TH ? d->name_th : d->name_en;
            /* "name x count" */
            int n = 0; while (nm[n] && n < 60) { line[n] = nm[n]; n++; }
            line[n++] = ' '; line[n++] = 'x';
            int c = s->count; char digits[6]; int dn = 0;
            do { digits[dn++] = (char)('0' + c % 10); c /= 10; } while (c && dn < 5);
            line[n++] = ' ';
            while (dn) line[n++] = digits[--dn];
            line[n] = 0;
            draw_row(y, line, row == menu_cursor);
            y += 18; row++;
            if (y > 230) break;
        }
        if (row == 0) gfx_text(32, 60, loc_str(INV_EMPTY), C_HINT);
        gfx_text(32, 244, "X: use/equip   []: sort", C_HINT);
        break;
    }
    case 1: {
        const PlayerStats *s = &g->stats;
        char v[16];
        const StrId labels[] = { STAT_LV, STAT_EXP, STAT_HP, STAT_MP, STAT_ATK, STAT_DEF, STAT_STR, STAT_VIT, STAT_AGI };
        for (int i = 0; i < 9; i++) {
            int val = 0;
            switch (i) {
            case 0: val = s->level; break;
            case 1: val = s->exp; break;
            case 2: val = s->hp; break;
            case 3: val = s->mp; break;
            case 4: val = stats_atk(s, equip_atk(g)); break;
            case 5: val = stats_def(s, equip_def(g)); break;
            case 6: val = s->str; break;
            case 7: val = s->vit; break;
            case 8: val = s->agi; break;
            }
            int k = 0; char rev[12]; int rn = 0;
            if (val < 0) { v[k++] = '-'; val = -val; }
            do { rev[rn++] = (char)('0' + val % 10); val /= 10; } while (val && rn < 10);
            while (rn) v[k++] = rev[--rn];
            v[k] = 0;
            int y = 52 + i * 16;
            gfx_text(32, y, loc_str(labels[i]), C_TEXT);
            gfx_text(200, y, v, C_TEXT);
        }
        gfx_text(260, 52, loc_str(STAT_POINTS), C_TEXT);
        int pts = s->stat_points, rn2 = 0; char pb[12];
        do { pb[rn2++] = (char)('0' + pts % 10); pts /= 10; } while (pts && rn2 < 10);
        int pk = 0; char pt[12];
        while (rn2) pt[pk++] = pb[--rn2];
        pt[pk] = 0;
        gfx_text(260, 70, pt, C_TEXT);
        const char *sn[5] = { "STR", "VIT", "AGI", "INT", "LUK" };
        for (int i = 0; i < 5; i++) {
            int y = 120 + i * 18;
            if (i == menu_stat_sel) gfx_fill_rect(256, y - 13, 150, 17, C_SELECT);
            gfx_text(262, y, sn[i], C_TEXT);
        }
        gfx_text(32, 244, loc_str(STAT_HINT), C_HINT);
        break;
    }
    case 2: {
        for (int i = 0; i < QUEST_COUNT; i++) {
            const QuestDef *d = &quest_defs[i];
            const char *st = g->quests[i].status == QS_DONE ? loc_str(QST_DONE)
                           : g->quests[i].status == QS_ACTIVE ? loc_str(QST_ACTIVE) : loc_str(QST_LOCKED);
            const char *nm = loc_get_lang() == LANG_TH ? d->name_th : d->name_en;
            int n = 0; while (nm[n] && n < 60) { line[n] = nm[n]; n++; }
            line[n++] = ' '; line[n++] = '-'; line[n++] = ' ';
            int k = 0; while (st[k] && n < 90) line[n++] = st[k++];
            line[n] = 0;
            draw_row(56 + i * 26, line, i == menu_cursor);
        }
        break;
    }
    case 3: {
        for (int i = 0; i < recipe_count; i++) {
            const Recipe *r = &recipes[i];
            const ItemDef *d = item_def(r->result);
            const char *nm = d ? (loc_get_lang() == LANG_TH ? d->name_th : d->name_en) : "?";
            int n = 0; while (nm[n] && n < 60) { line[n] = nm[n]; n++; }
            line[n] = 0;
            draw_row(54 + i * 22, line, i == menu_cursor);
        }
        gfx_text(32, 244, loc_str(CRAFT_NEED_STATION), C_HINT);
        break;
    }
    case 4: {
        for (int i = 0; i < PET_COUNT; i++) {
            const PetDef *d = &pet_defs[i];
            const PetState *p = &g->pets[i];
            const char *nm = loc_get_lang() == LANG_TH ? d->name_th : d->name_en;
            if (!p->owned) { draw_row(56 + i * 26, loc_str(PET_NONE), i == menu_cursor); continue; }
            int n = 0; while (nm[n] && n < 60) { line[n] = nm[n]; n++; }
            line[n] = 0;
            draw_row(56 + i * 26, line, i == menu_cursor);
            if (g->current_pet == i) gfx_text(360, 56 + i * 26, loc_str(PET_FOLLOW), C_HINT);
        }
        gfx_text(32, 244, "X: follow   []: feed   SELECT: mount", C_HINT);
        break;
    }
    case 5: {
        for (int i = 0; i < SAVE_SLOTS; i++) {
            const char *state = savefile_exists(i) ? loc_str(SAVE_DONE) : loc_str(SAVE_EMPTY);
            int n = 0; const char *s1 = loc_str(SAVE_SLOT);
            while (s1[n] && n < 30) { line[n] = s1[n]; n++; }
            line[n++] = ' '; line[n++] = (char)('1' + i); line[n++] = ' '; line[n++] = '-'; line[n++] = ' ';
            int k = 0; while (state[k] && n < 90) line[n++] = state[k++];
            line[n] = 0;
            draw_row(56 + i * 26, line, i == menu_cursor);
        }
        gfx_text(32, 244, "X: save   /\\: load", C_HINT);
        break;
    }
    }
    gfx_text(32, 262, loc_str(HINT_PAUSE), C_HINT);
}

int game_menu_wanted(void)
{
    int w = menu_open;
    menu_open = 0;
    return w;
}

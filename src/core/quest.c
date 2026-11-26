#include "quest.h"

/* ids 1..6, in story order. Requires chains the quests. */
const QuestDef quest_defs[QUEST_COUNT] = {
    { 1, QT_MAIN,   "ก้าวแรกสู่ Bloomvale", "First Steps in Bloomvale", 0,  0, 50,  30, 20, 3, 0 },
    { 2, QT_GATHER, "เก็บไม้มาซ่อม",       "Gather Wood",            10, 5, 80,  40, 0,  0, 1 },
    { 3, QT_HUNT,   "กำจัดสไลม์ 5 ตัว",     "Slime Hunt",             1,  5, 120, 60, 22, 2, 2 },
    { 4, QT_GATHER, "หาเห็ดเวท",           "Find Magic Mushrooms",   14, 4, 100, 50, 0,  0, 2 },
    { 5, QT_DUNGEON,"ถ้ำป่าเวท",            "Forest Cave",            0,  1, 300, 150,17, 1, 3 },
    { 6, QT_HUNT,   "ปราบราชาพุดดิ้ง",       "Pudding King",           2,  1, 800, 500,18, 1, 5 },
};

const QuestDef *quest_def(uint16_t id)
{
    for (int i = 0; i < QUEST_COUNT; i++) if (quest_defs[i].id == id) return &quest_defs[i];
    return 0;
}

static int idx(uint16_t id) { return (int)id - 1; }

int quest_accept(QuestState st[QUEST_COUNT], uint16_t id)
{
    const QuestDef *d = quest_def(id);
    if (!d || st[idx(id)].status != QS_LOCKED) return 0;
    if (d->requires && st[idx(d->requires)].status != QS_DONE) return 0;
    st[idx(id)].status = QS_ACTIVE;
    st[idx(id)].progress = 0;
    return 1;
}

void quest_on_kill(QuestState st[QUEST_COUNT], uint16_t monster_id)
{
    for (int i = 0; i < QUEST_COUNT; i++) {
        const QuestDef *d = &quest_defs[i];
        if (st[i].status == QS_ACTIVE && d->type == QT_HUNT && d->target == monster_id &&
            st[i].progress < d->target_count) st[i].progress++;
    }
}

void quest_refresh_gather(QuestState st[QUEST_COUNT], uint16_t item_id, int have)
{
    for (int i = 0; i < QUEST_COUNT; i++) {
        const QuestDef *d = &quest_defs[i];
        if (st[i].status == QS_ACTIVE && d->type == QT_GATHER && d->target == item_id) {
            st[i].progress = (uint16_t)(have > d->target_count ? d->target_count : have);
        }
    }
}

int quest_check_complete(QuestState st[QUEST_COUNT], uint16_t id)
{
    const QuestDef *d = quest_def(id);
    if (!d || st[idx(id)].status != QS_ACTIVE) return 0;
    if (st[idx(id)].progress < d->target_count) return 0;
    st[idx(id)].status = QS_DONE;
    return 1;
}

/* Story, exploration, treasure and dungeon quests advance through this call. */
void quest_set_progress(QuestState st[QUEST_COUNT], uint16_t id, int value)
{
    if (st[idx(id)].status == QS_ACTIVE) st[idx(id)].progress = (uint16_t)value;
}

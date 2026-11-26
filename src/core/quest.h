#ifndef ECLIPSE_QUEST_H
#define ECLIPSE_QUEST_H
#include <stdint.h>

typedef enum { QT_MAIN, QT_SIDE, QT_HUNT, QT_GATHER, QT_DUNGEON, QT_EXPLORE, QT_TREASURE } QuestType;
typedef enum { QS_LOCKED, QS_ACTIVE, QS_DONE } QuestStatus;

typedef struct {
    uint16_t id;
    QuestType type;
    const char *name_th, *name_en;
    uint16_t target;         /* monster id (hunt) or item id (gather) or 0 */
    uint16_t target_count;
    uint32_t exp;
    uint16_t gold;
    uint16_t reward_item, reward_count;
    uint16_t requires;       /* quest id that must be DONE first, 0 = none */
} QuestDef;

typedef struct { uint8_t status; uint16_t progress; } QuestState;

#define QUEST_COUNT 6
extern const QuestDef quest_defs[QUEST_COUNT];

const QuestDef *quest_def(uint16_t id);
int  quest_accept(QuestState st[QUEST_COUNT], uint16_t id);
void quest_on_kill(QuestState st[QUEST_COUNT], uint16_t monster_id);
void quest_refresh_gather(QuestState st[QUEST_COUNT], uint16_t item_id, int have);
/* Returns 1 when the quest is complete and marked DONE (caller grants rewards). */
void quest_set_progress(QuestState st[QUEST_COUNT], uint16_t id, int value);
int  quest_check_complete(QuestState st[QUEST_COUNT], uint16_t id);
#endif

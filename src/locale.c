#include "locale.h"

/* Localization IDs map to strings per language. Default: Thai. */
static const char *table[LANG_COUNT][STR_COUNT] = {
    [LANG_TH] = {
        "เริ่มเกม", "เล่นต่อ", "โหลดเกม", "ตั้งค่า", "ออกจากเกม",
        "ตั้งค่า", "ภาษา", "ภาษาไทย", "ขึ้น/ลง: เลือก    X: ยืนยัน", "ซ้าย/ขวา: เปลี่ยน    O: กลับ",
        "ยังไม่มีข้อมูลบันทึก", "กำลังพัฒนา... (Phase ถัดไป)", "กลับ",
    },
    [LANG_EN] = {
        "Start", "Continue", "Load", "Settings", "Exit",
        "Settings", "Language", "English", "UP/DOWN: select    X: confirm", "LEFT/RIGHT: change    O: back",
        "No save data yet", "Coming soon... (next phase)", "Back",
    },
};

static Lang current = LANG_TH;

void loc_set(Lang lang) { if (lang >= 0 && lang < LANG_COUNT) current = lang; }
Lang loc_get_lang(void) { return current; }
const char *loc_str(StrId id) { return table[current][id]; }

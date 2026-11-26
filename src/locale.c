#include "locale.h"

/* Localization IDs map to strings per language. Default: Thai. Keep both tables in the same order as StrId. */
static const char *table[LANG_COUNT][STR_COUNT] = {
    [LANG_TH] = {
        "เริ่มเกม", "เล่นต่อ", "โหลดเกม", "ตั้งค่า", "ออกจากเกม",
        "ตั้งค่า", "ภาษา", "ภาษาไทย", "กลับ",
        "ขึ้น/ลง: เลือก    X: ยืนยัน", "ซ้าย/ขวา: เปลี่ยน    O: กลับ", "ยังไม่มีข้อมูลบันทึก",
        "กระเป๋า", "ตัวละคร", "ภารกิจ", "คราฟต์", "สัตว์เลี้ยง", "บันทึก", "START: ปิดเมนู   L/R: เปลี่ยนแท็บ",
        "เลเวล", "ค่าประสบการณ์", "พลังชีวิต", "พลังเวท", "พลังโจมตี", "พลังป้องกัน", "พลังกาย", "พลังป้องกันร่างกาย", "ความคล่องตัว",
        "สติปัญญา", "โชค", "แต้มสถานะ", "X: เพิ่มแต้ม  ซ้าย/ขวา: เลือกค่า",
        "กระเป๋าว่าง", "ใช้", "สวมใส่", "จัดเรียง", "แบ่ง",
        "ยังไม่รับ", "กำลังทำ", "สำเร็จแล้ว", "รับภารกิจแล้ว", "ภารกิจสำเร็จ!", "ไม่มีภารกิจ", "รางวัล",
        "ผู้ให้ภารกิจ", "พ่อค้า", "โต๊ะคราฟต์", "X: คุย",
        "ยินดีต้อนรับสู่ Bloomvale! ลองไปเก็บไม้ตรงต้นไม้ดูสิ", "เก่งมาก! ไปที่ป่ากันเถอะ",
        "มีของดีให้ซื้อนะ ยาเล็กราคา 8 ทอง",
        "ซื้อแล้ว", "ทองไม่พอ", "คราฟต์สำเร็จ", "คราฟต์ไม่สำเร็จ", "วัตถุดิบไม่เพียงพอ", "ต้องอยู่ใกล้โต๊ะคราฟต์",
        "ยังไม่มีสัตว์เลี้ยง", "ตามฉัน", "ขี่แล้ว", "ลงแล้ว", "ให้ขนมแล้ว", "ต้องมีความสนิทมากกว่านี้",
        "ช่อง", "ว่าง", "บันทึกแล้ว", "โหลดแล้ว", "โหลดไม่สำเร็จ",
        "ยังไม่ปลดล็อกภูมิภาคนี้", "Bloomvale", "ป่าเวทมนตร์", "หุบเขาหิมะ", "ชายฝั่งปะการัง", "ถ้ำคริสตัล", "อาณาจักรเมฆา",
        "ราชาพุดดิ้งปรากฏตัว!", "เลเวลอัป!", "ปราบแล้ว", "พลังหมดแล้ว...", "เก็บได้แล้ว", "ยังไม่พร้อม รอสักครู่",
        "เข้าสู่ถ้ำ", "ใช้แต้มแล้ว", "X: โจมตี  O: กระโดด  △: หลบ  □: สกิล  R: วิ่ง  START: เมนู", "ไม่มีเป้าหมาย",
    },
    [LANG_EN] = {
        "Start", "Continue", "Load", "Settings", "Exit",
        "Settings", "Language", "English", "Back",
        "UP/DOWN: select    X: confirm", "LEFT/RIGHT: change    O: back", "No save data yet",
        "Bag", "Character", "Quests", "Craft", "Pets", "Save", "START: close   L/R: switch tab",
        "Level", "EXP", "HP", "MP", "ATK", "DEF", "STR", "VIT", "AGI", "INT", "LUK", "Stat points", "X: spend   LEFT/RIGHT: choose",
        "Bag is empty", "Use", "Equip", "Sort", "Split",
        "Locked", "Active", "Done", "Quest accepted", "Quest complete!", "No quest", "Reward",
        "Quest Giver", "Merchant", "Crafting Table", "X: talk",
        "Welcome to Bloomvale! Try gathering wood from the trees.", "Great job! Let's head to the forest.",
        "Want something? Small Potion for 8 gold.",
        "Bought", "Not enough gold", "Crafted", "Craft failed", "Not enough materials", "Stand near a crafting table",
        "No pet yet", "Follow me", "Mounted", "Dismounted", "Fed a treat", "Needs more friendship",
        "Slot", "Empty", "Saved", "Loaded", "Load failed",
        "This region is locked", "Bloomvale", "Whispering Woods", "Snowberry Valley", "Coral Coast", "Crystal Caverns", "Starcloud Kingdom",
        "The Pudding King appears!", "Level up!", "Defeated", "You were defeated...", "Gathered", "Not ready yet",
        "Entering the cave", "Point spent", "X: attack  O: jump  Tri: dodge  Sq: skill  R: run  START: menu", "No target",
    },
};

static Lang current = LANG_TH;

void loc_set(Lang lang) { if (lang >= 0 && lang < LANG_COUNT) current = lang; }
Lang loc_get_lang(void) { return current; }
const char *loc_str(StrId id) { return table[current][id]; }

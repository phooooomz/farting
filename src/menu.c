#include <pspctrl.h>
#include "menu.h"
#include "gfx.h"
#include "font.h"
#include "locale.h"

#define COL_BG       RGBA(0xE8, 0xF4, 0xFF, 0xFF)
#define COL_PANEL    RGBA(0xFF, 0xF6, 0xFB, 0xFF)
#define COL_SELECT   RGBA(0xFF, 0xC8, 0xE4, 0xFF)
#define COL_TEXT     RGBA(0x4A, 0x3B, 0x5C, 0xFF)
#define COL_DISABLED RGBA(0xA8, 0x9C, 0xB8, 0xFF)
#define COL_HINT     RGBA(0x7A, 0x6C, 0x90, 0xFF)

#define ROW_H    28
#define ROW_Y0   78
#define BASE_OFF 19

typedef enum { SCREEN_MAIN, SCREEN_SETTINGS } Screen;

static Screen screen;
static int    cursor;
static int    message_id = -1;

static const int      main_enabled[5]  = { 1, 0, 1, 1, 1 };
static const StrId    main_labels[5]   = { MENU_START, MENU_CONTINUE, MENU_LOAD, MENU_SETTINGS, MENU_EXIT };

static void center_text(int y, const char *s, uint32_t color)
{
    gfx_text((SCREEN_W - gfx_text_width(s)) / 2, y, s, color);
}

void menu_init(void)
{
    screen = SCREEN_MAIN;
    cursor = 0;
    message_id = -1;
}

MenuAction menu_update(const InputState *in)
{
    int count = (screen == SCREEN_MAIN) ? 5 : 2;
    message_id = -1;

    if (in->pressed & PSP_CTRL_UP)   cursor = (cursor + count - 1) % count;
    if (in->pressed & PSP_CTRL_DOWN) cursor = (cursor + 1) % count;

    if (screen == SCREEN_SETTINGS) {
        if (in->pressed & PSP_CTRL_CIRCLE) { screen = SCREEN_MAIN; cursor = 3; return MENU_NONE; }
        if (cursor == 0 && (in->pressed & (PSP_CTRL_CROSS | PSP_CTRL_LEFT | PSP_CTRL_RIGHT))) {
            loc_set(loc_get_lang() == LANG_TH ? LANG_EN : LANG_TH);
        }
        if (cursor == 1 && (in->pressed & PSP_CTRL_CROSS)) { screen = SCREEN_MAIN; cursor = 3; }
        return MENU_NONE;
    }

    if (in->pressed & PSP_CTRL_CROSS) {
        if (!main_enabled[cursor]) { message_id = MSG_NOT_AVAILABLE; return MENU_NONE; }
        switch (main_labels[cursor]) {
        case MENU_START:    return MENU_ACT_START;
        case MENU_LOAD:     return MENU_ACT_LOAD;
        case MENU_SETTINGS: screen = SCREEN_SETTINGS; cursor = 0; return MENU_NONE;
        case MENU_EXIT:     return MENU_ACT_EXIT;
        default: break;
        }
    }
    return MENU_NONE;
}

void menu_draw(void)
{
    gfx_begin_frame(COL_BG);
    gfx_fill_rect(120, 30, 240, 200, COL_PANEL);

    if (screen == SCREEN_MAIN) {
        center_text(48, "ECLIPSE", COL_TEXT);
        for (int i = 0; i < 5; i++) {
            int y = ROW_Y0 + i * ROW_H;
            if (i == cursor) gfx_fill_rect(150, y, 180, ROW_H - 4, COL_SELECT);
            uint32_t c = main_enabled[i] ? COL_TEXT : COL_DISABLED;
            center_text(y + BASE_OFF, loc_str(main_labels[i]), c);
        }
        center_text(236, loc_str(HINT_MENU), COL_HINT);
    } else {
        center_text(48, loc_str(SETTINGS_TITLE), COL_TEXT);
        int y = ROW_Y0;
        if (cursor == 0) gfx_fill_rect(150, y, 180, ROW_H - 4, COL_SELECT);
        center_text(y + BASE_OFF, loc_str(SETTINGS_LANGUAGE), COL_TEXT);
        center_text(y + BASE_OFF + ROW_H, loc_str(LANG_NAME), COL_HINT);
        if (cursor == 1) gfx_fill_rect(150, y + 2 * ROW_H, 180, ROW_H - 4, COL_SELECT);
        center_text(y + 2 * ROW_H + BASE_OFF, loc_str(MSG_BACK), COL_TEXT);
        center_text(236, loc_str(HINT_SETTINGS), COL_HINT);
    }

    if (message_id >= 0) center_text(220, loc_str(message_id), COL_HINT);
    gfx_end_frame();
}

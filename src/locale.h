#ifndef ECLIPSE_LOCALE_H
#define ECLIPSE_LOCALE_H

typedef enum { LANG_TH = 0, LANG_EN = 1, LANG_COUNT } Lang;

typedef enum {
    MENU_START, MENU_CONTINUE, MENU_LOAD, MENU_SETTINGS, MENU_EXIT,
    SETTINGS_TITLE, SETTINGS_LANGUAGE, LANG_NAME, HINT_MENU, HINT_SETTINGS,
    MSG_NOT_AVAILABLE, MSG_COMING_SOON, MSG_BACK,
    STR_COUNT
} StrId;

void        loc_set(Lang lang);
Lang        loc_get_lang(void);
const char *loc_str(StrId id);

#endif

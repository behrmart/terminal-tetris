#include "menu_theme.h"
#include <stdbool.h>
#include <ncurses.h>

static bool colors_enabled = false;

void menu_theme_prepare(void)
{
    static const short foreground[] = {
        COLOR_WHITE, COLOR_CYAN, COLOR_YELLOW, COLOR_GREEN,
        COLOR_RED, COLOR_MAGENTA, COLOR_BLACK
    };
    static const short background[] = {
        COLOR_BLUE, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK,
        COLOR_BLACK, COLOR_BLACK, COLOR_CYAN
    };
    colors_enabled = false;
    if (!has_colors() || COLORS < 8 || COLOR_PAIRS <= 22) {
        return;
    }
    for (short i = 0; i < 7; ++i) {
        if (init_pair((short)(16 + i), foreground[i], background[i]) == ERR) {
            return;
        }
    }
    colors_enabled = true;
}

void menu_theme_style(MenuStyle style)
{
    short pair = 0;
    attr_t attributes = A_NORMAL;
    if (style >= MENU_TITLE && style <= MENU_FIELD) {
        attributes = A_BOLD;
        if (colors_enabled) {
            pair = (short)(15 + (int)style);
        } else if (style == MENU_FIELD) {
            attributes = A_REVERSE;
        }
    }
    (void)attr_set(attributes, pair, NULL);
}

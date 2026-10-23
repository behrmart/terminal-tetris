#ifndef TERMINAL_MENU_THEME_H
#define TERMINAL_MENU_THEME_H

typedef enum {
    MENU_NORMAL, MENU_TITLE, MENU_HEADING, MENU_SCORE,
    MENU_SUCCESS, MENU_WARNING, MENU_KEYS, MENU_FIELD
} MenuStyle;

/* Call once after each successful terminal initialization.
 * Uses pairs 16..22; game palettes remain untouched.
 */
void menu_theme_prepare(void);
void menu_theme_style(MenuStyle style);
#endif

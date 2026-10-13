#ifndef TERMINAL_TETRIS_DISPLAY_H
#define TERMINAL_TETRIS_DISPLAY_H

#include <stdbool.h>

#include "game.h"

enum {
    DISPLAY_MIN_COLUMNS = 54,
    DISPLAY_MIN_ROWS = BOARD_HEIGHT + 6
};

bool display_init(void);
/* Dimensions reflect the latest resize handled by ncurses input polling. */
bool display_can_fit(void);
void display_render(const Game *game);
void display_shutdown(void);

#endif

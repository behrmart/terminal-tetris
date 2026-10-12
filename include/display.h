#ifndef TERMINAL_TETRIS_DISPLAY_H
#define TERMINAL_TETRIS_DISPLAY_H

#include <stdbool.h>

#include "game.h"

enum {
    DISPLAY_MIN_COLUMNS = BOARD_WIDTH * 2 + 4,
    DISPLAY_MIN_ROWS = BOARD_HEIGHT + 6
};

bool display_init(void);
void display_render(const Game *game);
void display_shutdown(void);

#endif

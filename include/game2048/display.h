#ifndef TERMINAL_GAME2048_DISPLAY_H
#define TERMINAL_GAME2048_DISPLAY_H

#include "game2048/game.h"

/* size 0 means the startup menu; gameplay sizes are 4 and 6. */
unsigned int game2048_display_min_columns(unsigned int size);
unsigned int game2048_display_min_rows(unsigned int size);
bool game2048_display_init(void);
bool game2048_display_can_fit(unsigned int size);
void game2048_display_menu(void);
void game2048_display_render(const Game2048 *game);
void game2048_display_shutdown(void);

#endif

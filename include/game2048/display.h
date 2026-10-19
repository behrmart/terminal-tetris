#ifndef TERMINAL_GAME2048_DISPLAY_H
#define TERMINAL_GAME2048_DISPLAY_H

#include "game2048/game.h"

/* Through Phase 2: print a noninteractive skeleton notice; never change game state.
 * The ncurses interface is reserved for Phase 4.
 */
void game2048_display_render(const Game2048 *game);

#endif

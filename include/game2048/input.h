#ifndef TERMINAL_GAME2048_INPUT_H
#define TERMINAL_GAME2048_INPUT_H

#include "game2048/game.h"

typedef struct {
    Game2048Action action;
    unsigned int selected_size; /* 4/6 at the menu; zero means no selection. */
} Game2048Input;

/* Poll after terminal initialization. Enter selects 4x4 at the menu.
 * Numeric selection is ignored during gameplay. Timeout/resize are inert.
 */
Game2048Input game2048_input_poll(void);

#endif

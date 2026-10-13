#ifndef TERMINAL_TETRIS_INPUT_H
#define TERMINAL_TETRIS_INPUT_H

#include "game.h"

/* Poll after display_init succeeds. Waits at most the display's input timeout
 * for an initial key. ncurses decodes arrow sequences; resize/unknown keys
 * and timeout return ACTION_NONE.
 */
Action input_poll(void);

#endif

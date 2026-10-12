#ifndef TERMINAL_TETRIS_INPUT_H
#define TERMINAL_TETRIS_INPUT_H

#include "game.h"

/* Poll after display_init succeeds. Only Q/q is bound in Milestone 1. */
Action input_poll(void);

#endif

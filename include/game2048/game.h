#ifndef TERMINAL_GAME2048_GAME_H
#define TERMINAL_GAME2048_GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "game2048/board.h"

typedef enum {
    GAME2048_ACTION_NONE,
    GAME2048_ACTION_LEFT,
    GAME2048_ACTION_RIGHT,
    GAME2048_ACTION_UP,
    GAME2048_ACTION_DOWN,
    GAME2048_ACTION_RESTART,
    GAME2048_ACTION_CONTINUE,
    GAME2048_ACTION_QUIT
} Game2048Action;

typedef enum {
    GAME2048_RUNNING,
    GAME2048_WON,
    GAME2048_OVER
} Game2048Status;

typedef struct {
    Game2048Board board;
    uint64_t score;
    uint32_t initial_seed;
    uint32_t random_state;
    Game2048Status status;
    bool win_acknowledged;
} Game2048;

/* Through Phase 2: initialize an EMPTY board and seed storage; no random tiles yet.
 * Invalid size or NULL returns false without changing game.
 */
bool game2048_init(Game2048 *game, unsigned int size, uint32_t seed);

/* Phase 2 placeholder: every action is inert. game must be initialized.
 * No elapsed-time API is needed for this turn-based game.
 */
void game2048_process_action(Game2048 *game, Game2048Action action);

#endif

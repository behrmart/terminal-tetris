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

/* Initialize a 4x4 or 6x6 board with two distinct random tiles.
 * All seeds, including zero, are valid. Each tile is 2 with probability
 * 90%, otherwise 4; positions are chosen uniformly from empty cells.
 * Invalid size or NULL returns false without changing game.
 */
bool game2048_init(Game2048 *game, unsigned int size, uint32_t seed);

/* Requires initialized state with invariants preserved; NULL is a no-op.
 * Changed moves add merge points (saturated) and spawn exactly one tile.
 * Ineffective moves leave the entire state unchanged, including the PRNG.
 * A new 2048-or-higher tile yields WON before loss detection. Movement is
 * frozen until CONTINUE acknowledges the win; it then resumes or yields
 * OVER if no moves remain. Acknowledged wins never interrupt play again.
 * RESTART works from every status, preserving size and original seed.
 * NONE/QUIT/unknown actions are inert; quit belongs to the host.
 * There is no elapsed-time API.
 */
void game2048_process_action(Game2048 *game, Game2048Action action);

#endif

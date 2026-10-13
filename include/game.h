#ifndef TERMINAL_TETRIS_GAME_H
#define TERMINAL_TETRIS_GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "tetromino.h"

typedef enum {
    ACTION_NONE, ACTION_LEFT, ACTION_RIGHT, ACTION_SOFT_DROP,
    ACTION_HARD_DROP, ACTION_ROTATE_CW, ACTION_ROTATE_CCW,
    ACTION_PAUSE, ACTION_RESTART, ACTION_QUIT
} Action;

typedef enum {
    GAME_RUNNING, GAME_PAUSED, GAME_OVER
} GameStatus;

enum { GAME_MAX_ELAPSED_MS = 100 };

typedef struct {
    Board board;
    Piece active;
    bool has_active_piece;
    TetrominoType next;
    uint64_t score;
    unsigned int lines;
    unsigned int level;
    unsigned int gravity_elapsed_ms;
    GameStatus status;
    uint32_t random_state;
    TetrominoType bag[TETROMINO_COUNT];
    unsigned int bag_index;
} Game;

/* Initialize before other game calls; callers must preserve engine invariants.
 * All seeds (including zero) are valid. Initialization spawns an active piece
 * and draws a next piece from per-game shuffled seven-piece bags.
 * Same initial seed, actions and elapsed updates produce the same state.
 */
void game_init(Game *game, uint32_t seed);

/* Move left/right or rotate if the candidate fits (no wall kicks).
 * Soft drop moves one row, or locks immediately when blocked.
 * Hard drop lands and locks one piece. Locking advances next and spawns;
 * a blocked spawn sets GAME_OVER and clears has_active_piece.
 * Actions are ignored unless running with an active piece. None, quit,
 * pause, restart and unknown actions are no-ops in Milestone 3.
 * Quit remains host-owned; pause/restart will arrive in Milestone 4.
 */
void game_process_action(Game *game, Action action);

/* Caller supplies elapsed milliseconds; the engine never reads a clock.
 * Future gravity must cap each update to GAME_MAX_ELAPSED_MS, discard excess
 * elapsed time, and ignore time while paused. Milestone 3 is still a no-op.
 */
void game_update(Game *game, unsigned int elapsed_ms);

#endif

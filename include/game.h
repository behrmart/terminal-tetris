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

/* Preconditions: game points to a valid Game.
 * Same initial seed, actions and elapsed updates must produce the same state.
 */
void game_init(Game *game, uint32_t seed);

/* Engine actions are separate from time updates. ACTION_QUIT is host-owned.
 * Milestone 1: all actions are intentionally no-ops.
 */
void game_process_action(Game *game, Action action);

/* Caller supplies elapsed milliseconds; the engine never reads a clock.
 * Future gravity must cap each update to GAME_MAX_ELAPSED_MS, discard excess
 * elapsed time, and ignore time while paused. Milestone 1 is a no-op.
 */
void game_update(Game *game, unsigned int elapsed_ms);

#endif

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
    uint32_t initial_seed;
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

/* Move/rotate only if the candidate fits; no wall kicks. Blocked downward
 * attempts lock immediately. Hard drop locks exactly one piece. Locking
 * clears lines, awards 100/300/500/800 times the pre-clear level, then spawns.
 * Level starts at 1 and increases every 10 lines. No drop points are awarded.
 * Pause toggles running/paused and resets fractional gravity time.
 * Restart works from any status and replays the original seed.
 * Other actions are ignored unless running. Quit remains host-owned.
 */
void game_process_action(Game *game, Action action);

/* Gravity interval: 1000 ms at level 1, 100 ms less per level, floor 100 ms.
 * Each update accepts at most GAME_MAX_ELAPSED_MS; excess is discarded.
 * At most one downward attempt occurs per call. Sub-interval time is kept,
 * but locking/spawning discards remainder. Paused/over games ignore time.
 * The host must reset its clock baseline after pause/resume, restart and
 * recovery from an undersized terminal. The engine never reads OS clocks.
 */
unsigned int game_gravity_interval_ms(const Game *game);
void game_update(Game *game, unsigned int elapsed_ms);

#endif

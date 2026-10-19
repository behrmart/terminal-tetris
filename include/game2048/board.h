#ifndef TERMINAL_GAME2048_BOARD_H
#define TERMINAL_GAME2048_BOARD_H

#include <stdbool.h>
#include <stdint.h>

enum {
    GAME2048_SMALL_SIZE = 4,
    GAME2048_MAX_SIZE = 6,
    GAME2048_TARGET_EXPONENT = 11,
    GAME2048_MAX_EXPONENT = 63
};

/* 0 is empty; n represents 2^n. Merges must not exceed exponent 63. */
typedef uint8_t Game2048Tile;

typedef struct {
    Game2048Tile cells[GAME2048_MAX_SIZE][GAME2048_MAX_SIZE];
    unsigned int size;
} Game2048Board;

typedef enum {
    GAME2048_LEFT,
    GAME2048_RIGHT,
    GAME2048_UP,
    GAME2048_DOWN
} Game2048Direction;

typedef struct {
    bool changed;
    uint64_t score_gained;
} Game2048MoveResult;

/* Accept 4 or 6. Clear all storage, including unused cells.
 * Invalid sizes or NULL return false without changing the board.
 */
bool game2048_board_init(Game2048Board *board, unsigned int size);

/* Slide toward direction and merge equal neighbors once per move.
 * Returns whether any active cell changed and the merged values' sum,
 * saturated at UINT64_MAX. No tiles are spawned.
 * Exponent-63 tiles can slide but cannot merge. Inactive storage is preserved.
 * NULL, invalid size/direction or active exponent >63: return {false, 0}
 * without changing any cells.
 */
Game2048MoveResult game2048_board_move(Game2048Board *board,
                                     Game2048Direction direction);

/* True if at least one direction would change the board. Does not mutate.
 * Empty boards and invalid boards return false.
 */
bool game2048_board_has_moves(const Game2048Board *board);

#endif

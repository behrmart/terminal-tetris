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

/* 0 is empty; n represents 2^n. Future merges must not exceed exponent 63. */
typedef uint8_t Game2048Tile;

typedef struct {
    Game2048Tile cells[GAME2048_MAX_SIZE][GAME2048_MAX_SIZE];
    unsigned int size;
} Game2048Board;

typedef struct {
    bool changed;
    uint64_t score_gained;
} Game2048MoveResult;

/* Accept 4 or 6. Clear all storage, including unused cells.
 * Invalid sizes or NULL return false without changing the board.
 */
bool game2048_board_init(Game2048Board *board, unsigned int size);

#endif

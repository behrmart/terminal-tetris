#ifndef TERMINAL_TETRIS_BOARD_H
#define TERMINAL_TETRIS_BOARD_H

#include <stdbool.h>

#include "tetromino.h"

enum { BOARD_WIDTH = 10, BOARD_HEIGHT = 20 };

typedef enum {
    CELL_EMPTY = 0,
    CELL_I, CELL_O, CELL_T, CELL_S, CELL_Z, CELL_J, CELL_L
} Cell;

typedef struct {
    Cell cells[BOARD_HEIGHT][BOARD_WIDTH];
} Board;

/* board must point to valid storage. */
void board_init(Board *board);

/* All four occupied cells must be inside the visible board and empty.
 * Empty space in the local piece box may extend outside the board.
 * NULL pointers and invalid piece types/rotations are rejected.
 */
bool board_can_place(const Board *board, const Piece *piece);

/* Store the piece identity in four cells only if placement is valid.
 * On failure the board is unchanged. Does not clear rows or move the piece.
 */
bool board_lock_piece(Board *board, const Piece *piece);

#endif

#ifndef TERMINAL_TETRIS_BOARD_H
#define TERMINAL_TETRIS_BOARD_H

enum { BOARD_WIDTH = 10, BOARD_HEIGHT = 20 };

typedef enum {
    CELL_EMPTY = 0,
    CELL_I, CELL_O, CELL_T, CELL_S, CELL_Z, CELL_J, CELL_L
} Cell;

typedef struct {
    Cell cells[BOARD_HEIGHT][BOARD_WIDTH];
} Board;

void board_init(Board *board);

#endif

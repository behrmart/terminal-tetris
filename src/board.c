#include "board.h"

void board_init(Board *board)
{
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            board->cells[y][x] = CELL_EMPTY;
        }
    }
}

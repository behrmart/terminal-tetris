#include "game2048/board.h"

#include <stddef.h>

bool game2048_board_init(Game2048Board *board, unsigned int size)
{
    if (board == NULL ||
        (size != GAME2048_SMALL_SIZE && size != GAME2048_MAX_SIZE)) {
        return false;
    }
    *board = (Game2048Board){0};
    board->size = size;
    return true;
}

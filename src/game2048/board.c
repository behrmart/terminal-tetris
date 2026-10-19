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

static bool board_valid(const Game2048Board *board)
{
    if (board == NULL ||
        (board->size != GAME2048_SMALL_SIZE && board->size != GAME2048_MAX_SIZE)) {
        return false;
    }
    for (unsigned int y = 0U; y < board->size; ++y) {
        for (unsigned int x = 0U; x < board->size; ++x) {
            if (board->cells[y][x] > GAME2048_MAX_EXPONENT) {
                return false;
            }
        }
    }
    return true;
}

/* Offset zero is the destination edge; callers validate direction first. */
static Game2048Tile *line_cell(Game2048Board *board, Game2048Direction direction,
                              unsigned int line, unsigned int offset)
{
    switch (direction) {
    case GAME2048_LEFT:
        return &board->cells[line][offset];
    case GAME2048_RIGHT:
        return &board->cells[line][board->size - 1U - offset];
    case GAME2048_UP:
        return &board->cells[offset][line];
    case GAME2048_DOWN:
        return &board->cells[board->size - 1U - offset][line];
    default:
        return NULL; /* Unreachable after direction validation. */
    }
}

Game2048MoveResult game2048_board_move(Game2048Board *board,
                                     Game2048Direction direction)
{
    Game2048MoveResult result = {false, 0U};
    if (!board_valid(board) ||
        (unsigned int)direction > (unsigned int)GAME2048_DOWN) {
        return result;
    }

    for (unsigned int line = 0U; line < board->size; ++line) {
        Game2048Tile packed[GAME2048_MAX_SIZE] = {0U};
        Game2048Tile merged[GAME2048_MAX_SIZE] = {0U};
        unsigned int count = 0U;
        for (unsigned int offset = 0U; offset < board->size; ++offset) {
            const Game2048Tile tile = *line_cell(board, direction, line, offset);
            if (tile != 0U) {
                packed[count++] = tile;
            }
        }

        unsigned int output = 0U;
        for (unsigned int input = 0U; input < count; ++input) {
            Game2048Tile tile = packed[input];
            if (input + 1U < count && tile == packed[input + 1U] &&
                tile < GAME2048_MAX_EXPONENT) {
                tile = (Game2048Tile)(tile + 1U);
                ++input; /* Consume the pair; the result cannot merge again. */
                const uint64_t value = UINT64_C(1) << tile;
                result.score_gained = value > UINT64_MAX - result.score_gained ?
                                      UINT64_MAX : result.score_gained + value;
            }
            merged[output++] = tile;
        }

        for (unsigned int offset = 0U; offset < board->size; ++offset) {
            Game2048Tile *destination = line_cell(board, direction, line, offset);
            if (*destination != merged[offset]) {
                result.changed = true;
                *destination = merged[offset];
            }
        }
    }
    return result;
}

bool game2048_board_has_moves(const Game2048Board *board)
{
    if (!board_valid(board)) {
        return false;
    }
    /* Reuse move semantics so capped tiles and empty boards stay consistent.
     * Four copies of at most 36 cells are bounded and require no allocation.
     */
    for (int direction = GAME2048_LEFT; direction <= GAME2048_DOWN; ++direction) {
        Game2048Board candidate = *board;
        if (game2048_board_move(&candidate, (Game2048Direction)direction).changed) {
            return true;
        }
    }
    return false;
}

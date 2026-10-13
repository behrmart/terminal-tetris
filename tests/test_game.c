#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "board.h"
#include "game.h"
#include "tetromino.h"

int main(void)
{
    Board board;
    Game game;

    _Static_assert(BOARD_WIDTH == 10, "Board width must be 10");
    _Static_assert(BOARD_HEIGHT == 20, "Board height must be 20");
    _Static_assert(TETROMINO_COUNT == 7, "There must be seven piece types");

    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            board.cells[y][x] = CELL_L;
        }
    }
    board_init(&board);
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            assert(board.cells[y][x] == CELL_EMPTY);
        }
    }

    game_init(&game, UINT32_C(42));
    assert(game.status == GAME_RUNNING);
    assert(!game.has_active_piece);
    assert(game.score == 0U);
    assert(game.lines == 0U);
    assert(game.level == 1U);
    assert(game.gravity_elapsed_ms == 0U);
    assert(game.random_state == UINT32_C(42));
    assert(game.bag_index == (unsigned int)TETROMINO_COUNT);

    /* Until physics exists, even a very long update cannot advance play. */
    game_update(&game, UINT_MAX);
    assert(game.gravity_elapsed_ms == 0U);
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            assert(game.board.cells[y][x] == CELL_EMPTY);
        }
    }
    (void)puts("Game initialization tests passed.");
    return 0;
}

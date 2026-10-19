#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#include "game2048/game.h"

static void assert_equal(const Game2048 *a, const Game2048 *b)
{
    assert(a->board.size == b->board.size);
    assert(a->score == b->score);
    assert(a->initial_seed == b->initial_seed);
    assert(a->random_state == b->random_state);
    assert(a->status == b->status);
    assert(a->win_acknowledged == b->win_acknowledged);
    for (int y = 0; y < GAME2048_MAX_SIZE; ++y) {
        for (int x = 0; x < GAME2048_MAX_SIZE; ++x) {
            assert(a->board.cells[y][x] == b->board.cells[y][x]);
        }
    }
}

int main(void)
{
    static const unsigned int sizes[] = {4U, 6U};
    static const uint32_t seeds[] = {UINT32_C(0), UINT32_C(42), UINT32_MAX};
    Game2048 game;

    _Static_assert(GAME2048_MAX_SIZE == 6, "Storage must support 6x6");
    _Static_assert(GAME2048_MAX_EXPONENT < 64, "Tile values must fit uint64_t");

    for (unsigned int size = 0U; size < 2U; ++size) {
        for (unsigned int seed = 0U; seed < 3U; ++seed) {
            assert(game2048_init(&game, sizes[size], seeds[seed]));
            assert(game.board.size == sizes[size]);
            assert(game.score == 0U && game.status == GAME2048_RUNNING);
            assert(!game.win_acknowledged);
            assert(game.initial_seed == seeds[seed]);
            assert(game.random_state == seeds[seed]);
            for (int y = 0; y < GAME2048_MAX_SIZE; ++y) {
                for (int x = 0; x < GAME2048_MAX_SIZE; ++x) {
                    assert(game.board.cells[y][x] == 0U);
                }
            }
            const Game2048 before = game;
            for (int action = GAME2048_ACTION_NONE;
                 action <= GAME2048_ACTION_QUIT; ++action) {
                game2048_process_action(&game, (Game2048Action)action);
                assert_equal(&game, &before);
            }
        }
    }

    /* Shrinking a reused board clears inactive storage as well. */
    game.board.cells[5][5] = 8U;
    game.score = 100U;
    assert(game2048_init(&game, 4U, UINT32_C(7)));
    assert(game.board.cells[5][5] == 0U && game.score == 0U);

    const Game2048 before = game;
    static const unsigned int invalid_sizes[] = {0U, 3U, 5U, 7U, UINT_MAX};
    for (unsigned int i = 0U; i < 5U; ++i) {
        assert(!game2048_init(&game, invalid_sizes[i], UINT32_C(9)));
        assert_equal(&game, &before);
        assert(!game2048_board_init(&game.board, invalid_sizes[i]));
        assert_equal(&game, &before);
    }
    assert(!game2048_init(NULL, 4U, UINT32_C(0)));
    assert(!game2048_board_init(NULL, 6U));

    game.board.cells[3][3] = 4U;
    assert(game2048_board_init(&game.board, 6U));
    assert(game.board.size == 6U && game.board.cells[3][3] == 0U);
    (void)puts("2048 Phase 1 structure tests passed (4x4, 6x6, seeds, validation).");
    return 0;
}

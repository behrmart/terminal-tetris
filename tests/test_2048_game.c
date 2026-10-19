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

static void empty_fixture(Game2048 *game, unsigned int size)
{
    assert(game2048_init(game, size, UINT32_C(42)));
    assert(game2048_board_init(&game->board, size));
}

static void assert_one_spawn(const Game2048Board *before, const Game2048Board *after)
{
    unsigned int added = 0U;
    for (unsigned int y = 0U; y < GAME2048_MAX_SIZE; ++y) {
        for (unsigned int x = 0U; x < GAME2048_MAX_SIZE; ++x) {
            if (before->cells[y][x] != after->cells[y][x]) {
                assert(y < after->size && x < after->size);
                assert(before->cells[y][x] == 0U);
                assert(after->cells[y][x] == 1U || after->cells[y][x] == 2U);
                ++added;
            }
        }
    }
    assert(added == 1U);
}

static void test_initialization(void)
{
    bool saw_two = false;
    bool saw_four = false;
    for (unsigned int size = 4U; size <= 6U; size += 2U) {
        for (uint32_t seed = 0U; seed < 128U; ++seed) {
            Game2048 game;
            Game2048 same;
            assert(game2048_init(&game, size, seed));
            assert(game2048_init(&same, size, seed));
            assert_equal(&game, &same);
            assert(game.board.size == size && game.score == 0U);
            assert(game.status == GAME2048_RUNNING && !game.win_acknowledged);
            assert(game.initial_seed == seed);
            unsigned int count = 0U;
            for (unsigned int y = 0U; y < GAME2048_MAX_SIZE; ++y) {
                for (unsigned int x = 0U; x < GAME2048_MAX_SIZE; ++x) {
                    const Game2048Tile tile = game.board.cells[y][x];
                    if (y >= size || x >= size) {
                        assert(tile == 0U);
                    } else if (tile != 0U) {
                        assert(tile == 1U || tile == 2U);
                        saw_two = saw_two || tile == 1U;
                        saw_four = saw_four || tile == 2U;
                        ++count;
                    }
                }
            }
            assert(count == 2U);
            assert(game2048_board_has_moves(&game.board));
        }
    }
    assert(saw_two && saw_four); /* Coverage, not a statistical probability test. */
}

static void test_moves_and_no_ops(void)
{
    static const Game2048Action actions[] = {
        GAME2048_ACTION_LEFT, GAME2048_ACTION_RIGHT,
        GAME2048_ACTION_UP, GAME2048_ACTION_DOWN
    };
    static const Game2048Direction directions[] = {
        GAME2048_LEFT, GAME2048_RIGHT, GAME2048_UP, GAME2048_DOWN
    };
    for (unsigned int size = 4U; size <= 6U; size += 2U) {
        Game2048 game;
        for (unsigned int i = 0U; i < 4U; ++i) {
            empty_fixture(&game, size);
            game.board.cells[1][1] = 1U;
            game.board.cells[2][1] = 1U;
            game.score = 17U;
            Game2048Board moved = game.board;
            const Game2048MoveResult result = game2048_board_move(&moved, directions[i]);
            assert(result.changed);
            game2048_process_action(&game, actions[i]);
            assert(game.score == 17U + result.score_gained);
            assert_one_spawn(&moved, &game.board);
        }
        empty_fixture(&game, size);
        game.board.cells[0][0] = 1U;
        const Game2048 before = game;
        static const Game2048Action inert[] = {
            GAME2048_ACTION_LEFT, GAME2048_ACTION_UP, GAME2048_ACTION_NONE,
            GAME2048_ACTION_CONTINUE, GAME2048_ACTION_QUIT, (Game2048Action)-1
        };
        for (size_t i = 0U; i < sizeof(inert) / sizeof(inert[0]); ++i) {
            game2048_process_action(&game, inert[i]);
            assert_equal(&game, &before);
        }

        empty_fixture(&game, size);
        game.board.cells[0][0] = 1U;
        game.board.cells[0][1] = 1U;
        game.score = UINT64_MAX - 1U;
        game2048_process_action(&game, GAME2048_ACTION_LEFT);
        assert(game.score == UINT64_MAX);
    }
}

/* Full after a left move and its spawn, with no neighboring equal tiles. */
static void loss_fixture(Game2048 *game, unsigned int size, bool win)
{
    empty_fixture(game, size);
    for (unsigned int y = 0U; y < size; ++y) {
        for (unsigned int x = 0U; x < size; ++x) {
            game->board.cells[y][x] = (Game2048Tile)(3U + (x + y) % 2U);
        }
    }
    for (unsigned int x = 0U; x < size; ++x) {
        game->board.cells[0][x] = (Game2048Tile)(5U + x % 2U);
    }
    if (win) {
        game->board.cells[0][0] = 10U;
        game->board.cells[0][1] = 10U;
    } else {
        game->board.cells[0][0] = 0U;
    }
}

static void test_win_loss_continue(void)
{
    for (unsigned int size = 4U; size <= 6U; size += 2U) {
        Game2048 game;
        empty_fixture(&game, size);
        game.board.cells[0][0] = 10U;
        game.board.cells[0][1] = 10U;
        game2048_process_action(&game, GAME2048_ACTION_LEFT);
        assert(game.status == GAME2048_WON && game.score == 2048U);
        assert(game.board.cells[0][0] == 11U);
        const Game2048 won = game;
        for (int action = GAME2048_ACTION_NONE; action <= GAME2048_ACTION_DOWN; ++action) {
            game2048_process_action(&game, (Game2048Action)action);
            assert_equal(&game, &won);
        }
        game2048_process_action(&game, GAME2048_ACTION_CONTINUE);
        Game2048 expected = won;
        expected.win_acknowledged = true;
        expected.status = GAME2048_RUNNING;
        assert_equal(&game, &expected); /* Continue neither spawns nor consumes RNG. */
        game2048_process_action(&game, GAME2048_ACTION_DOWN);
        assert(game.status == GAME2048_RUNNING && game.win_acknowledged);

        loss_fixture(&game, size, false);
        game2048_process_action(&game, GAME2048_ACTION_LEFT);
        assert(game.status == GAME2048_OVER);
        assert(!game2048_board_has_moves(&game.board));
        const Game2048 over = game;
        game2048_process_action(&game, GAME2048_ACTION_RIGHT);
        game2048_process_action(&game, GAME2048_ACTION_CONTINUE);
        assert_equal(&game, &over);

        loss_fixture(&game, size, true);
        game2048_process_action(&game, GAME2048_ACTION_LEFT);
        assert(game.status == GAME2048_WON);
        assert(!game2048_board_has_moves(&game.board));
        const uint32_t rng = game.random_state;
        game2048_process_action(&game, GAME2048_ACTION_CONTINUE);
        assert(game.status == GAME2048_OVER && game.win_acknowledged);
        assert(game.random_state == rng);
    }
}

static void test_restart_and_replay(void)
{
    static const uint32_t seeds[] = {UINT32_C(0), UINT32_C(42), UINT32_MAX};
    for (unsigned int size = 4U; size <= 6U; size += 2U) {
        for (unsigned int seed = 0U; seed < 3U; ++seed) {
            Game2048 first;
            Game2048 second;
            Game2048 unrelated;
            assert(game2048_init(&first, size, seeds[seed]));
            assert(game2048_init(&second, size, seeds[seed]));
            assert(game2048_init(&unrelated, size, UINT32_C(123)));
            const Game2048 fresh = first;
            for (unsigned int step = 0U; step < 160U; ++step) {
                const Game2048Action action =
                    (Game2048Action)((unsigned int)GAME2048_ACTION_LEFT + step % 4U);
                game2048_process_action(&first, action);
                game2048_process_action(&unrelated, GAME2048_ACTION_DOWN);
                game2048_process_action(&second, action);
                assert_equal(&first, &second);
                if (step % 40U == 39U) {
                    game2048_process_action(&first, GAME2048_ACTION_RESTART);
                    game2048_process_action(&second, GAME2048_ACTION_RESTART);
                    assert_equal(&first, &fresh);
                    assert_equal(&first, &second);
                }
            }
            for (int status = GAME2048_RUNNING; status <= GAME2048_OVER; ++status) {
                first.status = (Game2048Status)status;
                first.win_acknowledged = true;
                first.score = 123U;
                first.board.cells[size - 1U][size - 1U] = 8U;
                game2048_process_action(&first, GAME2048_ACTION_RESTART);
                assert_equal(&first, &fresh);
            }
        }
    }
}

static void test_validation(void)
{
    Game2048 game;
    assert(game2048_init(&game, 6U, UINT32_C(1)));
    game.board.cells[5][5] = 8U;
    assert(game2048_init(&game, 4U, UINT32_MAX));
    assert(game.board.cells[5][5] == 0U);
    const Game2048 before = game;
    static const unsigned int invalid[] = {0U, 3U, 5U, 7U, UINT_MAX};
    for (unsigned int i = 0U; i < 5U; ++i) {
        assert(!game2048_init(&game, invalid[i], UINT32_C(0)));
        assert_equal(&game, &before);
    }
    assert(!game2048_init(NULL, 4U, UINT32_C(0)));
    game2048_process_action(NULL, GAME2048_ACTION_RESTART);
}

int main(void)
{
    test_initialization();
    test_moves_and_no_ops();
    test_win_loss_continue();
    test_restart_and_replay();
    test_validation();
    (void)puts("2048 game tests passed (spawning, score, win/loss, continue, restart, replay).");
    return 0;
}

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#include "game2048/board.h"

static void assert_board(const Game2048Board *a, const Game2048Board *b)
{
    assert(a->size == b->size);
    for (int y = 0; y < GAME2048_MAX_SIZE; ++y) {
        for (int x = 0; x < GAME2048_MAX_SIZE; ++x) {
            assert(a->cells[y][x] == b->cells[y][x]);
        }
    }
}

/* Test vectors are expressed from the destination edge inward. */
static void set_line(Game2048Board *board, Game2048Direction direction,
                     unsigned int line, const Game2048Tile *values)
{
    for (unsigned int i = 0U; i < board->size; ++i) {
        switch (direction) {
        case GAME2048_LEFT:  board->cells[line][i] = values[i]; break;
        case GAME2048_RIGHT: board->cells[line][board->size - 1U - i] = values[i]; break;
        case GAME2048_UP:    board->cells[i][line] = values[i]; break;
        case GAME2048_DOWN:  board->cells[board->size - 1U - i][line] = values[i]; break;
        }
    }
}

static void test_moves(void)
{
    static const struct {
        Game2048Tile input[6];
        Game2048Tile expected[6];
        uint64_t score;
        bool changed;
        unsigned int minimum_size;
    } cases[] = {
        {{0}, {0}, 0U, false, 4U},
        {{1, 2, 3, 4}, {1, 2, 3, 4}, 0U, false, 4U},
        {{0, 0, 0, 1}, {1}, 0U, true, 4U},
        {{1, 1, 1, 1}, {2, 2}, 8U, true, 4U},
        {{1, 1, 2}, {2, 2}, 4U, true, 4U},
        {{1, 0, 1, 1}, {2, 1}, 4U, true, 4U},
        {{1, 1, 2, 2}, {2, 3}, 12U, true, 4U},
        {{2, 1, 1}, {2, 2}, 4U, true, 4U},
        {{1, 2, 2, 1}, {1, 3, 1}, 8U, true, 4U},
        {{63, 63}, {63, 63}, 0U, false, 4U},
        {{0, 63, 0, 63}, {63, 63}, 0U, true, 4U},
        {{62, 62}, {63}, UINT64_C(1) << 63, true, 4U},
        {{62, 62, 62, 62}, {63, 63}, UINT64_MAX, true, 4U},
        {{1, 1, 1, 1, 1, 1}, {2, 2, 2}, 12U, true, 6U},
        {{1, 1, 2, 2, 3, 3}, {2, 3, 4}, 28U, true, 6U},
        {{1, 0, 0, 0, 0, 1}, {2}, 4U, true, 6U}
    };
    for (unsigned int size = 4U; size <= 6U; size += 2U) {
        for (int direction = GAME2048_LEFT; direction <= GAME2048_DOWN; ++direction) {
            for (unsigned int line = 0U; line < size; ++line) {
                for (size_t test = 0U; test < sizeof(cases) / sizeof(cases[0]); ++test) {
                    if (cases[test].minimum_size > size) {
                        continue;
                    }
                    Game2048Board board;
                    assert(game2048_board_init(&board, size));
                    /* Inactive rows/columns must neither merge nor get erased. */
                    for (unsigned int y = 0U; y < 6U; ++y) {
                        for (unsigned int x = 0U; x < 6U; ++x) {
                            if (y >= size || x >= size) {
                                board.cells[y][x] = 60U;
                            }
                        }
                    }
                    Game2048Board expected = board;
                    set_line(&board, (Game2048Direction)direction, line, cases[test].input);
                    set_line(&expected, (Game2048Direction)direction, line, cases[test].expected);
                    const Game2048MoveResult result =
                        game2048_board_move(&board, (Game2048Direction)direction);
                    assert(result.changed == cases[test].changed);
                    assert(result.score_gained == cases[test].score);
                    assert_board(&board, &expected);
                }
            }
        }
    }
}

static void test_direction_and_score(void)
{
    /* Explicit coordinates independently check reversal/orientation. */
    Game2048Board board;
    assert(game2048_board_init(&board, 4U));
    board.cells[0][0] = 1U;
    board.cells[0][1] = 1U;
    board.cells[0][2] = 2U;
    Game2048MoveResult result = game2048_board_move(&board, GAME2048_RIGHT);
    assert(result.score_gained == 4U);
    assert(board.cells[0][0] == 0U && board.cells[0][1] == 0U);
    assert(board.cells[0][2] == 2U && board.cells[0][3] == 2U);
    result = game2048_board_move(&board, GAME2048_RIGHT);
    assert(result.changed && result.score_gained == 8U);
    assert(board.cells[0][2] == 0U && board.cells[0][3] == 3U);

    assert(game2048_board_init(&board, 6U));
    board.cells[0][2] = 1U;
    board.cells[2][2] = 1U;
    board.cells[4][2] = 2U;
    result = game2048_board_move(&board, GAME2048_DOWN);
    assert(result.score_gained == 4U);
    assert(board.cells[5][2] == 2U && board.cells[4][2] == 2U);
    for (int y = 0; y < 4; ++y) {
        assert(board.cells[y][2] == 0U);
    }

    assert(game2048_board_init(&board, 4U));
    board.cells[0][0] = 1U;
    board.cells[0][1] = 1U;
    board.cells[1][0] = 2U;
    board.cells[1][1] = 2U;
    result = game2048_board_move(&board, GAME2048_LEFT);
    assert(result.score_gained == 12U);
    board.cells[0][0] = 62U;
    board.cells[0][1] = 62U;
    board.cells[1][0] = 62U;
    board.cells[1][1] = 62U;
    result = game2048_board_move(&board, GAME2048_LEFT);
    assert(result.score_gained == UINT64_MAX);
}

static void test_available_moves(void)
{
    for (unsigned int size = 4U; size <= 6U; size += 2U) {
        Game2048Board board;
        assert(game2048_board_init(&board, size));
        assert(!game2048_board_has_moves(&board));
        for (unsigned int y = 0U; y < size; ++y) {
            for (unsigned int x = 0U; x < size; ++x) {
                board.cells[y][x] = (Game2048Tile)(1U + (x + y) % 2U);
            }
        }
        const Game2048Board blocked = board;
        assert(!game2048_board_has_moves(&board));
        assert_board(&board, &blocked);

        board.cells[size - 1U][size - 1U] = 0U;
        Game2048Board before = board;
        assert(game2048_board_has_moves(&board));
        assert_board(&board, &before);

        board = blocked;
        board.cells[0][0] = board.cells[0][1]; /* Horizontal pair. */
        before = board;
        assert(game2048_board_has_moves(&board));
        assert_board(&board, &before);
        board = blocked;
        board.cells[0][0] = board.cells[1][0]; /* Vertical pair. */
        assert(game2048_board_has_moves(&board));

        for (unsigned int y = 0U; y < size; ++y) {
            for (unsigned int x = 0U; x < size; ++x) {
                board.cells[y][x] = GAME2048_MAX_EXPONENT;
            }
        }
        assert(!game2048_board_has_moves(&board));
    }
}

static void test_invalid_inputs(void)
{
    Game2048Board board;
    assert(game2048_board_init(&board, 4U));
    board.cells[0][0] = 1U;
    board.cells[0][1] = 1U;
    const Game2048Board valid = board;
    static const Game2048Direction invalid[] = {
        (Game2048Direction)-1, (Game2048Direction)4, (Game2048Direction)INT_MAX
    };
    for (size_t i = 0U; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        const Game2048MoveResult result = game2048_board_move(&board, invalid[i]);
        assert(!result.changed && result.score_gained == 0U);
        assert_board(&board, &valid);
    }
    board.cells[3][3] = 64U; /* Validate entire board before any mutation. */
    Game2048Board before = board;
    Game2048MoveResult result = game2048_board_move(&board, GAME2048_LEFT);
    assert(!result.changed && result.score_gained == 0U);
    assert_board(&board, &before);
    assert(!game2048_board_has_moves(&board));
    board = valid;
    board.size = UINT_MAX;
    before = board;
    result = game2048_board_move(&board, GAME2048_LEFT);
    assert(!result.changed && result.score_gained == 0U);
    assert_board(&board, &before);
    assert(!game2048_board_has_moves(&board));
    result = game2048_board_move(NULL, GAME2048_LEFT);
    assert(!result.changed && result.score_gained == 0U);
    assert(!game2048_board_has_moves(NULL));
}

int main(void)
{
    test_moves();
    test_direction_and_score();
    test_available_moves();
    test_invalid_inputs();
    (void)puts("2048 board tests passed (4x4/6x6, directions, merges, score, limits, moves).");
    return 0;
}

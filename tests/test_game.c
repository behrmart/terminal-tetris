#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "game.h"

static void assert_piece(const Piece *a, const Piece *b)
{
    assert(a->type == b->type);
    assert(a->rotation == b->rotation);
    assert(a->x == b->x);
    assert(a->y == b->y);
}

static void assert_game(const Game *a, const Game *b)
{
    assert_piece(&a->active, &b->active);
    assert(a->has_active_piece == b->has_active_piece);
    assert(a->next == b->next);
    assert(a->score == b->score);
    assert(a->lines == b->lines);
    assert(a->level == b->level);
    assert(a->gravity_elapsed_ms == b->gravity_elapsed_ms);
    assert(a->status == b->status);
    assert(a->initial_seed == b->initial_seed);
    assert(a->random_state == b->random_state);
    assert(a->bag_index == b->bag_index);
    for (int i = 0; i < TETROMINO_COUNT; ++i) {
        assert(a->bag[i] == b->bag[i]);
    }
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            assert(a->board.cells[y][x] == b->board.cells[y][x]);
        }
    }
}

static int locked_cells(const Game *game)
{
    int count = 0;
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (game->board.cells[y][x] != CELL_EMPTY) {
                ++count;
            }
        }
    }
    return count;
}

static void fixture(Game *game, TetrominoType type, int x, int y)
{
    game_init(game, UINT32_C(42));
    game->active = (Piece){type, 0U, x, y};
    assert(board_can_place(&game->board, &game->active));
}

static void test_initialization_and_bags(void)
{
    static const uint32_t seeds[] = {UINT32_C(0), UINT32_C(42), UINT32_MAX};
    for (unsigned int seed = 0U; seed < 3U; ++seed) {
        Game game;
        Game duplicate;
        Game independent;
        game_init(&game, seeds[seed]);
        game_init(&duplicate, seeds[seed]);
        game_init(&independent, UINT32_C(123));
        assert_game(&game, &duplicate);
        assert(game.status == GAME_RUNNING);
        assert(game.has_active_piece);
        assert(game.active.rotation == 0U);
        assert(game.active.x == 3 && game.active.y == 0);
        assert(board_can_place(&game.board, &game.active));
        assert(locked_cells(&game) == 0);
        assert(game.score == 0U && game.lines == 0U && game.level == 1U);
        assert(game.gravity_elapsed_ms == 0U);
        assert(game.bag_index == 2U);

        /* Consume twenty full bags, including preview crossing bag boundaries.
         * Clearing the fixture board isolates bag generation from board buildup.
         */
        for (int bag = 0; bag < 20; ++bag) {
            unsigned int counts[TETROMINO_COUNT] = {0U};
            for (int i = 0; i < TETROMINO_COUNT; ++i) {
                assert((unsigned int)game.active.type <
                       (unsigned int)TETROMINO_COUNT);
                ++counts[game.active.type];
                const TetrominoType expected_next = game.next;
                board_init(&game.board);
                board_init(&duplicate.board);
                board_init(&independent.board);
                game_process_action(&game, ACTION_HARD_DROP);
                /* Interleave another seed to expose accidental global RNG. */
                game_process_action(&independent, ACTION_HARD_DROP);
                game_process_action(&duplicate, ACTION_HARD_DROP);
                assert_game(&game, &duplicate);
                assert(game.active.type == expected_next);
                assert(game.has_active_piece && game.status == GAME_RUNNING);
                assert(locked_cells(&game) == TETROMINO_CELL_COUNT);
            }
            for (int i = 0; i < TETROMINO_COUNT; ++i) {
                assert(counts[i] == 1U);
            }
        }
        game_init(&game, seeds[seed]);
        game_init(&duplicate, seeds[seed]);
        assert_game(&game, &duplicate); /* Reinitialization resets dirty state. */
        assert(locked_cells(&game) == 0);
    }

    Game zero;
    Game other;
    game_init(&zero, UINT32_C(0));
    game_init(&other, UINT32_C(42));
    bool different = false;
    for (int i = 0; i < TETROMINO_COUNT; ++i) {
        if (zero.bag[i] != other.bag[i]) {
            different = true;
        }
    }
    assert(different);
}

static void test_horizontal_movement(void)
{
    Game game;
    fixture(&game, TETROMINO_O, 3, 5);
    game_process_action(&game, ACTION_LEFT);
    assert(game.active.x == 2);
    game_process_action(&game, ACTION_RIGHT);
    assert(game.active.x == 3);
    assert(locked_cells(&game) == 0);

    for (int i = 0; i < BOARD_WIDTH; ++i) {
        game_process_action(&game, ACTION_LEFT);
    }
    assert(game.active.x == -1); /* O occupies offsets 1 and 2. */
    Game before = game;
    game_process_action(&game, ACTION_LEFT);
    assert_game(&game, &before);
    for (int i = 0; i < BOARD_WIDTH; ++i) {
        game_process_action(&game, ACTION_RIGHT);
    }
    assert(game.active.x == BOARD_WIDTH - 3);
    before = game;
    game_process_action(&game, ACTION_RIGHT);
    assert_game(&game, &before);

    fixture(&game, TETROMINO_O, 3, 5);
    game.board.cells[5][3] = CELL_J;
    before = game;
    game_process_action(&game, ACTION_LEFT);
    assert_game(&game, &before);
    game.board.cells[5][6] = CELL_Z;
    before = game;
    game_process_action(&game, ACTION_RIGHT);
    assert_game(&game, &before);
}

static void test_rotation(void)
{
    Game game;
    for (int type = 0; type < TETROMINO_COUNT; ++type) {
        fixture(&game, (TetrominoType)type, 3, 5);
        const Game before = game;
        for (unsigned int i = 1U; i <= 4U; ++i) {
            game_process_action(&game, ACTION_ROTATE_CW);
            assert(game.active.rotation == i % 4U);
        }
        assert_game(&game, &before);
        game_process_action(&game, ACTION_ROTATE_CCW);
        assert(game.active.rotation == 3U);
        game_process_action(&game, ACTION_ROTATE_CW);
        assert_game(&game, &before);
        for (unsigned int i = 0U; i < 4U; ++i) {
            game_process_action(&game, ACTION_ROTATE_CCW);
        }
        assert_game(&game, &before);
    }

    fixture(&game, TETROMINO_I, 3, 5);
    game.board.cells[5][5] = CELL_T;
    Game before = game;
    game_process_action(&game, ACTION_ROTATE_CW);
    assert_game(&game, &before);
    game.board.cells[5][4] = CELL_T;
    before = game;
    game_process_action(&game, ACTION_ROTATE_CCW);
    assert_game(&game, &before);

    fixture(&game, TETROMINO_I, 3, 5);
    game.active = (Piece){TETROMINO_I, 1U, -2, 5};
    assert(board_can_place(&game.board, &game.active));
    before = game;
    game_process_action(&game, ACTION_ROTATE_CW);
    assert_game(&game, &before); /* No wall kick. */

    fixture(&game, TETROMINO_I, 3, 18);
    before = game;
    game_process_action(&game, ACTION_ROTATE_CW);
    assert_game(&game, &before); /* Cannot rotate through the floor. */
}

static void test_drops(void)
{
    Game game;
    fixture(&game, TETROMINO_O, 3, 17);
    const TetrominoType next = game.next;
    game_process_action(&game, ACTION_SOFT_DROP);
    assert(game.active.y == 18);
    assert(locked_cells(&game) == 0); /* Landing alone does not lock. */
    game_process_action(&game, ACTION_SOFT_DROP);
    assert(locked_cells(&game) == 4);
    assert(game.board.cells[18][4] == CELL_O);
    assert(game.board.cells[18][5] == CELL_O);
    assert(game.board.cells[19][4] == CELL_O);
    assert(game.board.cells[19][5] == CELL_O);
    assert(game.active.type == next);
    assert(game.active.y == 0 && game.active.rotation == 0U);

    fixture(&game, TETROMINO_O, 3, 0);
    const TetrominoType hard_next = game.next;
    game_process_action(&game, ACTION_HARD_DROP);
    assert(locked_cells(&game) == 4);
    assert(game.board.cells[18][4] == CELL_O);
    assert(game.board.cells[19][5] == CELL_O);
    assert(game.active.type == hard_next && game.active.y == 0);

    fixture(&game, TETROMINO_O, 3, 0);
    game.board.cells[10][4] = CELL_J;
    game_process_action(&game, ACTION_HARD_DROP);
    assert(locked_cells(&game) == 5);
    assert(game.board.cells[8][4] == CELL_O);
    assert(game.board.cells[8][5] == CELL_O);
    assert(game.board.cells[9][4] == CELL_O);
    assert(game.board.cells[9][5] == CELL_O);
    assert(game.board.cells[10][4] == CELL_J);

    /* Soft drop also locks on existing blocks, not just on the floor. */
    fixture(&game, TETROMINO_O, 3, 8);
    game.board.cells[10][4] = CELL_J;
    game_process_action(&game, ACTION_SOFT_DROP);
    assert(game.active.y == 0);
    assert(locked_cells(&game) == 5);
    assert(game.board.cells[9][4] == CELL_O);

    fixture(&game, TETROMINO_I, 3, 0);
    for (int x = 0; x < BOARD_WIDTH; ++x) {
        if (x < 3 || x > 6) {
            game.board.cells[19][x] = CELL_L;
        }
    }
    game_process_action(&game, ACTION_HARD_DROP);
    for (int x = 0; x < BOARD_WIDTH; ++x) {
        assert(game.board.cells[19][x] == CELL_EMPTY);
    }
    assert(game.lines == 1U && game.score == 100U && game.level == 1U);
}

static void test_game_over_and_no_ops(void)
{
    Game game;
    fixture(&game, TETROMINO_O, -1, 5);
    game.next = TETROMINO_O;
    game.board.cells[0][4] = CELL_T;
    game_process_action(&game, ACTION_HARD_DROP);
    assert(game.status == GAME_OVER);
    assert(!game.has_active_piece);
    assert(locked_cells(&game) == 5);
    assert(game.board.cells[0][4] == CELL_T);
    const Game over = game;
    for (int action = ACTION_NONE; action <= ACTION_QUIT; ++action) {
        if (action == ACTION_RESTART) {
            continue;
        }
        game_process_action(&game, (Action)action);
        assert_game(&game, &over);
    }
    game_update(&game, UINT_MAX);
    assert_game(&game, &over);

    game_init(&game, UINT32_C(0));
    const Game running = game;
    static const Action ignored[] = {
        ACTION_NONE, ACTION_QUIT, (Action)-1
    };
    for (unsigned int i = 0U; i < 3U; ++i) {
        game_process_action(&game, ignored[i]);
        assert_game(&game, &running);
    }
    game_update(&game, 0U);
    assert_game(&game, &running);
    game.status = GAME_PAUSED; /* Paused games ignore movement and time. */
    const Game paused = game;
    game_process_action(&game, ACTION_HARD_DROP);
    game_update(&game, UINT_MAX);
    assert_game(&game, &paused);
}

static void test_action_replay(void)
{
    static const Action sequence[] = {
        ACTION_LEFT, ACTION_ROTATE_CW, ACTION_SOFT_DROP, ACTION_RIGHT,
        ACTION_ROTATE_CCW, ACTION_HARD_DROP
    };
    Game first;
    Game second;
    game_init(&first, UINT32_C(987));
    game_init(&second, UINT32_C(987));
    for (unsigned int i = 0U; i < 120U; ++i) {
        if (i % 6U == 0U) {
            board_init(&first.board);
            board_init(&second.board);
        }
        game_process_action(&first, sequence[i % 6U]);
        game_update(&first, i * 11U);
        game_process_action(&second, sequence[i % 6U]);
        game_update(&second, i * 11U);
        assert_game(&first, &second);
        assert(first.status == GAME_RUNNING);
    }
}

static void test_gravity(void)
{
    Game game;
    fixture(&game, TETROMINO_O, 3, 0);
    assert(game_gravity_interval_ms(&game) == 1000U);
    for (int i = 0; i < 9; ++i) {
        game_update(&game, 100U);
    }
    assert(game.active.y == 0 && game.gravity_elapsed_ms == 900U);
    game_update(&game, 99U);
    assert(game.active.y == 0);
    game_update(&game, 1U);
    assert(game.active.y == 1 && game.gravity_elapsed_ms == 0U);
    game_update(&game, UINT_MAX);
    assert(game.active.y == 1 && game.gravity_elapsed_ms == 100U);
    game_update(&game, 0U);
    assert(game.active.y == 1 && game.gravity_elapsed_ms == 100U);

    for (unsigned int level = 1U; level <= 12U; ++level) {
        game.level = level;
        game.lines = (level - 1U) * 10U;
        assert(game_gravity_interval_ms(&game) ==
               (level < 10U ? 1000U - (level - 1U) * 100U : 100U));
    }
    game.gravity_elapsed_ms = 99U;
    const int old_y = game.active.y;
    game_update(&game, UINT_MAX);
    assert(game.active.y == old_y + 1 && game.gravity_elapsed_ms == 99U);
    game_update(&game, 0U);
    assert(game.active.y == old_y + 1); /* No backlog from the stall. */

    game.active.y = 18;
    const TetrominoType next = game.next;
    game_update(&game, UINT_MAX);
    assert(locked_cells(&game) == 4);
    assert(game.active.type == next && game.active.y == 0);
    assert(game.gravity_elapsed_ms == 0U); /* No time inherited by next piece. */
}

static void test_pause_restart(void)
{
    Game game;
    Game fresh;
    game_init(&game, UINT32_C(0));
    game_init(&fresh, UINT32_C(0));
    game_update(&game, 90U);
    game_process_action(&game, ACTION_PAUSE);
    assert(game.status == GAME_PAUSED && game.gravity_elapsed_ms == 0U);
    const Game paused = game;
    for (int action = ACTION_NONE; action <= ACTION_ROTATE_CCW; ++action) {
        game_process_action(&game, (Action)action);
        assert_game(&game, &paused);
    }
    game_update(&game, UINT_MAX);
    assert_game(&game, &paused);
    game_process_action(&game, ACTION_PAUSE);
    assert_game(&game, &fresh);
    game_update(&game, 100U);
    assert(game.active.y == 0 && game.gravity_elapsed_ms == 100U);

    for (int status = GAME_RUNNING; status <= GAME_OVER; ++status) {
        game_process_action(&game, ACTION_HARD_DROP);
        game.status = (GameStatus)status;
        game.score = 12345U;
        game.lines = 24U;
        game.level = 3U;
        game.gravity_elapsed_ms = 42U;
        game_process_action(&game, ACTION_RESTART);
        assert_game(&game, &fresh);
    }
}

static void line_fixture(Game *game, unsigned int rows)
{
    fixture(game, TETROMINO_I, 2, 0);
    game->active.rotation = 1U; /* Vertical I in column 4. */
    for (int y = BOARD_HEIGHT - (int)rows; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (x != 4) {
                game->board.cells[y][x] = CELL_J;
            }
        }
    }
}

static void test_scoring_and_levels(void)
{
    static const uint64_t awards[] = {0U, 100U, 300U, 500U, 800U};
    Game game;
    for (unsigned int rows = 1U; rows <= 4U; ++rows) {
        line_fixture(&game, rows);
        game.lines = 19U;
        game.level = 2U;
        game.score = 50U;
        game_process_action(&game, ACTION_HARD_DROP);
        assert(game.score == 50U + awards[rows] * 2U);
        assert(game.lines == 19U + rows && game.level == 3U);
        assert(locked_cells(&game) == 4 - (int)rows);
        assert(game.status == GAME_RUNNING);
        assert(game_gravity_interval_ms(&game) == 800U);
    }
    line_fixture(&game, 1U);
    game.lines = 8U;
    game_process_action(&game, ACTION_HARD_DROP);
    assert(game.lines == 9U && game.level == 1U);
    line_fixture(&game, 1U);
    game.lines = 9U;
    game_process_action(&game, ACTION_HARD_DROP);
    assert(game.lines == 10U && game.level == 2U && game.score == 100U);

    line_fixture(&game, 4U);
    game.active.y = 16;
    game.gravity_elapsed_ms = 900U;
    game_update(&game, 100U);
    assert(game.lines == 4U && game.score == 800U);
    assert(locked_cells(&game) == 0 && game.active.y == 0);
    assert(game.gravity_elapsed_ms == 0U);

    line_fixture(&game, 4U);
    game.lines = UINT_MAX - 1U;
    game.level = game.lines / 10U + 1U;
    game.score = UINT64_MAX - 1U;
    game_process_action(&game, ACTION_HARD_DROP);
    assert(game.lines == UINT_MAX && game.score == UINT64_MAX);
    assert(game.level == UINT_MAX / 10U + 1U);

    /* Clear before checking the next spawn; a rescued board is not game over. */
    fixture(&game, TETROMINO_I, 2, 0);
    game.active.rotation = 1U;
    game.next = TETROMINO_O;
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (x != 4) {
                game.board.cells[y][x] = CELL_L;
            }
        }
    }
    game.board.cells[4][4] = CELL_T;
    game_process_action(&game, ACTION_SOFT_DROP);
    assert(game.lines == 4U && game.score == 800U);
    assert(game.status == GAME_RUNNING && game.has_active_piece);
    assert(game.active.type == TETROMINO_O);
}

int main(void)
{
    test_initialization_and_bags();
    test_horizontal_movement();
    test_rotation();
    test_drops();
    test_game_over_and_no_ops();
    test_action_replay();
    test_gravity();
    test_pause_restart();
    test_scoring_and_levels();
    (void)puts("Game tests passed (movement, bags, gravity, scoring, pause, restart, replay).");
    return 0;
}

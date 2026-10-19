#include <assert.h>
#include <limits.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>

#include "display.h"
#include "input.h"

static void expect_text(int row, int column, const char *text)
{
    for (int i = 0; text[i] != 0; ++i) {
        assert((mvinch(row, column + i) & A_CHARTEXT) ==
               (chtype)(unsigned char)text[i]);
    }
}

static void test_rendering(void)
{
    Game game;
    game_init(&game, UINT32_C(1));
    for (int type = 0; type < TETROMINO_COUNT; ++type) {
        board_init(&game.board);
        game.active = (Piece){(TetrominoType)type, 0U, 3, 0};
        game.next = (TetrominoType)type;
        game.board.cells[19][0] = CELL_Z;
        const TetrominoCell *cells = tetromino_cells(game.active.type, 0U);
        display_render(&game);
        expect_text(0, 2, "TERMINAL TETRIS");
        expect_text(2, 2, "+--------------------+");
        expect_text(23, 2, "+--------------------+");
        expect_text(2, 27, "NEXT");
        expect_text(9, 27, "SCORE");
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            expect_text(y + 3, 2, "|");
            expect_text(y + 3, 23, "|");
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                bool occupied = x == 0 && y == 19;
                for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                    if (x == 3 + cells[i].x && y == cells[i].y) {
                        occupied = true;
                    }
                }
                expect_text(y + 3, 3 + x * 2, occupied ? "[]" : "  ");
            }
        }
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                bool occupied = false;
                for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                    if (x == cells[i].x && y == cells[i].y) {
                        occupied = true;
                    }
                }
                expect_text(4 + y, 27 + x * 2, occupied ? "[]" : "  ");
            }
        }
        assert(game.active.type == (TetrominoType)type);
        assert(game.active.x == 3 && game.active.y == 0);
        assert(game.board.cells[19][0] == CELL_Z);
    }

    game.score = UINT64_MAX;
    game.lines = UINT_MAX;
    game.level = UINT_MAX;
    game.status = GAME_PAUSED;
    display_render(&game);
    expect_text(10, 27, "18446744073709551615");
    expect_text(15, 27, "PAUSED");
    char expected[32];
    (void)snprintf(expected, sizeof(expected), "LINES: %u", UINT_MAX);
    expect_text(12, 27, expected);
    (void)snprintf(expected, sizeof(expected), "LEVEL: %u", UINT_MAX);
    expect_text(13, 27, expected);
    game.status = GAME_OVER;
    game.has_active_piece = false;
    display_render(&game);
    expect_text(15, 27, "GAME OVER");
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            expect_text(y + 3, 3 + x * 2, x == 0 && y == 19 ? "[]" : "  ");
        }
    }
}

static void test_input(void)
{
    static const struct {
        int key;
        Action action;
    } bindings[] = {
        {KEY_LEFT, ACTION_LEFT}, {KEY_RIGHT, ACTION_RIGHT},
        {KEY_DOWN, ACTION_SOFT_DROP}, {KEY_UP, ACTION_ROTATE_CW},
        {'x', ACTION_ROTATE_CW}, {'X', ACTION_ROTATE_CW},
        {'z', ACTION_ROTATE_CCW}, {'Z', ACTION_ROTATE_CCW},
        {' ', ACTION_HARD_DROP}, {'p', ACTION_PAUSE}, {'P', ACTION_PAUSE},
        {'r', ACTION_RESTART}, {'R', ACTION_RESTART},
        {'q', ACTION_QUIT}, {'Q', ACTION_QUIT},
        {KEY_RESIZE, ACTION_NONE}, {'?', ACTION_NONE}
    };
    for (unsigned int i = 0U; i < sizeof(bindings) / sizeof(bindings[0]); ++i) {
        assert(ungetch(bindings[i].key) != ERR);
        assert(input_poll() == bindings[i].action);
    }
}

int main(void)
{
    Game game;
    game_init(&game, UINT32_C(1));
    assert(display_init());
    assert(resizeterm(DISPLAY_MIN_ROWS, DISPLAY_MIN_COLUMNS) != ERR);
    assert(display_can_fit());
    test_rendering();
    test_input();
    bool main_menu = false;
    assert(ungetch('m') != ERR);
    assert(input_poll_navigation(&main_menu) == ACTION_NONE && main_menu);
    assert(ungetch('M') != ERR);
    assert(input_poll_navigation(&main_menu) == ACTION_NONE && main_menu);
    assert(ungetch('?') != ERR);
    assert(input_poll_navigation(&main_menu) == ACTION_NONE && !main_menu);
    assert(resizeterm(2, 40) != ERR);
    assert(!display_can_fit());
    display_render(&game);
    expect_text(0, 0, "Need 54x26 - game frozen.");
    assert(resizeterm(1, 1) != ERR);
    assert(!display_can_fit());
    display_render(&game);
    assert(resizeterm(DISPLAY_MIN_ROWS, DISPLAY_MIN_COLUMNS) != ERR);
    assert(display_can_fit());
    display_render(&game);
    expect_text(15, 27, "RUNNING");
    display_shutdown();
    display_shutdown();
    (void)puts("Terminal rendering and input tests passed.");
    return 0;
}

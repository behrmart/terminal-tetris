#include <assert.h>
#include <ncurses.h>
#include <stdio.h>

#include "game2048/display.h"
#include "game2048/input.h"
#include "menu_theme.h"

static void expect_text(int row, int column, const char *text)
{
    for (int i = 0; text[i] != 0; ++i) {
        assert((mvinch(row, column + i) & A_CHARTEXT) ==
               (chtype)(unsigned char)text[i]);
    }
}

static void test_rendering(unsigned int size)
{
    Game2048 game;
    assert(game2048_init(&game, size, UINT32_C(1)));
    assert(game2048_board_init(&game.board, size));
    game.board.cells[0][0] = 1U;
    game.board.cells[0][1] = 63U;
    game.score = UINT64_MAX;
    const int columns = (int)game2048_display_min_columns(size);
    const int rows = (int)game2048_display_min_rows(size);
    assert(resizeterm(rows, columns) != ERR);
    assert(game2048_display_can_fit(size));
    game2048_display_render(&game);
    const int board_left = (columns - ((int)size * 8 + 1)) / 2;
    expect_text(0, 1, "TERMINAL 2048");
    expect_text(1, 1, "Score: 18446744073709551615");
    expect_text(4, board_left, "+-------+");
    expect_text(5, board_left + 4, "2");
    expect_text(5, board_left + 10, "2^63");
    expect_text(4 + (int)size * 3, board_left, "+-------+");
    expect_text(5 + (int)size * 3, 1, "RUNNING");
    assert(game.board.cells[0][0] == 1U && game.board.cells[0][1] == 63U);
    assert(game.score == UINT64_MAX);
    game.status = GAME2048_WON;
    game2048_display_render(&game);
    expect_text(5 + (int)size * 3, 1, "2048 reached! C: continue");
    game.status = GAME2048_OVER;
    game2048_display_render(&game);
    expect_text(5 + (int)size * 3, 1, "GAME OVER - R: restart");
    assert(resizeterm(rows - 1, columns) != ERR);
    assert(!game2048_display_can_fit(size));
    game2048_display_render(&game);
    expect_text(0, 0, "Need ");
    assert(resizeterm(rows, columns - 1) != ERR);
    assert(!game2048_display_can_fit(size));
    assert(resizeterm(1, 1) != ERR);
    game2048_display_render(&game);
    assert(resizeterm(rows, columns) != ERR);
    game2048_display_render(&game);
    expect_text(5 + (int)size * 3, 1, "GAME OVER - R: restart");
}

static void test_input(void)
{
    static const struct {
        int key;
        Game2048Action action;
        unsigned int size;
    } bindings[] = {
        {KEY_LEFT, GAME2048_ACTION_LEFT, 0U},
        {KEY_RIGHT, GAME2048_ACTION_RIGHT, 0U},
        {KEY_UP, GAME2048_ACTION_UP, 0U},
        {KEY_DOWN, GAME2048_ACTION_DOWN, 0U},
        {'r', GAME2048_ACTION_RESTART, 0U}, {'R', GAME2048_ACTION_RESTART, 0U},
        {'c', GAME2048_ACTION_CONTINUE, 0U}, {'C', GAME2048_ACTION_CONTINUE, 0U},
        {'q', GAME2048_ACTION_QUIT, 0U}, {'Q', GAME2048_ACTION_QUIT, 0U},
        {'4', GAME2048_ACTION_NONE, 4U}, {'6', GAME2048_ACTION_NONE, 6U},
        {10, GAME2048_ACTION_NONE, 4U}, {KEY_ENTER, GAME2048_ACTION_NONE, 4U},
        {KEY_RESIZE, GAME2048_ACTION_NONE, 0U}, {'?', GAME2048_ACTION_NONE, 0U}
    };
    for (size_t i = 0U; i < sizeof(bindings) / sizeof(bindings[0]); ++i) {
        assert(ungetch(bindings[i].key) != ERR);
        const Game2048Input input = game2048_input_poll();
        assert(input.action == bindings[i].action);
        assert(input.selected_size == bindings[i].size);
    }
}

static void dispatch_key(Game2048 *game, int key)
{
    assert(ungetch(key) != ERR);
    const Game2048Input input = game2048_input_poll();
    game2048_process_action(game, input.action);
    game2048_display_render(game);
}

/* Exercise real input -> engine -> screen transitions for both board sizes. */
static void test_playthrough(unsigned int size)
{
    Game2048 game;
    Game2048 initial;
    assert(game2048_init(&game, size, UINT32_C(42)));
    initial = game;
    assert(resizeterm((int)game2048_display_min_rows(size),
                     (int)game2048_display_min_columns(size)) != ERR);
    assert(game2048_board_init(&game.board, size));
    game.board.cells[0][0] = 10U;
    game.board.cells[0][1] = 10U;
    dispatch_key(&game, KEY_LEFT);
    assert(game.score == UINT64_C(2048));
    assert(game.board.cells[0][0] == 11U);
    assert(game.status == GAME2048_WON);
    expect_text(1, 1, "Score: 2048");
    expect_text(5 + (int)size * 3, 1, "2048 reached! C: continue");
    const uint32_t frozen_random = game.random_state;
    dispatch_key(&game, KEY_RIGHT);
    assert(game.random_state == frozen_random);
    assert(game.board.cells[0][0] == 11U);
    dispatch_key(&game, 'C');
    assert(game.status == GAME2048_RUNNING && game.win_acknowledged);
    expect_text(5 + (int)size * 3, 1, "RUNNING");

    /* A full checkerboard cannot move; acknowledging its win reveals loss. */
    for (unsigned int y = 0U; y < size; ++y) {
        for (unsigned int x = 0U; x < size; ++x) {
            game.board.cells[y][x] = (Game2048Tile)(1U + (x + y) % 2U);
        }
    }
    game.board.cells[0][0] = 11U;
    game.status = GAME2048_WON;
    game.win_acknowledged = false;
    dispatch_key(&game, 'c');
    assert(game.status == GAME2048_OVER);
    expect_text(5 + (int)size * 3, 1, "GAME OVER - R: restart");
    dispatch_key(&game, 'R');
    assert(game.status == initial.status && game.score == initial.score);
    assert(game.random_state == initial.random_state);
    assert(game.initial_seed == initial.initial_seed);
    assert(game.board.size == initial.board.size);
    assert(game.win_acknowledged == initial.win_acknowledged);
    for (unsigned int y = 0U; y < GAME2048_MAX_SIZE; ++y) {
        for (unsigned int x = 0U; x < GAME2048_MAX_SIZE; ++x) {
            assert(game.board.cells[y][x] == initial.board.cells[y][x]);
        }
    }
    expect_text(1, 1, "Score: 0");
    expect_text(5 + (int)size * 3, 1, "RUNNING");
}
int main(void)
{
    assert(game2048_display_init());
    assert(resizeterm(9, 42) != ERR);
    game2048_display_menu();
    expect_text(0, 1, "TERMINAL 2048");
    expect_text(3, 1, "4: 4x4 classic    6: 6x6");
    if (has_colors() && COLORS >= 8 && COLOR_PAIRS > 22) {
        short foreground = 0, background = 0;
        short pair = (short)PAIR_NUMBER(mvinch(0, 1));
        assert(pair_content(pair, &foreground, &background) != ERR);
        assert(foreground == COLOR_WHITE && background == COLOR_BLUE);
        pair = (short)PAIR_NUMBER(mvinch(3, 1));
        assert(pair_content(pair, &foreground, &background) != ERR);
        assert(foreground == COLOR_YELLOW && background == COLOR_BLACK);
        /* The login field is highlighted without modifying tile colors. */
        menu_theme_style(MENU_FIELD);
        (void)mvaddstr(8, 1, "player");
        pair = (short)PAIR_NUMBER(mvinch(8, 1));
        assert(pair_content(pair, &foreground, &background) != ERR);
        assert(foreground == COLOR_BLACK && background == COLOR_CYAN);
        assert(pair_content(1, &foreground, &background) != ERR);
        assert(foreground == COLOR_BLACK && background == COLOR_CYAN);
    } else {
        assert(PAIR_NUMBER(mvinch(0, 1)) == 0);
        assert((mvinch(0, 1) & A_BOLD) != 0U);
        menu_theme_style(MENU_FIELD);
        (void)mvaddstr(8, 1, "player");
        assert((mvinch(8, 1) & A_REVERSE) != 0U);
    }
    menu_theme_style(MENU_NORMAL);
    assert(getattrs(stdscr) == A_NORMAL);
    test_rendering(4U);
    test_rendering(6U);
    test_input();
    assert(ungetch('m') != ERR);
    assert(game2048_input_poll().main_menu);
    assert(ungetch('M') != ERR);
    assert(game2048_input_poll().main_menu);
    assert(ungetch('?') != ERR);
    assert(!game2048_input_poll().main_menu);
    test_playthrough(4U);
    test_playthrough(6U);
    game2048_display_shutdown();
    game2048_display_shutdown();
    (void)puts("2048 terminal rendering and input tests passed.");
    return 0;
}

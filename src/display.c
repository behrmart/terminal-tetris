#include <ncurses.h>

#include "display.h"

static bool initialized = false;
static bool colors_enabled = false;
static int previous_cursor = ERR;

bool display_init(void)
{
    if (initscr() == NULL) {
        return false;
    }
    initialized = true;
    if (cbreak() == ERR || noecho() == ERR || keypad(stdscr, TRUE) == ERR) {
        display_shutdown();
        return false;
    }
    previous_cursor = curs_set(0);
    timeout(50);
    if (has_colors() && start_color() != ERR &&
        init_pair(1, COLOR_CYAN, COLOR_BLACK) != ERR) {
        colors_enabled = true;
    }
    return true;
}

void display_render(const Game *game)
{
    int rows = 0;
    int columns = 0;
    (void)game; /* Only an empty field is drawn in Milestone 1. */

    getmaxyx(stdscr, rows, columns);
    (void)erase();
    if (rows < DISPLAY_MIN_ROWS || columns < DISPLAY_MIN_COLUMNS) {
        /* Leave the last column unused, including on one-column terminals. */
        if (rows > 0 && columns > 1) {
            (void)mvaddnstr(0, 0, "Terminal too small: need 24x26.", columns - 1);
        }
        if (rows > 1 && columns > 1) {
            (void)mvaddnstr(1, 0, "Press Q to quit", columns - 1);
        }
        (void)refresh();
        return;
    }

    const int left = (columns - (BOARD_WIDTH * 2 + 2)) / 2;
    const int top = (rows - DISPLAY_MIN_ROWS) / 2;
    const int right = left + BOARD_WIDTH * 2 + 1;
    const int bottom = top + BOARD_HEIGHT + 3;

    if (colors_enabled) {
        (void)attron(COLOR_PAIR(1));
    }
    (void)mvaddstr(top, left, "TERMINAL TETRIS");
    /* ASCII borders avoid alternate-character-set mismatches over SSH. */
    (void)mvaddch(top + 2, left, '+');
    (void)mvhline(top + 2, left + 1, '-', BOARD_WIDTH * 2);
    (void)mvaddch(top + 2, right, '+');
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        (void)mvaddch(top + 3 + y, left, '|');
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            (void)mvaddstr(top + 3 + y, left + 1 + x * 2, "  ");
        }
        (void)mvaddch(top + 3 + y, right, '|');
    }
    (void)mvaddch(bottom, left, '+');
    (void)mvhline(bottom, left + 1, '-', BOARD_WIDTH * 2);
    (void)mvaddch(bottom, right, '+');
    if (colors_enabled) {
        (void)attroff(COLOR_PAIR(1));
    }
    (void)mvaddstr(bottom + 2, left, "Press Q to quit");
    (void)refresh();
}

void display_shutdown(void)
{
    if (initialized) {
        if (previous_cursor != ERR) {
            (void)curs_set(previous_cursor);
        }
        (void)endwin();
        initialized = false;
        colors_enabled = false;
        previous_cursor = ERR;
    }
}

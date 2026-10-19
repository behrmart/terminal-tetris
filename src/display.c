#include "display.h"

#include <inttypes.h>
#include <ncurses.h>
#include <stdio.h>

static bool initialized = false;
static bool colors_enabled = false;
static int previous_cursor = ERR;

bool display_init(void)
{
    static const short colors[] = {
        COLOR_BLACK, COLOR_CYAN, COLOR_YELLOW, COLOR_MAGENTA,
        COLOR_GREEN, COLOR_RED, COLOR_BLUE, COLOR_WHITE
    };
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
    if (has_colors() && start_color() != ERR && COLORS >= 8 && COLOR_PAIRS > 7) {
        colors_enabled = true;
        for (short pair = 1; pair <= 7; ++pair) {
            if (init_pair(pair, COLOR_BLACK, colors[pair]) == ERR) {
                colors_enabled = false;
                break;
            }
        }
    }
    return true;
}

bool display_can_fit(void)
{
    return getmaxy(stdscr) >= DISPLAY_MIN_ROWS &&
           getmaxx(stdscr) >= DISPLAY_MIN_COLUMNS;
}

static void draw_cell(int row, int column, Cell cell)
{
    if (cell == CELL_EMPTY) {
        (void)mvaddstr(row, column, "  ");
        return;
    }
    if (colors_enabled) {
        (void)attron(COLOR_PAIR((int)cell));
    }
    (void)mvaddstr(row, column, "[]");
    if (colors_enabled) {
        (void)attroff(COLOR_PAIR((int)cell));
    }
}

static Cell piece_cell(TetrominoType type)
{
    static const Cell identities[TETROMINO_COUNT] = {
        CELL_I, CELL_O, CELL_T, CELL_S, CELL_Z, CELL_J, CELL_L
    };
    return identities[type];
}

void display_render(const Game *game)
{
    int rows = 0;
    int columns = 0;
    getmaxyx(stdscr, rows, columns);
    (void)erase();
    if (!display_can_fit()) {
        char message[80];
        (void)snprintf(message, sizeof(message), "Need %dx%d - game frozen.",
                       DISPLAY_MIN_COLUMNS, DISPLAY_MIN_ROWS);
        /* Avoid the bottom-right cell even on a one-column terminal. */
        if (rows > 0 && columns > 1) {
            (void)mvaddnstr(0, 0, message, columns - 1);
        }
        if (rows > 1 && columns > 1) {
            (void)mvaddnstr(1, 0, "M: main menu  Q: quit", columns - 1);
        }
        (void)refresh();
        return;
    }

    const int left = (columns - 50) / 2;
    const int top = (rows - DISPLAY_MIN_ROWS) / 2;
    const int right = left + BOARD_WIDTH * 2 + 1;
    const int bottom = top + BOARD_HEIGHT + 3;
    const int panel = left + 25;

    (void)attron(A_BOLD);
    (void)mvaddstr(top, left, "TERMINAL TETRIS");
    (void)attroff(A_BOLD);
    /* ASCII borders remain compatible with PuTTY character-set settings. */
    (void)mvaddch(top + 2, left, '+');
    (void)mvhline(top + 2, left + 1, '-', BOARD_WIDTH * 2);
    (void)mvaddch(top + 2, right, '+');
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        (void)mvaddch(top + 3 + y, left, '|');
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            draw_cell(top + 3 + y, left + 1 + x * 2, game->board.cells[y][x]);
        }
        (void)mvaddch(top + 3 + y, right, '|');
    }
    if (game->has_active_piece) {
        const TetrominoCell *cells =
            tetromino_cells(game->active.type, game->active.rotation);
        if (cells != NULL) {
            for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                const int x = game->active.x + cells[i].x;
                const int y = game->active.y + cells[i].y;
                if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
                    draw_cell(top + 3 + y, left + 1 + x * 2,
                              piece_cell(game->active.type));
                }
            }
        }
    }
    (void)mvaddch(bottom, left, '+');
    (void)mvhline(bottom, left + 1, '-', BOARD_WIDTH * 2);
    (void)mvaddch(bottom, right, '+');

    (void)mvaddstr(top + 2, panel, "NEXT");
    const TetrominoCell *preview = tetromino_cells(game->next, 0U);
    if (preview != NULL) {
        for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
            draw_cell(top + 4 + preview[i].y, panel + preview[i].x * 2,
                      piece_cell(game->next));
        }
    }
    (void)mvaddstr(top + 9, panel, "SCORE");
    (void)mvprintw(top + 10, panel, "%" PRIu64, game->score);
    (void)mvprintw(top + 12, panel, "LINES: %u", game->lines);
    (void)mvprintw(top + 13, panel, "LEVEL: %u", game->level);
    (void)attron(A_BOLD);
    const char *status = game->status == GAME_OVER ? "GAME OVER" :
                         game->status == GAME_PAUSED ? "PAUSED" : "RUNNING";
    (void)mvaddstr(top + 15, panel, status);
    (void)attroff(A_BOLD);
    (void)mvaddstr(top + 17, panel, "P: pause / resume");
    (void)mvaddstr(top + 18, panel, "R: restart");
    (void)mvaddstr(top + 19, panel, "Q: quit");
    (void)mvaddstr(top + 20, panel, "M: main menu");
    (void)mvaddstr(top + 21, panel, "Z: counterclockwise");
    (void)mvaddstr(top + 22, panel, "Up/X: clockwise");
    (void)mvaddstr(bottom + 2, left, "Arrows: move  Space: drop");
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

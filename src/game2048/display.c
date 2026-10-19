#include "game2048/display.h"

#include <inttypes.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

enum { CELL_WIDTH = 7, ROW_HEIGHT = 3 };
static bool initialized = false;
static bool colors_enabled = false;
static int previous_cursor = ERR;

unsigned int game2048_display_min_columns(unsigned int size)
{
    const unsigned int board_width = size * (CELL_WIDTH + 1U) + 1U;
    return board_width + 2U > 42U ? board_width + 2U : 42U;
}

unsigned int game2048_display_min_rows(unsigned int size)
{
    return size == 0U ? 9U : size * ROW_HEIGHT + 10U;
}

bool game2048_display_init(void)
{
    static const short backgrounds[] = {
        COLOR_BLACK, COLOR_CYAN, COLOR_GREEN, COLOR_YELLOW,
        COLOR_MAGENTA, COLOR_BLUE, COLOR_RED, COLOR_WHITE
    };
    if (initscr() == NULL) {
        return false;
    }
    initialized = true;
    if (cbreak() == ERR || noecho() == ERR || keypad(stdscr, TRUE) == ERR) {
        game2048_display_shutdown();
        return false;
    }
    previous_cursor = curs_set(0);
    timeout(50);
    if (has_colors() && start_color() != ERR && COLORS >= 8 && COLOR_PAIRS > 7) {
        colors_enabled = true;
        for (short pair = 1; pair <= 7; ++pair) {
            const short foreground =
                backgrounds[pair] == COLOR_BLUE || backgrounds[pair] == COLOR_RED ?
                COLOR_WHITE : COLOR_BLACK;
            if (init_pair(pair, foreground, backgrounds[pair]) == ERR) {
                colors_enabled = false;
                break;
            }
        }
    }
    return true;
}

bool game2048_display_can_fit(unsigned int size)
{
    return getmaxx(stdscr) >= (int)game2048_display_min_columns(size) &&
           getmaxy(stdscr) >= (int)game2048_display_min_rows(size);
}

static void draw_small(unsigned int size)
{
    char message[80];
    const int columns = getmaxx(stdscr);
    const int rows = getmaxy(stdscr);
    (void)snprintf(message, sizeof(message), "Need %ux%u - enlarge terminal.",
                   game2048_display_min_columns(size), game2048_display_min_rows(size));
    if (rows > 0 && columns > 1) {
        (void)mvaddnstr(0, 0, message, columns - 1);
    }
    if (rows > 1 && columns > 1) {
        (void)mvaddnstr(1, 0, "Input suspended. Q: quit", columns - 1);
    }
}

void game2048_display_menu(void)
{
    (void)erase();
    if (!game2048_display_can_fit(0U)) {
        draw_small(0U);
    } else {
        const int left = (getmaxx(stdscr) - 40) / 2;
        const int top = (getmaxy(stdscr) - 9) / 2;
        (void)attron(A_BOLD);
        (void)mvaddstr(top, left, "TERMINAL 2048");
        (void)attroff(A_BOLD);
        (void)mvaddstr(top + 2, left, "Choose your board:");
        (void)mvaddstr(top + 3, left, "4: 4x4 classic    6: 6x6");
        (void)mvaddstr(top + 5, left, "Enter: 4x4       Q: quit");
        (void)mvaddstr(top + 7, left, "Join equal tiles to reach 2048.");
    }
    (void)refresh();
}

static void draw_tile(int row, int column, Game2048Tile tile)
{
    char label[32] = "";
    if (tile != 0U) {
        const uint64_t value = UINT64_C(1) << tile;
        (void)snprintf(label, sizeof(label), "%" PRIu64, value);
        if (strlen(label) > CELL_WIDTH) {
            (void)snprintf(label, sizeof(label), "2^%u", (unsigned int)tile);
        }
    }
    const int pair = tile == 0U ? 0 : 1 + ((int)tile - 1) % 7;
    if (colors_enabled && pair != 0) {
        (void)attron(COLOR_PAIR(pair));
    }
    (void)mvaddstr(row, column, "       ");
    (void)mvaddstr(row + 1, column, "       ");
    if (tile != 0U) {
        const int padding = (CELL_WIDTH - (int)strlen(label)) / 2;
        (void)mvaddstr(row, column + padding, label);
    }
    if (colors_enabled && pair != 0) {
        (void)attroff(COLOR_PAIR(pair));
    }
}

void game2048_display_render(const Game2048 *game)
{
    (void)erase();
    const unsigned int size = game->board.size;
    if (!game2048_display_can_fit(size)) {
        draw_small(size);
        (void)refresh();
        return;
    }
    const int board_width = (int)size * (CELL_WIDTH + 1) + 1;
    const int board_left = (getmaxx(stdscr) - board_width) / 2;
    const int left = (getmaxx(stdscr) - (int)game2048_display_min_columns(size) + 2) / 2;
    const int top = (getmaxy(stdscr) - (int)game2048_display_min_rows(size)) / 2;
    const int board_top = top + 4;
    (void)attron(A_BOLD);
    (void)mvaddstr(top, left, "TERMINAL 2048");
    (void)attroff(A_BOLD);
    (void)mvprintw(top + 1, left, "Score: %" PRIu64, game->score);
    (void)mvprintw(top + 2, left, "%ux%u | Arrows: move", size, size);

    for (unsigned int y = 0U; y <= size; ++y) {
        const int row = board_top + (int)y * ROW_HEIGHT;
        for (unsigned int x = 0U; x < size; ++x) {
            const int column = board_left + (int)x * (CELL_WIDTH + 1);
            (void)mvaddch(row, column, '+');
            (void)mvhline(row, column + 1, '-', CELL_WIDTH);
            if (y < size) {
                (void)mvaddch(row + 1, column, '|');
                (void)mvaddch(row + 2, column, '|');
                draw_tile(row + 1, column + 1, game->board.cells[y][x]);
            }
        }
        (void)mvaddch(row, board_left + board_width - 1, '+');
        if (y < size) {
            (void)mvaddch(row + 1, board_left + board_width - 1, '|');
            (void)mvaddch(row + 2, board_left + board_width - 1, '|');
        }
    }
    const int bottom = board_top + (int)size * ROW_HEIGHT;
    const char *status = game->status == GAME2048_WON ?
                         "2048 reached! C: continue" :
                         game->status == GAME2048_OVER ? "GAME OVER - R: restart" : "RUNNING";
    (void)attron(A_BOLD);
    (void)mvaddstr(bottom + 1, left, status);
    (void)attroff(A_BOLD);
    (void)mvaddstr(bottom + 3, left, "R: restart  Q: quit");
    (void)mvaddstr(bottom + 4, left, "Large tiles use exact 2^n notation.");
    (void)refresh();
}

void game2048_display_shutdown(void)
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

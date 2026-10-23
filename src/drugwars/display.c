#include "drugwars/display.h"
#include <inttypes.h>
#include <ncurses.h>

/* Dedicated pairs leave the shared Tetris/2048 pairs (1..7) untouched. */
enum {
    DW_TEXT = 8, DW_CYAN, DW_GREEN, DW_GOLD,
    DW_RED, DW_PURPLE, DW_SELECTED, DW_TITLE
};
static bool colors_enabled = false;

void dw_display_prepare(void)
{
    static const short foreground[] = {
        COLOR_WHITE, COLOR_CYAN, COLOR_GREEN, COLOR_YELLOW,
        COLOR_RED, COLOR_MAGENTA, COLOR_BLACK, COLOR_WHITE
    };
    static const short background[] = {
        COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK,
        COLOR_BLACK, COLOR_BLACK, COLOR_CYAN, COLOR_BLUE
    };
    colors_enabled = false;
    if (!has_colors() || COLORS < 8 || COLOR_PAIRS <= DW_TITLE) {
        return;
    }
    for (short i = 0; i < 8; ++i) {
        if (init_pair((short)(DW_TEXT + i), foreground[i], background[i]) == ERR) {
            return;
        }
    }
    colors_enabled = true;
}
static void style(short pair, attr_t attributes)
{
    short color_pair = 0;
    if (colors_enabled) {
        color_pair = pair;
    }
    (void)attr_set(attributes, color_pair, NULL);
}
bool dw_display_can_fit(void)
{
    return getmaxy(stdscr) >= DW_MIN_ROWS && getmaxx(stdscr) >= DW_MIN_COLUMNS;
}
void dw_display_render(const DwGame *game, const DwUi *ui,
                       const char *username, uint64_t best)
{
    (void)attr_set(A_NORMAL, 0, NULL);
    (void)erase();
    const int columns = getmaxx(stdscr);
    if (!dw_display_can_fit()) {
        style(DW_GOLD, A_BOLD);
        if (columns > 1) {
            (void)mvaddnstr(0, 0, "Need 72x25 - M: main menu  Q: quit", columns - 1);
        }
        (void)attr_set(A_NORMAL, 0, NULL);
        (void)refresh();
        return;
    }
    const int x = (columns - DW_MIN_COLUMNS) / 2;
    const int y = (getmaxy(stdscr) - DW_MIN_ROWS) / 2;
    style(DW_TITLE, A_BOLD);
    (void)mvhline(y, x, ' ', 70);
    (void)mvaddstr(y, x, "TERMINAL DRUG WARS");
    style(DW_CYAN, A_BOLD);
    (void)mvprintw(y + 1, x, "Player: %-16s ", username);
    style(DW_GOLD, A_BOLD);
    (void)printw("Best: %" PRIu64, best);
    style(DW_TEXT, A_NORMAL);
    (void)mvprintw(y + 2, x, "Day %u/30  %-14s ", game->day, dw_places[game->place]);
    style(game->health <= 30U ? DW_RED : game->health <= 60U ? DW_GOLD : DW_GREEN,
          A_BOLD);
    (void)printw("Health: %u  ", game->health);
    const unsigned int used = dw_used(game);
    style(used >= game->capacity ? DW_GOLD : DW_CYAN, A_BOLD);
    (void)printw("Stock: %u/%u", used, game->capacity);
    style(DW_GREEN, A_BOLD);
    (void)mvprintw(y + 3, x, "Cash: %" PRIu64 "  ", game->cash);
    style(DW_CYAN, A_BOLD);
    (void)printw("Bank: %" PRIu64 "  ", game->bank);
    style(game->debt == 0U ? DW_GREEN : DW_RED, A_BOLD);
    (void)printw("Debt: %" PRIu64, game->debt);
    style(DW_GOLD, A_BOLD);
    (void)mvprintw(y + 4, x, "Net worth: %" PRIu64, dw_net_worth(game));
    style(DW_TEXT, A_NORMAL);
    (void)addstr("  (negative net worth shown as 0)");
    style(DW_CYAN, A_NORMAL);
    (void)mvhline(y + 5, x, '-', 70);
    style(DW_PURPLE, A_BOLD);
    (void)mvaddstr(y + 6, x, "   Goods          Price    Owned");
    (void)mvaddstr(y + 6, x + 42, "Destinations (T, then 1-6)");
    for (unsigned int i = 0U; i < DW_GOODS; ++i) {
        const bool selected = i == ui->selected;
        if (selected) {
            style(DW_SELECTED, colors_enabled ? A_BOLD : A_REVERSE);
            (void)mvprintw(y + 7 + (int)i, x, ">%u %-12s %8u %7u",
                          i + 1U, dw_goods[i], game->prices[i], game->inventory[i]);
        } else {
            style(DW_TEXT, A_NORMAL);
            (void)mvprintw(y + 7 + (int)i, x, " %u %-12s ", i + 1U, dw_goods[i]);
            style(DW_GOLD, A_NORMAL);
            (void)printw("%8u ", game->prices[i]);
            style(game->inventory[i] != 0U ? DW_GREEN : DW_TEXT,
                  game->inventory[i] != 0U ? A_BOLD : A_NORMAL);
            (void)printw("%7u", game->inventory[i]);
        }
        style(i == game->place ? DW_CYAN : DW_TEXT,
              i == game->place ? A_BOLD : A_NORMAL);
        (void)mvprintw(y + 7 + (int)i, x + 42, "%u: %s%s", i + 1U,
                      dw_places[i], i == game->place ? " *" : "");
    }
    style(DW_CYAN, A_NORMAL);
    (void)mvhline(y + 13, x, '-', 70);
    style(DW_CYAN, A_BOLD);
    (void)mvaddnstr(y + 14, x, game->message, 70);
    if (game->status != DW_RUNNING) {
        style(game->status == DW_FINISHED ? DW_GREEN : DW_RED, A_BOLD);
        (void)mvprintw(y + 16, x, "%s  Final score: %" PRIu64,
                      game->status == DW_FINISHED ? "RUN COMPLETE" : "GAME OVER",
                      game->final_score);
    } else if (ui->rejected) {
        style(DW_RED, A_BOLD);
        (void)mvaddstr(y + 16, x, "Action rejected: check funds, stock, capacity, destination or day.");
    } else if (ui->prompt == DW_TRAVEL) {
        style(DW_GOLD, A_BOLD);
        (void)mvaddstr(y + 16, x, "TRAVEL: choose a different destination 1-6. Esc cancels.");
    } else if (ui->prompt != DW_NONE) {
        style(DW_GOLD, A_BOLD);
        const char *label = ui->prompt == DW_BUY ? "BUY" :
                            ui->prompt == DW_SELL ? "SELL" :
                            ui->prompt == DW_REPAY ? "REPAY" :
                            ui->prompt == DW_DEPOSIT ? "DEPOSIT" : "WITHDRAW";
        (void)mvprintw(y + 16, x, "%s amount: %" PRIu64 "  Digits / A: max / Enter / Esc",
                      label, ui->has_amount ? ui->amount : UINT64_C(1));
    } else {
        style(DW_TEXT, A_NORMAL);
        (void)mvaddstr(y + 16, x, "Select goods with Up/Down or 1-6, then B: buy or S: sell.");
    }
    style(DW_CYAN, A_NORMAL);
    (void)mvaddstr(y + 18, x, "T: travel  P: repay debt  D: deposit  W: withdraw");
    (void)mvaddstr(y + 19, x, "U: +50 capacity ($5000, max 300)  E: settle on day 30");
    style(DW_PURPLE, A_BOLD);
    (void)mvaddstr(y + 20, x, "R: restart same seed  M: main menu  Q: quit");
    style(DW_TEXT, A_DIM);
    (void)mvaddstr(y + 22, x, "Travel adds 10% debt interest. Scores count only completed runs.");
    (void)mvaddstr(y + 23, x, "All prices and events are fictional game mechanics.");
    (void)attr_set(A_NORMAL, 0, NULL);
    (void)refresh();
}

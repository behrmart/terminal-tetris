#include "drugwars/display.h"
#include <inttypes.h>
#include <ncurses.h>

bool dw_display_can_fit(void)
{
    return getmaxy(stdscr) >= DW_MIN_ROWS && getmaxx(stdscr) >= DW_MIN_COLUMNS;
}
void dw_display_render(const DwGame *game, const DwUi *ui,
                       const char *username, uint64_t best)
{
    (void)erase();
    const int columns = getmaxx(stdscr);
    if (!dw_display_can_fit()) {
        if (columns > 1) {
            (void)mvaddnstr(0, 0, "Need 72x25 - M: main menu  Q: quit", columns - 1);
        }
        (void)refresh();
        return;
    }
    const int x = (columns - DW_MIN_COLUMNS) / 2;
    const int y = (getmaxy(stdscr) - DW_MIN_ROWS) / 2;
    const bool color = has_colors() && COLORS >= 8 && COLOR_PAIRS > 1;
    if (color) {
        (void)attron(COLOR_PAIR(1));
    }
    (void)attron(A_BOLD);
    (void)mvaddstr(y, x, "TERMINAL DRUG WARS");
    (void)attroff(A_BOLD);
    if (color) {
        (void)attroff(COLOR_PAIR(1));
    }
    (void)mvprintw(y + 1, x, "Player: %-16s Best: %" PRIu64, username, best);
    (void)mvprintw(y + 2, x, "Day %u/30  %-14s Health: %u  Stock: %u/%u",
                  game->day, dw_places[game->place], game->health,
                  dw_used(game), game->capacity);
    (void)mvprintw(y + 3, x, "Cash: %" PRIu64 "  Bank: %" PRIu64 "  Debt: %" PRIu64,
                  game->cash, game->bank, game->debt);
    (void)mvprintw(y + 4, x, "Net worth: %" PRIu64 "  (negative net worth shown as 0)",
                  dw_net_worth(game));
    (void)mvhline(y + 5, x, '-', 70);
    (void)mvaddstr(y + 6, x, "   Goods          Price    Owned");
    (void)mvaddstr(y + 6, x + 42, "Destinations (T, then 1-6)");
    for (unsigned int i = 0U; i < DW_GOODS; ++i) {
        if (i == ui->selected) {
            (void)attron(A_REVERSE);
        }
        (void)mvprintw(y + 7 + (int)i, x, "%c%u %-12s %8u %7u",
                      i == ui->selected ? '>' : ' ', i + 1U, dw_goods[i],
                      game->prices[i], game->inventory[i]);
        if (i == ui->selected) {
            (void)attroff(A_REVERSE);
        }
        (void)mvprintw(y + 7 + (int)i, x + 42, "%u: %s%s", i + 1U,
                      dw_places[i], i == game->place ? " *" : "");
    }
    (void)mvhline(y + 13, x, '-', 70);
    (void)mvaddnstr(y + 14, x, game->message, 70);
    if (game->status != DW_RUNNING) {
        (void)mvprintw(y + 16, x, "%s  Final score: %" PRIu64,
                      game->status == DW_FINISHED ? "RUN COMPLETE" : "GAME OVER",
                      game->final_score);
    } else if (ui->rejected) {
        (void)mvaddstr(y + 16, x, "Action rejected: check funds, stock, capacity, destination or day.");
    } else if (ui->prompt == DW_TRAVEL) {
        (void)mvaddstr(y + 16, x, "TRAVEL: choose a different destination 1-6. Esc cancels.");
    } else if (ui->prompt != DW_NONE) {
        const char *label = ui->prompt == DW_BUY ? "BUY" :
                            ui->prompt == DW_SELL ? "SELL" :
                            ui->prompt == DW_REPAY ? "REPAY" :
                            ui->prompt == DW_DEPOSIT ? "DEPOSIT" : "WITHDRAW";
        (void)mvprintw(y + 16, x, "%s amount: %" PRIu64 "  Digits / A: max / Enter / Esc",
                      label, ui->has_amount ? ui->amount : UINT64_C(1));
    } else {
        (void)mvaddstr(y + 16, x, "Select goods with Up/Down or 1-6, then B: buy or S: sell.");
    }
    (void)mvaddstr(y + 18, x, "T: travel  P: repay debt  D: deposit  W: withdraw");
    (void)mvaddstr(y + 19, x, "U: +50 capacity ($5000, max 300)  E: settle on day 30");
    (void)mvaddstr(y + 20, x, "R: restart same seed  M: main menu  Q: quit");
    (void)mvaddstr(y + 22, x, "Travel adds 10% debt interest. Scores count only completed runs.");
    (void)mvaddstr(y + 23, x, "All prices and events are fictional game mechanics.");
    (void)refresh();
}

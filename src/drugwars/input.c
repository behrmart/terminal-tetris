#include "drugwars/input.h"
#include <ncurses.h>

static uint64_t minimum(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}
static uint64_t maximum_amount(const DwUi *ui, const DwGame *game)
{
    switch (ui->prompt) {
    case DW_BUY:
        return minimum(game->capacity - dw_used(game),
                       game->cash / game->prices[ui->selected]);
    case DW_SELL:
        return minimum(game->inventory[ui->selected],
                       (DW_MONEY_LIMIT - game->cash) / game->prices[ui->selected]);
    case DW_REPAY: return minimum(game->cash, game->debt);
    case DW_DEPOSIT: return minimum(game->cash, DW_MONEY_LIMIT - game->bank);
    case DW_WITHDRAW: return minimum(game->bank, DW_MONEY_LIMIT - game->cash);
    default: return 0U;
    }
}
DwInput dw_input_poll(DwUi *ui, const DwGame *game)
{
    DwInput input = {0};
    const int key = getch();
    if (key == 'q' || key == 'Q') {
        input.quit = true;
    } else if (key == 'm' || key == 'M') {
        input.menu = true;
    } else if (key == 'r' || key == 'R') {
        *ui = (DwUi){0};
        input.action.type = DW_RESTART;
    } else if (key == 27) {
        ui->prompt = DW_NONE;
        ui->rejected = false;
    } else if (game->status != DW_RUNNING) {
        return input;
    } else if (ui->prompt == DW_TRAVEL) {
        if (key >= '1' && key <= '6') {
            input.action = (DwAction){DW_TRAVEL, (unsigned int)(key - '1'), 0U};
            ui->prompt = DW_NONE;
        }
    } else if (ui->prompt != DW_NONE) {
        if (key >= '0' && key <= '9') {
            const uint64_t digit = (uint64_t)(key - '0');
            if (ui->amount <= (DW_MONEY_LIMIT - digit) / 10U) {
                ui->amount = ui->amount * 10U + digit;
                ui->has_amount = true;
            }
        } else if (key == KEY_BACKSPACE || key == 127 || key == 8) {
            ui->amount /= 10U;
        } else if (key == 'a' || key == 'A') {
            ui->amount = maximum_amount(ui, game);
            ui->has_amount = true;
        } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
            input.action = (DwAction){ui->prompt, ui->selected,
                                     ui->has_amount ? ui->amount : 1U};
            ui->prompt = DW_NONE;
        }
    } else {
        if (key == KEY_UP) {
            ui->selected = (ui->selected + DW_GOODS - 1U) % DW_GOODS;
        } else if (key == KEY_DOWN) {
            ui->selected = (ui->selected + 1U) % DW_GOODS;
        } else if (key >= '1' && key <= '6') {
            ui->selected = (unsigned int)(key - '1');
        } else {
            switch (key) {
            case 'b': case 'B': ui->prompt = DW_BUY; break;
            case 's': case 'S': ui->prompt = DW_SELL; break;
            case 't': case 'T': ui->prompt = DW_TRAVEL; break;
            case 'p': case 'P': ui->prompt = DW_REPAY; break;
            case 'd': case 'D': ui->prompt = DW_DEPOSIT; break;
            case 'w': case 'W': ui->prompt = DW_WITHDRAW; break;
            case 'u': case 'U': input.action.type = DW_UPGRADE; break;
            case 'e': case 'E': input.action.type = DW_FINISH; break;
            default: break;
            }
            ui->amount = 0U;
            ui->has_amount = false;
            ui->rejected = false;
        }
    }
    return input;
}

#include "drugwars/game.h"
#include <stdio.h>

const char *const dw_goods[DW_GOODS] = {
    "Cocaine", "Heroin", "Acid", "Weed", "Speed", "Ludes"
};
const char *const dw_places[DW_PLACES] = {
    "Bronx", "Brooklyn", "Manhattan", "Queens", "Staten Island", "Coney Island"
};
static uint32_t random_next(DwGame *game)
{
    game->random_state = (uint32_t)((uint64_t)game->random_state +
                                  UINT64_C(0x9e3779b9));
    uint32_t value = game->random_state;
    value ^= value >> 16;
    value = (uint32_t)((uint64_t)value * UINT64_C(0x7feb352d));
    value ^= value >> 15;
    value = (uint32_t)((uint64_t)value * UINT64_C(0x846ca68b));
    return value ^ (value >> 16);
}
static unsigned int random_below(DwGame *game, unsigned int bound)
{
    const uint32_t threshold = (uint32_t)((UINT64_C(1) << 32) % bound);
    uint32_t value;
    do {
        value = random_next(game);
    } while (value < threshold);
    return (unsigned int)(value % bound);
}
static void message(DwGame *game, const char *text)
{
    (void)snprintf(game->message, sizeof(game->message), "%s", text);
}
static void market(DwGame *game)
{
    static const unsigned int low[DW_GOODS] = {15000U, 5000U, 1000U, 100U, 150U, 10U};
    static const unsigned int high[DW_GOODS] = {30000U, 14000U, 4000U, 900U, 900U, 100U};
    for (unsigned int i = 0U; i < DW_GOODS; ++i) {
        game->prices[i] = low[i] + random_below(game, high[i] - low[i] + 1U);
    }
}
unsigned int dw_used(const DwGame *game)
{
    unsigned int used = 0U;
    for (unsigned int i = 0U; i < DW_GOODS; ++i) {
        used += game->inventory[i];
    }
    return used;
}
uint64_t dw_net_worth(const DwGame *game)
{
    uint64_t assets = game->cash + game->bank;
    for (unsigned int i = 0U; i < DW_GOODS; ++i) {
        assets += (uint64_t)game->inventory[i] * game->prices[i];
    }
    return assets > game->debt ? assets - game->debt : 0U;
}
void dw_init(DwGame *game, uint32_t seed)
{
    *game = (DwGame){0};
    game->seed = seed;
    game->random_state = seed;
    game->cash = UINT64_C(2000);
    game->debt = UINT64_C(5500);
    game->day = 1U;
    game->health = 100U;
    game->capacity = 100U;
    market(game);
    message(game, "30 days. Buy low, sell high, repay debt. Travel advances time.");
}
static void finish(DwGame *game)
{
    game->final_score = dw_net_worth(game);
    game->status = DW_FINISHED;
    message(game, "Run complete. Final score is assets minus debt (minimum zero).");
}
bool dw_process(DwGame *game, DwAction action)
{
    if (action.type == DW_RESTART) {
        dw_init(game, game->seed);
        return true;
    }
    if (game->status != DW_RUNNING) {
        return false;
    }
    const uint64_t amount = action.amount;
    const unsigned int index = action.index;
    switch (action.type) {
    case DW_BUY:
        if (index >= DW_GOODS || amount == 0U ||
            amount > game->capacity - dw_used(game) ||
            amount > game->cash / game->prices[index]) {
            return false;
        }
        game->cash -= amount * game->prices[index];
        game->inventory[index] += (unsigned int)amount;
        message(game, "Purchase complete.");
        break;
    case DW_SELL:
        if (index >= DW_GOODS || amount == 0U || amount > game->inventory[index] ||
            amount > (DW_MONEY_LIMIT - game->cash) / game->prices[index]) {
            return false;
        }
        game->cash += amount * game->prices[index];
        game->inventory[index] -= (unsigned int)amount;
        message(game, "Sale complete.");
        break;
    case DW_REPAY:
        if (amount == 0U || amount > game->cash || amount > game->debt) {
            return false;
        }
        game->cash -= amount;
        game->debt -= amount;
        message(game, "Debt repaid.");
        break;
    case DW_DEPOSIT:
        if (amount == 0U || amount > game->cash || amount > DW_MONEY_LIMIT - game->bank) {
            return false;
        }
        game->cash -= amount;
        game->bank += amount;
        message(game, "Cash deposited. Bank balance is protected from travel events.");
        break;
    case DW_WITHDRAW:
        if (amount == 0U || amount > game->bank || amount > DW_MONEY_LIMIT - game->cash) {
            return false;
        }
        game->bank -= amount;
        game->cash += amount;
        message(game, "Cash withdrawn.");
        break;
    case DW_UPGRADE:
        if (game->capacity >= 300U || game->cash < UINT64_C(5000)) {
            return false;
        }
        game->cash -= UINT64_C(5000);
        game->capacity += 50U;
        message(game, "Bought 50 extra inventory spaces.");
        break;
    case DW_FINISH:
        if (game->day != DW_DAYS) {
            return false;
        }
        finish(game);
        break;
    case DW_TRAVEL: {
        if (index >= DW_PLACES || index == game->place) {
            return false;
        }
        if (game->day == DW_DAYS) {
            finish(game);
            break;
        }
        game->place = index;
        ++game->day;
        const uint64_t interest = game->debt / 10U;
        game->debt = interest > DW_MONEY_LIMIT - game->debt ?
                     DW_MONEY_LIMIT : game->debt + interest;
        market(game);
        const unsigned int event = random_below(game, 10U);
        const unsigned int good = random_below(game, DW_GOODS);
        message(game, "Arrived safely. New market prices; debt gained 10% interest.");
        if (event == 0U) {
            game->prices[good] *= 3U;
            (void)snprintf(game->message, sizeof(game->message),
                          "Market shortage: %s prices have surged!", dw_goods[good]);
        } else if (event == 1U) {
            game->prices[good] /= 3U;
            (void)snprintf(game->message, sizeof(game->message),
                          "Market surplus: %s prices have crashed!", dw_goods[good]);
        } else if (event == 2U) {
            game->cash -= game->cash / 10U;
            message(game, "Mugging! Lost 10% of carried cash. Bank funds are safe.");
        } else if (event == 3U) {
            for (unsigned int i = 0U; i < DW_GOODS; ++i) {
                game->inventory[i] -= (game->inventory[i] + 3U) / 4U;
            }
            game->health = game->health > 10U ? game->health - 10U : 0U;
            message(game, "Police encounter: lost a quarter of stock and 10 health.");
            if (game->health == 0U) {
                game->status = DW_DEAD;
                game->final_score = 0U;
                message(game, "Run ended: health depleted. Final score: 0.");
            }
        } else if (event == 4U) {
            const unsigned int free_space = game->capacity - dw_used(game);
            game->inventory[good] += free_space < 5U ? free_space : 5U;
            message(game, "Lucky find! Picked up stock, subject to inventory space.");
        }
        break;
    }
    default:
        return false;
    }
    return true;
}

#include "drugwars/game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void equal(const DwGame *a, const DwGame *b)
{
    assert(a->seed == b->seed && a->random_state == b->random_state);
    assert(a->cash == b->cash && a->bank == b->bank && a->debt == b->debt);
    assert(a->day == b->day && a->place == b->place && a->health == b->health);
    assert(a->capacity == b->capacity && a->status == b->status);
    assert(a->final_score == b->final_score && strcmp(a->message, b->message) == 0);
    for (unsigned int i = 0U; i < DW_GOODS; ++i) {
        assert(a->inventory[i] == b->inventory[i] && a->prices[i] == b->prices[i]);
    }
}
static void rejected(DwGame *game, DwAction action)
{
    const DwGame before = *game;
    assert(!dw_process(game, action));
    equal(game, &before);
}
static void transactions(void)
{
    DwGame game;
    dw_init(&game, 0U);
    assert(game.day == 1U && game.cash == 2000U && game.debt == 5500U);
    assert(game.capacity == 100U && dw_used(&game) == 0U);
    game.prices[0] = 10U;
    assert(dw_process(&game, (DwAction){DW_BUY, 0U, 100U}));
    assert(game.cash == 1000U && dw_used(&game) == 100U);
    rejected(&game, (DwAction){DW_BUY, 0U, 1U});
    rejected(&game, (DwAction){DW_BUY, 6U, 1U});
    rejected(&game, (DwAction){DW_SELL, 0U, 101U});
    rejected(&game, (DwAction){DW_SELL, 0U, UINT64_MAX});
    assert(dw_process(&game, (DwAction){DW_SELL, 0U, 50U}));
    assert(game.cash == 1500U && dw_used(&game) == 50U);
    assert(dw_process(&game, (DwAction){DW_DEPOSIT, 0U, 1000U}));
    assert(game.cash == 500U && game.bank == 1000U);
    rejected(&game, (DwAction){DW_WITHDRAW, 0U, 1001U});
    assert(dw_process(&game, (DwAction){DW_WITHDRAW, 0U, 500U}));
    assert(dw_process(&game, (DwAction){DW_REPAY, 0U, 1000U}));
    assert(game.cash == 0U && game.debt == 4500U && game.bank == 500U);
    rejected(&game, (DwAction){DW_REPAY, 0U, 1U});
    rejected(&game, (DwAction){DW_UPGRADE, 0U, 0U});
    game.cash = 20000U;
    for (unsigned int i = 0U; i < 4U; ++i) {
        assert(dw_process(&game, (DwAction){DW_UPGRADE, 0U, 0U}));
    }
    assert(game.capacity == 300U && game.cash == 0U);
    rejected(&game, (DwAction){DW_UPGRADE, 0U, 0U});
    game.cash = DW_MONEY_LIMIT;
    rejected(&game, (DwAction){DW_SELL, 0U, 1U});
    rejected(&game, (DwAction){DW_WITHDRAW, 0U, 1U});
    game.bank = DW_MONEY_LIMIT;
    rejected(&game, (DwAction){DW_DEPOSIT, 0U, 1U});
    rejected(&game, (DwAction){DW_REPAY, 0U, UINT64_MAX});
    rejected(&game, (DwAction){DW_FINISH, 0U, 0U});
    rejected(&game, (DwAction){DW_TRAVEL, game.place, 0U});
    rejected(&game, (DwAction){DW_TRAVEL, 6U, 0U});
    rejected(&game, (DwAction){DW_NONE, 0U, 0U});
}
static void replay_and_events(void)
{
    bool surge = false, crash = false, mugging = false, police = false, find = false;
    for (uint32_t seed = 0U; seed < 128U; ++seed) {
        DwGame a, b, initial;
        dw_init(&a, seed);
        dw_init(&b, seed);
        initial = a;
        for (unsigned int turn = 1U; turn < DW_DAYS; ++turn) {
            a.health = 100U;
            b.health = 100U; /* Death is tested separately; keep full replay runs. */
            const uint64_t debt = a.debt;
            const DwAction action = {DW_TRAVEL, (a.place + 1U) % DW_PLACES, 0U};
            assert(dw_process(&a, action) && dw_process(&b, action));
            equal(&a, &b);
            assert(a.day == turn + 1U && a.debt == debt + debt / 10U);
            assert(dw_used(&a) <= a.capacity);
            for (unsigned int i = 0U; i < DW_GOODS; ++i) {
                assert(a.prices[i] > 0U && a.prices[i] <= 90000U);
            }
            surge |= strstr(a.message, "shortage") != NULL;
            crash |= strstr(a.message, "surplus") != NULL;
            mugging |= strstr(a.message, "Mugging") != NULL;
            police |= strstr(a.message, "Police") != NULL;
            find |= strstr(a.message, "Lucky") != NULL;
        }
        assert(a.day == 30U && a.status == DW_RUNNING);
        const uint64_t worth = dw_net_worth(&a);
        assert(dw_process(&a, (DwAction){DW_FINISH, 0U, 0U}));
        assert(a.status == DW_FINISHED && a.final_score == worth);
        rejected(&a, (DwAction){DW_BUY, 0U, 1U});
        assert(dw_process(&b, (DwAction){DW_TRAVEL, (b.place + 1U) % DW_PLACES, 0U}));
        equal(&a, &b);
        assert(dw_process(&a, (DwAction){DW_RESTART, 0U, 0U}));
        equal(&a, &initial);
    }
    assert(surge && crash && mugging && police && find);
}
static void boundaries(void)
{
    bool died = false;
    for (uint32_t seed = 0U; seed < 128U; ++seed) {
        DwGame game;
        dw_init(&game, seed);
        game.health = 10U;
        game.debt = DW_MONEY_LIMIT;
        game.bank = 999U;
        game.inventory[0] = 100U;
        assert(dw_process(&game, (DwAction){DW_TRAVEL, 1U, 0U}));
        assert(game.debt == DW_MONEY_LIMIT && game.bank == 999U);
        assert(dw_used(&game) <= 100U);
        if (game.status == DW_DEAD) {
            died = true;
            assert(game.final_score == 0U && game.health == 0U);
            rejected(&game, (DwAction){DW_SELL, 0U, 1U});
            assert(dw_process(&game, (DwAction){DW_RESTART, 0U, 0U}));
            assert(game.health == 100U);
        }
    }
    assert(died);
    DwGame game;
    dw_init(&game, UINT32_MAX);
    game.cash = 10000U;
    game.bank = 2000U;
    game.debt = 1000U;
    game.inventory[0] = 2U;
    game.prices[0] = 100U;
    game.day = 30U;
    assert(dw_net_worth(&game) == 11200U);
    assert(dw_process(&game, (DwAction){DW_FINISH, 0U, 0U}));
    assert(game.final_score == 11200U);
}
int main(void)
{
    transactions();
    replay_and_events();
    boundaries();
    (void)puts("Drug Wars tests passed (trades, funds, debt, events, replay, 30 days, scoring).");
    return 0;
}

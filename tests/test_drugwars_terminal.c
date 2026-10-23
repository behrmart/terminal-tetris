#include "display.h"
#include "drugwars/display.h"
#include <assert.h>
#include <ncurses.h>
#include <stdio.h>
static void text_at(int row, int column, const char *text)
{
    for (int i = 0; text[i] != 0; ++i) {
        assert((mvinch(row, column + i) & A_CHARTEXT) == (chtype)(unsigned char)text[i]);
    }
}
static DwInput key(DwUi *ui, const DwGame *game, int code)
{
    assert(ungetch(code) != ERR);
    return dw_input_poll(ui, game);
}
int main(void)
{
    assert(display_init());
    assert(resizeterm(25, 72) != ERR);
    DwGame game;
    DwUi ui = {0};
    dw_init(&game, 0U);
    dw_display_render(&game, &ui, "tester", UINT64_MAX);
    text_at(0, 0, "TERMINAL DRUG WARS");
    text_at(2, 0, "Day 1/30");
    assert(key(&ui, &game, '6').action.type == DW_NONE && ui.selected == 5U);
    (void)key(&ui, &game, 'B');
    dw_display_render(&game, &ui, "tester", 0U);
    text_at(16, 0, "BUY amount: 1");
    (void)key(&ui, &game, 'a');
    const DwInput buy = key(&ui, &game, '\n');
    assert(buy.action.type == DW_BUY && buy.action.amount > 0U);
    assert(dw_process(&game, buy.action));
    assert(dw_used(&game) > 0U);
    (void)key(&ui, &game, 't');
    assert(key(&ui, &game, '2').action.index == 1U);
    (void)key(&ui, &game, 'p');
    (void)key(&ui, &game, '1');
    (void)key(&ui, &game, '0');
    (void)key(&ui, &game, KEY_BACKSPACE);
    assert(key(&ui, &game, '\n').action.amount == 1U);
    assert(key(&ui, &game, 'M').menu && key(&ui, &game, 'Q').quit);
    assert(key(&ui, &game, 'r').action.type == DW_RESTART);
    game.day = 30U;
    game.cash = 10000U;
    assert(dw_process(&game, key(&ui, &game, 'e').action));
    dw_display_render(&game, &ui, "tester", 0U);
    text_at(16, 0, "RUN COMPLETE");
    assert(resizeterm(2, 40) != ERR);
    assert(!dw_display_can_fit());
    dw_display_render(&game, &ui, "tester", 0U);
    text_at(0, 0, "Need 72x25");
    assert(resizeterm(1, 1) != ERR);
    dw_display_render(&game, &ui, "tester", 0U);
    assert(resizeterm(25, 72) != ERR);
    dw_display_render(&game, &ui, "tester", 0U);
    text_at(16, 0, "RUN COMPLETE");
    display_shutdown();
    (void)puts("Drug Wars screen/input checks passed.");
    return 0;
}

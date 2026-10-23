#ifndef TERMINAL_DRUGWARS_INPUT_H
#define TERMINAL_DRUGWARS_INPUT_H
#include "drugwars/game.h"
typedef struct {
    unsigned int selected;
    DwActionType prompt;
    uint64_t amount;
    bool has_amount;
    bool rejected;
} DwUi;
typedef struct {
    DwAction action;
    bool quit;
    bool menu;
} DwInput;
DwInput dw_input_poll(DwUi *ui, const DwGame *game);
#endif

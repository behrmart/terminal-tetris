#ifndef TERMINAL_DRUGWARS_GAME_H
#define TERMINAL_DRUGWARS_GAME_H
#include <stdbool.h>
#include <stdint.h>

enum { DW_GOODS = 6, DW_PLACES = 6, DW_DAYS = 30 };
#define DW_MONEY_LIMIT UINT64_C(1000000000000)
typedef enum { DW_RUNNING, DW_FINISHED, DW_DEAD } DwStatus;
typedef enum {
    DW_NONE, DW_BUY, DW_SELL, DW_TRAVEL, DW_REPAY, DW_DEPOSIT,
    DW_WITHDRAW, DW_UPGRADE, DW_FINISH, DW_RESTART
} DwActionType;
typedef struct {
    DwActionType type;
    unsigned int index; /* Good or destination, depending on action. */
    uint64_t amount;
} DwAction;
typedef struct {
    uint32_t seed;
    uint32_t random_state;
    uint64_t cash, bank, debt, final_score;
    unsigned int day, place, health, capacity;
    unsigned int inventory[DW_GOODS];
    unsigned int prices[DW_GOODS];
    DwStatus status;
    char message[96];
} DwGame;

extern const char *const dw_goods[DW_GOODS];
extern const char *const dw_places[DW_PLACES];
void dw_init(DwGame *game, uint32_t seed);
/* Initialized state required. Invalid actions leave all state unchanged.
 * Trading/banking do not advance time. Travel advances one day, interest,
 * prices and one abstract event. On day 30, travel or FINISH settles the run.
 * RESTART reuses the original seed. No clock, keyboard or file dependencies.
 */
bool dw_process(DwGame *game, DwAction action);
unsigned int dw_used(const DwGame *game);
uint64_t dw_net_worth(const DwGame *game);
#endif

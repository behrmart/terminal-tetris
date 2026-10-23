#ifndef TERMINAL_DRUGWARS_DISPLAY_H
#define TERMINAL_DRUGWARS_DISPLAY_H
#include "drugwars/input.h"
enum { DW_MIN_ROWS = 25, DW_MIN_COLUMNS = 72 };
/* Prepare dedicated color pairs after each successful display_init(). */
void dw_display_prepare(void);
bool dw_display_can_fit(void);
void dw_display_render(const DwGame *game, const DwUi *ui,
                       const char *username, uint64_t best);
#endif

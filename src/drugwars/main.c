#define _POSIX_C_SOURCE 200809L
#include "session.h"
#include "display.h"
#include "drugwars/display.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int drugwars_run(const volatile sig_atomic_t *stop_requested, ScoreProfile *profile)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        (void)fputs("Could not read the monotonic clock.\n", stderr);
        return EXIT_FAILURE;
    }
    DwGame game;
    DwUi ui = {0};
    dw_init(&game, (uint32_t)now.tv_sec ^ (uint32_t)now.tv_nsec);
    if (!display_init()) {
        (void)fputs("Could not initialize the terminal.\n", stderr);
        return EXIT_FAILURE;
    }
    bool was_ready = dw_display_can_fit();
    bool menu = false;
    dw_display_render(&game, &ui, profile->username, profile->best[SCORE_DRUGWARS]);
    while (!*stop_requested) {
        const DwUi previous_ui = ui;
        const DwInput input = dw_input_poll(&ui, &game);
        if (input.quit || *stop_requested) {
            break;
        }
        if (input.menu) {
            menu = true;
            break;
        }
        const bool ready = dw_display_can_fit();
        if (!ready || !was_ready) {
            ui = previous_ui;
        } else if (input.action.type != DW_NONE) {
            ui.rejected = !dw_process(&game, input.action);
            if (game.status != DW_RUNNING) {
                scores_record(profile, SCORE_DRUGWARS, game.final_score);
            }
        }
        was_ready = ready;
        dw_display_render(&game, &ui, profile->username, profile->best[SCORE_DRUGWARS]);
    }
    display_shutdown();
    return menu ? SESSION_MENU : EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "session.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "display.h"
#include "game.h"
#include "input.h"


static bool monotonic_ms(uint64_t *milliseconds)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return false;
    }
    *milliseconds = (uint64_t)now.tv_sec * UINT64_C(1000) +
                    (uint64_t)now.tv_nsec / UINT64_C(1000000);
    return true;
}

int tetris_run(const volatile sig_atomic_t *stop_requested)
{
    Game game;
    uint64_t previous = 0U;
    bool clock_failed = false;
    bool menu_requested = false;
    if (!monotonic_ms(&previous)) {
        (void)fputs("Could not read the monotonic clock.\n", stderr);
        return EXIT_FAILURE;
    }
    /* Fresh launch, fresh seed; restart replays this launch's sequence. */
    game_init(&game, (uint32_t)previous);
    if (!display_init()) {
        (void)fputs("Could not initialize the terminal.\n", stderr);
        return EXIT_FAILURE;
    }

    bool was_ready = display_can_fit();
    display_render(&game);
    /* Do not charge terminal initialization time to the first piece. */
    if (!monotonic_ms(&previous)) {
        clock_failed = true;
    }
    while (!*stop_requested && !clock_failed) {
        const Action action = input_poll_navigation(&menu_requested);
        if (action == ACTION_QUIT || *stop_requested) {
            break;
        }
        if (menu_requested) {
            break;
        }
        uint64_t now = 0U;
        if (!monotonic_ms(&now)) {
            clock_failed = true;
            break;
        }
        const bool ready = display_can_fit();
        if (!ready || !was_ready) {
            /* Freeze while undersized. Discard recovery input and timing,
             * preserving the user's running/paused/game-over state.
             */
            game.gravity_elapsed_ms = 0U;
        } else if (action == ACTION_PAUSE || action == ACTION_RESTART) {
            /* Transitions never inherit time spent before the keypress. */
            game_process_action(&game, action);
        } else {
            const uint64_t elapsed = now >= previous ? now - previous : 0U;
            const unsigned int bounded =
                elapsed > (uint64_t)GAME_MAX_ELAPSED_MS ?
                (unsigned int)GAME_MAX_ELAPSED_MS : (unsigned int)elapsed;
            game_update(&game, bounded);
            game_process_action(&game, action);
        }
        previous = now;
        was_ready = ready;
        display_render(&game);
    }

    display_shutdown();
    if (clock_failed) {
        (void)fputs("Could not read the monotonic clock.\n", stderr);
        return EXIT_FAILURE;
    }
    return menu_requested ? SESSION_MENU : EXIT_SUCCESS;
}

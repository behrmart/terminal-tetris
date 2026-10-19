#define _POSIX_C_SOURCE 200809L

#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game2048/display.h"
#include "game2048/input.h"

static volatile sig_atomic_t stop_requested = 0;

static void request_stop(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

int main(void)
{
    Game2048 game;
    bool playing = false;
    bool failed = false;
    struct sigaction handler = {0};
    handler.sa_handler = request_stop;
    (void)setlocale(LC_ALL, "");
    if (sigemptyset(&handler.sa_mask) != 0 ||
        sigaction(SIGINT, &handler, NULL) != 0 ||
        sigaction(SIGTERM, &handler, NULL) != 0 ||
        sigaction(SIGHUP, &handler, NULL) != 0) {
        (void)fputs("Could not install exit handlers.\n", stderr);
        return EXIT_FAILURE;
    }
    if (!game2048_display_init()) {
        (void)fputs("Could not initialize the terminal.\n", stderr);
        return EXIT_FAILURE;
    }
    game2048_display_menu();
    bool was_ready = game2048_display_can_fit(0U);
    while (!stop_requested) {
        const Game2048Input input = game2048_input_poll();
        if (stop_requested || input.action == GAME2048_ACTION_QUIT) {
            break;
        }
        const unsigned int size = playing ? game.board.size : 0U;
        const bool ready = game2048_display_can_fit(size);
        if (ready && was_ready) {
            if (!playing && input.selected_size != 0U) {
                struct timespec now;
                if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
                    failed = true;
                    break;
                }
                const uint32_t seed = (uint32_t)now.tv_sec ^ (uint32_t)now.tv_nsec;
                if (!game2048_init(&game, input.selected_size, seed)) {
                    failed = true;
                    break;
                }
                playing = true;
            } else if (playing) {
                game2048_process_action(&game, input.action);
            }
        }
        /* Discard input while undersized and on the first recovery poll. */
        was_ready = game2048_display_can_fit(playing ? game.board.size : 0U);
        if (playing) {
            game2048_display_render(&game);
        } else {
            game2048_display_menu();
        }
    }
    game2048_display_shutdown();
    if (failed) {
        (void)fputs("Could not initialize the game or read the clock.\n", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

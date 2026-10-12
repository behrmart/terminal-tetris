#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "game.h"
#include "input.h"

static volatile sig_atomic_t stop_requested = 0;

static void request_stop(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

int main(void)
{
    Game game;

    (void)setlocale(LC_ALL, "");
    if (signal(SIGINT, request_stop) == SIG_ERR ||
        signal(SIGTERM, request_stop) == SIG_ERR) {
        (void)fputs("Could not install exit handlers.\n", stderr);
        return EXIT_FAILURE;
    }
    game_init(&game, UINT32_C(1));
    if (!display_init()) {
        (void)fputs("Could not initialize the terminal.\n", stderr);
        return EXIT_FAILURE;
    }

    while (!stop_requested) {
        display_render(&game);
        if (input_poll() == ACTION_QUIT) {
            break;
        }
    }

    display_shutdown();
    return EXIT_SUCCESS;
}

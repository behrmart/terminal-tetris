#define _POSIX_C_SOURCE 200809L

#include "session.h"
#include "display.h"

#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t stop_requested = 0;

static void request_stop(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

/* Return 1/2 for a game, zero for quit, or -1 for initialization failure. */
static int select_game(void)
{
    if (!display_init()) {
        return -1;
    }
    int selection = 0;
    (void)flushinp();
    bool was_ready = getmaxy(stdscr) >= 9 && getmaxx(stdscr) >= 32;
    while (!stop_requested) {
        const int rows = getmaxy(stdscr);
        const int columns = getmaxx(stdscr);
        const bool ready = rows >= 9 && columns >= 32;
        (void)erase();
        if (ready) {
            const int top = (rows - 9) / 2;
            const int left = (columns - 30) / 2;
            (void)mvaddstr(top, left, "TERMINAL GAMES");
            (void)mvaddstr(top + 2, left, "1: Tetris");
            (void)mvaddstr(top + 3, left, "2: 2048");
            (void)mvaddstr(top + 5, left, "Q: quit");
            (void)mvaddstr(top + 7, left, "Select a game to start.");
        } else if (rows > 0 && columns > 1) {
            (void)mvaddnstr(0, 0, "Need 32x9 - Q: quit", columns - 1);
        }
        (void)refresh();
        const int key = getch();
        if (key == 'q' || key == 'Q' || stop_requested) {
            break;
        }
        if (ready && was_ready && (key == '1' || key == '2')) {
            selection = key - '0';
            break;
        }
        was_ready = ready;
    }
    display_shutdown();
    return selection;
}

int main(int argc, char **argv)
{
    (void)argc;
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
    const char *name = strrchr(argv[0], '/');
    name = name == NULL ? argv[0] : name + 1;
    int selection = strcmp(name, "terminal-tetris") == 0 ? 1 :
                    strcmp(name, "terminal-2048") == 0 ? 2 : 0;
    while (!stop_requested) {
        if (selection == 0) {
            selection = select_game();
            if (selection < 0) {
                (void)fputs("Could not initialize the terminal.\n", stderr);
                return EXIT_FAILURE;
            }
            if (selection == 0 || stop_requested) {
                break;
            }
        }
        const int result = selection == 1 ? tetris_run(&stop_requested) :
                                           game2048_run(&stop_requested);
        if (result != SESSION_MENU) {
            return result;
        }
        selection = 0;
    }
    return EXIT_SUCCESS;
}
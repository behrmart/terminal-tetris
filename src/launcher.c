#define _POSIX_C_SOURCE 200809L

#include "session.h"
#include "display.h"
#include "menu_theme.h"

#include <inttypes.h>
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

/* Return 1 on login, 0 on cancellation, -1 on terminal failure. */
static int choose_username(char name[SCORE_NAME_MAX + 1])
{
    if (!display_init()) {
        return -1;
    }
    menu_theme_prepare();
    size_t length = 0U;
    name[0] = '\0';
    int result = 0;
    while (!stop_requested) {
        const int rows = getmaxy(stdscr);
        const int columns = getmaxx(stdscr);
        const bool ready = rows >= 9 && columns >= 32;
        menu_theme_style(MENU_NORMAL);
        (void)erase();
        if (ready) {
            const int top = (rows - 9) / 2;
            const int left = (columns - 30) / 2;
            menu_theme_style(MENU_TITLE);
            (void)mvhline(top, left, ' ', 30);
            (void)mvaddstr(top, left, "PLAYER LOGIN");
            menu_theme_style(MENU_HEADING);
            (void)mvaddstr(top + 2, left, "Username:");
            menu_theme_style(MENU_FIELD);
            (void)mvhline(top + 3, left, ' ', 18);
            (void)mvaddstr(top + 3, left, name);
            menu_theme_style(MENU_NORMAL);
            (void)mvaddstr(top + 5, left, "1-16 letters/digits, _ or -");
            (void)mvaddstr(top + 6, left, "Names are case-sensitive.");
            menu_theme_style(MENU_KEYS);
            (void)mvaddstr(top + 7, left, "Enter: play  Esc: quit");
        } else if (rows > 0 && columns > 1) {
            menu_theme_style(MENU_WARNING);
            (void)mvaddnstr(0, 0, "Need 32x9 - Esc: quit", columns - 1);
        }
        menu_theme_style(MENU_NORMAL);
        (void)refresh();
        const int key = getch();
        if (key == 27 || stop_requested) {
            break;
        }
        if (!ready) {
            continue;
        }
        if (key == KEY_BACKSPACE || key == 127 || key == 8) {
            if (length > 0U) {
                name[--length] = '\0';
            }
        } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
            if (scores_valid_name(name)) {
                result = 1;
                break;
            }
        } else if (length < SCORE_NAME_MAX &&
                   ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') ||
                    (key >= '0' && key <= '9') || key == '_' || key == '-')) {
            name[length++] = (char)key;
            name[length] = '\0';
        }
    }
    display_shutdown();
    return result;
}
/* Return 1/2/3 for a game, zero for quit, or -1 for initialization failure. */
static int select_game(const ScoreProfile *profile)
{
    if (!display_init()) {
        return -1;
    }
    menu_theme_prepare();
    int selection = 0;
    (void)flushinp();
    bool was_ready = getmaxy(stdscr) >= 9 && getmaxx(stdscr) >= 32;
    while (!stop_requested) {
        const int rows = getmaxy(stdscr);
        const int columns = getmaxx(stdscr);
        const bool ready = rows >= 9 && columns >= 32;
        menu_theme_style(MENU_NORMAL);
        (void)erase();
        if (ready) {
            const int top = (rows - 9) / 2;
            const int left = (columns - 30) / 2;
            menu_theme_style(MENU_TITLE);
            (void)mvhline(top, left, ' ', 30);
            (void)mvaddstr(top, left, "TERMINAL GAMES");
            menu_theme_style(MENU_HEADING);
            (void)mvprintw(top + 1, left, "Player: %s", profile->username);
            menu_theme_style(MENU_SCORE);
            (void)mvprintw(top + 2, left, "1:Tetris %" PRIu64, profile->best[SCORE_TETRIS]);
            menu_theme_style(MENU_HEADING);
            (void)mvaddstr(top + 3, left, "2: 2048");
            menu_theme_style(MENU_SCORE);
            (void)mvprintw(top + 4, left, "4x4 best: %" PRIu64, profile->best[SCORE_2048_4]);
            (void)mvprintw(top + 5, left, "6x6 best: %" PRIu64, profile->best[SCORE_2048_6]);
            (void)mvprintw(top + 6, left, "3:DrugWars %" PRIu64, profile->best[SCORE_DRUGWARS]);
            menu_theme_style(profile->save_failed ? MENU_WARNING : MENU_SUCCESS);
            (void)mvaddstr(top + 7, left, profile->save_failed ?
                          "Save failed; retry on exit" : "Personal best scores");
            menu_theme_style(MENU_KEYS);
            (void)mvaddstr(top + 8, left, "1/2/3: play  Q: quit");
        } else if (rows > 0 && columns > 1) {
            menu_theme_style(MENU_WARNING);
            (void)mvaddnstr(0, 0, "Need 32x9 - Q: quit", columns - 1);
        }
        menu_theme_style(MENU_NORMAL);
        (void)refresh();
        const int key = getch();
        if (key == 'q' || key == 'Q' || stop_requested) {
            break;
        }
        if (ready && was_ready && (key >= '1' && key <= '3')) {
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
    char username[SCORE_NAME_MAX + 1];
    const int login = choose_username(username);
    if (login <= 0) {
        return login == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    ScoreProfile profile;
    if (!scores_open(&profile, username, NULL)) {
        (void)fputs("Could not load/create scores. Check the state directory, permissions,\n"
                    "or score file format. Existing files were not replaced.\n", stderr);
        return EXIT_FAILURE;
    }
    const char *name = strrchr(argv[0], '/');
    name = name == NULL ? argv[0] : name + 1;
    int selection = strcmp(name, "terminal-tetris") == 0 ? 1 :
                    strcmp(name, "terminal-2048") == 0 ? 2 :
                    strcmp(name, "terminal-drugwars") == 0 ? 3 : 0;
    while (!stop_requested) {
        if (selection == 0) {
            selection = select_game(&profile);
            if (selection < 0) {
                (void)fputs("Could not initialize the terminal.\n", stderr);
                return EXIT_FAILURE;
            }
            if (selection == 0 || stop_requested) {
                break;
            }
        }
        const int result = selection == 1 ? tetris_run(&stop_requested, &profile) :
                           selection == 2 ? game2048_run(&stop_requested, &profile) :
                                            drugwars_run(&stop_requested, &profile);
        if (!scores_save(&profile)) {
            (void)fprintf(stderr, "Could not save scores to %s.\n", profile.path);
            if (result != SESSION_MENU) {
                return EXIT_FAILURE;
            }
        }
        if (result != SESSION_MENU) {
            return result;
        }
        selection = 0;
    }
    if (!scores_save(&profile)) {
        (void)fprintf(stderr, "Could not save scores to %s.\n", profile.path);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

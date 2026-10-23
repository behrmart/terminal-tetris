#ifndef TERMINAL_GAMES_SESSION_H
#define TERMINAL_GAMES_SESSION_H

#include <signal.h>
#include "scores.h"

/* Host-only navigation; never passed to a game engine. */
enum { SESSION_MENU = 2 };
int tetris_run(const volatile sig_atomic_t *stop_requested, ScoreProfile *profile);
int game2048_run(const volatile sig_atomic_t *stop_requested, ScoreProfile *profile);

int drugwars_run(const volatile sig_atomic_t *stop_requested, ScoreProfile *profile);

#endif

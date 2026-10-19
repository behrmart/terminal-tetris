#include "game2048/display.h"

#include <stdio.h>

void game2048_display_render(const Game2048 *game)
{
    (void)puts("TERMINAL 2048 - Phase 3 game engine");
    (void)printf("Initialized %ux%u board with two tiles.\n", game->board.size, game->board.size);
    (void)puts("Supported board sizes: 4x4 and 6x6.");
    (void)puts("Spawning, scoring, win/loss and restart are implemented; playable controls are pending.");
}

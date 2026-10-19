#include "game2048/display.h"

#include <stdio.h>

void game2048_display_render(const Game2048 *game)
{
    (void)puts("TERMINAL 2048 - Phase 1 project skeleton");
    (void)printf("Initialized empty %ux%u board.\n", game->board.size, game->board.size);
    (void)puts("Supported board sizes: 4x4 and 6x6.");
    (void)puts("Movement, merging, spawning and the playable interface are not implemented.");
}

#include <stdlib.h>

#include "game2048/display.h"
#include "game2048/game.h"

int main(void)
{
    Game2048 game;
    if (!game2048_init(&game, GAME2048_SMALL_SIZE, UINT32_C(1))) {
        return EXIT_FAILURE;
    }
    game2048_display_render(&game);
    return EXIT_SUCCESS;
}

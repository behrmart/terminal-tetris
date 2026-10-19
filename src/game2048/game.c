#include "game2048/game.h"

#include <stddef.h>

bool game2048_init(Game2048 *game, unsigned int size, uint32_t seed)
{
    if (game == NULL ||
        (size != GAME2048_SMALL_SIZE && size != GAME2048_MAX_SIZE)) {
        return false;
    }
    *game = (Game2048){0};
    (void)game2048_board_init(&game->board, size);
    game->initial_seed = seed;
    game->random_state = seed;
    game->status = GAME2048_RUNNING;
    return true;
}

void game2048_process_action(Game2048 *game, Game2048Action action)
{
    (void)game;
    (void)action;
}

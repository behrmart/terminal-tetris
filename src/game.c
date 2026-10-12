#include "game.h"

void game_init(Game *game, uint32_t seed)
{
    *game = (Game){0};
    board_init(&game->board);
    game->level = 1U;
    game->status = GAME_RUNNING;
    game->random_state = seed;
    game->bag_index = (unsigned int)TETROMINO_COUNT;
}

void game_process_action(Game *game, Action action)
{
    (void)game;
    (void)action;
}

void game_update(Game *game, unsigned int elapsed_ms)
{
    (void)game;
    (void)elapsed_ms;
}

#include "game2048/game.h"

#include <stddef.h>

/* Per-game Weyl sequence with a 32-bit mixer. Zero is a valid seed.
 * Fixed-width arithmetic gives identical results across supported targets.
 */
static uint32_t random_next(Game2048 *game)
{
    game->random_state = (uint32_t)((uint64_t)game->random_state +
                                   UINT64_C(0x9e3779b9));
    uint32_t value = game->random_state;
    value ^= value >> 16;
    value = (uint32_t)((uint64_t)value * UINT64_C(0x7feb352d));
    value ^= value >> 15;
    value = (uint32_t)((uint64_t)value * UINT64_C(0x846ca68b));
    return value ^ (value >> 16);
}

/* Rejection removes modulo bias. Callers use bounds 1..36.
 * The generator visits distinct outputs, so fewer than 36 can be rejected
 * consecutively (the rejected range is smaller than bound).
 */
static unsigned int random_below(Game2048 *game, unsigned int bound)
{
    const uint32_t threshold = (uint32_t)((UINT64_C(1) << 32) % bound);
    uint32_t value;
    do {
        value = random_next(game);
    } while (value < threshold);
    return (unsigned int)(value % bound);
}

static void spawn_tile(Game2048 *game)
{
    unsigned int empty = 0U;
    for (unsigned int y = 0U; y < game->board.size; ++y) {
        for (unsigned int x = 0U; x < game->board.size; ++x) {
            if (game->board.cells[y][x] == 0U) {
                ++empty;
            }
        }
    }
    if (empty == 0U) {
        return;
    }
    unsigned int chosen = random_below(game, empty);
    const Game2048Tile tile = random_below(game, 10U) == 0U ? 2U : 1U;
    for (unsigned int y = 0U; y < game->board.size; ++y) {
        for (unsigned int x = 0U; x < game->board.size; ++x) {
            if (game->board.cells[y][x] == 0U) {
                if (chosen == 0U) {
                    game->board.cells[y][x] = tile;
                    return;
                }
                --chosen;
            }
        }
    }
}

static void update_status(Game2048 *game)
{
    if (!game->win_acknowledged) {
        for (unsigned int y = 0U; y < game->board.size; ++y) {
            for (unsigned int x = 0U; x < game->board.size; ++x) {
                if (game->board.cells[y][x] >= GAME2048_TARGET_EXPONENT) {
                    game->status = GAME2048_WON;
                    return; /* Acknowledge a win before presenting a loss. */
                }
            }
        }
    }
    game->status = game2048_board_has_moves(&game->board) ?
                   GAME2048_RUNNING : GAME2048_OVER;
}

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
    spawn_tile(game);
    spawn_tile(game);
    return true;
}

void game2048_process_action(Game2048 *game, Game2048Action action)
{
    if (game == NULL) {
        return;
    }
    if (action == GAME2048_ACTION_RESTART) {
        (void)game2048_init(game, game->board.size, game->initial_seed);
        return;
    }
    if (action == GAME2048_ACTION_CONTINUE) {
        if (game->status == GAME2048_WON) {
            game->win_acknowledged = true;
            update_status(game);
        }
        return;
    }
    if (game->status != GAME2048_RUNNING) {
        return;
    }

    Game2048Direction direction;
    switch (action) {
    case GAME2048_ACTION_LEFT:  direction = GAME2048_LEFT; break;
    case GAME2048_ACTION_RIGHT: direction = GAME2048_RIGHT; break;
    case GAME2048_ACTION_UP:    direction = GAME2048_UP; break;
    case GAME2048_ACTION_DOWN:  direction = GAME2048_DOWN; break;
    default: return; /* Quit is host-owned; none/unknown actions are inert. */
    }
    const Game2048MoveResult result = game2048_board_move(&game->board, direction);
    if (!result.changed) {
        return; /* In particular, never advance random state on a no-op. */
    }
    game->score = result.score_gained > UINT64_MAX - game->score ?
                  UINT64_MAX : game->score + result.score_gained;
    spawn_tile(game);
    update_status(game);
}

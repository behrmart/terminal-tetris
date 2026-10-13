#include "game.h"

#include <limits.h>

/* Fixed-width LCG: reproducible on every target, including seed zero.
 * The wide intermediate makes the intended modulo-2^32 conversion explicit.
 * This is a game generator, not a cryptographic generator.
 */
static uint32_t random_next(Game *game)
{
    game->random_state = (uint32_t)(
        UINT64_C(1664525) * game->random_state + UINT64_C(1013904223));
    return game->random_state;
}

static TetrominoType draw_piece(Game *game)
{
    if (game->bag_index == (unsigned int)TETROMINO_COUNT) {
        for (unsigned int i = 0U; i < (unsigned int)TETROMINO_COUNT; ++i) {
            game->bag[i] = (TetrominoType)i;
        }
        for (unsigned int i = (unsigned int)TETROMINO_COUNT - 1U; i > 0U; --i) {
            const unsigned int j = (unsigned int)(random_next(game) % (i + 1U));
            const TetrominoType saved = game->bag[i];
            game->bag[i] = game->bag[j];
            game->bag[j] = saved;
        }
        game->bag_index = 0U;
    }
    return game->bag[game->bag_index++];
}

static void spawn_piece(Game *game)
{
    game->active = (Piece){
        .type = game->next,
        .rotation = 0U,
        .x = (BOARD_WIDTH - 4) / 2,
        .y = 0
    };
    game->next = draw_piece(game);
    game->gravity_elapsed_ms = 0U;
    game->has_active_piece = board_can_place(&game->board, &game->active);
    if (!game->has_active_piece) {
        game->status = GAME_OVER;
    }
}

static bool try_move(Game *game, int dx, int dy)
{
    Piece candidate = game->active;
    /* Initialized active pieces have board-bounded origins. */
    candidate.x += dx;
    candidate.y += dy;
    if (!board_can_place(&game->board, &candidate)) {
        return false;
    }
    game->active = candidate;
    return true;
}

static void rotate_piece(Game *game, unsigned int increment)
{
    Piece candidate = game->active;
    candidate.rotation = (candidate.rotation + increment) %
                         (unsigned int)TETROMINO_ROTATION_COUNT;
    if (board_can_place(&game->board, &candidate)) {
        game->active = candidate;
    }
}

static void lock_and_spawn(Game *game)
{
    if (!board_lock_piece(&game->board, &game->active)) {
        game->has_active_piece = false;
        game->status = GAME_OVER;
        return;
    }
    const unsigned int cleared = board_clear_lines(&game->board);
    /* Valid play clears at most four rows per lock. */
    static const unsigned int points[] = {0U, 100U, 300U, 500U, 800U};
    const unsigned int scoring_lines = cleared > 4U ? 4U : cleared;
    const uint64_t award = (uint64_t)points[scoring_lines] * game->level;
    game->score = award > UINT64_MAX - game->score ?
                  UINT64_MAX : game->score + award;
    game->lines = cleared > UINT_MAX - game->lines ?
                  UINT_MAX : game->lines + cleared;
    game->level = game->lines / 10U + 1U;
    spawn_piece(game);
}

void game_init(Game *game, uint32_t seed)
{
    *game = (Game){0};
    board_init(&game->board);
    game->level = 1U;
    game->status = GAME_RUNNING;
    game->initial_seed = seed;
    game->random_state = seed;
    game->bag_index = (unsigned int)TETROMINO_COUNT;
    game->next = draw_piece(game);
    spawn_piece(game);
}

void game_process_action(Game *game, Action action)
{
    if (action == ACTION_RESTART) {
        game_init(game, game->initial_seed);
        return;
    }
    if (action == ACTION_PAUSE) {
        if (game->status == GAME_RUNNING || game->status == GAME_PAUSED) {
            game->status = game->status == GAME_RUNNING ? GAME_PAUSED : GAME_RUNNING;
            game->gravity_elapsed_ms = 0U;
        }
        return;
    }
    if (game->status != GAME_RUNNING || !game->has_active_piece) {
        return;
    }
    switch (action) {
    case ACTION_LEFT:
        (void)try_move(game, -1, 0);
        break;
    case ACTION_RIGHT:
        (void)try_move(game, 1, 0);
        break;
    case ACTION_ROTATE_CW:
        rotate_piece(game, 1U);
        break;
    case ACTION_ROTATE_CCW:
        rotate_piece(game, (unsigned int)TETROMINO_ROTATION_COUNT - 1U);
        break;
    case ACTION_SOFT_DROP:
        if (!try_move(game, 0, 1)) {
            lock_and_spawn(game);
        }
        break;
    case ACTION_HARD_DROP:
        /* At most BOARD_HEIGHT attempts; never drop the newly spawned piece. */
        for (int step = 0; step < BOARD_HEIGHT; ++step) {
            if (!try_move(game, 0, 1)) {
                break;
            }
        }
        lock_and_spawn(game);
        break;
    default:
        /* None/quit and unknown actions are no-ops. */
        break;
    }
}

unsigned int game_gravity_interval_ms(const Game *game)
{
    if (game->level >= 10U) {
        return 100U;
    }
    return 1000U - (game->level - 1U) * 100U;
}

void game_update(Game *game, unsigned int elapsed_ms)
{
    if (game->status != GAME_RUNNING || !game->has_active_piece) {
        return;
    }
    const unsigned int accepted = elapsed_ms > (unsigned int)GAME_MAX_ELAPSED_MS ?
                                  (unsigned int)GAME_MAX_ELAPSED_MS : elapsed_ms;
    game->gravity_elapsed_ms += accepted;
    const unsigned int interval = game_gravity_interval_ms(game);
    if (game->gravity_elapsed_ms >= interval) {
        game->gravity_elapsed_ms -= interval;
        if (!try_move(game, 0, 1)) {
            lock_and_spawn(game);
        }
    }
}

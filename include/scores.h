#ifndef TERMINAL_GAMES_SCORES_H
#define TERMINAL_GAMES_SCORES_H

#include <stdbool.h>
#include <stdint.h>

enum { SCORE_NAME_MAX = 16, SCORE_PATH_MAX = 4096, SCORE_MODES = 3 };
typedef enum { SCORE_TETRIS, SCORE_2048_4, SCORE_2048_6 } ScoreMode;
typedef struct {
    char username[SCORE_NAME_MAX + 1];
    char path[SCORE_PATH_MAX];
    uint64_t best[SCORE_MODES];
    bool save_failed;
} ScoreProfile;

/* Host-only persistence; engines never include this header.
 * Names are case-sensitive ASCII letters/digits, underscore or hyphen.
 * directory is an absolute score directory, or NULL for the XDG/HOME default.
 * Invalid/malformed existing files are errors, never silently overwritten.
 */
bool scores_valid_name(const char *name);
bool scores_open(ScoreProfile *profile, const char *name, const char *directory);
bool scores_save(ScoreProfile *profile);
/* Track and save only improvements. Failed saves retain the in-memory best. */
void scores_record(ScoreProfile *profile, ScoreMode mode, uint64_t score);

#endif

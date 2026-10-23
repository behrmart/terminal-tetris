#define _POSIX_C_SOURCE 200809L
#include "scores.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

bool scores_valid_name(const char *name)
{
    if (name == NULL || name[0] == '\0') {
        return false;
    }
    size_t length = 0U;
    for (; name[length] != '\0'; ++length) {
        const char c = name[length];
        if (length >= SCORE_NAME_MAX ||
            !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static bool make_directory(const char *directory)
{
    char path[SCORE_PATH_MAX];
    const size_t length = strlen(directory);
    if (directory[0] != '/' || length >= sizeof(path)) {
        return false;
    }
    (void)memcpy(path, directory, length + 1U);
    for (size_t i = 1U; i <= length; ++i) {
        if (path[i] == '/' || path[i] == '\0') {
            const char saved = path[i];
            path[i] = '\0';
            struct stat info;
            if (mkdir(path, 0700) != 0 && errno != EEXIST) {
                return false;
            }
            if (stat(path, &info) != 0 || !S_ISDIR(info.st_mode)) {
                return false;
            }
            path[i] = saved;
        }
    }
    return true;
}

/* Strict decimal parser rejects signs, overflow and trailing garbage. */
static bool read_value(FILE *file, uint64_t *value)
{
    uint64_t result = 0U;
    unsigned int digits = 0U;
    int c;
    while ((c = fgetc(file)) != '\n') {
        if (c < '0' || c > '9' || ++digits > 20U) {
            return false;
        }
        const uint64_t digit = (uint64_t)(c - '0');
        if (result > (UINT64_MAX - digit) / UINT64_C(10)) {
            return false;
        }
        result = result * UINT64_C(10) + digit;
    }
    *value = result;
    return digits != 0U;
}

static bool read_scores(const char *path, uint64_t best[SCORE_MODES])
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return errno == ENOENT;
    }
    char version[32];
    bool ok = fgets(version, sizeof(version), file) != NULL;
    unsigned int count = 0U;
    if (ok && strcmp(version, "TERMINAL-GAMES-SCORES-1\n") == 0) {
        count = 3U;
    } else if (ok && strcmp(version, "TERMINAL-GAMES-SCORES-2\n") == 0) {
        count = SCORE_MODES;
    } else {
        ok = false;
    }
    for (unsigned int i = 0U; i < SCORE_MODES; ++i) {
        best[i] = 0U;
    }
    for (unsigned int i = 0U; ok && i < count; ++i) {
        ok = read_value(file, &best[i]);
    }
    if (ok) {
        ok = fgetc(file) == EOF && !ferror(file);
    }
    if (fclose(file) != 0) {
        ok = false;
    }
    return ok;
}

bool scores_open(ScoreProfile *profile, const char *name, const char *directory)
{
    if (profile == NULL || !scores_valid_name(name)) {
        return false;
    }
    *profile = (ScoreProfile){0};
    (void)memcpy(profile->username, name, strlen(name) + 1U);
    char default_directory[SCORE_PATH_MAX];
    if (directory == NULL) {
        const char *base = getenv("XDG_STATE_HOME");
        int written;
        if (base != NULL && base[0] == '/') {
            written = snprintf(default_directory, sizeof(default_directory),
                               "%s/terminal-games/scores", base);
        } else {
            base = getenv("HOME");
            if (base == NULL || base[0] != '/') {
                return false;
            }
            written = snprintf(default_directory, sizeof(default_directory),
                               "%s/.local/state/terminal-games/scores", base);
        }
        if (written < 0 || (size_t)written >= sizeof(default_directory)) {
            return false;
        }
        directory = default_directory;
    }
    if (!make_directory(directory)) {
        return false;
    }
    const int written = snprintf(profile->path, sizeof(profile->path),
                                 "%s/%s.txt", directory, name);
    if (written < 0 || (size_t)written >= sizeof(profile->path) ||
        !read_scores(profile->path, profile->best)) {
        return false;
    }
    return scores_save(profile); /* Create even a new player's zero scores. */
}

bool scores_save(ScoreProfile *profile)
{
    char lock_path[SCORE_PATH_MAX + 16];
    char temporary[SCORE_PATH_MAX + 16];
    (void)snprintf(lock_path, sizeof(lock_path), "%s.lock", profile->path);
    (void)snprintf(temporary, sizeof(temporary), "%s.XXXXXX", profile->path);
    const int lock = open(lock_path, O_CREAT | O_RDWR, 0600);
    if (lock < 0) {
        profile->save_failed = true;
        return false;
    }
    bool ok = flock(lock, LOCK_EX | LOCK_NB) == 0;
    uint64_t merged[SCORE_MODES] = {0};
    if (ok) {
        ok = read_scores(profile->path, merged);
    }
    if (ok) {
        for (unsigned int i = 0U; i < SCORE_MODES; ++i) {
            if (profile->best[i] > merged[i]) {
                merged[i] = profile->best[i];
            }
        }
        const int descriptor = mkstemp(temporary);
        if (descriptor < 0) {
            ok = false;
        } else {
            FILE *file = fdopen(descriptor, "w");
            if (file == NULL) {
                (void)close(descriptor);
                ok = false;
            } else {
                ok = fputs("TERMINAL-GAMES-SCORES-2\n", file) >= 0;
                for (unsigned int i = 0U; ok && i < SCORE_MODES; ++i) {
                    ok = fprintf(file, "%" PRIu64 "\n", merged[i]) >= 0;
                }
                if (fflush(file) != 0 || fsync(descriptor) != 0) {
                    ok = false;
                }
                if (fclose(file) != 0) {
                    ok = false;
                }
            }
            if (ok) {
                ok = rename(temporary, profile->path) == 0;
            }
            if (!ok) {
                (void)unlink(temporary);
            }
        }
    }
    if (ok) {
        for (unsigned int i = 0U; i < SCORE_MODES; ++i) {
            profile->best[i] = merged[i];
        }
    }
    (void)close(lock); /* Releases the lock on every path. */
    profile->save_failed = !ok;
    return ok;
}

void scores_record(ScoreProfile *profile, ScoreMode mode, uint64_t score)
{
    if (mode < SCORE_TETRIS || mode > SCORE_DRUGWARS) {
        return;
    }
    if (score > profile->best[mode]) {
        profile->best[mode] = score;
        (void)scores_save(profile);
    }
}

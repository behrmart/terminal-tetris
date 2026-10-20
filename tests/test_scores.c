#define _POSIX_C_SOURCE 200809L
#include "scores.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

int main(void)
{
    char directory[] = "/tmp/terminal-games-scores-XXXXXX";
    assert(mkdtemp(directory) != NULL);
    assert(scores_valid_name("Alice_2-3"));
    assert(!scores_valid_name(""));
    assert(!scores_valid_name(NULL));
    assert(!scores_valid_name("../alice"));
    assert(!scores_valid_name("two words"));
    assert(!scores_valid_name("abcdefghijklmnopq"));
    ScoreProfile alice;
    ScoreProfile stale;
    ScoreProfile loaded;
    assert(scores_open(&alice, "Alice", directory));
    assert(scores_open(&stale, "Alice", directory));
    scores_record(&alice, SCORE_TETRIS, UINT64_MAX);
    scores_record(&alice, SCORE_2048_4, UINT64_C(1024));
    scores_record(&stale, SCORE_2048_6, UINT64_C(4096));
    scores_record(&stale, SCORE_TETRIS, UINT64_C(1));
    assert(scores_open(&loaded, "Alice", directory));
    assert(loaded.best[0] == UINT64_MAX);
    assert(loaded.best[1] == UINT64_C(1024));
    assert(loaded.best[2] == UINT64_C(4096));

    ScoreProfile bob;
    assert(scores_open(&bob, "Bob", directory));
    assert(bob.best[0] == 0U && bob.best[1] == 0U && bob.best[2] == 0U);
    char lock_path[SCORE_PATH_MAX + 16];
    (void)snprintf(lock_path, sizeof(lock_path), "%s.lock", bob.path);
    const int lock = open(lock_path, O_RDWR);
    assert(lock >= 0 && flock(lock, LOCK_EX | LOCK_NB) == 0);
    scores_record(&bob, SCORE_TETRIS, UINT64_C(100));
    assert(bob.save_failed && bob.best[0] == UINT64_C(100));
    assert(close(lock) == 0);
    assert(scores_save(&bob) && !bob.save_failed);
    assert(scores_open(&loaded, "Bob", directory));
    assert(loaded.best[0] == UINT64_C(100));

    /* Bad files must survive failed reads/saves byte-for-byte. */
    static const char *bad[] = {
        "garbage\n", "TERMINAL-GAMES-SCORES-1\n-1\n0\n0\n",
        "TERMINAL-GAMES-SCORES-1\n18446744073709551616\n0\n0\n",
        "TERMINAL-GAMES-SCORES-1\n1\n2\n",
        "TERMINAL-GAMES-SCORES-1\n1\n2\n3\nextra\n"
    };
    for (size_t i = 0U; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        FILE *file = fopen(bob.path, "w");
        assert(file != NULL && fputs(bad[i], file) >= 0);
        assert(fclose(file) == 0);
        assert(!scores_open(&loaded, "Bob", directory));
        assert(!scores_save(&bob));
        char buffer[128] = {0};
        file = fopen(bob.path, "r");
        assert(file != NULL);
        assert(fread(buffer, 1U, sizeof(buffer) - 1U, file) == strlen(bad[i]));
        assert(fclose(file) == 0 && strcmp(buffer, bad[i]) == 0);
    }
    assert(!scores_open(&loaded, "bad/name", directory));
    assert(!scores_open(&loaded, "Alice", "relative"));
    assert(!scores_open(&loaded, "Alice", bob.path)); /* Not a directory. */
    assert(unlink(bob.path) == 0);
    assert(unlink(lock_path) == 0);
    assert(unlink(alice.path) == 0);
    (void)snprintf(lock_path, sizeof(lock_path), "%s.lock", alice.path);
    assert(unlink(lock_path) == 0);
    assert(rmdir(directory) == 0);
    (void)puts("Score tests passed (names, reload, modes, max values, merge, locks, malformed files).");
    return 0;
}

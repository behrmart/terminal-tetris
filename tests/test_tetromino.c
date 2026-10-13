#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "tetromino.h"

static bool contains(const TetrominoCell *cells, int x, int y)
{
    for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
        if (cells[i].x == x && cells[i].y == y) {
            return true;
        }
    }
    return false;
}

int main(void)
{
    /* Readable, independent specification of the initial silhouettes. */
    static const char *const initial[TETROMINO_COUNT] = {
        "...." "####" "...." "....",
        ".##." ".##." "...." "....",
        ".#.." "###." "...." "....",
        ".##." "##.." "...." "....",
        "##.." ".##." "...." "....",
        "#..." "###." "...." "....",
        "..#." "###." "...." "...."
    };

    for (int type = 0; type < TETROMINO_COUNT; ++type) {
        const TetrominoCell *first = tetromino_cells((TetrominoType)type, 0U);
        assert(first != NULL);
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                assert(contains(first, x, y) == (initial[type][y * 4 + x] == '#'));
            }
        }
        for (unsigned int rotation = 0U;
             rotation < (unsigned int)TETROMINO_ROTATION_COUNT; ++rotation) {
            const TetrominoCell *cells =
                tetromino_cells((TetrominoType)type, rotation);
            const TetrominoCell *next =
                tetromino_cells((TetrominoType)type, (rotation + 1U) % 4U);
            assert(cells != NULL && next != NULL);
            for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                assert(cells[i].x >= 0 && cells[i].x < 4);
                assert(cells[i].y >= 0 && cells[i].y < 4);
                for (int j = i + 1; j < TETROMINO_CELL_COUNT; ++j) {
                    assert(cells[i].x != cells[j].x || cells[i].y != cells[j].y);
                }
                /* Check geometric clockwise rotation, including 3 -> 0. */
                if (type == TETROMINO_O) {
                    assert(contains(next, cells[i].x, cells[i].y));
                } else {
                    const int pivot_twice = type == TETROMINO_I ? 3 : 2;
                    assert(contains(next, pivot_twice - cells[i].y, cells[i].x));
                }
            }
        }
    }
    assert(tetromino_cells((TetrominoType)-1, 0U) == NULL);
    assert(tetromino_cells(TETROMINO_COUNT, 0U) == NULL);
    assert(tetromino_cells(TETROMINO_I, 4U) == NULL);
    assert(tetromino_cells(TETROMINO_I, UINT_MAX) == NULL);
    (void)puts("Tetromino tests passed (seven shapes, four rotations).");
    return 0;
}

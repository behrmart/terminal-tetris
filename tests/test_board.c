#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "board.h"

static void assert_same_board(const Board *left, const Board *right)
{
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            assert(left->cells[y][x] == right->cells[y][x]);
        }
    }
}

static void assert_rejected(Board *board, const Piece *piece)
{
    const Board before = *board;
    assert(!board_can_place(board, piece));
    assert(!board_lock_piece(board, piece));
    assert_same_board(board, &before);
}

static void test_boundaries(void)
{
    Board board;
    board_init(&board);
    const Board empty = board;

    for (int type = 0; type < TETROMINO_COUNT; ++type) {
        for (unsigned int rotation = 0U; rotation < 4U; ++rotation) {
            const TetrominoCell *cells =
                tetromino_cells((TetrominoType)type, rotation);
            /* Small origins allow an independent direct-addition oracle. */
            for (int y = -4; y <= BOARD_HEIGHT; ++y) {
                for (int x = -4; x <= BOARD_WIDTH; ++x) {
                    const Piece piece = {(TetrominoType)type, rotation, x, y};
                    bool expected = true;
                    for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                        const int cell_x = x + cells[i].x;
                        const int cell_y = y + cells[i].y;
                        if (cell_x < 0 || cell_x >= BOARD_WIDTH ||
                            cell_y < 0 || cell_y >= BOARD_HEIGHT) {
                            expected = false;
                        }
                    }
                    assert(board_can_place(&board, &piece) == expected);
                    if (!expected) {
                        assert_rejected(&board, &piece);
                    }
                }
            }
        }
    }
    assert_same_board(&board, &empty);

    Piece piece = {TETROMINO_I, 0U, 0, -1};
    assert(board_can_place(&board, &piece)); /* Empty top margin is allowed. */
    piece = (Piece){TETROMINO_I, 1U, -2, 0};
    assert(board_can_place(&board, &piece)); /* Empty left margin is allowed. */
    piece.x = INT_MIN;
    assert_rejected(&board, &piece);
    piece.x = INT_MAX;
    assert_rejected(&board, &piece);
    piece.x = 0;
    piece.y = INT_MIN;
    assert_rejected(&board, &piece);
    piece.y = INT_MAX;
    assert_rejected(&board, &piece);
}

static void test_lock_and_overlap(void)
{
    static const Cell expected_identity[TETROMINO_COUNT] = {
        CELL_I, CELL_O, CELL_T, CELL_S, CELL_Z, CELL_J, CELL_L
    };
    Board board;

    for (int type = 0; type < TETROMINO_COUNT; ++type) {
        for (unsigned int rotation = 0U; rotation < 4U; ++rotation) {
            const Piece piece = {(TetrominoType)type, rotation, 3, 5};
            const TetrominoCell *cells =
                tetromino_cells(piece.type, piece.rotation);
            board_init(&board);
            board.cells[19][9] = CELL_Z;
            Board expected = board;
            for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                expected.cells[piece.y + cells[i].y][piece.x + cells[i].x] =
                    expected_identity[type];
            }
            assert(board_can_place(&board, &piece));
            assert(board_lock_piece(&board, &piece));
            assert_same_board(&board, &expected);
            assert_rejected(&board, &piece); /* Cannot overwrite locked cells. */

            for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                board_init(&board);
                board.cells[piece.y + cells[i].y][piece.x + cells[i].x] = CELL_L;
                assert_rejected(&board, &piece);
            }

            /* A locked block in an empty part of the 4x4 box is harmless. */
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {
                    bool occupied = false;
                    for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
                        if (cells[i].x == x && cells[i].y == y) {
                            occupied = true;
                        }
                    }
                    if (!occupied) {
                        board_init(&board);
                        board.cells[piece.y + y][piece.x + x] = CELL_J;
                        assert(board_can_place(&board, &piece));
                    }
                }
            }
        }
    }

    /* A rotation can be obstructed even when the current pose fits. */
    board_init(&board);
    Piece piece = {TETROMINO_I, 0U, 3, 5};
    board.cells[5][5] = CELL_T;
    assert(board_can_place(&board, &piece));
    piece.rotation = 1U;
    assert_rejected(&board, &piece);
}

static void test_invalid_inputs(void)
{
    Board board;
    board_init(&board);
    Piece piece = {TETROMINO_COUNT, 0U, 0, 0};
    assert_rejected(&board, &piece);
    piece.type = (TetrominoType)-1;
    assert_rejected(&board, &piece);
    piece.type = TETROMINO_T;
    piece.rotation = UINT_MAX;
    assert_rejected(&board, &piece);
    assert_rejected(&board, NULL);
    assert(!board_can_place(NULL, &piece));
    assert(!board_lock_piece(NULL, &piece));
}

int main(void)
{
    test_boundaries();
    test_lock_and_overlap();
    test_invalid_inputs();
    (void)puts("Board tests passed (bounds, collision, atomic locking).");
    return 0;
}

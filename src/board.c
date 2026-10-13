#include "board.h"

#include <stddef.h>

void board_init(Board *board)
{
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            board->cells[y][x] = CELL_EMPTY;
        }
    }
}

bool board_can_place(const Board *board, const Piece *piece)
{
    if (board == NULL || piece == NULL) {
        return false;
    }
    const TetrominoCell *cells = tetromino_cells(piece->type, piece->rotation);
    if (cells == NULL) {
        return false;
    }

    for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
        /* Check before adding, so even INT_MIN/INT_MAX origins are safe. */
        if (piece->x < -cells[i].x ||
            piece->x >= BOARD_WIDTH - cells[i].x ||
            piece->y < -cells[i].y ||
            piece->y >= BOARD_HEIGHT - cells[i].y) {
            return false;
        }
        const int x = piece->x + cells[i].x;
        const int y = piece->y + cells[i].y;
        if (board->cells[y][x] != CELL_EMPTY) {
            return false;
        }
    }
    return true;
}

bool board_lock_piece(Board *board, const Piece *piece)
{
    static const Cell identities[TETROMINO_COUNT] = {
        CELL_I, CELL_O, CELL_T, CELL_S, CELL_Z, CELL_J, CELL_L
    };

    if (!board_can_place(board, piece)) {
        return false;
    }
    const TetrominoCell *cells = tetromino_cells(piece->type, piece->rotation);
    for (int i = 0; i < TETROMINO_CELL_COUNT; ++i) {
        const int x = piece->x + cells[i].x;
        const int y = piece->y + cells[i].y;
        board->cells[y][x] = identities[piece->type];
    }
    return true;
}

unsigned int board_clear_lines(Board *board)
{
    int destination = BOARD_HEIGHT - 1;
    unsigned int cleared = 0U;
    for (int source = BOARD_HEIGHT - 1; source >= 0; --source) {
        bool full = true;
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (board->cells[source][x] == CELL_EMPTY) {
                full = false;
                break;
            }
        }
        if (full) {
            ++cleared;
        } else {
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                board->cells[destination][x] = board->cells[source][x];
            }
            --destination;
        }
    }
    for (int y = destination; y >= 0; --y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            board->cells[y][x] = CELL_EMPTY;
        }
    }
    return cleared;
}

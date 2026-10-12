#ifndef TERMINAL_TETRIS_TETROMINO_H
#define TERMINAL_TETRIS_TETROMINO_H

typedef enum {
    TETROMINO_I, TETROMINO_O, TETROMINO_T, TETROMINO_S,
    TETROMINO_Z, TETROMINO_J, TETROMINO_L, TETROMINO_COUNT
} TetrominoType;

typedef struct {
    TetrominoType type;
    unsigned int rotation;
    int x;
    int y;
} Piece;

/* Rotation tables and occupied-cell queries belong to a later milestone. */

#endif

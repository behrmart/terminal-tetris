#ifndef TERMINAL_TETRIS_TETROMINO_H
#define TERMINAL_TETRIS_TETROMINO_H

typedef enum {
    TETROMINO_I, TETROMINO_O, TETROMINO_T, TETROMINO_S,
    TETROMINO_Z, TETROMINO_J, TETROMINO_L, TETROMINO_COUNT
} TetrominoType;

enum { TETROMINO_CELL_COUNT = 4, TETROMINO_ROTATION_COUNT = 4 };

typedef struct {
    int x;
    int y;
} TetrominoCell;

typedef struct {
    TetrominoType type;
    unsigned int rotation;
    int x;
    int y;
} Piece;

/* Four read-only offsets inside a local 4 x 4 box, or NULL for an invalid
 * type/rotation. Rotation indices 0..3 advance clockwise; O is invariant.
 * Coordinates increase rightward and downward. Returned storage is static.
 */
const TetrominoCell *tetromino_cells(TetrominoType type,
                                    unsigned int rotation);

#endif

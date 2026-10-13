# Terminal Tetris

A native Ubuntu Linux terminal Tetris project, written in ISO C11 with
ncurses. **Milestone 2:** all seven shape tables, four rotation states per
shape, board placement validation and atomic piece locking. The terminal
still displays an empty 10 x 20 field. Movement, spawning, gravity, scoring,
clearing, previews, pause and restart are not implemented yet.

## Build and run

Install the Ubuntu toolchain:

```sh
sudo apt update
sudo apt install build-essential pkg-config libncurses-dev gdb git
make clean && make
make test
make run
```

The executable is build/terminal-tetris. Run it in an interactive terminal
with a valid TERM setting. Press Q or q to quit. SIGINT (Ctrl-C) and
SIGTERM request orderly shutdown and restoration of terminal settings.

The display requires at least **24 columns x 26 rows**. Each board cell
uses two terminal columns. Smaller terminals show a clipped size message
and remain responsive to Q; enlarging the terminal restores the field.
The title and border use cyan where color is available, with a monochrome
fallback. Borders use plain ASCII (+, - and |) for compatibility with
PuTTY sessions that display alternate-character-set borders as letters.
Keyboard polling waits up to 50 ms to avoid a busy loop.

## Make targets

- make / make all: build with GCC, debug symbols and strict warnings.
- make run: build and launch the program.
- make debug: build and launch the program under GDB.
- make test: compile and run game, board and tetromino tests without ncurses.
- make clean: remove generated files from build/.

Compiler flags: -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g.
ncurses flags are resolved through pkg-config. Header dependencies are
generated automatically for application objects.

## Layout and API

- include/: module interfaces and fixed-size data structures.
- src/board.c: board initialization, placement validation and atomic locking.
- src/tetromino.c: read-only shape tables and validated offset queries.
- src/game.c: deterministic initialization and inert action/time API.
- src/input.c: terminal key translation.
- src/display.c: ncurses initialization, rendering and cleanup.
- src/main.c: lifecycle and shutdown signals.
- tests/test_game.c: dimensions, empty board and initial state checks.
- tests/test_tetromino.c: silhouettes, clockwise rotation and invalid queries.
- tests/test_board.c: boundaries, overlaps, identities and failed-lock atomicity.
- AGENTS.md: contributor architecture, conventions and verification rules.

The engine accepts game_process_action(game, action) separately from
game_update(game, elapsed_ms). Neither function implements gameplay yet.
Initialization accepts a seed and reserves per-game bag/random state.
No engine module depends on ncurses, OS clocks or keyboard APIs.

Future timing must cap elapsed updates at 100 ms, discard excessive time,
ignore paused time, and reset the host clock baseline after resume or
terminal recovery. The current program has no clock or gravity to catch up.

## Milestone 2 engine API

- tetromino_cells(type, rotation): return four const offsets in a local
  4 x 4 box; invalid types or indices outside 0..3 return NULL.
- board_can_place(board, piece): read-only bounds and overlap check.
- board_lock_piece(board, piece): validate all four cells before storing
  their piece identity. Return false without modifying the board on failure.

Coordinates increase rightward and downward. Rotations advance clockwise.
I uses a (1.5, 1.5) pivot, J/L/S/T/Z use (1, 1), and O stays fixed.
These are shape definitions only; no rotation actions or wall kicks occur.
Empty margins may extend outside the board, but every occupied cell must
be within the visible 10 x 20 area. There are no hidden spawn rows.
Extreme origins are rejected before addition to avoid signed overflow.
Locking does not clear rows or change the supplied piece.

Tests cover all 28 shape states, clockwise wraparound, occupied-cell bounds,
overlap with each block, harmless blocks in empty shape margins, invalid
types/rotations, extreme coordinates and board preservation after failure.

## Next milestones

3. Deterministic movement, spawning and game-state transitions.
4. Gravity, line clearing, scoring, levels, pause and restart.
5. Full terminal gameplay, preview and integration checks.

These milestones require separate authorization. No modern Tetris
Guideline compliance is claimed.

## Verification status

Milestone 1 verification was reported passed by the user on Hyrule-2:

- make clean && make: successful GCC build with all required warning flags;
  no compiler warnings or errors appeared in the supplied output.
- make test: passed ("Milestone 1 structure tests passed.").
- make run: reported passed; the supplied transcript shows the executable
  launching and returning to the shell without a reported error.

These results supersede the initial authoring-host tooling limitation.
Specific Q/q, signal cleanup, undersized-terminal and resize-recovery checks
were not individually reported.

The user subsequently confirmed that the ASCII border displays correctly
in PuTTY.

Milestone 2 build and tests were verified by the user on Hyrule-2 on
2026-10-13. The supplied output confirms:

- make clean && make: successful GCC build with all required warning flags,
  with no compiler warnings or errors.
- make test: all three suites passed: game initialization, board bounds /
  collision / atomic locking, and seven tetromino shapes / four rotations.

These results supersede the earlier authoring-host verification limitation.
No terminal rendering code was changed for Milestone 2.

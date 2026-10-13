# Terminal Tetris

A native Ubuntu Linux terminal Tetris project, written in ISO C11 with
ncurses. **Milestone 4:** the engine supports seeded seven-piece bags,
movement, gravity, line clearing, scoring, levels, pause and restart.
The terminal still displays an empty 10 x 20 field and accepts Q only;
interactive gameplay and preview rendering are not wired up yet.

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
- src/board.c: board initialization, placement validation, locking and line compaction.
- src/tetromino.c: read-only shape tables and validated offset queries.
- src/game.c: deterministic gameplay, gravity, scoring and state transitions.
- src/input.c: terminal key translation.
- src/display.c: ncurses initialization, rendering and cleanup.
- src/main.c: lifecycle and shutdown signals.
- tests/test_game.c: bags, movement, gravity, scoring, pause, restart and replay.
- tests/test_tetromino.c: silhouettes, clockwise rotation and invalid queries.
- tests/test_board.c: boundaries, overlaps, locking and line compaction.
- AGENTS.md: contributor architecture, conventions and verification rules.

The engine accepts game_process_action(game, action) separately from
game_update(game, elapsed_ms). Actions implement manual gameplay in the
engine; elapsed-time updates apply gravity.
Initialization accepts a seed and maintains per-game bag/random state.
No engine module depends on ncurses, OS clocks or keyboard APIs.

Elapsed updates are capped at 100 ms and discard excess time. The engine
ignores paused time. Milestone 5 must reset the host clock baseline after
resume, restart and recovery from an undersized terminal.

## Milestone 2 engine API

- tetromino_cells(type, rotation): return four const offsets in a local
  4 x 4 box; invalid types or indices outside 0..3 return NULL.
- board_can_place(board, piece): read-only bounds and overlap check.
- board_lock_piece(board, piece): validate all four cells before storing
  their piece identity. Return false without modifying the board on failure.

Coordinates increase rightward and downward. Rotations advance clockwise.
I uses a (1.5, 1.5) pivot, J/L/S/T/Z use (1, 1), and O stays fixed.
The shape module only defines offsets; game actions apply rotations without
wall kicks.
Empty margins may extend outside the board, but every occupied cell must
be within the visible 10 x 20 area. There are no hidden spawn rows.
Extreme origins are rejected before addition to avoid signed overflow.
board_lock_piece does not clear rows or change the supplied piece.
board_clear_lines removes full rows, preserves the order of surviving rows,
clears vacated top rows and returns the number removed.

Tests cover all 28 shape states, clockwise wraparound, occupied-cell bounds,
overlap with each block, harmless blocks in empty shape margins, invalid
types/rotations, extreme coordinates and board preservation after failure.

## Milestone 3 behavior

game_init clears state, shuffles a seven-piece bag using a fixed-width
per-game generator, and prepares an active piece plus the next-piece type.
Each bag contains every tetromino once; all seeds, including zero, work.
Identical seeds and action/time sequences produce identical game states.

Pieces spawn at rotation 0, origin (3, 0). Left/right and clockwise/
counterclockwise actions commit only when all occupied cells fit.
Soft drop moves one row or locks immediately when blocked. Hard drop
finds the landing position in at most 20 attempts and locks exactly one
piece. Locking promotes the preview to active and draws the next preview.
A blocked spawn sets GAME_OVER and has_active_piece to false; subsequent
movement/time updates cannot change that state. ACTION_RESTART starts a
new game using the original seed, including after game over.

The active piece is never written into the board until it locks.

Tests cover multiple seeds, twenty seven-piece bags per seed, independent
interleaved games, preview promotion, wall/block collisions, both rotations,
soft/hard drop locking, blocked spawns and deterministic action replay.

## Milestone 4 behavior

- Gravity starts at 1000 ms per row and decreases by 100 ms per level,
  reaching a minimum of 100 ms at level 10. game_gravity_interval_ms exposes
  the current interval.
- game_update accepts at most 100 ms per call and discards excess, even for
  UINT_MAX elapsed time. At most one downward attempt occurs per call.
  Fractional time is retained on movement and discarded on locking/spawning.
- Every lock clears full rows before checking the next spawn. Clearing
  1/2/3/4 rows earns 100/300/500/800 points times the level before that clear.
  Soft/hard drops do not award points.
- Level starts at 1 and increases every 10 total cleared lines. Score and
  line counters saturate at their type limits rather than wrap.
- ACTION_PAUSE toggles running/paused and resets fractional gravity time.
  While paused, movement and elapsed time are ignored. Pause does nothing
  after game over.
- ACTION_RESTART works from running, paused or game-over state. It resets
  the board, score, lines, level, timing and generator to the original seed.
  To choose a different seed, call game_init explicitly.

Tests include adjacent/separated/top/all-row compaction, each scoring tier,
level thresholds, gravity timing and stall caps, clearing before spawning,
pause/resume, restart from all statuses and counter saturation.

## Next milestone

5. Full terminal gameplay, preview and integration checks.

Milestone 5 requires separate authorization. No modern Tetris
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

Milestone 3 build and tests were verified by the user on Hyrule-2 on
2026-10-13. The supplied output confirms:

- make clean && make: successful GCC build with all required warning flags,
  with no compiler warnings or errors.
- make test: all three suites passed: game (bags, spawning, movement, drops,
  game over and replay), board, and tetromino.

These results supersede the earlier authoring-host verification limitation.
No terminal rendering or input code was changed for Milestone 3.

Milestone 4 build and tests were verified by the user on Hyrule-2 on
2026-10-13. The supplied output confirms:

- make clean && make: successful GCC build with all required warning flags,
  with no compiler warnings or errors.
- make test: all three suites passed: game (movement, bags, gravity, scoring,
  pause, restart and replay), board (including line clearing), and tetromino.

These results supersede the earlier authoring-host verification limitation.
No terminal rendering, input or host clock integration changed in this milestone.

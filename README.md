# Terminal Tetris

A native Ubuntu Linux terminal Tetris project, written in ISO C11 with
ncurses. **Milestone 5:** playable terminal Tetris with colored pieces,
a next-piece preview, gravity, line clearing, scoring, levels, pause,
restart and game-over handling.

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
SIGTERM/SIGHUP request orderly shutdown and restoration of terminal settings.

The display requires at least **54 columns x 26 rows**. Each board cell
uses two terminal columns. Pieces have colored backgrounds; monochrome
terminals still show [] blocks. Borders use plain ASCII for PuTTY.
Smaller terminals freeze gameplay and accept only Q. Enlarging the window
restores the previous running/paused/game-over state without catching up.
The first recovery input is discarded; press a key again after the display
returns. Input polling waits up to 50 ms for an initial key; ncurses also
waits for the remainder of terminal escape sequences such as arrow keys.

## Controls

| Key | Action |
| --- | --- |
| Left / Right | Move |
| Down | Soft drop |
| Up / X | Rotate clockwise |
| Z | Rotate counterclockwise |
| Space | Hard drop |
| P | Pause / resume |
| R | Restart (also after game over) |
| Q | Quit |

Letter controls accept either case. Restart replays the current launch's
piece sequence. A new launch seeds a new sequence from the monotonic clock.
The sidebar shows the next piece, score, lines, level and game status.

## Make targets

- make / make all: build with GCC, debug symbols and strict warnings.
- make run: build and launch the program.
- make debug: build and launch the program under GDB.
- make test: compile and run game, board and tetromino tests without ncurses.
- make test-terminal: run ncurses screen/input checks and Linux PTY smoke tests.
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
- src/main.c: monotonic game loop, resize freezing and shutdown signals.
- tests/test_game.c: bags, movement, gravity, scoring, pause, restart and replay.
- tests/test_tetromino.c: silhouettes, clockwise rotation and invalid queries.
- tests/test_board.c: boundaries, overlaps, locking and line compaction.
- tests/test_terminal.c: ncurses screen buffer and input mapping checks.
- tests/test_terminal.py: Linux PTY rendering, startup, resize and cleanup checks.
- AGENTS.md: contributor architecture, conventions and verification rules.

The engine accepts game_process_action(game, action) separately from
game_update(game, elapsed_ms). Actions implement manual gameplay in the
engine; elapsed-time updates apply gravity.
Initialization accepts a seed and maintains per-game bag/random state.
No engine module depends on ncurses, OS clocks or keyboard APIs.

Elapsed updates are capped at 100 ms and discard excess time. The engine
ignores paused time. The host resets its clock baseline after
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

## Terminal integration checks

The game is C11 with ncurses; Python 3 is used only by the optional PTY
test harness. Install it if needed, then run:

```sh
sudo apt install python3
make test-terminal
```

The terminal tests inspect all seven active shapes and previews, borders,
status text, large numeric values, bindings and resize rendering in both
color and monochrome modes. PTY checks launch the actual executable and
verify Q/q and SIGINT/SIGTERM exits restore terminal attributes. They also
exercise undersized startup and resize recovery.

For a final PuTTY check, run make run and verify movement, both rotations,
soft/hard drop, increasing score after clearing a row, pause/resume,
restart, game over and resize freezing/recovery. Core tests cover the
underlying deterministic mechanics; the PTY smoke tests do not automate
an entire game or verify PuTTY's visual appearance.

The game uses immediate locking, no wall kicks, no hold/ghost piece and
no persistent high scores. No modern Tetris Guideline compliance is claimed.

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

Milestone 5 terminal verification was reported by the user on Hyrule-2 on
2026-10-13:

- make test-terminal: compiled the terminal test executable with no warnings
  or errors in the supplied output; all rendering/input checks passed for
  xterm and vt100.
- PTY startup/cleanup checks passed for q, Q, SIGINT and SIGTERM.
- Small-terminal and resize recovery checks passed.
- make run launched and returned to the shell. The supplied PuTTY capture
  shows an active piece, next-piece preview, score, lines, level, running
  status and controls in the expected layout.

A fresh make clean && make and make test result for Milestone 5 was not
included in this report. The complete manual gameplay checklist and color
appearance cannot be confirmed from the plain-text capture alone.

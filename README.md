# Terminal Games

A native Ubuntu Linux terminal Tetris project, written in ISO C11 with
ncurses. **Milestone 5:** playable terminal Tetris with colored pieces,
a next-piece preview, gravity, line clearing, scoring, levels, pause,
restart and game-over handling.

## Terminal 2048 (Phase 5)

Terminal 2048 now has a playable ncurses interface for 4x4 and 6x6 boards,
with colored tiles, score, win/loss messages, restart and continued play
after reaching 2048.

    make all
    make test-2048
    make test-2048-terminal
    make run-2048

The launcher links both games; their engine modules remain isolated. You can also launch ./build/terminal-2048
directly. At startup press 4 for 4x4, 6 for 6x6, or Enter for 4x4. To change
board size, press M and select 2048 again.

| Key | 2048 action |
| --- | --- |
| Arrow keys | Slide and merge |
| R | Restart the same size and seed |
| C | Continue after the win message |
| Q | Quit from the menu or game |

Letter keys accept either case. The window must be at least 42x22 for 4x4
or 51x28 for 6x6 (the menu requires 42x9). Smaller windows show a message,
accept M (main menu) or Q (quit), and otherwise leave the game unchanged. Recovery discards the first
input; press a key again after the board returns. Tiles that exceed seven
decimal digits use exact power notation, such as 2^63, to keep cells aligned.
Colors fall back to plain numeric tiles on monochrome terminals.

The 2048 frontend tests require Python 3 and Linux pseudo-terminals, just
like the existing Tetris terminal tests. They cover both sizes, maximum
values, statuses, input mappings, resize recovery and normal/signal cleanup.
Run both modes in PuTTY to verify actual appearance and interactive play.

The isolated modules live in include/game2048 and src/game2048:
board handles fixed-size storage; game owns score, seed and status;
input/display implement the terminal interface; main owns lifecycle and selection.
tests/test_2048_game.c checks both sizes, spawning, score, no-op preservation,
win/loss, continuation, restart and deterministic replay. tests/test_2048_board.c covers slides,
merges, bounds, score gains and move availability. Engine tests do not link
ncurses or Tetris.

make run opens the main menu; make run-tetris launches Tetris directly.
Existing debug, test, test-terminal and package targets retain their Tetris entry points. Use make package-2048 for the separate 2048 archive.

Phase 5 adds input/engine/rendering integration checks and a separate binary package.

2048 Phase 1 was verified by the user on Hyrule-2 on 2026-10-19:

- make clean && make: both executables built with no warnings or errors
  in the supplied output.
- make test: all three Tetris engine test suites passed.
- make test-2048: structure tests passed for 4x4/6x6 boards, seeds and validation.
- make run-2048: the expected skeleton notice appeared and the program
  returned to the shell without a reported error.

These Phase 1 results superseded the authoring-host tooling limitation
for that phase.

Phase 2 board API:

- game2048_board_move(board, direction) slides and merges without spawning.
  It returns changed and score_gained; invalid inputs return false/zero
  without altering the board.
- game2048_board_has_moves(board) checks whether any direction would change
  cells. It is read-only; empty and invalid boards return false. This is a
  board query, not a game-over transition.

Tiles store exponents (1 means 2, 2 means 4). Equal tiles merge once per move:
2 2 2 2 becomes 4 4; 2 2 4 becomes 4 4; 2 0 2 2 becomes 4 2 when moved left.
The score gain is the sum of merged values, saturated at UINT64_MAX.
Exponent-63 tiles may slide but cannot merge further. Only active cells are
validated or modified; unused storage is preserved.

2048 Phase 2 was verified by the user on Hyrule-2 on 2026-10-19:

- make clean && make: both executables built with no warnings or errors
  in the supplied output.
- make test: all three Tetris engine test suites passed.
- make test-2048: initialization and board tests passed, covering both sizes,
  directions, merges, score gains, limits and move availability.
- make run-2048: the expected Phase 2 notice appeared and the program
  returned to the shell without a reported error.

These results supersede the authoring-host verification limitation.
Phase 3 engine behavior:

- Initialization places two tiles in distinct empty cells. Spawns choose an
  empty position uniformly and a value of 2 (90%) or 4 (10%).
- Each changed move adds its merge points, saturating total score, and spawns
  exactly one tile. Ineffective moves consume no randomness and change no state.
- Reaching 2048 or higher pauses at WON. Continue acknowledges the win once;
  play resumes if moves remain, otherwise the status becomes OVER. Win takes
  priority when the same move also leaves no legal moves.
- Restart from any status resets score, board and win acknowledgment and
  replays the original seed at the same board size.
- The game engine reads no clocks or keyboard input and has no ncurses dependency.

2048 Phase 3 was verified by the user on Hyrule-2 on 2026-10-19:

- make clean && make: both executables built with no warnings or errors
  in the supplied output.
- make test: all three Tetris engine test suites passed.
- make test-2048: game tests passed for spawning, score, win/loss, continue,
  restart and replay; board tests passed for both sizes and all directions.
- make run-2048: the Phase 3 notice reported a 4x4 board initialized with two
  tiles, and the executable returned to the shell without a reported error.

These results supersede the authoring-host verification limitation.
Phase 4 was verified by the user's Hyrule-2 output: both binaries built
without warnings, both engine suites and both terminal suites passed,
and make run-2048 returned without a reported error.
Phase 5 adds checks for input-to-screen win/continue/loss/restart transitions,
SIGHUP cleanup and standalone package execution. Phase 5 Linux execution
is pending because Make/GCC are unavailable on the Windows authoring host.

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
Smaller terminals freeze gameplay and accept M (main menu) or Q (quit). Enlarging the window
restores the previous running/paused/game-over state without catching up.
The first recovery input is discarded; press a key again after the display
returns. Input polling waits up to 50 ms for an initial key; ncurses also
waits for the remainder of terminal escape sequences such as arrow keys.

## Run without Make or create a binary package

After building once, launch directly:

    ./build/terminal-tetris

To generate an archive containing the executable and run instructions:

    make package

The output is build/terminal-tetris-linux.tar.gz. Copy it to a compatible
Linux machine, then extract and run:

    tar -xzf terminal-tetris-linux.tar.gz
    ./terminal-tetris/terminal-tetris

No build tools are needed to play. The destination needs the same CPU
architecture and compatible libc, ncurses and terminfo runtime libraries.
This is a Linux binary archive, not a Windows executable or a static binary.
See packaging/README.txt for optional installation in ~/.local/bin.

make clean removes the archive along with the other build artifacts.

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

- make / make all: build the launcher and both direct entry points with GCC, debug symbols and strict warnings.
- make run-2048: build and launch the playable 2048 size selector.
- make test-2048-terminal: run 2048 screen/input and PTY checks (Python 3).
- make test-2048: run isolated 2048 initialization and board tests without ncurses.
- make run: open the shared main menu.
- make run-tetris: launch Tetris directly.
- make test-menu: test switching, resizing and menu cleanup under PTYs.
- make package-games: package the shared launcher with both games.
- make package: build a Linux binary archive with run instructions.
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

## 2048 binary package and final verification

Build and run all engine, terminal and package checks on Ubuntu:

    make clean && make
    make check

make check includes test, test-2048, test-terminal, test-2048-terminal and
test-2048-package. Python 3 and Linux PTYs are needed for terminal/package
tests only; engine tests remain independent of ncurses and Python.
The package test checks exact archive contents and executable permissions,
then extracts to a temporary directory and launches both sizes from outside
the source tree. Terminal tests connect keyboard mappings to engine state
and rendered win, continue, loss and restart screens under xterm and vt100.

To create just the 2048 archive:

    make package-2048

Output: build/terminal-2048-linux.tar.gz. On a compatible Linux machine:

    tar -xzf terminal-2048-linux.tar.gz
    ./terminal-2048/terminal-2048

No Make or source tree is needed to play. Use the same CPU architecture
and compatible libc, ncurses and terminfo runtime libraries. This is a
dynamically linked Linux archive, not a Windows executable or static binary.
See packaging/README-2048.txt for optional installation in ~/.local/bin.
make clean removes both games' build artifacts and archives.

Before distribution, play both sizes in PuTTY: select a board, move in all
four directions, confirm merges and score changes, restart, resize below
minimum and recover, then quit. Check color and monochrome readability.
Automated transition fixtures cover win/continue/loss; PTY checks do not
replace a full manual playthrough or verify PuTTY's visual appearance.
No undo, persistent high scores or saved games are implemented.

## Main menu and returning from games

    make run
    # Or launch directly after building:
    ./build/terminal-games

Choose 1 for Tetris or 2 for 2048. The 2048 board selector then offers
4x4 or 6x6. Q quits the entire application from any screen. Press M in
either game (including paused/game-over states) or in the 2048 board
selector to return to the main menu. M discards the current session;
selecting again creates a fresh game. Letters accept either case.
M/Q work even when undersized. The main menu requires 32x9.

Direct entry points remain ./build/terminal-tetris and ./build/terminal-2048.
Each executable includes both games so returning to the menu also works
in an extracted package without sibling files. The executable's basename
selects its initial screen; renamed copies open the main menu.

src/launcher.c owns process signals and top-level navigation. The existing
main.c files now provide bounded game sessions declared in include/session.h.
Each session restores ncurses before returning to the launcher. Navigation
stays in host/input code; neither engine knows about the menu. Tetris's
clock baseline is initialized afresh on every session.

Create the shared package with make package-games. Extract and launch:

    tar -xzf build/terminal-games-linux.tar.gz
    ./terminal-games/terminal-games

Verification for this change: make clean && make, then make check.
The menu PTY suite covers repeated switching under xterm/vt100, both 2048
sizes, returning from paused Tetris and the board selector, direct entry
points, undersized navigation, Q/q and SIGINT/SIGTERM/SIGHUP restoration.
The extracted 2048 package test also switches into Tetris without siblings.
Linux execution of these changes remains pending on the authoring host,
which lacks Make/GCC. Check appearance and switching in PuTTY as well.

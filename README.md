# Terminal Tetris

A native Ubuntu Linux terminal Tetris project, written in ISO C11 with
ncurses. **Milestone 1 only:** terminal initialization and an empty 10 x 20
playing field. Movement, pieces, gravity, scoring, clearing, previews,
pause and restart are not implemented yet.

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
fallback. Keyboard polling waits up to 50 ms to avoid a busy loop.

## Make targets

- make / make all: build with GCC, debug symbols and strict warnings.
- make run: build and launch the program.
- make debug: build and launch the program under GDB.
- make test: compile and run engine structure tests without ncurses.
- make clean: remove generated files from build/.

Compiler flags: -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g.
ncurses flags are resolved through pkg-config. Header dependencies are
generated automatically for application objects.

## Layout and API

- include/: module interfaces and fixed-size data structures.
- src/board.c: empty board initialization.
- src/tetromino.c: piece-count invariant; future shape tables.
- src/game.c: deterministic initialization and inert action/time API.
- src/input.c: terminal key translation.
- src/display.c: ncurses initialization, rendering and cleanup.
- src/main.c: lifecycle and shutdown signals.
- tests/test_game.c: dimensions, empty board and initial state checks.
- AGENTS.md: contributor architecture, conventions and verification rules.

The engine accepts game_process_action(game, action) separately from
game_update(game, elapsed_ms). Neither function implements gameplay yet.
Initialization accepts a seed and reserves per-game bag/random state.
No engine module depends on ncurses, OS clocks or keyboard APIs.

Future timing must cap elapsed updates at 100 ms, discard excessive time,
ignore paused time, and reset the host clock baseline after resume or
terminal recovery. Milestone 1 has no clock or gravity to catch up.

## Next milestones

1. Board placement, shape tables and collision tests.
2. Deterministic movement, spawning and game-state transitions.
3. Gravity, line clearing, scoring, levels, pause and restart.
4. Full terminal gameplay, preview and integration checks.

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
were not individually reported. No Milestone 2 gameplay is implemented.

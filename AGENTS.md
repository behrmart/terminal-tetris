# Terminal Tetris contributor instructions

## Scope and architecture
This is a native Ubuntu Linux terminal game using C11, GCC, GNU Make and
ncurses. Milestone 5 connects the engine to keyboard input, monotonic timing,
colored rendering, preview and status display. Preserve the engine/terminal
boundary and the portable ASCII borders used for PuTTY compatibility.

- board: locked 10-column by 20-row storage, placement checks, atomic locking and row compaction.
- tetromino: seven piece types and four read-only clockwise rotation states.
- game: deterministic state, actions and elapsed-time updates.
- input: translate ncurses input into actions.
- display: terminal lifecycle and read-only rendering.
- main: lifecycle, input dispatch, monotonic time and resize freezing.

The engine (board, tetromino, game) must never include ncurses, keyboard,
clock or other OS APIs. Keep terminal types out of public headers.
Use fixed-size arrays and structs; avoid heap allocation and unnecessary
abstractions. The active piece remains separate from locked board cells.

## API and timing
Keep game_process_action and game_update separate. Initialization accepts
an explicit seed. Equivalent seeds, actions and elapsed updates must yield
equivalent game state. Do not use global random state.
Actions implement movement, drops, pause and restart; elapsed updates apply
gravity. Pause toggles running/paused and clears the accumulator. Restart
works from every status and reinitializes with the original seed. Other
actions are ignored unless running; none/quit/unknown actions are no-ops.
Clear lines after locking and before spawning. Award 100/300/500/800 times
the pre-clear level for 1/2/3/4 lines. No drop points. Start at level 1 and
increase every 10 total lines. Saturate score and line counters on overflow.
Gravity is 1000 ms at level 1, minus 100 ms per level, with a 100 ms floor.
Use an explicit fixed-width per-game PRNG and shuffled seven-piece bags.
Every seed, including zero, must work. Spawn at rotation 0, origin (3, 0).
Soft drop locks on a blocked downward attempt; hard drop locks exactly one
piece. Advance the preview on spawn. Blocked spawn means game over with no
active piece. Keep actions bounded and commit only valid candidate moves.
Shape queries reject invalid types and rotation indices (valid: 0..3).
Only occupied cells constrain placement; empty local margins may lie outside
the board. Occupied cells above the visible board are rejected. Failed locks
must leave the board unchanged. Rotation tables do not perform wall kicks.

Cap elapsed time to GAME_MAX_ELAPSED_MS (100 ms) per update and discard excess, without accumulating a backlog.
Ignore elapsed time while paused and clear the gravity accumulator on
pause/resume. The host must reset its clock baseline on resume, restart
and recovery from an undersized terminal. While undersized, accept only
quit, clear fractional gravity time and preserve the current game status.
Each update performs at most one downward attempt; spawning discards any
remaining gravity time. Engine updates must never read clocks.
Keep quit and terminal resizing in the host.

## Coding conventions
Use ISO C11, four-space indentation, braces, snake_case functions and
variables, PascalCase typedefs, uppercase constants, and include guards.
Include each module's own header. Use const for read-only parameters.
Validate occupied coordinates before accessing arrays. Avoid mutable
global state in the engine. Signal handlers may only set sig_atomic_t
flags; restore the terminal in normal control flow.
Use explicit casts only when a conversion is known to be safe.

## Build requirements
Ubuntu packages: build-essential, pkg-config, libncurses-dev, gdb, git.
Compile with GCC and:
-std=c11 -Wall -Wextra -Wpedantic -Wconversion -g
Discover ncurses compiler/linker flags through pkg-config.
Required targets: all, run, debug, test, clean. The additional test-terminal
target uses Python 3 (standard library only) and Linux pseudo-terminals.
The game engine must compile and test without linking ncurses.

## Verification
Run make clean && make and fix all warnings, then run make test.
For terminal changes also run make test-terminal. It checks screen contents,
input mapping, resize recovery and terminal restoration under PTYs.
Keep make test independent of ncurses and Python.
For terminal changes, verify Q/q exit, interruption cleanup, normal and
undersized terminals, and resize recovery in a real terminal or PTY.
Report unavailable tooling and unexecuted checks honestly.
Gameplay tests must cover collisions, rotations, line compaction,
deterministic generation, pause behavior and bounded elapsed-time handling.
Do not compare whole structs with memcmp because of padding.

## Terminal behavior
Minimum size: 54 columns by 26 rows. Render two columns per cell, using
colored backgrounds where supported and [] in monochrome. Display is
read-only. Use arrows for movement (Up rotates clockwise), X/Z for rotations,
Space for hard drop, P for pause, R for restart and Q for quit (both cases).
Use POSIX CLOCK_MONOTONIC only in main; timestamp differences are capped
before converting to engine milliseconds. Advance gravity before ordinary
actions; pause/restart discard that loop's elapsed time. New launches seed
from monotonic milliseconds; restart reuses the launch seed.
SIGINT/SIGTERM/SIGHUP handlers only set a flag. Restore terminal settings
before printing clock errors or returning from the application.

## 2048 Phase 1 scope
The repository also contains a separate Terminal 2048 skeleton under
include/game2048 and src/game2048. The rules above about Tetris gravity,
tetrominoes, scoring and terminal layout apply only to Tetris.
Shared C11 conventions and the engine/terminal dependency boundary apply
to both games. Do not refactor working Tetris modules for 2048.

2048 board/game modules use only ISO C11 and expose game2048-prefixed APIs.
Use Game2048-prefixed types, GAME2048 constants and distinct include guards.
Keep a fixed 6x6 tile array with an active size of 4 or 6. Tiles are
exponents: 0 is empty, n represents 2^n, with a maximum exponent of 63.
Invalid initialization must leave state unchanged. Store the explicit
initial seed and per-game random state. There is no elapsed-time API.

Phase 1 initializes empty boards and provides inert action/input stubs.
The executable prints a noninteractive notice and exits normally. Do not
add merging, movement, random tiles or an ncurses interface until their
phases are requested. Win acknowledgment, overflow behavior and spawning
are design contracts for subsequent phases, not implemented gameplay.

make all builds both executables. Existing run/debug/test/test-terminal/
package targets retain their Tetris behavior. Use run-2048 and test-2048
for the new game. 2048 engine tests must not compile/link Tetris, ncurses
or OS-specific code. Object files live under build/game2048 to avoid clashes.
For 2048 changes run make clean && make, make test and make test-2048.
Run make run-2048 to check skeleton startup and normal exit.

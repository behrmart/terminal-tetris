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
M (main menu) or quit, clear fractional gravity time and preserve the current game status.
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

## 2048 Phase 5 scope
The repository also contains a separate playable Terminal 2048 game under
include/game2048 and src/game2048. The rules above about Tetris gravity,
tetrominoes, scoring and terminal layout apply only to Tetris.
Shared C11 conventions and the engine/terminal dependency boundary apply
to both games. Preserve Tetris engine behavior when sharing host lifecycle code.

2048 board/game modules use only ISO C11 and expose game2048-prefixed APIs.
Use Game2048-prefixed types, GAME2048 constants and distinct include guards.
Keep a fixed 6x6 tile array with an active size of 4 or 6. Tiles are
exponents: 0 is empty, n represents 2^n, with a maximum exponent of 63.
Invalid initialization must leave state unchanged. Store the explicit
initial seed and per-game random state. There is no elapsed-time API.

Phase 2 implements board slides/merges and read-only move availability.
Map directions to a single destination-first line compaction/merge routine.
Merge each input tile at most once per move. Return changed and score gained;
saturate the score gain at UINT64_MAX. Exponent-63 tiles slide but never merge.
Reject invalid sizes, directions and active tile exponents before mutation.
Preserve storage outside the active board. Empty boards have no changing move.

Phase 3 initializes two distinct random tiles and connects game actions to
board moves. Use a fixed-width per-game PRNG; all seeds including zero work.
Select empty positions uniformly; spawn 2 with 90% probability and 4 with
10%. Use rejection sampling to avoid modulo bias. A changed move adds merge
points with saturation and spawns exactly one tile. Ineffective moves leave
the entire state unchanged, including PRNG state.

Check for an unacknowledged tile >=2048 before loss detection. WON freezes
movement until CONTINUE, which acknowledges once and either resumes or
sets OVER if no moves remain. Restart works from any status, retaining
size and original seed. None/quit/unknown actions are inert; quit is host-owned.

Phase 4 provides a startup selector (4/6; Enter defaults to 4), arrows for
movement, R for restart, C for continue after winning and Q for quit.
Numeric selections are ignored after starting a game. Letter controls accept
both cases. The display reads state only and uses ASCII borders for PuTTY.
Use seven-column cells; values longer than seven digits use exact 2^n
notation rather than truncation. Minimum sizes are 42x22 (4x4), 51x28 (6x6)
and 42x9 (menu). Keep game state unchanged while undersized; accept only M (main menu) or quit,
and discard the first recovery input. There is no gameplay timer.
Read the monotonic clock in main only to seed a newly selected game.
Signal handlers only set a flag; normal control flow restores the terminal.
Phase 5 provides package-2048 and test-2048-package; keep Tetris packaging unchanged.
Tests must cover both sizes, seeds, one-tile spawning, no-op preservation,
score saturation, win/loss priority, continue, restart and replay.

make all builds terminal-games and both direct entry points. make run opens
the main menu; run-tetris launches Tetris. Existing debug/test/test-terminal/
package targets retain their Tetris entry points. Use run-2048 and test-2048
for the new game. 2048 engine tests must not compile/link Tetris, ncurses
or OS-specific code. Object files live under build/game2048 to avoid clashes.
For 2048 changes run make clean && make, make test and make test-2048.
Run make test-2048-terminal for frontend screen/input and PTY cleanup checks
(Python 3 standard library). Run make run-2048 for manual gameplay in both sizes.
Preserve existing Tetris test targets and keep make test-2048 free of ncurses.

Phase 5 verification: make check runs both engine suites, both terminal
suites and the extracted 2048 package test. Package only the executable and
packaging/README-2048.txt, preserving executable permissions. The archive
requires a compatible Linux architecture and runtime libraries; do not claim
a static or universal executable. Package tests must launch from outside the
source tree and check terminal restoration for both sizes.
Keep test-2048 independent of ncurses/Python. Report unexecuted Linux checks.

## Shared main menu
src/launcher.c owns signals and calls tetris_run/game2048_run via session.h.
The three executable names share one linked program: terminal-games starts
at the menu, terminal-tetris/terminal-2048 start at the named game. Do not
spawn subprocesses or depend on sibling binaries for navigation.
M/m returns SESSION_MENU after normal terminal cleanup, discarding the game.
Q/q exits the process. Navigation is host-only, including while undersized;
engine action enums and state remain independent of menu handling.
Selecting a game starts fresh with new clock baseline and seed.
Run make test-menu (included in make check) for navigation changes.
package-games bundles the shared menu executable and its README.

## Persistent player scores
The launcher prompts for a case-sensitive 1-16 character ASCII username
(letters, digits, underscore, hyphen). Esc cancels login; Q is a valid letter
there and retains its quit behavior after login.
src/scores.c is host-only persistence, never an engine dependency. Store
separate personal bests for Tetris, 2048 4x4 and 2048 6x6 under XDG_STATE_HOME
or HOME/.local/state, outside source/build/package folders.
Save improvements immediately, retain in-memory maxima on failure and report
errors. Merge concurrent writers under a lock and use atomic replacement.
Reject malformed files without overwriting them. Never lower a best score.
Use temporary directories for tests; never touch real player data.
Run make test-scores and make test-login (both included in make check).
Keep make test engine-only and independent of ncurses/Python.

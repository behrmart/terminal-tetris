# Terminal Tetris contributor instructions

## Scope and architecture
This is a native Ubuntu Linux terminal game using C11, GCC, GNU Make and
ncurses. Milestone 4 includes deterministic movement, gravity, line clearing,
scoring, levels, pause and restart in the engine. The terminal still shows
an empty field. Do not add terminal gameplay integration until requested.

- board: locked 10-column by 20-row storage, placement checks, atomic locking and row compaction.
- tetromino: seven piece types and four read-only clockwise rotation states.
- game: deterministic state, actions and elapsed-time updates.
- input: translate ncurses input into actions.
- display: terminal lifecycle and read-only rendering.
- main: application lifecycle, input dispatch and future OS time measurement.

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
and recovery from an undersized terminal when integrated in Milestone 5.
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
Required targets: all, run, debug, test, clean.
The game engine must compile and test without linking ncurses.

## Verification
Run make clean && make and fix all warnings, then run make test.
For terminal changes, verify Q/q exit, interruption cleanup, normal and
undersized terminals, and resize recovery in a real terminal or PTY.
Report unavailable tooling and unexecuted checks honestly.
Gameplay tests must cover collisions, rotations, line compaction,
deterministic generation, pause behavior and bounded elapsed-time handling.
Do not compare whole structs with memcmp because of padding.

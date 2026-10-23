Terminal Tetris - Linux binary package

Run from the extracted terminal-tetris directory:

    ./terminal-tetris

No compiler, Make, Python or source tree is needed to play. This dynamically
linked executable needs compatible Linux libc, ncurses and terminfo runtime
libraries on the destination machine. Use the same CPU architecture and a
compatible Ubuntu release as the build machine; this is not a Windows EXE
or a universal static binary.

If a library is missing, inspect the dependencies on your trusted copy:

    ldd ./terminal-tetris

Use a terminal of at least 54 columns by 26 rows (PuTTY over SSH works).
Controls:
    Left/Right: move     Down: soft drop     Up/X: rotate clockwise
    Z: rotate counterclockwise              Space: hard drop
    P: pause/resume      R: restart          Q: quit

Restart replays this launch's piece sequence.

Optional installation for your user (no sudo):

    mkdir -p "$HOME/.local/bin"
    install -m 755 ./terminal-tetris "$HOME/.local/bin/terminal-tetris"
    "$HOME/.local/bin/terminal-tetris"

If ~/.local/bin is on your PATH, just type terminal-tetris.

M: return to the main menu (both cases), discarding the current game.
The main menu offers 1: Tetris, 2: 2048, Q: quit. Selecting starts fresh.
All three games are included in this executable; no sibling executable is needed.
M is also available when undersized and at the 2048 board selector.
Q continues to exit the whole application.

At startup choose a username: 1-16 ASCII letters/digits, underscore or hyphen.
Names are case-sensitive. Enter confirms; Backspace edits; Esc cancels.
The main menu shows personal bests for Tetris and each 2048 size.
New bests save automatically to:
    $XDG_STATE_HOME/terminal-games/scores/USERNAME.txt
or, when XDG_STATE_HOME is unset/relative:
    $HOME/.local/state/terminal-games/scores/USERNAME.txt
Restart and relaunch preserve bests. Quit and relaunch to change player.
This is a local player label, not a password-protected account.
The state directory must be writable. Invalid score files cause an explicit
error rather than being reset. Scores are not included in this package.

Drug Wars: choose 3 from the main menu (72x25 terminal).
Select goods with Up/Down or 1-6. B/S buy/sell, T then 1-6 travels,
P repays debt, D/W deposits/withdraws. Enter confirms amounts, A selects
the maximum, Esc cancels. U buys capacity, R restarts, M returns, Q quits.
Travel advances a day and adds 10% debt interest. On day 30 press E to settle.
Only completed runs save a best score: assets minus debt, floored at zero.
A health-depleted run scores zero. Prices/events are fictional mechanics.
Score files upgrade automatically from version 1 to version 2 with the
Drug Wars best appended. Existing Tetris/2048 bests are preserved.
Older executables cannot read upgraded profiles; use the rebuilt binaries.

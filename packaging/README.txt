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
Both games are included in this executable; no sibling executable is needed.
M is also available when undersized and at the 2048 board selector.
Q continues to exit the whole application.

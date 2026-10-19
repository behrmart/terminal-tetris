Terminal 2048 - Linux binary package

Run: ./terminal-2048
Select 4 or 6 for board size; Enter defaults to 4.
Arrows: move. R: restart. C: continue after winning. Q: quit.
Letter controls accept both cases. Restart reuses the launch seed.
Quit and relaunch to change size or seed.
Minimum terminal: menu 42x9, 4x4 42x22, 6x6 51x28.
Undersized terminals accept only Q. First recovery input is discarded.
ASCII borders support PuTTY; monochrome terminals show plain numbers.
Large tiles use exact 2^n notation. No undo or persistent saved games.

No compiler, Make, Python or source tree is needed to play.
Requires the same CPU architecture and compatible Linux libc, ncurses
and terminfo runtime libraries as the build machine. Use a valid TERM.
This archive is dynamically linked, not a Windows EXE or static binary.
Inspect dependencies on your trusted copy: ldd ./terminal-2048

Optional installation (no sudo):
    mkdir -p "$HOME/.local/bin"
    install -m 755 ./terminal-2048 "$HOME/.local/bin/terminal-2048"
    "$HOME/.local/bin/terminal-2048"

If ~/.local/bin is on PATH, type terminal-2048.
M: return to the main menu (both cases), discarding the current game.
The main menu offers 1: Tetris, 2: 2048, Q: quit. Selecting starts fresh.
Both games are included in this executable; no sibling executable is needed.
M is also available when undersized and at the 2048 board selector.
Q continues to exit the whole application.

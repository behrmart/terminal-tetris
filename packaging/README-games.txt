Terminal Games - Linux binary package

Run ./terminal-games to open the main menu.
1: Tetris    2: 2048    Q: quit
2048 then offers 4x4 or 6x6 (press 4 or 6; Enter defaults to 4).

In either game, M returns to the main menu and discards the current game.
Selecting again starts fresh. M also works at the 2048 size selector and
when the terminal is too small. Q exits the application everywhere.
Controls accept uppercase or lowercase letters.

Tetris: arrows move (Up rotates), X/Z rotate, Space drops, P pauses, R restarts.
2048: arrows slide, R restarts, C continues after reaching 2048.
Tetris minimum: 54x26. 2048: 42x22 for 4x4, 51x28 for 6x6.
Main menu: 32x9. Board selector: 42x9. ASCII borders support PuTTY.

No compiler, Make, Python or sibling executable is needed to play.
Requires the same CPU architecture and compatible Linux libc, ncurses
and terminfo runtime libraries as the build machine. Use a valid TERM.
Not a Windows EXE or universal static binary.
To inspect dependencies on your trusted copy: ldd ./terminal-games

Optional user installation:
    mkdir -p "$HOME/.local/bin"
    install -m 755 ./terminal-games "$HOME/.local/bin/terminal-games"

If ~/.local/bin is on PATH, type terminal-games.
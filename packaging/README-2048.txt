Terminal 2048 - Linux binary package

Run: ./terminal-2048
Select 4 or 6 for board size; Enter defaults to 4.
Arrows: move. R: restart. C: continue after winning. Q: quit.
Letter controls accept both cases. Restart reuses the launch seed.
Quit and relaunch to change size or seed.
Minimum terminal: menu 42x9, 4x4 42x22, 6x6 51x28.
Undersized terminals accept only Q. First recovery input is discarded.
ASCII borders support PuTTY; monochrome terminals show plain numbers.
Large tiles use exact 2^n notation. No undo or saved board state.

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

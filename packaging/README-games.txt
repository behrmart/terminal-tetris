Terminal Games - Linux binary package

Run ./terminal-games to open the main menu.
1: Tetris    2: 2048    3: Drug Wars    Q: quit
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

#!/usr/bin/env python3
"""Third-game lifecycle and navigation through Linux pseudo terminals."""
import signal
from test_terminal import Terminal


def run_checks():
    for term in ("xterm", "vt100"):
        terminal = Terminal("test-drugwars-terminal", rows=25, columns=72, term=term)
        try:
            terminal.expect(b"Drug Wars screen/input checks passed.")
            terminal.finish()
        finally:
            terminal.close()
        print(f"Drug Wars screen/input passed ({term}).")
    for action in (b"q", b"Q", signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
        terminal = Terminal("terminal-drugwars", rows=25, columns=72)
        try:
            terminal.expect(b"TERMINAL DRUG WARS")
            terminal.send(b"6ba\nsa\nt2")
            terminal.send(b"r")
            terminal.resize(2, 40)
            terminal.expect(b"Need 72x25")
            terminal.send(b"t2b99\nr")
            terminal.output.clear()
            terminal.resize(25, 72)
            terminal.expect(b"TERMINAL DRUG WARS")
            if isinstance(action, bytes):
                terminal.send(action)
            else:
                terminal.process.send_signal(action)
            terminal.finish()
        finally:
            terminal.close()
    terminal = Terminal("terminal-games", rows=25, columns=72)
    try:
        terminal.expect(b"3:DrugWars")
        terminal.send(b"3")
        terminal.expect(b"TERMINAL DRUG WARS")
        terminal.output.clear()
        terminal.send(b"m")
        terminal.expect(b"TERMINAL GAMES")
        terminal.send(b"3")
        terminal.output.clear()
        # 29 moves reach day 30; then E settles the completed run.
        for day in range(29):
            terminal.send(b"t2" if day % 2 == 0 else b"t1")
        terminal.send(b"e")
        terminal.read()
        assert b"RUN COMPLETE" in terminal.output or b"GAME OVER" in terminal.output
        terminal.send(b"r")
        terminal.send(b"m")
        terminal.send(b"q")
        terminal.finish()
    finally:
        terminal.close()
    print("Drug Wars menu, 30-day run, restart, resize and cleanup passed.")


if __name__ == "__main__":
    run_checks()

#!/usr/bin/env python3
"""Linux PTY coverage of game selection and repeated session transitions."""
import signal
from test_terminal import Terminal


def send_expect(terminal, keys, text):
    terminal.output.clear()
    terminal.send(keys)
    terminal.expect(text)


def run_checks():
    for term in ("xterm", "vt100"):
        terminal = Terminal("terminal-games", rows=28, columns=54, term=term)
        try:
            terminal.expect(b"TERMINAL GAMES")
            for size in (b"4", b"6"):
                send_expect(terminal, b"1", b"TERMINAL TETRIS")
                terminal.send(b"p")
                send_expect(terminal, b"M", b"TERMINAL GAMES")
                send_expect(terminal, b"2", b"Choose your board:")
                send_expect(terminal, size, b"RUNNING")
                send_expect(terminal, b"m", b"TERMINAL GAMES")
            send_expect(terminal, b"2", b"Choose your board:")
            send_expect(terminal, b"M", b"TERMINAL GAMES")
            terminal.send(b"q")
            terminal.finish()
        finally:
            terminal.close()
        print(f"Main menu and repeated game switching passed ({term}).")

    for executable in ("terminal-tetris", "terminal-2048"):
        terminal = Terminal(executable, rows=28, columns=54)
        try:
            terminal.expect(b"TERMINAL")
            send_expect(terminal, b"m", b"TERMINAL GAMES")
            terminal.send(b"Q")
            terminal.finish()
        finally:
            terminal.close()

    terminal = Terminal("terminal-games", rows=2, columns=40)
    try:
        terminal.expect(b"Need 32x9")
        terminal.send(b"12")
        terminal.output.clear()
        terminal.resize(28, 54)
        terminal.expect(b"TERMINAL GAMES")
        send_expect(terminal, b"1", b"TERMINAL TETRIS")
        terminal.resize(2, 40)
        terminal.read()
        send_expect(terminal, b"m", b"Need 32x9")
        terminal.send(b"q")
        terminal.finish()
    finally:
        terminal.close()

    for exit_action in (b"q", b"Q", signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
        terminal = Terminal("terminal-games", rows=28, columns=54)
        try:
            terminal.expect(b"TERMINAL GAMES")
            if isinstance(exit_action, bytes):
                terminal.send(exit_action)
            else:
                terminal.process.send_signal(exit_action)
            terminal.finish()
        finally:
            terminal.close()
    print("Direct launch, undersized navigation and menu cleanup passed.")


if __name__ == "__main__":
    run_checks()
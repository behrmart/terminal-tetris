#!/usr/bin/env python3
"""Frontend and integration checks; reuses the existing Linux PTY test helper."""
import signal

from test_terminal import Terminal


def run_checks():
    for term in ("xterm", "vt100"):
        terminal = Terminal("test-2048-terminal", rows=28, columns=54, term=term)
        try:
            terminal.expect(b"2048 terminal rendering and input tests passed.")
            terminal.finish()
        finally:
            terminal.close()
        print(f"2048 screen/input checks passed ({term}).")

    for size in (b"4", b"6"):
        for exit_action in (b"q", b"Q", signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
            terminal = Terminal("terminal-2048", rows=28, columns=54)
            try:
                terminal.expect(b"Choose your board:")
                terminal.send(size)
                terminal.expect(b"Score:")
                terminal.expect(b"RUNNING")
                terminal.send(b"rc")
                terminal.resize(1, 1)
                terminal.read()
                assert terminal.process.poll() is None
                terminal.send(b"rc")
                terminal.resize(28, 54)
                terminal.read()
                if isinstance(exit_action, bytes):
                    terminal.send(exit_action)
                else:
                    terminal.process.send_signal(exit_action)
                terminal.finish()
            finally:
                terminal.close()
            print(f"2048 {size.decode()}x{size.decode()} resize/cleanup passed ({exit_action!r}).")

    terminal = Terminal("terminal-2048", rows=2, columns=40)
    try:
        terminal.expect(b"Need 42x9")
        terminal.send(b"6")
        terminal.resize(28, 54)
        terminal.expect(b"Choose your board:")
        terminal.send(b"q")
        terminal.finish()
    finally:
        terminal.close()
    print("2048 undersized menu recovery passed.")


if __name__ == "__main__":
    run_checks()

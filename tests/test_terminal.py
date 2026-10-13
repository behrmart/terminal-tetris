#!/usr/bin/env python3
"""Linux PTY checks. Run after building via make test-terminal."""
import errno
import fcntl
import os
from pathlib import Path
import pty
import select
import signal
import struct
import subprocess
import termios
import time

ROOT = Path(__file__).resolve().parents[1]


class Terminal:
    def __init__(self, executable, rows=26, columns=54, term="xterm"):
        self.master, self.slave = pty.openpty()
        self.output = bytearray()
        self.original = termios.tcgetattr(self.slave)
        self.resize(rows, columns, notify=False)
        env = dict(os.environ, TERM=term)
        env.pop("LINES", None)
        env.pop("COLUMNS", None)
        self.process = subprocess.Popen(
            [str(ROOT / "build" / executable)],
            stdin=self.slave, stdout=self.slave, stderr=self.slave,
            env=env, start_new_session=True,
        )

    def resize(self, rows, columns, notify=True):
        fcntl.ioctl(self.slave, termios.TIOCSWINSZ,
                    struct.pack("HHHH", rows, columns, 0, 0))
        if notify:
            self.process.send_signal(signal.SIGWINCH)

    def read(self, duration=0.15):
        end = time.monotonic() + duration
        while time.monotonic() < end:
            readable, _, _ = select.select([self.master], [], [],
                                          max(0, end - time.monotonic()))
            if readable:
                try:
                    data = os.read(self.master, 65536)
                except OSError as error:
                    if error.errno == errno.EIO:
                        break
                    raise
                if not data:
                    break
                self.output.extend(data)

    def expect(self, text):
        deadline = time.monotonic() + 5
        while text not in self.output and time.monotonic() < deadline:
            self.read()
            if self.process.poll() is not None:
                break
        assert text in self.output, (text, repr(bytes(self.output[-2000:])))

    def send(self, keys):
        os.write(self.master, keys)
        self.read()

    def finish(self):
        deadline = time.monotonic() + 5
        while self.process.poll() is None and time.monotonic() < deadline:
            self.read()
        assert self.process.poll() == 0, repr(bytes(self.output[-2000:]))
        assert termios.tcgetattr(self.slave) == self.original, "Terminal not restored"

    def close(self):
        if self.process.poll() is None:
            self.process.kill()
            self.process.wait()
        os.close(self.master)
        os.close(self.slave)


def run_checks():
    for term in ("xterm", "vt100"):
        terminal = Terminal("test-terminal", term=term)
        try:
            terminal.expect(b"Terminal rendering and input tests passed.")
            terminal.finish()
        finally:
            terminal.close()
        print(f"Rendering/input checks passed ({term}).")

    for exit_action in (b"q", b"Q", signal.SIGINT, signal.SIGTERM):
        terminal = Terminal("terminal-tetris")
        try:
            terminal.expect(b"TERMINAL TETRIS")
            terminal.expect(b"NEXT")
            terminal.send(b"pxz ")
            terminal.send(b"p")
            terminal.send(b"xz ")
            terminal.send(b"r")
            if isinstance(exit_action, bytes):
                terminal.send(exit_action)
            else:
                terminal.process.send_signal(exit_action)
            terminal.finish()
        finally:
            terminal.close()
        print(f"Startup/cleanup passed ({exit_action!r}).")

    terminal = Terminal("terminal-tetris", rows=2, columns=40)
    try:
        terminal.expect(b"Need 54x26")
        terminal.send(b" xzpr")
        assert terminal.process.poll() is None
        terminal.resize(26, 54)
        terminal.expect(b"TERMINAL TETRIS")
        terminal.resize(1, 1)
        terminal.read()
        assert terminal.process.poll() is None
        terminal.resize(30, 80)
        terminal.read()
        terminal.send(b"Q")
        terminal.finish()
    finally:
        terminal.close()
    print("Small-terminal and resize recovery checks passed.")


if __name__ == "__main__":
    run_checks()

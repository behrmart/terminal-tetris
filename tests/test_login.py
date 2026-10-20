#!/usr/bin/env python3
"""Login, on-disk persistence and cleanup; never touches real player files."""
from pathlib import Path
import signal
import tempfile
from test_terminal import Terminal


def run_checks():
    with tempfile.TemporaryDirectory(prefix="terminal-login-") as directory:
        scores = Path(directory) / "terminal-games/scores"
        terminal = Terminal("terminal-games", login=False, state_dir=directory)
        try:
            terminal.expect(b"PLAYER LOGIN")
            terminal.send(b"\n../")
            assert not scores.exists(), "Invalid login created a profile"
            terminal.send(b"Alicx\x7fe\n")
            terminal.expect(b"Player: Alice")
            terminal.send(b"q")
            terminal.finish()
        finally:
            terminal.close()
        score_file = scores / "Alice.txt"
        assert score_file.read_text() == "TERMINAL-GAMES-SCORES-1\n0\n0\n0\n"
        # A known saved profile verifies the visible reload without random play.
        score_file.write_text("TERMINAL-GAMES-SCORES-1\n123\n456\n789\n")
        terminal = Terminal("terminal-games", login=False, state_dir=directory)
        try:
            terminal.expect(b"PLAYER LOGIN")
            terminal.send(b"Alice\n")
            terminal.expect(b"1:Tetris 123")
            terminal.expect(b"4x4 best: 456")
            terminal.expect(b"6x6 best: 789")
            terminal.send(b"2")
            terminal.expect(b"Choose your board:")
            terminal.send(b"4")
            terminal.expect(b"RUNNING")
            terminal.send(b"r")
            terminal.send(b"m")
            terminal.expect(b"TERMINAL GAMES")
            terminal.send(b"q")
            terminal.finish()
        finally:
            terminal.close()
        values = score_file.read_text().splitlines()
        assert values[1] == "123" and int(values[2]) >= 456 and values[3] == "789"

    with tempfile.TemporaryDirectory(prefix="terminal-score-play-") as directory:
        terminal = Terminal("terminal-2048", state_dir=directory)
        score_file = Path(directory) / "terminal-games/scores/test_player.txt"
        try:
            terminal.expect(b"Choose your board:")
            terminal.send(b"4")
            terminal.expect(b"RUNNING")
            for _ in range(30):
                terminal.send(b"\x1bOD\x1bOB\x1bOC\x1bOA")
                if int(score_file.read_text().splitlines()[2]) > 0:
                    break
            saved = int(score_file.read_text().splitlines()[2])
            assert saved > 0, "Played merges did not reach persistent scores"
            terminal.send(b"r")
            assert int(score_file.read_text().splitlines()[2]) == saved
            terminal.send(b"q")
            terminal.finish()
        finally:
            terminal.close()
    for action in (b"\x1b", signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
        terminal = Terminal("terminal-games", rows=2, columns=40, login=False)
        try:
            terminal.expect(b"Need 32x9")
            if isinstance(action, bytes):
                terminal.send(action)
            else:
                terminal.process.send_signal(action)
            terminal.finish()
        finally:
            terminal.close()

    terminal = Terminal("terminal-games", rows=2, columns=40, login=False)
    try:
        terminal.expect(b"Need 32x9")
        terminal.resize(26, 54)
        terminal.expect(b"PLAYER LOGIN")
        terminal.send(b"resize_user\n")
        terminal.expect(b"Player: resize_user")
        terminal.send(b"q")
        terminal.finish()
    finally:
        terminal.close()
    print("Login validation, profile reload, restart preservation and cleanup passed.")


if __name__ == "__main__":
    run_checks()

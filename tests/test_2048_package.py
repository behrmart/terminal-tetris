#!/usr/bin/env python3
"""Check archive contents and run outside the source tree."""
import os
from pathlib import Path
import tarfile
import tempfile
from test_terminal import ROOT, Terminal


def run_checks():
    expected = {"terminal-2048/terminal-2048", "terminal-2048/README.txt"}
    with tempfile.TemporaryDirectory(prefix="terminal-2048-") as directory:
        destination = Path(directory)
        with tarfile.open(ROOT / "build/terminal-2048-linux.tar.gz", "r:gz") as archive:
            members = archive.getmembers()
            assert len(members) == 2
            assert {m.name for m in members} == expected
            for member in members:
                assert member.isfile(), member.name
                target = destination / member.name
                target.parent.mkdir(parents=True, exist_ok=True)
                with archive.extractfile(member) as source:
                    target.write_bytes(source.read())
                target.chmod(member.mode & 0o777)
        binary = destination / "terminal-2048/terminal-2048"
        assert binary.stat().st_mode & 0o111, "Missing executable permissions"
        assert binary.read_bytes() == (ROOT / "build/terminal-2048").read_bytes()
        assert (binary.parent / "README.txt").read_bytes() == (
            ROOT / "packaging/README-2048.txt").read_bytes()
        previous = Path.cwd()
        try:
            os.chdir(destination)
            for size in (b"4", b"6"):
                terminal = Terminal(str(binary), rows=28, columns=54)
                try:
                    terminal.expect(b"Choose your board:")
                    terminal.send(size)
                    terminal.expect(b"RUNNING")
                    terminal.output.clear()
                    terminal.send(b"m")
                    terminal.expect(b"TERMINAL GAMES")
                    terminal.output.clear()
                    terminal.send(b"1")
                    terminal.expect(b"TERMINAL TETRIS")
                    terminal.send(b"q")
                    terminal.finish()
                finally:
                    terminal.close()
        finally:
            os.chdir(previous)
    print("2048 package contents, permissions, standalone launch and cleanup passed.")


if __name__ == "__main__":
    run_checks()
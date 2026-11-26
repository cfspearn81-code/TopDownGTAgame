"""Compare an actual Outer Loop save with Base Village's native resave.

Usage: base_migration.py OLD_ELF NEW_ELF REPLAY_SO FROZEN_XDG
FROZEN_XDG is a disposable mixed-career synthesized from the previous
release, not a personal save folder. The old ELF is needed to prove this
tests the exact preceding encoding rather than a guessed save layout.
"""
from pathlib import Path
import os
import shutil
import struct
import subprocess
import sys
import tempfile


def words(blob, off, count=1):
    data = struct.unpack_from("<" + "I" * count, blob, off)
    return data[0] if count == 1 else data


def checksum(data):
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xffffffff
    return h


def validated(career):
    raw = career.read_bytes()
    assert len(raw) == 1044 and words(raw, 0, 2) == (0x44433131, 11)
    assert words(raw, 300) == checksum(raw[:300])
    assert words(raw, 1040) == checksum(raw[:1040])
    return raw


old, new, replay, fixture = (Path(x).resolve() for x in sys.argv[1:])
with tempfile.TemporaryDirectory(prefix="Dead Channel Base Village migration ") as tmp:
    xdg = Path(tmp) / "player state"
    shutil.copytree(fixture, xdg)
    folder = xdg / "DeadChannel"
    career = folder / "career.dat"
    env = dict(os.environ, XDG_DATA_HOME=str(xdg), SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
               LD_PRELOAD=str(replay), DC_LINUX_REPLAY="resave")

    def run(exe):
        subprocess.run([exe], check=True, timeout=30, env=env)

    run(old)
    before = validated(career)
    assert words(before, 968) & 0x80000000
    adjacent = {p.name: p.read_bytes() for p in folder.iterdir()
                if p.is_file() and p.name not in {"career.dat", ".afterhours.lock"}}
    assert adjacent, "fixture must include protected alternate/backup"

    run(new)
    after = validated(career)
    assert words(after, 968) & 0x80000000
    assert after[8:60] == before[8:60]
    assert after[256:296] == before[256:296]
    club, garage, phone, world = 304, 432, 688, 944
    for off, length in [(club, 8 * 4), (garage, 64 * 4),
                        (phone + 5 * 4, 4), (world + 4, 23 * 4)]:
        assert after[off:off + length] == before[off:off + length], off
    assert words(after, phone + 58 * 4, 6) == (0,) * 6, "Trap House changed without opening"
    elapsed = words(after, 252) - words(before, 252)
    assert 5 <= elapsed <= 30
    assert words(after, club + 10 * 4) - words(before, club + 10 * 4) == elapsed
    assert words(before, phone + 12 * 4) - words(after, phone + 12 * 4) == elapsed
    assert words(before, world) - words(after, world) == elapsed
    assert {p.name: p.read_bytes() for p in folder.iterdir() if p.is_file()
            and p.name not in {"career.dat", ".afterhours.lock"}} == adjacent
    print(f"Base Village native: exact prior-family Rev11 loaded; {elapsed} live frames, "
          "unchanged old ownership/upgrades, 6 clean house fields, "
          f"{len(adjacent)} adjacent records byte-identical.")

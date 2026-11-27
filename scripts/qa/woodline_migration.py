"""Re-save a disposable mixed Crossroads career through the real Woodline ELF.

Arguments: EXACT_PREVIOUS_ELF EXACT_WOODLINE_ELF REPLAY_SO DISPOSABLE_XDG.
DISPOSABLE_XDG contains a synthetic mixed fixture, never a player directory.
"""
from pathlib import Path
import os
import shutil
import struct
import subprocess
import sys
import tempfile

previous, current, replay, fixture = [Path(x).resolve() for x in sys.argv[1:]]
words = lambda b, at: struct.unpack_from("<I", b, at)[0]
f32 = lambda b, at: struct.unpack_from("<f", b, at)[0]


def fnv(raw):
    h = 2166136261
    for value in raw:
        h = ((h ^ value) * 16777619) & 0xffffffff
    return h


def read(slot):
    b = slot.read_bytes()
    assert len(b) == 1044 and words(b, 4) == 11
    assert words(b, 300) == fnv(b[:300]) and words(b, 1040) == fnv(b[:1040])
    return b


with tempfile.TemporaryDirectory(prefix="Dead Channel Woodline migration ") as tmp:
    xdg = Path(tmp) / "data"
    shutil.copytree(fixture, xdg)
    folder = xdg / "DeadChannel"
    slot = folder / "career.dat"
    env = dict(os.environ, XDG_DATA_HOME=str(xdg), SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
               LD_PRELOAD=str(replay), DC_LINUX_REPLAY="resave")

    # This file is authored by the previous binary, not copied verbatim into
    # a mock reader. It deliberately has existing business and camera state.
    subprocess.run([previous], check=True, timeout=35, env=env)
    base = read(slot)
    assert words(base, 968) & 0x80000000
    changed = bytearray(base)
    # Insert stock only for the fixture; the old real writer will reopen it.
    # Stocked compact-to-expanded migration is independently exercised by
    # crossroads_saves.py through the actual earlier market/UI flow.
    changed[267] = 2
    struct.pack_into("<I", changed, 684, 0x80000000)
    struct.pack_into("<I", changed, 300, fnv(changed[:300]))
    struct.pack_into("<I", changed, 1040, fnv(changed[:1040]))
    slot.write_bytes(changed)
    subprocess.run([previous], check=True, timeout=35, env=env)
    before = read(slot)
    extras = {p.name: p.read_bytes() for p in folder.iterdir() if p.is_file()
              and p.name not in ("career.dat", ".afterhours.lock")}

    subprocess.run([current], check=True, timeout=35, env=env)
    after = read(slot)
    assert after[8:60] == before[8:60] and after[267] == before[267] == 2
    assert after[256:296] == before[256:296]
    assert after[432:688] == before[432:688]              # no involuntary forest state
    assert after[432+49*4:432+53*4] == bytes(16)
    assert after[304:304+10*4] == before[304:304+10*4]    # old club financial state
    assert after[688+58*4:688+64*4] == before[688+58*4:688+64*4]
    elapsed = words(after, 252) - words(before, 252)
    assert 5 <= elapsed <= 35
    assert words(after, 304+10*4) - words(before, 304+10*4) == elapsed
    assert words(before, 688+12*4) - words(after, 688+12*4) == elapsed
    assert all(abs(f32(after, at) - f32(before, at)) < 0.02 for at in (60, 64))
    assert extras and all((folder/name).read_bytes() == raw for name, raw in extras.items())
    print(f"Native migration: actual prior Crossroads resave kept both med and expanded data; "
          f"{elapsed} live frames, exact old club/house/garage values and {len(extras)} adjacent records; "
          "unused Woodline fields stayed zero.")

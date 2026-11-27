"""Verify a former decorative queue patron through the native SDL launcher.
Usage: python3 scripts/qa/outdoor_cast_host.py build/DeadChannel [capture-dir].
Only temporary careers are created. No real profile or external package needed.
"""
from itertools import groupby
from pathlib import Path
import os
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
EXE = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / "build/DeadChannel"
RECORD = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else None
KEEPS = [10, 15, 25, 34, 42]


def checked(*cmd, **kwargs):
    subprocess.run(cmd, timeout=180, check=True, **kwargs)


def label_rgb(path):
    with path.open("rb") as f:
        assert f.readline().strip() == b"P6"
        w, h = map(int, f.readline().split())
        assert (w, h) == (1152, 690)
        assert f.readline().strip() == b"255"
        pixels = f.read()
    # Bounded crop across the dialogue's nameplate; the center of the
    # street scene and HUD are outside this crop. No OCR dependency.
    return b"".join(pixels[(y*w+260)*3:(y*w+515)*3] for y in range(217, 242))


def paper_count(rgb):
    return sum(rgb[i:i+3] == b"\xf7\xf1\xdb" for i in range(0, len(rgb), 3))


def career(blob):
    assert len(blob) in (304, 1044)
    digest = 2166136261
    for byte in blob[:-4]:
        digest = ((digest ^ byte) * 16777619) & 0xffffffff
    assert digest == struct.unpack_from("<I", blob, len(blob)-4)[0]
    return struct.unpack_from("<ff", blob, 60)


with tempfile.TemporaryDirectory(prefix="dc-outdoor-cast-") as tmp:
    d = Path(tmp)
    data, shots = d / "profile", d / "shots"
    (data / "DeadChannel").mkdir(parents=True)
    shots.mkdir()
    seed = data / "DeadChannel/career.dat"
    cc = os.getenv("CC", "cc")
    checked(cc, "-O0", ROOT/"scripts/qa/outdoor_cast_seed.c", "-o", d/"seed")
    checked(d/"seed", seed)
    start = career(seed.read_bytes())
    checked(cc, "-std=c11", "-O2", "-shared", "-fPIC",
            ROOT/"scripts/qa/linux_replay.c", "-ldl", "-o", d/"replay.so")
    keys = [0] * 55
    for frame, key in ((1,1024), (6,2097152), (12,80), (31,128), (38,2097152), (48,512)):
        keys[frame] = key
    keys[19:24] = [1] * 5  # Try moving/steering while the selectable menu is open.
    tape = f"{len(keys)} 0\n" + "".join(f"{len(tuple(group))} {key}\n" for key, group in groupby(keys))
    env = dict(os.environ, XDG_DATA_HOME=str(data), SDL_VIDEODRIVER="dummy",
               SDL_AUDIODRIVER="dummy", SDL_RENDER_DRIVER="software",
               LD_PRELOAD=str(d/"replay.so"), DC_LINUX_REPLAY="",
               DC_LINUX_SHOW=str(shots), DC_LINUX_CAPTURE_FRAMES=",".join(map(str, KEEPS)),
               DC_LINUX_SNAPSHOT=str(d/"final.dat"))
    checked(EXE, input=tape.encode(), env=env)
    bg, open_menu, held_menu, close_menu, overhead = [label_rgb(shots/f"frame-{n}.ppm") for n in KEEPS]
    counts = list(map(paper_count, (bg, open_menu, held_menu, close_menu, overhead)))
    assert counts[1] > max(counts[0], counts[3], counts[4]) + 65, counts
    assert counts[1] == counts[2], counts
    assert career((d/"final.dat").read_bytes()) == start
    if RECORD:
        RECORD.mkdir(parents=True, exist_ok=True)
        for n in KEEPS:
            shutil.copyfile(shots/f"frame-{n}.ppm", RECORD/f"frame-{n}.ppm")
    print("Native SDL: C, named queue contact, movement hold, Esc, overhead, and valid save at the unchanged curb in 55 keyboard frames.")

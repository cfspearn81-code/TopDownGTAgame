"""Actual SDL2 launch, controls, global panels and safe wharf-on-reload."""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

elf, interposer, fixture, seen = (Path(p).resolve() for p in sys.argv[1:])
seen.mkdir(parents=True, exist_ok=True)
UP, DOWN, LEFT, RIGHT, USE, BRAKE, ESC = 1, 2, 4, 8, 16, 64, 128
MAP, SAVE, ENTER, HELP, PHONE = 256, 512, 1024, 2048, 524288

def plan(steps):
    frames = sum(length for length, _ in steps)
    return (f"{frames} 0\n" + "".join(f"{length} {key}\n" for length, key in steps)).encode()

def image(saved, frame):
    f = saved / f"frame-{frame}.ppm"
    b = f.read_bytes()
    magic, dims, maxval, pixels = b.split(b"\n", 3)
    assert magic == b"P6" and maxval == b"255"
    w, h = map(int, dims.split())
    assert len(pixels) == w*h*3
    return pixels

def contains(rgb, coded):
    return sum(rgb[i:i+3] == coded for i in range(0, len(rgb), 3))

def xdg_inspect(slot, point):
    raw = slot.read_bytes()
    assert len(raw) == 1044 and struct.unpack_from("<II", raw) == (0x44433131, 11)
    h = 2166136261
    for v in raw[:-4]:
        h = ((h ^ v)*16777619) & 0xffffffff
    assert struct.unpack_from("<I", raw, 1040) == (h,)
    x, y = struct.unpack_from("<II", raw, 968)
    assert (x & 0x7fffffff, y) == (point[0]*64, point[1]*64), "An afloat save must restore solid dock land"

with tempfile.TemporaryDirectory(prefix="dc-native-harbor-") as t:
    dst = Path(t) / "DeadChannel"
    dst.mkdir()
    slot = dst / "career.dat"
    subprocess.run([str(fixture),"--fixture",str(slot)],check=True)
    env = dict(os.environ, XDG_DATA_HOME=t, SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy", LD_PRELOAD=str(interposer),
               DC_LINUX_REPLAY="")
    files = seen / "voyage"
    files.mkdir(exist_ok=True)
    tape = [(3,0),(1,ENTER),(6,0),(1,USE),(5,0),(82,UP),(23,UP|RIGHT),(23,UP|LEFT),
            (65,BRAKE),(1,HELP),(7,0),(1,ESC),(5,0),(1,PHONE),(7,0),(1,PHONE),(5,0),
            (1,MAP),(7,0),(1,MAP),(5,0),(1,SAVE),(9,0)]
    captures = [9,15,96,120,143,208,214,228,242,258]
    subprocess.run([str(elf)], input=plan(tape),check=True,timeout=45,
                   env=env|dict(DC_LINUX_SHOW=str(files),DC_LINUX_CAPTURE_FRAMES=",".join(map(str,captures))))
    hud=bytes.fromhex("4f7e81")
    wharf=bytes.fromhex("62a6a0")
    assert contains(image(files,9),wharf)>25 and contains(image(files,9),hud)<15
    for t0 in (15,96,120,143,208,258):
        assert contains(image(files,t0),hud)>25, f"Did not see boat HUD at tick {t0}"
    assert image(files,96) != image(files,15) and image(files,120) != image(files,96)
    for t0 in (214,228,242):
        assert contains(image(files,t0),hud)<25, f"Expected global modal at tick {t0}"
    xdg_inspect(slot,(2948,2206))
    files = seen / "reload"
    files.mkdir(exist_ok=True)
    subprocess.run([str(elf)],input=plan([(3,0),(1,ENTER),(7,0),(1,USE),(3,0),(1,USE),(7,0)]),
                   check=True,timeout=45,env=env|dict(DC_LINUX_SHOW=str(files),DC_LINUX_CAPTURE_FRAMES="9,14,20"))
    assert contains(image(files,9),wharf)>25 and contains(image(files,9),hud)<15
    assert contains(image(files,14),hud)>25
    assert contains(image(files,20),wharf)>25 and contains(image(files,20),hud)<15
    xdg_inspect(slot,(2948,2206))
    print("Native ELF: public apron reload, E, sustained thrust and both steering directions, F1/Signal/Atlas, safe F5 while afloat, dock reload and re-usable boat passed.")

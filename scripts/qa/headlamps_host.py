"""Render actual native SDL2 headlamp cones through both cameras and F2."""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

elf, interposer, fixture, captures = (Path(p).resolve() for p in sys.argv[1:])
captures.mkdir(parents=True, exist_ok=True)
ENTER, USE, CAMERA, GFX = 1024, 16, 2097152, 4096

def replay(events):
    n = sum(ticks for ticks, key in events)
    return (f"{n} 0\n" + "".join(f"{ticks} {key}\n" for ticks, key in events)).encode()

def ppm(name):
    b = (captures / name).read_bytes()
    magic, dims, depth, data = b.split(b"\n", 3)
    assert magic == b"P6" and depth == b"255"
    w, h = map(int, dims.split())
    assert len(data) == 3*w*h
    return w, h, data

def counts(frm, x0, y0, x1, y1):
    # The beam core on dark asphalt is richer in red/yellow than the
    # authored asphalt, beige road stripes or the surrounding sky.
    w, _, rgb = ppm(frm)
    n = 0
    for y in range(y0,y1):
        for x in range(x0,x1):
            i = (y*w+x)*3
            r,g,b = rgb[i:i+3]
            n += r>66 and r>=g and g>b+13
    return n

with tempfile.TemporaryDirectory(prefix="dc-light-host-") as temp:
    dst = Path(temp) / "DeadChannel"
    dst.mkdir()
    subprocess.run([str(fixture), "--fixture", str(dst/"career.dat")], check=True)
    env = dict(os.environ, XDG_DATA_HOME=temp, SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy", LD_PRELOAD=str(interposer),
               DC_LINUX_REPLAY="", DC_LINUX_SHOW=str(captures),
               DC_LINUX_CAPTURE_FRAMES="14,24,34,44,54")
    tape = replay([(3,0),(1,ENTER),(5,0),(1,USE),(8,0),(1,CAMERA),(9,0),(1,GFX),
                   (9,0),(1,GFX),(9,0),(1,CAMERA),(11,0)])
    subprocess.run([str(elf)], env=env, input=tape, check=True, timeout=30)
    b = (dst / "career.dat").read_bytes()
    assert struct.unpack_from("<II",b) == (0x44433131,10), "C twice should restore the exact zero camera setting"
    for frame in ("frame-14.ppm", "frame-54.ppm"):
        left = counts(frame, 327,323,416,341)
        center = counts(frame, 327,343,416,350)
        right = counts(frame, 327,351,416,365)
        assert left>160 and right>160 and center<10, (frame,left,center,right)
    for frame in ("frame-24.ppm","frame-44.ppm"):
        left = counts(frame, 498,306,539,366)
        center = counts(frame, 548,306,564,366)
        right = counts(frame, 570,306,620,366)
        assert left>18 and right>60 and center<9, (frame,left,center,right)
    assert counts("frame-34.ppm", 327,323,416,365)<20
    print("Native SDL2: E boards the car; C changes camera, F2 suppresses the twin beams, F2 restores them and C returns overhead.")

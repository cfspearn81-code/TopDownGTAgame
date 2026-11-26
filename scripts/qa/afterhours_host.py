"""Replay Afterhours P/F7 and save migration through the actual native ELF."""
import gzip
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

exe, interposer = (Path(p).resolve() for p in sys.argv[1:])
NONE, ENTER, LEFT, DOWN, PHONE, ALT, SAVE = 0,1024,4,2,524288,1048576,512
EXT_PHONE = 304+(32+64)*4

def tape(*masks):
    return (f"{len(masks)} 0\n" + "".join(f"1 {v}\n" for v in masks)).encode()

def valid(p, rev):
    b = p.read_bytes()
    assert len(b) == {10:304,11:1044}[rev]
    assert struct.unpack_from("<II", b) == (0x44433131,rev)
    h = 2166136261
    for v in b[:-4]:
        h = ((h ^ v)*16777619) & 0xffffffff
    assert struct.unpack_from("<I",b,len(b)-4)[0] == h
    return b

with tempfile.TemporaryDirectory(prefix="dc-afterhours-sdl-") as td:
    env = dict(os.environ, XDG_DATA_HOME=td, SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
               LD_PRELOAD=str(interposer), DC_LINUX_REPLAY="")
    folder = Path(td)/"DeadChannel"
    result = folder/"career.dat"
    run = lambda plan: subprocess.run([str(exe)], env=env, input=plan, check=True, timeout=20)
    run(tape(NONE,ENTER,NONE,ENTER,NONE,PHONE,NONE,LEFT,NONE,ENTER,NONE,ALT,NONE,SAVE,NONE))
    b = valid(result,11)
    assert struct.unpack_from("<II",b,EXT_PHONE) == (1,0), "Settings did not respond to P"
    snaps = list(folder.glob(".pre-afterhours-career.dat-r10-*.bak"))
    assert len(snaps)==1
    raw_backup=valid(snaps[0],10)
    run(tape(NONE,ENTER,NONE,ALT,NONE,LEFT,NONE,LEFT,NONE,DOWN,NONE,ENTER,NONE,ALT,NONE,SAVE,NONE))
    b = valid(result,11)
    assert struct.unpack_from("<II",b,EXT_PHONE) == (1,1), "Custom F7 mapping or station was not retained"
    assert len(list(folder.glob(".pre-afterhours-*.bak")))==1 and snaps[0].read_bytes() == raw_backup
    print("Native SDL Afterhours: P creates a rebind, clean quit saves; F7 works on relaunch, Glasswave selection persists, original Rev10 backup stays byte-identical.")

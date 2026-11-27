"""Actual launcher replay for the former Freight Parkway lamp position.
Usage after both native builds: python3 scripts/qa/outer_lamp_host.py [frame-dir].
Uses only disposable careers; optional PPMs are delivered by SDL itself."""
import os
from pathlib import Path
import struct
import subprocess as sp
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
cc = os.environ.get("CC", "cc")
def run(*cmd, **kwargs):
    return sp.run(cmd, check=True, timeout=180, **kwargs)

with tempfile.TemporaryDirectory(prefix="dc-lamp-shoulder-") as tmp:
    d = Path(tmp)
    run(cc, "-std=c11", "-O0", root/"scripts/qa/outer_lamp_drive_plan.c", "-o", d/"plan")
    run(d/"plan", d/"drive")
    tape = (d/"drive.plan").read_bytes()
    env = dict(os.environ, DC_TRIP_SEED="136")
    run(cc, "-O2", root/"scripts/check_pe.c", "-o", d/"pe")
    run(d/"pe", root/"build/DeadChannel.exe", "--tour", input=tape,
        env=dict(env, DC_TRIP_LOAD=str(d/"drive.dat"), DC_TRIP_SAVE=str(d/"pe.dat")))
    run(cc, "-O2", "-fPIC", "-shared", root/"scripts/qa/linux_replay.c", "-ldl", "-o", d/"sdl.so")
    (d/"DeadChannel").mkdir()
    (d/"DeadChannel"/"career.dat").write_bytes((d/"drive.dat").read_bytes())
    if len(sys.argv)>1:
        dest = Path(sys.argv[1]).resolve()
        dest.mkdir(parents=True, exist_ok=True)
        frames = ",".join(r.split()[0] for r in (d/"drive.frames").read_text().splitlines())
        env.update(DC_LINUX_SHOW=str(dest), DC_LINUX_CAPTURE_FRAMES=frames)
    run(root/"build/DeadChannel", input=tape, env=dict(env, SDL_VIDEODRIVER="dummy",
        SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy", XDG_DATA_HOME=str(d),
        LD_PRELOAD=str(d/"sdl.so"), DC_LINUX_REPLAY="", DC_LINUX_SNAPSHOT=str(d/"elf.dat")))
    elf = (d/"elf.dat").read_bytes()
    assert len(elf)==1044 and elf[:304] == (d/"pe.dat").read_bytes()[:304]
    assert struct.unpack_from("<I",elf,304+(32+60)*4)[0]==0  # C back to overhead
    x,y=struct.unpack_from("<II",elf,968)
    assert (x&0x7fffffff)/64 > 2218 and y/64 == 1007
    print("PASS: native SDL and simulated Win32 crossed Freight Parkway's former post at (2215,1007), toggled both views, and saved matching careers.")

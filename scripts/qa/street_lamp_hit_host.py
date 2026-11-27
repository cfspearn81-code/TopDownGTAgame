"""Launch the real apps with the same input: board, C, hit, hold W, reverse.
Optional directory receives native SDL frames. Careers are disposable."""
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

with tempfile.TemporaryDirectory(prefix="dc-lamp-collision-") as scratch:
    d = Path(scratch)
    run(cc, "-std=c11", "-O0", root/"scripts/qa/street_lamp_hit_plan.c", "-o", d/"plan")
    run(d/"plan", d/"drive")
    tape = (d/"drive.plan").read_bytes()
    head, inputs = tape.split(b"\n", 1)
    tape = head.split()[0]+b" 0\n"+inputs  # check matching bytes after both hosts
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
    src=(d/"drive.dat").read_bytes()
    elf=(d/"elf.dat").read_bytes()
    win=(d/"pe.dat").read_bytes()
    assert elf[:304]==win[:304] and elf[:304]!=src[:304]
    x,y=struct.unpack_from("<ff",elf,60)
    assert 400 < x < 440 and 535 < y < 560,(x,y)
    assert 90 < struct.unpack_from("<i",elf,52)[0] < 100
    print(f"PASS: {head.split()[0].decode()} actual keyboard frames; native SDL and simulated Win32 match after impact, 120 ticks holding throttle, and reversing away.")

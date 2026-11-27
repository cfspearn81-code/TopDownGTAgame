"""Drive the same 220-keyboard-frame downtown route in both desktop releases.
Run after scripts/build-linux.sh and scripts/build.sh. Optional positional
directory receives two unaltered Linux screenshots. No existing slots touched."""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
def run(*cmd, **kwargs):
    return subprocess.run(cmd, check=True, timeout=180, **kwargs)

with tempfile.TemporaryDirectory(prefix="chase-city-") as scratch:
    tmp = Path(scratch)
    cc = os.environ.get("CC", "cc")
    run(cc, "-O0", root/"scripts/qa/chase_city_plan.c", "-o", tmp/"plan")
    run(tmp/"plan", tmp/"drive")
    tape = (tmp/"drive.plan").read_bytes()
    frames, expected = tape.splitlines()[0].split()
    run(cc, "-O2", root/"scripts/check_pe.c", "-o", tmp/"pe")
    run(cc, "-O2", "-fPIC", "-shared", root/"scripts/qa/linux_replay.c", "-ldl", "-o", tmp/"shim.so")
    settings = dict(os.environ, DC_TRIP_SEED="136")
    run(tmp/"pe", root/"build/DeadChannel.exe", "--tour", input=tape,
        env=dict(settings, DC_TRIP_LOAD=str(tmp/"drive.dat"), DC_TRIP_SAVE=str(tmp/"win.dat")))
    (tmp/"DeadChannel").mkdir()
    (tmp/"DeadChannel/career.dat").write_bytes((tmp/"drive.dat").read_bytes())
    if len(sys.argv)>1:
        dest = Path(sys.argv[1]).resolve()
        dest.mkdir(parents=True, exist_ok=True)
        settings.update(DC_LINUX_SHOW=str(dest), DC_LINUX_CAPTURE_FRAMES=",".join(
            s.split()[0] for s in (tmp/"drive.frames").read_text().splitlines()))
    run(root/"build/DeadChannel", input=tape, env=dict(settings, SDL_VIDEODRIVER="dummy",
        SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy", LD_PRELOAD=str(tmp/"shim.so"),
        XDG_DATA_HOME=str(tmp), DC_LINUX_REPLAY="", DC_LINUX_SNAPSHOT=str(tmp/"elf.dat")))
    result = (tmp/"elf.dat").read_bytes()
    assert result[:304] == (tmp/"win.dat").read_bytes()[:304]
    assert len(result)>=304 and struct.unpack_from("<I",result,300)[0] == int(expected,16)
    print(f"{frames.decode()} keyboard frames: E board, C, downtown turn, C return, save "
          f"{expected.decode()}; native SDL and simulated Win32 match.")

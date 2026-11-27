"""Native ELF, real SDL dummy audio: recovered output during an ordinary car ride."""
import itertools
import os
from pathlib import Path
import struct
import subprocess as sp
import tempfile

root = Path(__file__).resolve().parents[2]
cc = os.environ.get("CC", "cc")
with tempfile.TemporaryDirectory(prefix="dc-radio-reconnect-") as temp:
    d = Path(temp)
    def run(*args, **kwargs): return sp.run(args, check=True, timeout=180, **kwargs)
    run(cc, "-O0", root/"scripts/qa/outer_traffic_plan.c", "-o", d/"plan")
    run(d/"plan", d/"ride")
    run(cc, "-O2", "-shared", "-fPIC", root/"scripts/qa/linux_replay.c", "-ldl", "-o", d/"replay.so")
    run(cc, "-O2", "-shared", "-fPIC", root/"scripts/qa/linux_audio_reconnect.c", "-ldl", "-o", d/"audio.so")
    # Tune station 1 in the synthetic profile. Its gameplay is otherwise
    # unchanged: use the real driver's first E boarding, then leave the radio on.
    data = bytearray((d/"ride.dat").read_bytes()); assert len(data)==1044
    struct.pack_into("<I", data, 304+(32+64+1)*4, 1)
    hashval = 2166136261
    for value in data[:-4]: hashval = (hashval^value)*16777619 & 0xffffffff
    struct.pack_into("<I", data, len(data)-4, hashval)
    path = d/"xdg/DeadChannel"; path.mkdir(parents=True)
    (path/"career.dat").write_bytes(data)
    keys=[]
    for line in (d/"ride.plan").read_text().splitlines()[1:]:
        n, mask = map(int, line.split()); keys += [mask]*n
    keys = keys[:keys.index(16)+1] + [0]*600
    tape = str(len(keys))+" 0\n"+"".join(f"{len(list(g))} {k}\n" for k,g in itertools.groupby(keys))
    env = dict(os.environ,SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",SDL_AUDIODRIVER="dummy",
               DC_TRIP_SEED="136",XDG_DATA_HOME=str(path.parent),LD_PRELOAD=f"{d}/audio.so:{d}/replay.so")
    run(root/"build/DeadChannel",input=tape.encode(),env=env)

"""Native Linux SDL storefront and exact gear/wallet/relaunch regression."""
from pathlib import Path
import os
import struct
import subprocess
import sys
import tempfile

exe, preload, fixture = [Path(v).resolve() for v in sys.argv[1:]]
UP, DOWN, LEFT, E, ESC, F5, ENTER, PHONE = 1, 2, 4, 16, 128, 512, 1024, 524288
GEAR_WORD = 304 + (32+64+64+4)*4
PULSE_AND_SPARE = (1<<30)|0x4d

def tape(*keys):
    parts = [(1, 0)]
    for k in keys:
        parts += [(1, k), (1, 0)]
    return (f"{len(parts)} 0\n" + "".join(f"{n} {v}\n" for n,v in parts)).encode()

with tempfile.TemporaryDirectory(prefix="dc-shade-host-") as d:
    env = dict(os.environ, XDG_DATA_HOME=d, SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy")
    subprocess.run([fixture, "--seed"], check=True, timeout=35,
                   env={k:v for k,v in env.items() if k!="LD_PRELOAD"})
    env.update(LD_PRELOAD=str(preload), DC_LINUX_REPLAY="")
    fn=Path(d)/"DeadChannel"/"career.dat"
    def run(*events):
        subprocess.run([exe], check=True, timeout=55, input=tape(*events), env=env)
        b=fn.read_bytes()
        assert len(b)==1044 and struct.unpack_from("<II", b)==(0x44433131,11)
        h=2166136261
        for c in b[:-4]:h=((h^c)*16777619)&0xffffffff
        assert h==struct.unpack_from("<I",b,len(b)-4)[0]
        return struct.unpack_from("<i",b,8)[0],struct.unpack_from("<I",b,GEAR_WORD)[0]
    assert run(ENTER,PHONE,LEFT,UP,ENTER,PHONE,ENTER,E,E,DOWN,E,E,DOWN,E,E,ESC,F5) == (252,PULSE_AND_SPARE|(63<<8))
    assert run(ENTER,ENTER,E,ESC,F5) == (252,PULSE_AND_SPARE|(61<<8))
    assert run(ENTER,ENTER,ENTER,ESC,F5) == (252,PULSE_AND_SPARE|(63<<8))
    print("PASS native Linux Shade Supply: Settings waypoint, storefront input, three exact purchases, ownership/equip survive two process restarts, neighboring Pulse bits untouched.")

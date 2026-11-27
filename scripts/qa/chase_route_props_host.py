"""Native SDL regression for the scheduled Signal vending prop and C toggle.
Runs the same replay in an unmodified release launcher if available."""
import os
from pathlib import Path
import struct
import subprocess as sp
import sys
import tempfile

game, interposer, fixture, capture = map(lambda p:Path(p).resolve(),sys.argv[1:5])
before=Path(sys.argv[5]).resolve() if len(sys.argv)>5 else None
capture.mkdir(parents=True,exist_ok=True)
keys=((3,0),(1,1024),(7,0),(1,1<<21),(7,0),(1,16),(9,0),
      (1,128),(9,0),(1,512),(5,0))
tape=(f"{sum(t for t,_ in keys)} 0\n"+"".join(f"{t} {k}\n" for t,k in keys)).encode()

def play(exe, target):
    target.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="dc-street-props-") as d:
        slot=Path(d)/"DeadChannel/career.dat"
        slot.parent.mkdir()
        sp.run([fixture,"--vendor-fixture",slot],check=True,timeout=40)
        env=dict(os.environ, XDG_DATA_HOME=d, SDL_VIDEODRIVER="dummy",
                 SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
                 LD_PRELOAD=str(interposer),DC_LINUX_REPLAY="",DC_LINUX_SHOW=str(target),
                 DC_LINUX_CAPTURE_FRAMES="18,25,37")
        sp.run([exe],input=tape,env=env,check=True,timeout=70)
        data=slot.read_bytes()
        assert len(data)==1044 and struct.unpack_from("<I",data,304+4*(32+60))[0]==1
        return data

def frame(folder, number):
    p=(folder/f"frame-{number}.ppm").read_bytes().split(b"\n",3)
    assert p[0]==b"P6" and p[2]==b"255"
    return p[3]

def colors(data, *items):
    aligned=list(zip(data[::3],data[1::3],data[2::3]))
    return [aligned.count(tuple(bytes.fromhex(color))) for color in items]

result=play(game,capture)
# The 3D badge border is 425864; the street-only vendor callout uses f6aada.
for k in (18,37): assert min(colors(frame(capture,k),"425864","f6aada"))>=10
assert max(colors(frame(capture,25),"425864","f6aada"))<10
if before:
    original=capture/"before"
    other=play(before,original)
    assert result==other, "the same native keyboard sequence must save byte-identical careers"
    assert frame(capture,18)!=frame(original,18), "new 3D street art never reached the presented frame"
print("PASS native SDL: resumed at the scheduled vendor, C switched, E opened the onboard inventory, Esc returned to 3D, F5 saved." + (" Compared against the previous build: new scene pixels and byte-identical careers." if before else ""))

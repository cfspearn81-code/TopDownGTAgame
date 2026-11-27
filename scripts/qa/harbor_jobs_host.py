"""Native SDL regression for Harbor Office, actual input and safe on-water save."""
from pathlib import Path
import os, struct, subprocess, sys, tempfile

exe, interposer, fixture = (Path(v).resolve() for v in sys.argv[1:])
UP, DOWN, RIGHT, E, ESC, MAP, F5, ENTER, PHONE = 1, 2, 8, 16, 128, 256, 512, 1024, 524288
OFF = 304 + (32 + 64) * 4
def tape(*inputs):
    out = [(1,0)]
    for event in inputs:
        if isinstance(event,tuple): out.append(event)
        else: out.extend(((1,event),(1,0)))
    return (f"{sum(n for n,_ in out)} 0\n" + "".join(f"{n} {v}\n" for n,v in out)).encode()
def play(seed, callback):
    with tempfile.TemporaryDirectory(prefix="dc-native-harbor-") as d:
        env = dict(os.environ, XDG_DATA_HOME=d, SDL_VIDEODRIVER="dummy",
                   SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy")
        subprocess.run([fixture,"--seed",seed],env={k:v for k,v in env.items() if k!="LD_PRELOAD"},check=True,timeout=35)
        env.update(LD_PRELOAD=str(interposer), DC_LINUX_REPLAY="")
        file=Path(d)/"DeadChannel"/"career.dat"
        def run(*mask):
            subprocess.run([exe],env=env,input=tape(*mask),check=True,timeout=70)
            data=file.read_bytes()
            assert len(data)==1044 and struct.unpack_from("<II",data)==(0x44433131,11)
            h=2166136261
            for c in data[:-4]: h=((h^c)*16777619)&0xffffffff
            assert struct.unpack_from("<I",data,len(data)-4)[0]==h
            return struct.unpack_from("<i",data,8)[0],struct.unpack_from("<64I",data,OFF)
        callback(run)
def office(run):
    cash, w=run(ENTER,E,DOWN,ENTER,ENTER,ESC,F5)
    assert cash==500 and w[13:16]==(6,0,0)
    # A pending cancellation must not survive looking at Receipts, but
    # the final Night Wire card must remain visible after leaving Receipts.
    cash,w=run(ENTER,PHONE,RIGHT,RIGHT,UP,ENTER,MAP,MAP,ENTER,ESC,F5)
    assert cash==500 and w[13:16]==(6,0,0)
    cash,w=run(ENTER,PHONE,RIGHT,RIGHT,UP,MAP,MAP,ENTER,ENTER,ESC,F5)
    assert cash==500 and w[13:16]==(0,0,0)
def alley(run):
    cash,w=run(ENTER,E,E,F5)
    assert cash==500 and w[13:16]==(3,0,0)
    assert run(ENTER,F5)[1][13]==3
def far_buoy(run):
    cash,w=run(ENTER,E,(114,UP),E,F5)
    assert cash==685 and w[13:16]==(0,1,0), (cash,w[13:16])
    cash,w=run(ENTER,F5)
    assert cash==685 and w[13:16]==(0,1,0)
play("office",office)
play("stages",alley)
play("afloat",far_buoy)
print("PASS Linux SDL: Harbor Office accept/cancel, scripted automaton disable/carry, open-water skiff input and actual buoy payout, process relaunch without repeat claims.")

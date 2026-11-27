"""Exercise a compact stock save expanding through the real PE's Signal UI.
Requires compiled check_pe and exe; optional 3rd arg is original PE (no backup).
Uses only temporary fixture saves."""
import os
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
host, exe, *before = map(lambda p: Path(p).resolve(), sys.argv[1:])
def fnv(b):
    h = 2166136261
    for value in b:
        h = ((h ^ value) * 16777619) & 0xffffffff
    return h
def checked(b):
    assert int.from_bytes(b[-4:], "little") == fnv(b[:-4])
    return b
def run(pe, old, tape, stem):
    out = folder / (stem + ".dat")
    backup = folder / (stem + ".bak")
    env = dict(os.environ, DC_TRIP_LOAD=str(old), DC_TRIP_SAVE=str(out), DC_TRIP_BACKUP=str(backup))
    subprocess.run([host,pe,"--tour"], input=tape, env=env, check=True, timeout=45)
    return checked(out.read_bytes()),backup
def tape(*keys):
    seq=[0]
    for key in keys:
        seq.extend([key,0,0])
    return (f"{len(seq)} 0\n"+ "".join(f"1 {key}\n" for key in seq)).encode()
with tempfile.TemporaryDirectory(prefix="dc-windows-compat-") as tmp:
    folder = Path(tmp)
    seed = folder/"stock.dat"
    subprocess.run([host, exe, "-", seed], check=True, timeout=45)
    orig = bytearray(checked(seed.read_bytes()))
    assert len(orig) == 304
    orig[4:8] = (11).to_bytes(4, "little"); orig[267]=2
    orig[-4:] = fnv(orig[:-4]).to_bytes(4, "little")
    seed.write_bytes(orig)
    keys=tape(1024,524288,4,1024,128,512)
    if before:
        old,bb = run(before[0],seed,keys,"before")
        assert len(old)==1044 and not bb.exists()
    new,bb = run(exe,seed,keys,"after")
    assert len(new)==1044 and new[267]==2 and bb.read_bytes()==orig
    assert int.from_bytes(new[688:692],"little")==1
    n2,b2 = run(exe,folder/"after.dat",tape(1024,512),"again")
    assert len(n2)==1044 and n2[267]==2 and not b2.exists()
    print("Exact PE under simulated Win32: Signal expanded 304 to 1,044 bytes, preserved the exact compact 2-patch career, no repeat backup after reload.")
    # Re-use the launch player's profile seed to approach Bell Lane with an
    # open Revenue case. Confirm through the visible door/lease controls.
    rev = bytearray(orig)
    struct.pack_into("<I",rev,8,2100);struct.pack_into("<ff",rev,60,174.,944.)
    struct.pack_into("<I",rev,148,1);struct.pack_into("<I",rev,172,4)
    struct.pack_into("<II",rev,292,501,18000)
    rev[-4:]=fnv(rev[:-4]).to_bytes(4,"little")
    seed.write_bytes(rev)
    lease=tape(1024,1024,1024,1024,512)
    if before:
        old,_=run(before[0],seed,lease,"deedbefore")
        assert len(old)==1044 and struct.unpack_from("<I",old,8)[0]==1620 and old[688+58*4]&1
    ok,_=run(exe,seed,lease,"deedafter")
    assert len(ok)==304 and struct.unpack_from("<I",ok,8)[0]==2100 and ok[148]==1
    print("Simulated Windows controller path: Bell Lane correctly refused confirmation on a $480 deed during an open case.")
    if os.getenv("DC_ECONOMY_ELF") and os.getenv("DC_ECONOMY_PRELOAD"):
        env=dict(os.environ,SDL_VIDEODRIVER="dummy",SDL_AUDIODRIVER="dummy",SDL_RENDER_DRIVER="software",
                 XDG_DATA_HOME=str(folder/"xdg"),DC_TRIP_SEED="17",DC_LINUX_REPLAY="",
                 LD_PRELOAD=str(Path(os.environ["DC_ECONOMY_PRELOAD"]).resolve()))
        active=folder/"xdg/DeadChannel/career.dat"
        active.parent.mkdir(parents=True)
        for app,exp in [(os.getenv("DC_ECONOMY_OLD_ELF"),1620),(os.environ["DC_ECONOMY_ELF"],2100)]:
            if not app:continue
            active.write_bytes(rev)
            subprocess.run([str(Path(app).resolve())],input=lease,env=env,check=True,timeout=45)
            actual=checked(active.read_bytes())
            assert struct.unpack_from("<I",actual,8)[0]==exp and actual[148]==1
        print("Native Linux SDL controller path: Bell Lane case restriction confirmed.")

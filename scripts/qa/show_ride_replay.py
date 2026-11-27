"""Rebuild both launchers, then run: python3 scripts/qa/show_ride_replay.py.
A fictional career and all replay output live in an isolated temporary directory.
Real Linux SDL entry; Windows PE entry with simulated Win32, not real Windows."""
import os
from pathlib import Path
import struct
import subprocess as sp
import tempfile
r=Path(__file__).resolve().parents[2]
cc=os.environ.get("CC","cc")
with tempfile.TemporaryDirectory(prefix="show-ride-") as tmp:
    d=Path(tmp);prefix=d/"drive";store=d/"xdg/DeadChannel";store.mkdir(parents=True)
    def run(*args,**kwargs):return sp.run(args,check=True,timeout=180,**kwargs)
    run(cc,"-std=c11","-O0","-I",r/"src",r/"scripts/qa/show_ride_plan.c","-o",d/"plan")
    run(d/"plan",prefix)
    tape=prefix.with_suffix(".plan").read_bytes();fixture=prefix.with_suffix(".dat")
    run(cc,"-O2",r/"scripts/check_pe.c","-o",d/"pe")
    run(cc,"-O2","-fPIC","-shared",r/"scripts/qa/linux_replay.c","-ldl","-o",d/"sdl.so")
    env=dict(os.environ,DC_TRIP_SEED="17")
    # The simulated PlaySound stub tests one-shots only, not venue music loops.
    run(d/"pe",r/"build/DeadChannel.exe","--tour",input=tape,env=dict(env,DC_TRIP_LOAD=str(fixture),DC_TRIP_SAVE=str(d/"pe.dat"),DC_RADIO_UNAVAILABLE="1"))
    (store/"career.dat").write_bytes(fixture.read_bytes())
    run(r/"build/DeadChannel",input=tape,env=dict(env,SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",SDL_AUDIODRIVER="dummy",XDG_DATA_HOME=str(store.parent),DC_LINUX_REPLAY="",LD_PRELOAD=str(d/"sdl.so"),DC_LINUX_SNAPSHOT=str(d/"elf.dat")))
    b=(d/"pe.dat").read_bytes()
    assert b==(d/"elf.dat").read_bytes() and struct.unpack_from("<I",b,8)[0]==433 and struct.unpack_from("<I",b,196)[0]==0
    print(f"PASS: {tape.split()[0].decode()} keyboard frames. The Afterimage to Copper Playhouse: both launchers paid $115 clean once, and neither counted a taxi fare; identical career bytes.")

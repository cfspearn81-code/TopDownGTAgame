"""Run the same E theft/drive/reboard through both actual launchers.
Uses isolated saves, SDL dummy video and Linux-simulated Win32 only."""
import os
from pathlib import Path
import struct
import subprocess as sp
import tempfile
r=Path(__file__).resolve().parents[2]
cc=os.environ.get("CC","cc")
with tempfile.TemporaryDirectory(prefix="outer-fleet-") as tmp:
    d=Path(tmp); prefix=d/"drive"; store=d/"xdg/DeadChannel"; store.mkdir(parents=True)
    def run(*args,**kwargs): return sp.run(args,check=True,timeout=180,**kwargs)
    run(cc,"-std=c11","-O0",r/"scripts/qa/outer_traffic_plan.c","-o",d/"plan")
    run(d/"plan",prefix)
    tape=prefix.with_suffix(".plan").read_bytes(); fixture=prefix.with_suffix(".dat")
    run(cc,"-O2",r/"scripts/check_pe.c","-o",d/"pe")
    # The shared SDL replay helper snapshots the full explicit save.
    run(cc,"-O2","-fPIC","-shared",r/"scripts/qa/linux_replay.c","-ldl","-o",d/"sdl.so")
    env=dict(os.environ,DC_TRIP_SEED="136")
    run(d/"pe",r/"build/DeadChannel.exe","--tour",input=tape,env=dict(env,DC_TRIP_LOAD=str(fixture),DC_TRIP_SAVE=str(d/"pe.dat")))
    (store/"career.dat").write_bytes(fixture.read_bytes())
    run(r/"build/DeadChannel",input=tape,env=dict(env,SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",SDL_AUDIODRIVER="dummy",XDG_DATA_HOME=str(store.parent),DC_LINUX_REPLAY="",LD_PRELOAD=str(d/"sdl.so"),DC_LINUX_SNAPSHOT=str(d/"elf.dat")))
    b=(d/"elf.dat").read_bytes()
    assert len(b)==1044 and struct.unpack_from("<I",b,4)[0]==11
    # Windows replay verifier captures the original-career-size prefix.
    assert b[:304]==(d/"pe.dat").read_bytes()[:304]
    assert struct.unpack_from("<I",b,304+(32+60)*4)[0]==1  # C preference
    x,y=struct.unpack_from("<II",b,968)
    expectx,expecty=map(float,prefix.with_suffix(".expected").read_text().split())
    assert abs((x&0x7fffffff)/64-expectx)<.02 and abs(y/64-expecty)<.02
    assert struct.unpack_from("<I",b,28)[0]==0  # never became an owned car
    print(f"PASS: {tape.split()[0].decode()} identical keyboard frames: car boarded with E, chase toggled, driven, parked and boarded again; correct final location and no ownership added.")

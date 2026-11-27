"""Rebuild both launchers, then run: python3 scripts/qa/outer_taxi_replay.py.
A fictional career and all replay output live in an isolated temporary directory.
Real Linux SDL entry; Windows PE entry with simulated Win32, not real Windows."""
import os
from pathlib import Path
import struct
import subprocess as sp
import tempfile
import argparse
r=Path(__file__).resolve().parents[2]
cc=os.environ.get("CC","cc")
args=argparse.ArgumentParser()
args.add_argument("--radio",action="store_true")
args.add_argument("--outbound",nargs="?",const=1,type=int,choices=range(6),
                  help="test downtown to region 0..5 (default: Parkway Overlook)")
options=args.parse_args()
stops=("Glass Bluff","Parkway Overlook","Belt Workshops","Harbor Pier","Salt Reach","Windmill Curve")
seeds=(12492,12448,12350,12316,12416,12308)
pay=(204,147,130,204,218,244)
with tempfile.TemporaryDirectory(prefix="outer-taxi-") as tmp:
    d=Path(tmp);prefix=d/"drive";store=d/"xdg/DeadChannel";store.mkdir(parents=True)
    def run(*args,**kwargs):return sp.run(args,check=True,timeout=180,**kwargs)
    stop=options.outbound;outbound=stop is not None
    planner="outbound_taxi_plan.c" if outbound else "outer_taxi_plan.c"
    run(cc,"-std=c11","-O0","-I",r/"src",r/"scripts/qa"/planner,"-o",d/"plan")
    radio=outbound or options.radio
    run(d/"plan",prefix,*([str(stop)] if outbound else ["radio"] if radio else []))
    tape=prefix.with_suffix(".plan").read_bytes();fixture=prefix.with_suffix(".dat")
    run(cc,"-O2",r/"scripts/check_pe.c","-o",d/"pe")
    run(cc,"-O2","-fPIC","-shared",r/"scripts/qa/linux_replay.c","-ldl","-o",d/"sdl.so")
    env=dict(os.environ,DC_TRIP_SEED=str(seeds[stop]) if outbound else "136")
    run(d/"pe",r/"build/DeadChannel.exe","--tour",input=tape,env=dict(env,DC_TRIP_LOAD=str(fixture),DC_TRIP_SAVE=str(d/"pe.dat")))
    (store/"career.dat").write_bytes(fixture.read_bytes())
    run(r/"build/DeadChannel",input=tape,env=dict(env,SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",SDL_AUDIODRIVER="dummy",XDG_DATA_HOME=str(store.parent),DC_LINUX_REPLAY="",LD_PRELOAD=str(d/"sdl.so"),DC_LINUX_SNAPSHOT=str(d/"elf.dat")))
    b=(d/"pe.dat").read_bytes()
    fee=pay[stop] if outbound else 147
    assert b==(d/"elf.dat").read_bytes() and struct.unpack_from("<I",b,8)[0]==50+fee and struct.unpack_from("<I",b,196)[0]==2
    route=f"Wrecker Yard to {stops[stop]}" if outbound else "Parkway Overlook to Wrecker Yard"
    print(f"PASS: {tape.split()[0].decode()} keyboard frames. {route}: both launchers paid ${fee} once; identical career bytes. Radio/receipt: {radio}.")

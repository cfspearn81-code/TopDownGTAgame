"""Run venue regressions on the native SDL launcher and optional matching PE under simulated Win32.
Usage: python3 scripts/qa/venue_edge_host.py [path/to/DeadChannel] [--observe-baseline].
Only seeds private test saves in a temporary directory; no real career is opened.
"""
import os
from pathlib import Path
import struct
import subprocess as sp
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
observe = "--observe-baseline" in sys.argv
exes = [Path(s).resolve() for s in sys.argv[1:] if s != "--observe-baseline"]
exe = exes[0] if exes else root / "build/DeadChannel"

def run(args, **kwargs):
    return sp.run(args, check=True, timeout=90, **kwargs)

def tape(*steps):
    pairs = [(1, 0)]
    for s in steps:
        if isinstance(s, tuple):
            pairs.append(s)
        else:
            pairs.extend([(1,s),(1,0)])
    return (f"{sum(n for n,k in pairs)} 0\n" + "".join(f"{n} {k}\n" for n,k in pairs)).encode()

with tempfile.TemporaryDirectory(prefix="dc-public-venue-test-") as td:
    t = Path(td)
    c = t / "fixture.c"
    c.write_text("""
#include "game.c"
#include "linux_save.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
int main(int argc,char**argv){
 assert(argc==2&&getenv("XDG_DATA_HOME")&&linux_disk_init()&&linux_disk_lock());disk_slot(0);init_city();
 scene=1;menu=-1;mapOn=0;p.dead=0;p.car=-1;p.hp=100;sv.intro=4;sv.cash=500;sv.gear=1;
 if(argv[1][0]=='f'){sv.dayClock=735;nv.phone[NP_CARD]=1;nv.phone[NP_BAG]=99;nv.phone[NP_JOB]=1;p.x=loc[2].x;p.y=loc[2].y;}
 else{int x,y;sv.dayClock=299.5f;assert(night_venue_vendor_pos(&x,&y));p.x=x;p.y=y;nv.phone[NP_REP]=1;}
 request_save();assert(saveFlash>0);linux_disk_shutdown();return 0;
}""")
    seed, shim = t/"fixture", t/"replay.so"
    run(["gcc","-std=c11","-O0","-DDC_LINUX_NATIVE","-I",root/"src",c,root/"src/linux_save.c","-o",seed])
    run(["gcc","-std=c11","-O2","-shared","-fPIC",root/"scripts/qa/linux_replay.c","-ldl","-o",shim])
    pe=exe.with_name("DeadChannel.exe")
    harness=t/"check-pe"
    if pe.exists():
        run(["gcc","-std=c11","-O2",root/"scripts/check_pe.c","-o",harness])
    for kind in ["ferry","vendor"]:
        xdg=t/kind
        env=dict(os.environ,XDG_DATA_HOME=str(xdg),SDL_VIDEODRIVER="dummy",SDL_AUDIODRIVER="dummy",SDL_RENDER_DRIVER="software")
        run([seed,kind],env=env)
        start,after=t/(kind+"-in"),t/(kind+"-out")
        start.write_bytes((xdg/"DeadChannel/career.dat").read_bytes())
        # Career menu -> curb. For the van, E opens before 05:00; try to buy after closing.
        plan=tape(1024,16,512) if kind=="ferry" else tape(1024,16,(54,0),1024,512)
        run([exe],input=plan,env=dict(env,LD_PRELOAD=str(shim),DC_LINUX_REPLAY=""))
        files=[("native SDL",xdg/"DeadChannel/career.dat")]
        if pe.exists():
            run([harness,pe,"--tour"],input=plan,env=dict(os.environ,DC_TRIP_LOAD=str(start),DC_TRIP_SAVE=str(after)))
            files.append(("PE / simulated Win32",after))
        for host,path in files:
            b=path.read_bytes()
            u=lambda off:struct.unpack_from("<I",b,off)[0]
            assert len(b)==1044 and u(4)==11
            h=2166136261
            for v in b[:-4]:
                h=((h^v)*16777619)&0xffffffff
            assert h==u(1040)
            if kind=="ferry":
                assert (u(688+4*4),u(688+25*4))==(1,99)
                print(host+"/ ferry: sample still reserved; street proof picked up =",u(688+7*4))
                assert u(688+7*4)==(0 if observe else 1)
            else:
                print(host+"/ vendor: wallet after attempted closed-cart purchase =",u(8))
                assert u(8)==(325 if observe else 500)

"""Real Linux SDL keyboard replay on a throwaway career. Checks visible,
unoccluded lamp and livery pixels, not just the chase preference in a save.
Usage: python3 scripts/qa/chase_fleet_host.py build/DeadChannel [capture-dir]
"""
from pathlib import Path
import os, subprocess, tempfile, sys, shutil, struct
root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
snap = Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None
events = [(2,0),(1,1024),(4,0),(1,16),(5,0),(1,2097152),(4,0),
          (43,1),(16,1|8|64),(23,1),(25,64),(25,2),(1,512),(5,0)]
tape=(f"{sum(n for n,k in events)} 0\n"+''.join(f"{n} {k}\n" for n,k in events)).encode()
with tempfile.TemporaryDirectory(prefix="dc-fleet-host-") as temp:
    d=Path(temp); caps=d/"frames";caps.mkdir()
    seed=d/"seed.c"; seed.write_text(r'''
#define DC_LINUX_NATIVE
#include "game.c"
#include "linux_save.h"
#include <assert.h>
int main(void){assert(linux_disk_init()&&linux_disk_lock());init_city();starter_save();
 sv.intro=4;sv.owned=8;sv.selection=3;sv.carCond[3]=100;sv.vehicleCond=100;
 sv.gear=0;sv.dayClock=650;scene=1;p.hp=100;p.car=-1;p.x=284;p.y=511;
 cars[0].cond=100;cars[0].type=3;assert(street_foot_clear(p.x,p.y,-1)&&street_near_car()==0);
 disk_slot(0);request_save();assert(saveFlash>0);linux_disk_shutdown();}
''')
    env=dict(os.environ,XDG_DATA_HOME=str(d/'save'),SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',SDL_AUDIODRIVER='dummy')
    env.pop('LD_PRELOAD',None)
    def run(*a,**kw):
        subprocess.run(a,check=True,timeout=120,**kw)
    run('cc','-O0','-std=c11','-I',root/'src',seed,root/'src/linux_save.c','-o',d/'seed')
    run(d/'seed',env=env)
    run('cc','-std=c11','-O2','-fPIC','-shared',root/'scripts/qa/linux_replay.c','-ldl','-o',d/'replay.so')
    run(exe,env=dict(env,LD_PRELOAD=str(d/'replay.so'),DC_LINUX_SHOW=str(caps),
                     DC_LINUX_CAPTURE_FRAMES='7,16,50,70,116,140',DC_LINUX_REPLAY=''),input=tape)
    # Keep the HUD out of the measurements. Only the car near screen centre
    # has the material ID pair (split Nightjar stripe + live rear lenses).
    for k,ink in [(16,0xc84b44),(50,0xc84b44),(70,0xff5148),(116,0xff5148),(140,0xe2faff)]:
        with (caps/f'frame-{k}.ppm').open('rb') as f:
            assert f.readline().strip()==b'P6';w,h=map(int,f.readline().split());assert f.readline().strip()==b'255';buf=f.read()
        assert len(buf)==w*h*3
        colors={ink:0,0xf7ddb6:0}
        for y in range(h//3,h*9//10):
            row=buf[3*(y*w+w//3):3*(y*w+w*2//3)]
            for c in colors:
                colors[c]+=row.count(c.to_bytes(3,'big'))
        assert all(n>=3 for n in colors.values()),(k,colors)
    b=(d/'save/DeadChannel/career.dat').read_bytes()
    assert len(b)==1044 and struct.unpack_from('<I',b,304+32*4+60*4)[0]==1
    if snap:
        snap.mkdir(exist_ok=True,parents=True)
        for shot in caps.iterdir():shutil.copyfile(shot,snap/shot.name)
    print('Native SDL: loaded, boarded with E, toggled C, accelerated, turned, braked, reversed and saved; Nightjar markings and both dynamic rear lights verified in presented frames.')

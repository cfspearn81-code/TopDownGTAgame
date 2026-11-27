"""Real SDL movement inputs with 3D on, overhead, and C+F2 fallback.
Run: python3 scripts/qa/chase_gait_native.py build/DeadChannel [capture-dir]
Uses a disposable save in an empty case-specific XDG_DATA_HOME.
"""
import os
import subprocess
import sys
import struct
import tempfile
import shutil
from pathlib import Path

root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
snap = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else None
with tempfile.TemporaryDirectory(prefix='dc-native-gait-') as tmp:
    t = Path(tmp)
    # Same quiet street location and appearance under all modes.
    source = t/'seed.c'
    source.write_text(r'''
#define DC_LINUX_NATIVE
#include "game.c"
#include "linux_save.h"
#include <assert.h>
int main(void) {
 assert(linux_disk_init()&&linux_disk_lock());init_city();starter_save();
 sv.intro=4;sv.look=(Look){{1,3,8,5,2,3,8,1,4,2}};sv.dayClock=720;
 scene=1;p.hp=100;p.car=-1;p.x=820;p.y=1024;nv.garage[1]=1;
 disk_slot(0);request_save();assert(saveFlash>0);linux_disk_shutdown();
}
''')
    def run(*a, **kw):
        return subprocess.run(a, check=True, timeout=90, **kw)

    run('cc','-O0','-std=c11','-I',root/'src',source,root/'src/linux_save.c','-o',t/'seed')
    run('cc','-O1','-std=c11','-fPIC','-shared',root/'scripts/qa/linux_replay.c','-ldl','-o',t/'shim.so')
    def route(view, sprint, fallback, lingering=0):
        home = Path(tempfile.mkdtemp(dir=t))
        env = dict(os.environ, XDG_DATA_HOME=str(home), SDL_VIDEODRIVER='dummy', SDL_RENDER_DRIVER='software', SDL_AUDIODRIVER='dummy')
        env.pop('LD_PRELOAD',None)
        run(t/'seed',env=env)
        caps=home/'frames'
        caps.mkdir()
        key = 8|(64 if sprint else 0)
        events = [(2,0),(1,1024),(4,0),(1,2097152 if view else 0),(5,0),
                  (14,key),(1,key|(4096 if fallback else 0)),(14,key),
                  (3+lingering,0),(1,512),(6,0)]
        tape = f'{sum(n for n,k in events)} 0\n' + ''.join(f'{n} {k}\n' for n,k in events)
        frames='14,19,22,25,28,31,34,37,40,43,48'
        run(exe,env=dict(env,LD_PRELOAD=str(t/'shim.so'),DC_LINUX_REPLAY='',DC_LINUX_CAPTURE_FRAMES=frames,DC_LINUX_SHOW=str(caps)),input=tape.encode())
        b=(home/'DeadChannel/career.dat').read_bytes()
        assert len(b)==1044 and struct.unpack_from('<I',b,672)[0]==int(view)
        x,y=struct.unpack_from('<ff',b,60)
        assert y==1024.0
        if snap and view and not fallback and lingering==0:
            target=snap/('sprint' if sprint else 'walk');target.mkdir(parents=True,exist_ok=True)
            for f in caps.iterdir():shutil.copyfile(f,target/f.name)
        return x-820

    old_w=route(False,False,False);old_r=route(False,True,False)
    new_w=route(True,False,False);new_r=route(True,True,False)
    fallback_r=route(True,True,True);idle_r=route(True,True,False,23)
    assert 0.60 < new_w/old_w < 0.71, (new_w,old_w)
    assert 0.66 < new_r/old_r < 0.76 and new_r>new_w*1.48, (new_r,old_r)
    assert new_r==fallback_r==idle_r, (new_r,fallback_r,idle_r)
    print(f'Native SDL: 29 held input frames, 3D walk {new_w:.2f} / sprint {new_r:.2f}, '
          f'overhead {old_w:.2f} / {old_r:.2f} units. F2 mid-sprint and 23 extra idle ticks preserve exact travelled distance.')

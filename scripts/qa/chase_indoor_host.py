"""Enter, walk, exit and save at Vault 39 via the actual Linux launcher.
Usage: python3 scripts/qa/chase_indoor_host.py NEW_ELF [PUBLISHED_ELF] [CAPTURES]
Optional baseline compares paired save data and exact overhead RGB frames.
"""
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
root=Path(__file__).resolve().parents[2]
exe=Path(sys.argv[1]).resolve()
previous=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None
out=Path(sys.argv[3]).resolve() if len(sys.argv)>3 else None
events=[(2,0),(1,1024),(4,0),(1,2097152),(3,0),(1,16),(4,0),(14,4),(2,0),(12,8),(3,0),(1,16),(4,0),(1,512),(3,0)]
with tempfile.TemporaryDirectory(prefix="dc-indoor-cast-") as tmp:
    t=Path(tmp);qa=t/"seed.c";qa.write_text(r'''
#define DC_LINUX_NATIVE
#include "game.c"
#include "linux_save.h"
#include <assert.h>
int main(void){
 assert(linux_disk_init()&&linux_disk_lock());init_city();starter_save();
 sv.intro=4;sv.look=(Look){{2,4,8,5,1,3,8,1,4,2}};sv.gear=-1;sv.dayClock=650;
 sv.cash=70;scene=1;p.hp=100;p.car=-1;p.x=loc[1].x;p.y=loc[1].y;nv.garage[1]=1;
 disk_slot(0);request_save();assert(saveFlash>0);linux_disk_shutdown();
}
''')
    def run(*args,**kw):
        return subprocess.run(args,timeout=150,check=True,**kw)
    run('cc','-O0','-std=c11','-I',root/'src',qa,root/'src/linux_save.c','-o',t/'seed')
    run('cc','-O1','-std=c11','-fPIC','-shared',root/'scripts/qa/linux_replay.c','-ldl','-o',t/'replay.so')
    captures = {}
    def test(branch,host,enabled):
        label=branch+('-3D' if enabled else '-overhead');dest=t/label;dest.mkdir()
        saves=dest/'saves';frames=dest/'frames';frames.mkdir()
        e=dict(os.environ,XDG_DATA_HOME=str(saves),SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',SDL_AUDIODRIVER='dummy')
        e.pop('LD_PRELOAD',None);run(t/'seed',env=e)
        p=[(n,k if k!=2097152 or enabled else 0) for n,k in events]
        tape=f'{sum(n for n,k in p)} 0\n'+''.join(f'{n} {k}\n' for n,k in p)
        marks=(10,14,23,42,47)
        run(host,env=dict(e,LD_PRELOAD=str(t/'replay.so'),DC_LINUX_REPLAY='',DC_LINUX_CAPTURE_FRAMES=','.join(map(str,marks)),DC_LINUX_SHOW=str(frames)),input=tape.encode())
        save=(saves/'DeadChannel/career.dat').read_bytes()
        assert len(save)==1044 and struct.unpack_from('<I',save,672)[0]==enabled and struct.unpack_from('<ff',save,60)==(598.0,730.0)
        rendered=[(frames/f'frame-{i}.ppm').read_bytes() for i in marks]
        assert rendered[0]!=rendered[1] and rendered[1]!=rendered[2] and rendered[2]!=rendered[-1]
        captures[label]=(save,rendered)
        if out:
            side=out/label;side.mkdir(exist_ok=True,parents=True)
            for f in frames.iterdir():shutil.copyfile(f,side/f.name)
    test('updated',exe,True);test('updated',exe,False)
    if previous:
        test('published',previous,True);test('published',previous,False)
        assert captures['updated-overhead']==captures['published-overhead']  # independent of 3-D-only visuals
        assert captures['updated-3D'][0]==captures['published-3D'][0]
        assert captures['updated-3D'][1][1]!=captures['published-3D'][1][1]
    print('Native SDL: C at Vault 39, enter E, walk both bearings, exit E and F5; both view choices preserve the same street exit and save.'+
          (' Five overhead frames and same-mode careers are byte-identical to the supplied baseline.' if previous else ''))

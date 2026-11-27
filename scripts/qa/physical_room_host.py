"""Exact native SDL replay: talk, hit, knock down, preserve service gameplay.
Usage: python3 scripts/qa/physical_room_host.py build/DeadChannel [captures]
"""
from pathlib import Path
import os, shutil, struct, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
exe=Path(sys.argv[1]).resolve()
out=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None
def run(*a,**kw):
    return subprocess.run(a,check=True,timeout=150,**kw)
def count(path,rgb,ylo=0,yhi=1):
    with path.open('rb') as f:
        assert f.readline().strip()==b'P6'
        w,h=map(int,f.readline().split());assert f.readline().strip()==b'255';rest=f.read()
    assert len(rest)==w*h*3
    data=rest[3*w*int(h*ylo):3*w*int(h*yhi)]
    needle=rgb.to_bytes(3,'big')
    return sum(data[i:i+3]==needle for i in range(0,len(data),3))
with tempfile.TemporaryDirectory(prefix='dc-room-people-') as td:
    d=Path(td);compiler=os.getenv('CC','cc')
    run(compiler,'-std=c11','-O0','-DROOM_REPLAY',root/'scripts/qa/physical_room_people.c',root/'src/linux_save.c','-o',d/'plan')
    run(compiler,'-std=c11','-O1','-fPIC','-shared',root/'scripts/qa/linux_replay.c','-ldl','-o',d/'keys.so')
    for room,pref,gun in [('club',0,0),('club',1,1),('aquarium',1,0)]:
        name=f'{room}-{pref}-{gun}';p=d/name;p.mkdir();shade=p/'captures';shade.mkdir()
        env=dict(os.environ,XDG_DATA_HOME=str(p),SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',SDL_AUDIODRIVER='dummy')
        env.pop('LD_PRELOAD',None)
        r=run(d/'plan',room,str(pref),str(gun),env=env,capture_output=True)
        rows=r.stdout.decode().splitlines();markers={row.split()[1]:int(row.split()[2])+1 for row in rows if row.startswith('#')}
        tape='\n'.join(row for row in rows if not row.startswith('#'))+'\n'
        assert {'down','talk'}<=markers.keys()
        run(exe,env=dict(env,LD_PRELOAD=str(d/'keys.so'),DC_LINUX_REPLAY='',DC_LINUX_SHOW=str(shade),
                         DC_LINUX_CAPTURE_FRAMES=','.join(map(str,markers.values()))),input=tape.encode())
        talk=shade/f'frame-{markers["talk"]}.ppm';down=shade/f'frame-{markers["down"]}.ppm'
        assert count(talk,0x2a4950,.34,.65)>600 and count(down,0x2a4950,.34,.65)<600
        if not pref:assert count(down,0x987b73,.38,.84)>7
        save=(p/'DeadChannel/career.dat').read_bytes()
        assert len(save)==1044 and [struct.unpack_from('<I',save,i)[0] for i in (8,136,672)]==[100 if room=='aquarium' else 70,0,pref]
        if out:
            q=out/name;q.mkdir(exist_ok=True,parents=True);shutil.copyfile(talk,q/'talk.ppm');shutil.copyfile(down,q/'down.ppm')
        print(f'Native SDL: {name} {tape.splitlines()[0].split()[0]} key frames, real dialogue and damage; expected wallet and view persisted.')

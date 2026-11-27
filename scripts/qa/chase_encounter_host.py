"""Use actual keyboard routes against the native SDL binary in C view.
Two isolated, seeded careers; no simulation injection of encounters or money.
Usage: python3 scripts/qa/chase_encounter_host.py build/DeadChannel [capture-dir]
"""
from pathlib import Path
import os,struct,subprocess,sys,tempfile,shutil
root=Path(__file__).resolve().parents[2];exe=Path(sys.argv[1]).resolve()
out=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None
def run(*args,**kw):
    return subprocess.run(args,check=True,timeout=150,**kw)
def patch_pixels(path,ink):
    with path.open('rb') as f:
        assert f.readline().strip()==b'P6';w,h=map(int,f.readline().split())
        assert f.readline().strip()==b'255';buf=f.read()
    assert len(buf)==3*w*h
    # Central actor foreground only, not the colored key in the HUD.
    l,r=w*39//100,w*63//100;t,b=h*18//100,h*59//100
    rr=ink>>16;g=(ink>>8)&255;bb=ink&255
    return sum(buf[j]==rr and buf[j+1]==g and buf[j+2]==bb
               for y in range(t,b) for j in range((y*w+l)*3,(y*w+r)*3,3))
with tempfile.TemporaryDirectory(prefix='dc-cast-host-') as tmp:
    d=Path(tmp)
    run('cc','-std=c11','-O1','-I',root/'src',root/'scripts/qa/npc_keyboard_plan.c','-o',d/'plan')
    run('cc','-std=c11','-O2','-shared','-fPIC',root/'scripts/qa/linux_replay.c','-ldl','-o',d/'replay.so')
    for case,seed,hp,dirty in [('surrender',0,100,36),('defeat',2,84,17)]:
        xdg=d/case;disk=xdg/'DeadChannel';disk.mkdir(parents=True);actual=d/(case+'-captures');actual.mkdir()
        a=run(d/'plan',disk/'career.dat',d/'markers',case,capture_output=True)
        lines=a.stdout.decode().splitlines();keys=[key for line in lines[1:] for n,key in [map(int,line.split())] for _ in range(n)]
        assert keys[3]==0
        # One existing neutral frame becomes C. Same number of simulation
        # frames, same movement path, same contact and actual combat result.
        keys[3]=2097152;tape=f'{len(keys)} 0\n'+''.join(f'1 {k}\n' for k in keys)
        stamps={s.split()[-1]:int(s.split()[0])+1 for s in (d/'markers').read_text().splitlines()}
        env=dict(os.environ,XDG_DATA_HOME=str(xdg),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',SDL_RENDER_DRIVER='software',
                 DC_TRIP_SEED=str(seed),DC_LINUX_CAPTURE_FRAMES=','.join(map(str,sorted(set(stamps.values())))),DC_LINUX_SHOW=str(actual),
                 DC_LINUX_REPLAY='',LD_PRELOAD=str(d/'replay.so'))
        run(exe,input=tape.encode(),env=env)
        b=(disk/'career.dat').read_bytes();assert len(b)==1044
        read=lambda i:struct.unpack_from('<I',b,i)[0]
        assert [read(i) for i in (8,12,136,304+32*4+60*4)]==[70,hp,dirty,1]
        tests=[('drawn-without-firing',0xffcf77)]+([('hands-raised',0xdaff7a)] if case=='surrender' else [('refuses',0xff7b9c),('cash-drop',0xffcf77)])
        for label,c in tests:assert patch_pixels(actual/f'frame-{stamps[label]}.ppm',c)>5,(case,label)
        if out:
            (out/case).mkdir(exist_ok=True,parents=True)
            for label,n in stamps.items():shutil.copyfile(actual/f'frame-{n}.ppm',out/case/f'{label}.ppm')
        print(f'Native SDL C {case}: {len(keys)} input frames, verified 3-D foreground, 70 clean / {dirty} street, {hp} HP.')

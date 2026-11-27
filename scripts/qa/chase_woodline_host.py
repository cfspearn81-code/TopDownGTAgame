"""Real Linux SDL input in 3D, journal, and shop. Disposable saves only.
Usage: python3 scripts/qa/chase_woodline_host.py GAME replay.so forest_fixture [frames-dir]
"""
import hashlib, os, struct, subprocess, sys, tempfile
from pathlib import Path
exe, replay, fixture = (Path(p).resolve() for p in sys.argv[1:4])
export = Path(sys.argv[4]).resolve() if len(sys.argv)>4 else None
C, ENTER, E, ESC, M, DOWN, RIGHT, UP, SAVE, F2 = 2097152,1024,16,128,256,2,8,1,512,4096

def run(where,events,check):
    with tempfile.TemporaryDirectory(prefix='ch-wood-qa-') as d:
        env=dict(os.environ,XDG_DATA_HOME=d,SDL_RENDER_DRIVER='software',SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
        env.pop('LD_PRELOAD',None)
        subprocess.run([fixture,'--seed',where],env=env,check=True,timeout=60)
        paths=Path(d)/'renders';paths.mkdir()
        spans=[(1,0)];keys={};at=1
        for event in events:
            if isinstance(event,str): keys[event]=at
            elif isinstance(event,tuple): spans.append(event);at+=event[0]
            else: spans.extend([(1,event),(3,0)]);at+=4
        spans.append((2,0));count=sum(n for n,_ in spans)
        env.update(LD_PRELOAD=str(replay),DC_LINUX_REPLAY='',DC_LINUX_SHOW=str(paths),DC_LINUX_CAPTURE_FRAMES=','.join(str(k) for k in keys.values()))
        subprocess.run([exe],env=env,input=(f'{count} 0\n'+''.join(f'{n} {k}\n' for n,k in spans)).encode(),check=True,timeout=180)
        data=(Path(d)/'DeadChannel/career.dat').read_bytes();assert len(data)==1044
        garages=struct.unpack_from('<64I',data,304+32*4);assert garages[60]==1,'C pref failed to save on foot'
        check(garages)
        hashes={}; snapshots={}
        for name,f in keys.items():
            img=(paths/f'frame-{f}.ppm').read_bytes();assert img.startswith(b'P6\n')
            hashes[name]=hashlib.sha256(img).hexdigest(); snapshots[name]=img
            if export: export.mkdir(parents=True,exist_ok=True);(export/f'{where}-{name}.ppm').write_bytes(img)
        if 'planter' in hashes:
            # Bed 1 is selected and sits to the LEFT of the modal in this rig.
            # Filling it must not paint the still-empty bed on the other side.
            _, dims, _, before = snapshots['planter'].split(b'\n', 3)
            after = snapshots['growing'].split(b'\n', 3)[3];w,h=map(int,dims.split())
            def delta(l,r):
                count=0
                for y in range(h*28//100,h*65//100):
                    i=(y*w+w*l//100)*3;j=(y*w+w*r//100)*3
                    count+=sum(a!=b for a,b in zip(before[i:j],after[i:j]))
                return count
            assert delta(0,14)>100 and delta(86,100)==0, 'visual beds disagree with menu order'
        if 'overhead' in hashes and 'perspective' in hashes: assert hashes['overhead']!=hashes['perspective']
        print(f'{where}: {count} real SDL presentations; captures: {", ".join(hashes)}.' )
def ok(cond):
    assert cond
run('-1',[ENTER,'overhead',C,'perspective',E,'journal',E,M,ESC,F2,'low',F2,'restored',SAVE],lambda g:ok(g[49]&1))
run('0',[ENTER,C,'before-forage',E,'picked',E,SAVE],lambda g:ok((g[49]&(1<<12)) and g[50]&31==1))
for shop in range(2):
    run('p'+str(shop),[ENTER,C,E,(38,RIGHT),(19,UP),ENTER,'planter',E,'growing',ESC,SAVE],
        lambda g:ok(g[51+shop]&7==1 and (g[51+shop]>>3)>850))
print('PASS: foot C, F2 roundtrip, ranger survey/goals, one-shot collectible, two real indoor planter interactions and saved growth.')

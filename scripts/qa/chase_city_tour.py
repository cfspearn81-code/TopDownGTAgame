"""Run native SDL with the same full courier/taxi tape in both cameras.
Usage: python3 scripts/qa/chase_city_tour.py path/to/DeadChannel [capture-dir].
Only synthetic isolated careers. Optional captures are exact presented SDL frames.
"""
from pathlib import Path
from itertools import groupby
import gzip, os, struct, subprocess, sys, tempfile

r = Path(__file__).resolve().parents[2]
app = Path(sys.argv[1]).resolve()
capture = Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None
raw=gzip.decompress(Path(os.environ.get("DC_OPENING_TOUR",r/"scripts/qa/curb-lamp-tour.txt.gz")).read_bytes()).splitlines()
frames=[]
for row in raw[1:]:
    n,bits=map(int,row.split());frames.extend([bits]*n)
assert len(frames)==int(raw[0].split()[0]) and len(frames) in (3664,3682)
# Also exercise the same atlas/guide menu, including a C tap that must *not*
# switch preference while modal. Actual releases avoid typematic behavior.
panels=[256,0,2097152,0,128,0,2048,0,2097152,0,128,0,2097152,0,512,0]
with tempfile.TemporaryDirectory(prefix="dc-c-world-") as work:
    tmp=Path(work); so=tmp/"sdl.so"
    subprocess.run([os.environ.get("CC","cc"),"-O2","-fPIC","-shared",
                    r/"scripts/qa/linux_replay.c","-ldl","-o",so],check=True,timeout=90)
    saves=[]
    for name,on in (("overhead",False),("perspective",True)):
        keys=list(frames)
        # Foot outside apartment; enter dispatch, work/courier route, taxi
        # transfer, drive, drop-off, exit onto pavement. Also choose Low FX
        # in motion and restore 3D; no altered direction/time inputs.
        if on:keys[44]|=2097152
        keys[1640]|=4096;keys[1652]|=4096
        keys.extend(panels if on else [0 if x==2097152 else x for x in panels])
        tape=f"{len(keys)} 0\n"+"".join(f"{sum(1 for _ in v)} {k}\n" for k,v in groupby(keys))
        save=tmp/(name+".dat");env=dict(os.environ,LD_PRELOAD=str(so),DC_TRIP_SEED="17",
            XDG_DATA_HOME=str(tmp/name),DC_LINUX_REPLAY="",SDL_VIDEODRIVER="dummy",
            SDL_AUDIODRIVER="dummy",SDL_RENDER_DRIVER="software",DC_LINUX_SNAPSHOT=str(save))
        if capture:
            dest=capture/name;dest.mkdir(exist_ok=True,parents=True)
            env.update(DC_LINUX_SHOW=str(dest),DC_LINUX_CAPTURE_FRAMES="65,182,201,1502,1617,1646,1655,3658,3661,3666,3674,3678")
        subprocess.run([app],input=tape.encode(),env=env,check=True,timeout=360)
        b=save.read_bytes()
        assert len(b)==304 and struct.unpack_from("<II",b,8)==(318,100), (name,len(b))
        assert struct.unpack_from("<I",b,36)[0]==2 and struct.unpack_from("<I",b,196)[0]==1
        saves.append(b)
    assert saves[0]==saves[1], "3D changed final career bytes"
    print(f"{len(keys)} native SDL frames in each view; two jobs, cab drop-off, $318 clean / $100 street; C, F2, atlas, guide and exit; byte-identical saved careers.")

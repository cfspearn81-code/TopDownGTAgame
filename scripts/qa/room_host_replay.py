"""Exercise C in Apartment 4, on foot, and in both Afterimage rooms using
the Linux SDL host and no game-state injection. Usage:
  python3 scripts/qa/room_host_replay.py build/DeadChannel [output-folder]
Requires the repository, SDL2 and a C compiler. No display or real save."""
from itertools import groupby
from pathlib import Path
import os, struct, subprocess, sys, tempfile

def shot(path):
    with path.open("rb") as f:
        assert f.readline().rstrip()==b"P6"
        w,h=map(int,f.readline().split());assert f.readline().strip()==b"255"
        raw=f.read();assert (w,h)==(1152,690) and len(raw)==3*w*h
    return b"".join(raw[(y*w+200)*3:(y*w+940)*3] for y in range(220,510))

def run(root, exe, tmp, out):
    shim=tmp/"replay.so"
    subprocess.run([os.getenv("CC","cc"),"-std=c11","-O2","-shared","-fPIC",
                    root/"scripts/qa/linux_replay.c","-ldl","-o",shim],check=True)
    k=[0]*426
    for n,b in ((0,1024),(3,1024),(8,2097152),(20,2097152),(26,2097152),
        (88,16),(186,16),(193,2048),(198,128),(402,16),(411,2097152),(416,2097152),(421,512)):k[n]=b
    for s,e,b in ((32,88,2),(90,185,4),(205,369,8),(369,400,1)):k[s:e]=[b]*(e-s)
    tape=f"{len(k)} 0\n"+"".join(f"{len(tuple(g))} {v}\n" for v,g in groupby(k))
    xdg=tmp/"profile";xdg.mkdir();save=tmp/"QA.dat";out.mkdir(parents=True,exist_ok=True)
    env=dict(os.environ,SDL_VIDEODRIVER="dummy",SDL_AUDIODRIVER="dummy",SDL_RENDER_DRIVER="software",
        LD_PRELOAD=str(shim),XDG_DATA_HOME=str(xdg),DC_LINUX_REPLAY="",DC_LINUX_SHOW=str(out),
        DC_LINUX_CAPTURE_FRAMES="7,14,24,30,92,190,199,405,414,420",DC_LINUX_SNAPSHOT=str(save))
    subprocess.run([exe],input=tape.encode(),env=env,check=True,timeout=170)
    get=lambda n:shot(out/f"frame-{n}.ppm")
    diff=lambda a,b:sum(abs(i-j) for i,j in zip(a,b))/len(a)
    # A re-entry shot is intentionally still settling the new room lens.
    # Check both C shots differ strongly from overhead and share their
    # perspective composition, rather than requiring instant relock.
    for loc,frames,minimum in (("Apartment",(14,24,30),25),("Velvet Lounge",(405,414,420),16)):
        a,b,c=map(get,frames)
        off=min(diff(a,b),diff(c,b))
        assert off>minimum and 3*diff(a,c)<off, f"{loc} C-toggle did not restore perspective"
    raw=save.read_bytes();assert len(raw)==1044 and struct.unpack_from("<I",raw,672)[0]==1
    h=2166136261
    for v in raw[:-4]:h=((h^v)*16777619)&0xffffffff
    assert h==struct.unpack_from("<I",raw,1040)[0]
    print(f"PASS: fresh SDL Linux career, 426 keyboard frames; reversible C at home and in Velvet Lounge, on-foot exit, entry into The Afterimage, F1 return, 10 real frames, validated Rev11 camera preference ({h:08x}).")

if __name__=="__main__":
    root=Path(__file__).resolve().parents[2]
    exe=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/"build/DeadChannel"
    with tempfile.TemporaryDirectory() as temp:
        tmp=Path(temp);out=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else tmp/"frames"
        run(root,exe,tmp,out)

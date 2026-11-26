"""Replay the installed game binaries with generated city-listings test fixtures.
Run test-city-listings.sh first for the news tapes in build/verdict."""
from pathlib import Path
from itertools import groupby
import os,struct,tempfile,subprocess as sp
r=Path(__file__).resolve().parents[2];b=r/"build";v=b/"verdict"
def run(tag,case,tape,start,cash,dirty):
 win=case/"win.dat";unix=case/"unix.dat";xdg=case/"xdg";store=xdg/"DeadChannel";store.mkdir(parents=True)
 env=dict(os.environ,DC_TRIP_LOAD=str(start),DC_TRIP_SAVE=str(win),DC_TRIP_SEED="17")
 sp.run([b/"check_pe",b/"DeadChannel.exe","--tour"],env=env,input=tape,check=True,timeout=110)
 (store/"career.dat").write_bytes(start.read_bytes())
 env.update(SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",SDL_AUDIODRIVER="dummy",DC_LINUX_REPLAY="",
  DC_LINUX_SNAPSHOT=str(unix),XDG_DATA_HOME=str(xdg),LD_PRELOAD=str(b/"linux_replay.so"))
 sp.run([b/"DeadChannel"],env=env,input=tape,check=True,timeout=110)
 data=win.read_bytes();assert len(data)==304 and data==unix.read_bytes(),tag
 assert (struct.unpack_from("<i",data,8)[0],struct.unpack_from("<i",data,136)[0])==(cash,dirty),tag
 print(f"{tag}: PE and ELF equal explicit-save bytes; clean {cash}, dirty {dirty}.",flush=True)
with tempfile.TemporaryDirectory(prefix="vanta-city-") as tmp:
 tmp=Path(tmp);sp.run(["cc","-O2","-I",r/"src",r/"scripts/qa/verdict_street_fixture.c","-o",tmp/"fixture"],check=True)
 for name in ("switch","keeper"):
  d=tmp/name;d.mkdir();start=d/"start.dat";start.write_bytes(sp.check_output([tmp/"fixture"]+(["keeper"] if name=="keeper" else [])))
  actions=[0,0];edge=lambda key: actions.extend([key,0]);edge(1024);actions += [0]*5;edge(262144)
  for i in range(11 if name=="switch" else 12):edge(2)
  edge(1024);actions+= [0]*6;edge(16)
  if name=="switch":
   edge(2);edge(1024);actions+= [65]*6+[72]*139;edge(16);edge(512)
  else:
   for key,n in [(16,220),(1024,124),(1024,220),(16,222),(1024,124),(1024,228)]:edge(key);actions+= [0]*n
   edge(512)
  actions+= [0]*70
  tape=(f"{len(actions)} 0\n"+''.join(f"{len(list(g))} {key}\n" for key,g in groupby(actions))).encode()
  run(name,d,tape,start,75 if name=="keeper" else 0,90 if name=="switch" else 0)
 for t in (4,7,8):
  for branch in (0,1):
   name=f"news-{t}-{branch}";d=tmp/name;d.mkdir();f=v/name
   run(name,d,(f/"actions.txt").read_bytes(),f/"career.dat",{4:98,7:148,8:58}[t],9)

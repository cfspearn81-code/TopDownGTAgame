"""The two later Afterimage shifts: actual native SDL and Win64 (simulated Win32).
No relocation after a staged career loads; compare the final saved career and fee.
"""
import os,struct,subprocess as sp,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
def run(*args,**kwargs):return sp.run(args,check=True,timeout=300,**kwargs)
with tempfile.TemporaryDirectory(prefix='show-shifts-') as s:
 d=Path(s);run('cc','-w','-std=c11','-O0',root/'scripts/qa/show_ride_plan.c','-o',d/'tour')
 run('cc','-O2',root/'scripts/check_pe.c','-o',d/'pe')
 run('cc','-O2','-fPIC','-shared',root/'scripts/qa/linux_replay.c','-ldl','-o',d/'shim.so')
 for idx,fee in [(1,145),(2,165)]:
  ride=d/f'ride{idx}';run(d/'tour',ride,str(idx));tape=ride.with_suffix('.plan').read_bytes();fixture=ride.with_suffix('.dat')
  base=dict(os.environ,DC_TRIP_SEED='17')
  run(d/'pe',root/'build/DeadChannel.exe','--tour',input=tape,env=dict(base,DC_TRIP_LOAD=str(fixture),DC_TRIP_SAVE=str(d/'pe.dat'),DC_RADIO_UNAVAILABLE='1'))
  store=d/f'career{idx}'/'DeadChannel';store.mkdir(parents=True);(store/'career.dat').write_bytes(fixture.read_bytes())
  run(root/'build/DeadChannel',input=tape,env=dict(base,SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',SDL_AUDIODRIVER='dummy',XDG_DATA_HOME=str(store.parent),LD_PRELOAD=str(d/'shim.so'),DC_LINUX_REPLAY='',DC_LINUX_SNAPSHOT=str(d/'sdl.dat')))
  a=(d/'pe.dat').read_bytes();assert a==(d/'sdl.dat').read_bytes()
  assert struct.unpack_from('<I',a,8)[0]==318+fee and struct.unpack_from('<I',a,196)[0]==0
  print(f'PASS both hosts: later shift {idx}, {tape.split()[0].decode()} frames, wallet +${fee} once; no Dispatch fares. ',flush=True)

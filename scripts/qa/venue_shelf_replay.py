"""Play Jules' full route with real stock in the final three loft shelves.
Usage: python3 scripts/qa/venue_shelf_replay.py [--expect-old-loss]
Uses the exact executables in build/, isolated career files and simulated Win32.
"""
from pathlib import Path
import os,struct,subprocess as sp,tempfile,sys
r=Path(__file__).resolve().parents[2]
def digest(buf):
 h=2166136261
 for c in buf:h=((h^c)*16777619)&0xffffffff
 return h
def run(*args,**kw):return sp.run(args,check=True,timeout=180,**kw)
with tempfile.TemporaryDirectory(prefix='dc-shelf-ride-') as tmp:
 d=Path(tmp);plan=d/'ride'
 run('cc','-w','-std=c11','-O0',r/'scripts/qa/show_ride_plan.c','-o',d/'plan')
 run(d/'plan',plan)
 tape=plan.with_suffix('.plan').read_bytes();header,rest=tape.split(b'\n',1);tape=header.split()[0]+b' 0\n'+rest
 buf=bytearray(1044);buf[:304]=plan.with_suffix('.dat').read_bytes()
 struct.pack_into('<I',buf,4,11);struct.pack_into('<I',buf,300,digest(buf[:300]))
 stock=11|(7<<7)|(13<<14);struct.pack_into('<I',buf,1036,stock);struct.pack_into('<I',buf,1040,digest(buf[:1040]))
 fixture=d/'staged.dat';fixture.write_bytes(buf)
 run('cc','-w','-O1',r/'scripts/check_pe.c','-o',d/'pe')
 run('cc','-w','-O1','-fPIC','-shared',r/'scripts/qa/linux_replay.c','-ldl','-o',d/'sdl.so')
 S=d/'xdg/DeadChannel';S.mkdir(parents=True);(S/'career.dat').write_bytes(buf)
 env=dict(os.environ,DC_TRIP_SEED='17')
 out=d/'pe.dat';native=d/'elf.dat'
 run(d/'pe',r/'build/DeadChannel.exe','--tour',input=tape,env=dict(env,DC_TRIP_LOAD=str(fixture),DC_TRIP_SAVE=str(out),DC_RADIO_UNAVAILABLE='1'))
 run(r/'build/DeadChannel',input=tape,env=dict(env,SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',SDL_AUDIODRIVER='dummy',XDG_DATA_HOME=str(S.parent),DC_LINUX_REPLAY='',LD_PRELOAD=str(d/'sdl.so'),DC_LINUX_SNAPSHOT=str(native)))
 result=out.read_bytes();assert result==native.read_bytes()
 # Old build erased the last meaningful NightSave word and compacted to 304 bytes.
 value=(struct.unpack_from('<I',result,1036)[0] if len(result)==1044 else 0)&0x1fffff
 after=[(value>>(7*i))&127 for i in range(3)]
 print(f'After 2,274 actual input frames: [11, 7, 13] -> {after}; clean cash $318 -> ${struct.unpack_from("<I",result,8)[0]}, {len(result)}-byte save. Both hosts agreed.')
 assert struct.unpack_from('<I',result,8)[0]==433 and value==(0 if '--expect-old-loss' in sys.argv else stock)

"""Generate only synthetic saves and keyboard input. Drive the unmodified game entry points."""
from pathlib import Path
from itertools import groupby
import os,struct,subprocess,tempfile,hashlib
r=Path(__file__).resolve().parents[2];b=r/'build'
def unpack(blob,off,fmt='i'):return struct.unpack_from('<'+fmt,blob,off)[0]
def verify(data,cash):
 h=2166136261
 for v in data[:-4]:h=((h^v)*16777619)&0xffffffff
 assert len(data)==304 and h==unpack(data,300,'I') and unpack(data,4)==10
 assert (unpack(data,8),unpack(data,136),unpack(data,288))==(cash,18,73)
 assert struct.unpack_from('<ff',data,60)==(431,1742)
with tempfile.TemporaryDirectory(prefix='vanta-radio-qa-') as tmp:
 motifs={}
 for cash,track,late,muted in [(0,1,0,False),(4,2,0,False),(5,1,0,False),(5,2,1,False),(5,2,0,True)]:
  case=Path(tmp)/f'{cash}-{track}-{late}';store=case/'xdg/DeadChannel';store.mkdir(parents=True)
  start=case/'start.dat';win=case/'pe.dat';lin=case/'elf.dat'
  keys=subprocess.check_output([b/'a1plan',str(cash),str(track),str(late),start]);actions=list(map(int,keys.split()))
  tape=(f'{len(actions)} 0\n'+''.join(f'{len(list(v))} {key}\n' for key,v in groupby(actions))).encode()
  expected=cash+95-(5 if cash==5 else 0)
  env=dict(os.environ,DC_TRIP_SEED='8043',DC_TRIP_LOAD=str(start),DC_TRIP_SAVE=str(win),DC_RADIO_CAPTURE=str(case/'pe.pcm'))
  if muted:env['DC_RADIO_UNAVAILABLE']='1'
  subprocess.run([b/'check_pe',b/'DeadChannel.exe','--tour'],input=tape,env=env,check=True,timeout=90)
  (store/'career.dat').write_bytes(start.read_bytes())
  env.update(SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',SDL_AUDIODRIVER=('unavailable-test-device' if muted else 'dummy'),DC_RADIO_CAPTURE=str(case/'elf.pcm'),
             DC_LINUX_REPLAY='',DC_LINUX_SNAPSHOT=str(lin),XDG_DATA_HOME=str(store.parent),LD_PRELOAD=str(b/'linux_replay.so'))
  subprocess.run([b/'DeadChannel'],input=tape,env=env,check=True,timeout=90)
  for file in (win,lin):verify(file.read_bytes(),expected)
  if muted:
   assert not (case/'pe.pcm').exists() and not (case/'elf.pcm').exists()
   print('No SDL audio backend / no Windows multimedia DLL: job, payout, save still work.',flush=True)
  else:
   pcm=(case/'pe.pcm').read_bytes();assert pcm==(case/'elf.pcm').read_bytes() and len(pcm)==112000
   samples=struct.unpack('<56000h',pcm);assert max(samples)>5000 and min(samples)<-5000
   digest=hashlib.sha256(pcm).hexdigest();assert motifs.setdefault(track,digest)==digest
   print(f'Ident {track}: one matching 3.5s / 16 kHz PCM submission on each host: {digest}',flush=True)
  print(f'Both entry points: {len(actions)} frames at '+('$5 rail' if cash==5 else f'${cash} on-foot')+
        f', sleeve {track}, '+('encore' if late else 'on-time')+f', ${expected} clean; wrong-fund balances untouched.',flush=True)
 assert len(set(motifs.values()))==2

"""Black-fill regression for the exact native Windows PE under Win32 stubs.
By default verify the hotfix. To observe a prior release, give its absolute PE
path followed by 'observe'. Temporary profile; no user saves or GDI hardware."""
from pathlib import Path
import os,subprocess as sp,sys,tempfile
from itertools import groupby
r=Path(__file__).resolve().parents[2]
exe=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else r/'build/DeadChannel.exe'
mode=sys.argv[2] if len(sys.argv)>2 else 'verify'
prior=Path(sys.argv[3]).resolve() if len(sys.argv)>3 else None
assert mode in ('verify','observe')
with tempfile.TemporaryDirectory(prefix="dc-win-present-") as d:
 d=Path(d)
 sp.run(['cc','-O2',r/'scripts/check_pe.c','-o',d/'pe'],check=True)
 sp.run(['cc','-O2','-I',r/'src',r/'scripts/qa/wheel_buy_plan.c','-o',d/'fixture'],check=True)
 sp.run([d/'fixture',d/'start.dat'],stdout=sp.DEVNULL,check=True)
 keys=[0]*75;keys[1]=1024;keys[10:17]=[524288]*7;keys[68]=512
 tape=(f'{len(keys)} 0\n'+''.join(f'{len(list(g))} {k}\n' for k,g in groupby(keys))).encode()
 env=dict(os.environ,DC_PRESENT=mode,DC_TRIP_LOAD=str(d/'start.dat'),DC_TRIP_SAVE=str(d/'end.dat'))
 sp.run([d/'pe',exe,'--tour'],env=env,input=tape,check=True,timeout=60)

 if prior:
  baseline=d/'prior.dat'
  older=dict(env,DC_PRESENT='observe',DC_TRIP_SAVE=str(baseline))
  sp.run([d/'pe',prior,'--tour'],env=older,input=tape,check=True,timeout=60)
  assert baseline.read_bytes()==(d/'end.dat').read_bytes()
  print("New build and prior build wrote byte-identical careers after the same input trace.")

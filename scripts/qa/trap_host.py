"""Run the real Linux ELF through Signal, Bell Lane, construction and two relaunches."""
import os, struct, subprocess, sys, tempfile
from pathlib import Path

exe, preload, seed = (Path(p).resolve() for p in sys.argv[1:4])
# Optional exact Windows binary and simulated-Win32 host. A new process per
# leg reads the identical previous synthetic career on each platform.
win = [Path(p).resolve() for p in sys.argv[4:]]
assert len(win) in (0,2)
UP, DOWN, LEFT, RIGHT, E, SHIFT, ESC, F5, ENTER, PHONE = 1, 2, 4, 8, 16, 64, 128, 512, 1024, 524288
OFF = 304 + (32 + 64) * 4
def keys(*what):
    out=[(1,0)]
    for k in what:
        if isinstance(k,tuple):
            out.append(k)
        else:
            out.extend(((1,k),(1,0)))
    return (f'{sum(n for n,_ in out)} 0\n' + ''.join(f'{n} {k}\n' for n,k in out)).encode()
def save(path):
    b=path.read_bytes()
    assert len(b)==1044 and struct.unpack_from('<I',b,4)[0]==11
    h=2166136261
    for c in b[:-4]:
        h=((h ^ c)*16777619)&0xffffffff
    assert h == struct.unpack_from('<I',b,len(b)-4)[0], 'bad native save hash'
    return struct.unpack_from('<i',b,8)[0],struct.unpack_from('<64I',b,OFF)
with tempfile.TemporaryDirectory(prefix='dc-trap-native-') as td:
    env=dict(os.environ,XDG_DATA_HOME=td,SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',
             SDL_AUDIODRIVER='dummy',LD_PRELOAD=str(preload),DC_LINUX_REPLAY='',DC_TRIP_SEED='17')
    subprocess.run([seed,'--seed'],env={k:v for k,v in env.items() if k!='LD_PRELOAD'},check=True,timeout=20)
    where=Path(td)/'DeadChannel'/'career.dat'
    def replay(*masks):
        tape=keys(*masks)
        if win:
            wsave=Path(td)/'win-career.dat'
            subprocess.run([win[1],win[0],'--tour'],env=dict(os.environ,DC_TRIP_SEED='17',DC_TRIP_LOAD=str(where),DC_TRIP_SAVE=str(wsave),DC_RADIO_UNAVAILABLE='1'),input=tape,check=True,timeout=110)
        snap=Path(td)/'sdl-before-quit.dat'
        leg_env=dict(env,DC_LINUX_SNAPSHOT=str(snap)) if win else env
        subprocess.run([exe],env=leg_env,input=tape,check=True,timeout=110)
        # The ELF additionally saves on SDL_QUIT; compare the same last F5 frame
        # as --tour, which deliberately stops before delivering a Win32 close.
        if win: assert snap.read_bytes()==wsave.read_bytes(), 'Linux/Windows Bell Lane career mismatch on this leg'
    # Signal's default Inbox -> Right twice to Night Wire -> Up wraps to Trap House.
    replay(ENTER,PHONE,RIGHT,RIGHT,UP,ENTER,PHONE,ENTER,ENTER,ENTER,
           ENTER,ENTER,DOWN,ENTER,ENTER,DOWN,ENTER,ENTER,F5)
    cash, slots=save(where)
    assert cash == 1680 and slots[58] == 43 and slots[33] == 6, (cash,slots[58],slots[33])
    # Relaunch, walk to graded locker, store one exact Echo Bloom / Top Shelf, then book a rush job.
    replay(ENTER,ENTER,(61,SHIFT|UP|LEFT),(42,SHIFT|LEFT),E,UP,E,ESC,
           (167,SHIFT|RIGHT),E,DOWN,ENTER,ENTER,F5)
    cash, slots=save(where)
    assert cash == 1650 and slots[58] == 43 and slots[33] == 5
    assert (slots[60]>>10)&31 == 1 and slots[62]==2 and 2200<slots[61]<=2280, (slots[60:63])
    remain=slots[61]
    # Restart again and let the saved remainder elapse in the room, then claim the same booking once.
    replay(ENTER,ENTER,(61,SHIFT|UP|RIGHT),(40,SHIFT|RIGHT),E,
           (remain+5,0),DOWN,DOWN,ENTER,F5)
    cash, slots=save(where)
    assert cash == 1746 and slots[58] == 43 and slots[61]==0 and slots[62]==4 and slots[33]==5, (cash,slots[58:63])
    print('PASS native Linux SDL: signed deed, all three upgrades, grade-preserving shelf, persisted paid dispatch, relaunch and one $96 payout.')

    if win: print("PASS: reproducible Bell Lane saves matched the native Linux and simulated-Win32 launchers on all three full relaunches.")

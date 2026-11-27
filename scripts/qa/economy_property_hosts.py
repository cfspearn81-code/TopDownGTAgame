"""Replay full, generated-on-foot business routes against both native clients.
General input planner writes only disposable fixture/tape. No installed game or
real save is touched. The PE runner simulates Win32; Linux processes SDL events.
Usage: python3 economy_property_hosts.py planner pehost game.exe game-linux preload.so
An optional sixth argument is an older PE that must yield the same outcomes.
"""
from pathlib import Path
import os, tempfile, subprocess, time, struct, sys
planner, host, pe, elf, preload = [Path(x).resolve() for x in sys.argv[1:6]]
clients = [('PE', pe), ('ELF', elf)]
if len(sys.argv) > 6:
    clients.insert(0, ('prior PE', Path(sys.argv[6]).resolve()))
def fnv(b):
    h = 2166136261
    for v in b:
        h = ((h ^ v) * 16777619) & 0xffffffff
    return h
def word(b, offset):
    return struct.unpack_from('<I', b, offset)[0]
with tempfile.TemporaryDirectory(prefix='dc-property-routes-') as td:
    base = Path(td)
    for scenario, expected in [('grow', (310, 33, 400, 5)), ('legal', (364, 0, 400, 0)),
                               ('ferry', (451, 0, 400, 0)), ('wrecker', (584, 0, 400, 0)),
                               ('cinder', (260, 304, 400, 0))]:
        src = base / (scenario + '.seed')
        plan = subprocess.run([planner, scenario, src], check=True, stdout=subprocess.PIPE, timeout=120)
        tape = plan.stdout
        fixture = src.read_bytes()
        prior = None
        for label, app in clients:
            private = base / (scenario + '-' + label)
            private.mkdir()
            snapshot = private / 'snapshot'
            env = dict(os.environ, DC_TRIP_SEED='17')
            if label=='ELF':
                careers = private / 'xdg' / 'DeadChannel'
                careers.mkdir(parents=True)
                (careers / 'career.dat').write_bytes(fixture)
                env.update(SDL_AUDIODRIVER='dummy', SDL_VIDEODRIVER='dummy', SDL_RENDER_DRIVER='software',
                           XDG_DATA_HOME=str(careers.parent), DC_LINUX_REPLAY='', LD_PRELOAD=str(preload),
                           DC_LINUX_SNAPSHOT=str(snapshot))
                cmd = [app]
            else:
                cmd = [host, app, '--tour']
                env.update(DC_TRIP_LOAD=str(src), DC_TRIP_SAVE=str(snapshot))
            tick = time.monotonic()
            subprocess.run(cmd, env=env, input=tape, check=True, stdout=subprocess.DEVNULL, timeout=300)
            b = snapshot.read_bytes()
            state = tuple(word(b, offset) for offset in (8, 136, 288, 124))
            assert word(b,len(b)-4)==fnv(b[:-4]) and src.read_bytes()==fixture
            assert state == expected, (scenario,label,state,expected)
            assert prior is None or prior==b, (scenario,label,'cross-client state mismatch')
            prior = b
            print(scenario, label, tape.split()[0].decode(), 'real input frames',
                  'cash/street/bank/loft stock', state, 'digest',hex(word(b,len(b)-4)),
                  'wall seconds',round(time.monotonic()-tick,1),flush=True)

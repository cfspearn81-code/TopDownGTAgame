"""Studio-free SDL room tour: compare both cameras through the real launcher.

  python3 scripts/qa/chase_room_motion_host.py build/DeadChannel [captures]
  Runs synthetic saves only. It checks 3D at home and in a club side room,
  immediate native F2 fallback, and equivalent outcomes with or without C.
"""
import itertools
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
APP = Path(sys.argv[1]).resolve()
OUTPUT = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else None
C, F2 = 2097152, 4096
lines = (HERE / 'all_area_rooms_inputs.txt').read_text().splitlines()
keys = []
for line in lines[1:]:
    count, mask = map(int, line.split())
    keys.extend([mask] * count)
assert len(keys) == 426 and keys[13] == keys[16] == 0
keys[13] = keys[16] = F2
keys.extend([C, 0, 512, 0])  # Return preference to overhead; then explicitly save both.

def tape(perspective):
    bits = keys if perspective else [mask & ~C for mask in keys]
    return (f'{len(bits)} 0\n' + ''.join(
        f'{len(list(run))} {mask}\n' for mask, run in itertools.groupby(bits))).encode()

def patch(path, area=(95, 128, 400, 410)):
    magic, size, depth, pixels = path.read_bytes().split(b'\n', 3)
    w, h = map(int, size.split())
    assert magic == b'P6' and depth == b'255' and len(pixels) == 3*w*h
    x, y, r, b = area
    return b''.join(pixels[(q*w+x)*3:(q*w+r)*3] for q in range(y,b))

with tempfile.TemporaryDirectory(prefix='dc-room-motion-') as tmp:
    home = Path(tmp)
    sdl = home / 'input.so'
    subprocess.run(['cc', '-O2', '-shared', '-fPIC', HERE/'linux_replay.c',
                    '-ldl', '-o', sdl], check=True)
    savefiles, frames = [], []
    for name, perspective in [('overhead', False), ('3d', True)]:
        save = home/f'{name}.dat'
        caps = (OUTPUT or home)/name
        caps.mkdir(parents=True, exist_ok=True)
        frames.append(caps)
        env = dict(os.environ, LD_PRELOAD=str(sdl), XDG_DATA_HOME=str(home/f'data-{name}'),
                   SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                   SDL_RENDER_DRIVER='software', DC_TRIP_SEED='17', DC_LINUX_REPLAY='',
                   DC_LINUX_SNAPSHOT=str(save), DC_LINUX_SHOW=str(caps),
                   DC_LINUX_CAPTURE_FRAMES='11,15,19,31,45,92,199,385,405,415,421')
        subprocess.run([APP], env=env, input=tape(perspective), check=True, timeout=90)
        b = save.read_bytes()
        assert len(b) == 304 and struct.unpack_from('<II', b) == (0x44433131, 10)
        savefiles.append(b)
    # Compare actual room art only, not the C-mode notice or opt-in UI label.
    oh,perspective = frames
    for n in (11,19,31,45,199,405,421):
        assert patch(oh/f'frame-{n}.ppm') != patch(perspective/f'frame-{n}.ppm'), n
    for n in (15,415):
        assert patch(oh/f'frame-{n}.ppm') == patch(perspective/f'frame-{n}.ppm'), n
    assert savefiles[0] == savefiles[1], 'differences in the saved career after equal inputs'
    print('430 native SDL frames per view: apartment, immediate F2 fallback and restoration, street, Afterimage guide and back rooms, C off/on; identical career bytes.')

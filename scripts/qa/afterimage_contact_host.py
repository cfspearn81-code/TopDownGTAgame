#!/usr/bin/env python3
"""Native SDL route for the merged Afterimage (no real careers or GUI needed).
Usage: python3 scripts/qa/afterimage_contact_host.py build/DeadChannel [captures]
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
OUT = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else None
UP, DOWN, LEFT, RIGHT, USE, ESC, SAVE, ENTER, HELP, F2, C = (
    1, 2, 4, 8, 16, 128, 512, 1024, 2048, 4096, 2097152
)
SHOTS = dict(stage=173, stage_beat=177, guide=181, guide_hold=192,
             paused=200, pause_hold=218, low_fx=222, restored=228, lounge=545)


def tape(perspective):
    keys = [0] * 556
    # Create a fresh profile, exit Apartment Four, walk west to the sign,
    # enter directly with E. The two cameras diverge only inside the club.
    keys[0] = keys[3] = ENTER
    keys[8:64] = [DOWN] * 56
    keys[64] = USE
    keys[67:162] = [LEFT] * 95
    keys[163] = USE
    if perspective:
        keys[165] = C
    keys[179], keys[194] = HELP, ESC
    keys[198], keys[219] = ESC, ESC
    keys[220], keys[225] = F2, F2
    # C intentionally reduces walking pace. Both paths reach the same
    # southeast door and use it on the same tick; overhead simply waits longer.
    right, up = (252, 49) if perspective else (164, 31)
    keys[235:235+right] = [RIGHT] * right
    keys[488:488+up] = [UP] * up
    keys[539] = USE
    if perspective:
        keys[547] = C
    keys[552] = SAVE
    return (f'{len(keys)} 0\n' + ''.join(
        f'{len(list(run))} {mask}\n' for mask, run in itertools.groupby(keys))).encode()


def load(path):
    magic, size, depth, raw = path.read_bytes().split(b'\n', 3)
    w, h = map(int, size.split())
    assert magic == b'P6' and depth == b'255' and len(raw) == 3*w*h
    return w, h, raw


def sample(img, x, y):
    w, _, raw = img
    k = 3*(w*y+x)
    return int.from_bytes(raw[k:k+3], 'big')


def title_has(img, ink):
    # Room title at top right: public floor is rose, VIP lounge is gold.
    # Crop out the clock and the gold left-side venue masthead.
    w, h, _ = img
    expect = ink
    return any(sample(img, x, y) == expect
               for y in range(int(.044*h), int(.081*h))
               for x in range(int(.69*w), int(.95*w)))


def stage_pixels(img):
    # Restrict comparison to the room, not notices or title/radio bands.
    w, h, raw = img
    a, b = int(.33*w), int(.66*w)
    return b''.join(raw[3*(w*y+a):3*(w*y+b)] for y in range(int(.22*h), int(.62*h)))


with tempfile.TemporaryDirectory(prefix='dc-afterimage-contact-') as t:
    home = Path(t)
    shim = home / 'inputs.so'
    subprocess.run([os.getenv('CC', 'cc'), '-O2', '-shared', '-fPIC',
                    HERE/'linux_replay.c', '-ldl', '-o', shim], check=True)
    saves, frames = [], []
    for name, perspective in [('overhead', False), ('3d', True)]:
        take = (OUT or home)/name
        take.mkdir(parents=True, exist_ok=True)
        save = home/f'{name}.dat'
        env = dict(os.environ, LD_PRELOAD=str(shim), XDG_DATA_HOME=str(home/f'data-{name}'),
                   SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', SDL_RENDER_DRIVER='software',
                   DC_TRIP_SEED='17', DC_LINUX_REPLAY='', DC_LINUX_SNAPSHOT=str(save),
                   DC_LINUX_SHOW=str(take), DC_LINUX_CAPTURE_FRAMES=','.join(map(str, SHOTS.values())))
        subprocess.run([APP], env=env, input=tape(perspective), check=True, timeout=100)
        pic = {label: load(take/f'frame-{n}.ppm') for label, n in SHOTS.items()}
        for stamp in ('stage', 'restored', 'lounge'):
            assert sample(pic[stamp], 10, 35) == 0x141420, (name, stamp, 'not The Afterimage')
        assert pic['stage'] != pic['stage_beat'], (name, 'show did not animate')
        assert pic['guide'] == pic['guide_hold'], (name, 'field-guide backdrop moved')
        assert pic['paused'] == pic['pause_hold'], (name, 'paused room moved')
        assert title_has(pic['stage'], 0xf691b4) and not title_has(pic['stage'], 0xf4cd86), (name, 'stage title')
        assert title_has(pic['lounge'], 0xf4cd86) and not title_has(pic['lounge'], 0xf691b4), (name, 'VIP title')
        assert stage_pixels(pic['stage']) != stage_pixels(pic['lounge']), (name, 'VIP door')
        frames.append(pic)
        b = save.read_bytes()
        assert len(b) in (304, 1044) and struct.unpack_from('<I', b)[0] == 0x44433131
        saves.append(b)
    oh, c = frames
    assert stage_pixels(oh['stage']) != stage_pixels(c['stage']), 'C did not project the stage'
    assert stage_pixels(oh['low_fx']) == stage_pixels(c['low_fx']), 'F2 did not restore overhead'
    assert stage_pixels(oh['restored']) != stage_pixels(c['restored']), 'F2 lost the C preference'
    assert stage_pixels(oh['lounge']) != stage_pixels(c['lounge']), 'VIP did not use C perspective'
    assert saves[0] == saves[1], 'equal club footpaths and actions changed the saved career'
    print('1112 native SDL input frames: fresh profile to working direct club door, animated stage, '
          'frozen F1 and Pause, F2 parity, C VIP lounge and identical career bytes.')

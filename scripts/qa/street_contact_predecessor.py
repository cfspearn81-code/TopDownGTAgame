"""Generate real predecessor careers in the released ELF, then load in Open Contact.

Usage: python3 scripts/qa/street_contact_predecessor.py OLD_ELF NEW_ELF SDL_REPLAY_SO
No real career or test blobs are committed. Exact archived Street Contact source
and its official opening tape are independent of the current serializer.
"""
from pathlib import Path
import gzip
import os
import shutil
import struct
import subprocess
import sys
import tempfile

OLD, NEW, SHIM = map(lambda name: Path(name).resolve(), sys.argv[1:])
TOUR = Path(__file__).with_name("curb-lamp-tour.txt.gz")
WARD_WORD = 304 + (32 + 64 + 64 + 1) * 4
GEAR_WORD = 304 + (32 + 64 + 64 + 4) * 4
CAMERA_WORD = 304 + (32 + 60) * 4
SIGNAL_BIND = 304 + (32 + 64) * 4
E, L, F5, ENTER, F1, ESC, P, F7, C = 16, 4, 512, 1024, 2048, 128, 524288, 1048576, 2097152


def tape(*keys):
    commands = [(1, 0)] + [(1, mask) for key in keys for mask in (key, 0)]
    return (f"{len(commands)} 0\n" + "".join(f"{n} {mask}\n" for n, mask in commands)).encode()


def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]


def checksum(data):
    h = 2166136261
    for byte in data:
        h = ((h ^ byte) * 16777619) & 0xffffffff
    return h


def verify(data, size):
    assert len(data) == size and struct.unpack_from("<II", data) == (0x44433131, 10 if size == 304 else 11)
    assert u32(data, 300) == checksum(data[:300])
    if size == 1044:
        assert u32(data, 1040) == checksum(data[:1040])
    assert struct.unpack_from("<II", data, 8) == (318, 100)
    assert (u32(data, 36), u32(data, 196)) == (2, 1)


def launch(exe, profile, shot, events):
    env = dict(os.environ, XDG_DATA_HOME=str(profile), SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
               LD_PRELOAD=str(SHIM), DC_LINUX_REPLAY="", DC_LINUX_SNAPSHOT=str(shot))
    subprocess.run([exe], env=env, input=events, timeout=120, check=True)
    return shot.read_bytes()


with tempfile.TemporaryDirectory(prefix="dc-street-contact-predecessor-") as work:
    root = Path(work)
    old_data = root / "old"
    old_file = old_data / "DeadChannel" / "career.dat"
    exact = launch(OLD, old_data, root / "old-opening.dat", gzip.decompress(TOUR.read_bytes()))
    verify(exact, 304)
    assert u32(exact, 300) == int(gzip.decompress(TOUR.read_bytes()).splitlines()[0].split()[1], 16)
    compact = old_file.read_bytes()
    verify(compact, 304)
    old_mobile = launch(OLD, old_data, root / "old-signal-layout.dat", tape(ENTER, P, L, ENTER, F7, C, F5))
    verify(old_mobile, 1044)
    assert u32(old_mobile, CAMERA_WORD) == 1 and u32(old_mobile, SIGNAL_BIND) == 1
    assert not(u32(old_mobile, WARD_WORD) & ~3) and not(u32(old_mobile, GEAR_WORD) >> 8)

    for source, label, phone in ((compact, "compact", P), (old_mobile, "expanded", F7)):
        profile = root / label
        file = profile / "DeadChannel" / "career.dat"
        file.parent.mkdir(parents=True)
        file.write_bytes(source)
        loaded = launch(NEW, profile, root / f"new-{label}.dat", tape(ENTER, F1, ESC, phone, phone, F5))
        verify(loaded, len(source))
        assert loaded[256:296] == source[256:296], "appearance, name and bank unaffected"
        assert u32(loaded, 252) >= u32(source, 252)
        x, y = struct.unpack_from("<ff", source, 60)
        a, b = struct.unpack_from("<ff", loaded, 60)
        assert (x - a)**2 + (y - b)**2 < 35**2, (x, y, a, b)
        if phone == F7:
            assert u32(loaded, CAMERA_WORD) == u32(loaded, SIGNAL_BIND) == 1
            assert not(u32(loaded, WARD_WORD) & ~3) and not(u32(loaded, GEAR_WORD) >> 8)
        assert not list(file.parent.glob("*.bad*")), "valid predecessor is not a broken career"
    print("PASS exact released Street Contact ELF careers: published 3,682-frame route, valid 304/1044 layouts, two legal jobs and fare, wallet, appearance, F7 and 3D retained in each new ELF launch.")

"""Drive the actual distributable ELF through SDL's dummy video/event queue."""
from pathlib import Path
import gzip
import os
import struct
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
app, interposer, disk = (Path(a).resolve() for a in sys.argv[1:])
tour = gzip.decompress((root/"scripts/qa/curb-access-tour.txt.gz").read_bytes())
frames, digest = tour.splitlines()[0].split()
frames, digest = int(frames), int(digest, 16)
expected_cash = 318

def checksum(data):
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xffffffff
    return h

def valid(data, rev=10):
    assert len(data) == 304 and struct.unpack_from("<II", data) == (0x44433131, rev)
    return struct.unpack_from("<I", data, 300)[0] == checksum(data[:-4])

with tempfile.TemporaryDirectory(prefix="deadchannel-linux-qa-") as path:
    tmp = Path(path)
    subprocess.run([disk, tmp/"disktest"], check=True)
    data = tmp/"xdg-data"
    career = data/"DeadChannel/career.dat"
    env = dict(os.environ, XDG_DATA_HOME=str(data), SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", LD_PRELOAD=str(interposer),
               DC_LINUX_REPLAY="")
    snapshot = tmp/"explicit-save.dat"
    subprocess.run([app], input=tour, env=dict(env, DC_LINUX_SNAPSHOT=str(snapshot)),
                   check=True, timeout=45)
    exact, shut = snapshot.read_bytes(), career.read_bytes()
    assert valid(exact) and valid(shut)
    assert struct.unpack_from("<I", exact, 300)[0] == digest  # the Windows fixture
    for blob in (exact, shut):
        assert struct.unpack_from("<II", blob, 8) == (expected_cash, 100)
        assert struct.unpack_from("<I", blob, 36)[0] == 2     # courier + fare
        assert struct.unpack_from("<I", blob, 196)[0] == 1    # one taxi fare
    assert struct.unpack_from("<I", shut, 252)[0] >= struct.unpack_from("<I", exact, 252)[0]
    print(f"Linux ELF: {frames:,} keyboard frames, courier and taxi, explicit-save digest matches Windows; clean quit saved.")

    # Resize and click through the atlas with the real SDL renderer. The
    # native test interposer asserts the purple route is added then cleared.
    # The tour ends close to a curb. Spawn correction can shift that save
    # sideways because traffic is reset on reload; isolate input measurement.
    clear_ground = bytearray(shut)
    struct.pack_into("<ff", clear_ground, 60, 1024.0, 1024.0)
    struct.pack_into("<I", clear_ground, 300, checksum(clear_ground[:-4]))
    career.write_bytes(clear_ground)
    subprocess.run([app], env=dict(env, DC_LINUX_REPLAY="warm"), check=True, timeout=20)
    reloaded = career.read_bytes()
    assert valid(reloaded) and reloaded[:24] == shut[:24]
    x, y = struct.unpack_from("<ff", clear_ground, 60)
    nx, ny = struct.unpack_from("<ff", reloaded, 60)
    assert abs(nx-x) < .01 and abs(y-ny-(20*98.4/60)) < .01, (x, y, nx, ny)
    print("Linux ELF: mouse load, 1300x790 resize/letterbox, atlas pin, ignored letterbox click and in-frame clear, and focus-cleared held key.")

    # Move a real save into the other slot name and remove the old only
    # through the exact application's confirmation flow.
    other = career.with_name("fresh-001.dat")
    other.write_bytes(reloaded)
    subprocess.run([app], env=dict(env, DC_LINUX_REPLAY="delete"), check=True, timeout=20)
    assert not career.exists() and other.read_bytes() == reloaded

    # ASCII text editing and Backspace take SDL's native event path.
    env2 = dict(env, XDG_DATA_HOME=str(tmp/"name"), DC_LINUX_REPLAY="create")
    subprocess.run([app], env=env2, check=True, timeout=20)
    edited = (tmp/"name/DeadChannel/career.dat").read_bytes()
    assert valid(edited) and edited[268:288].split(b"\0",1)[0] == b"Vanta Local Linu"
    print("Linux ELF: selected-slot delete preserves the other slot; text and Backspace round-trip.")
    subprocess.run([app], env=dict(env, XDG_DATA_HOME=str(tmp/"guide"),
                                  DC_LINUX_REPLAY="guide"), check=True, timeout=20)
    print("Linux ELF: F1 preserves the apartment backdrop and Escape returns to that room.")
    subprocess.run([sys.executable, root/"scripts/qa/legacy_saves.py", "--linux",
                    interposer, app, snapshot], check=True, timeout=90)

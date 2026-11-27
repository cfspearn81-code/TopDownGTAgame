"""Run the release ELF through real SDL keyboard events and legacy/migrated disks."""
from pathlib import Path
import os
import struct
import subprocess
import sys
import tempfile

app, replay, fixture = (Path(arg).resolve() for arg in sys.argv[1:4])
shots = Path(sys.argv[4]).resolve() if len(sys.argv) > 4 else None
if shots:
    shots.mkdir(parents=True, exist_ok=True)

UP, DOWN, LEFT, RIGHT, ESC, MAP, SAVE, ENTER = 1, 2, 4, 8, 128, 256, 512, 1024
CAMERA, TAB = 2097152, 16777216
MAGIC = 0x44433131
STYLE = {304: 0, 1044: 948}  # Save11.night.world[1]


def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]


def checksum(data):
    state = 2166136261
    for byte in data:
        state = ((state ^ byte)*16777619) & 0xffffffff
    return state


def style(data):
    return (u32(data, STYLE[len(data)]) >> 2) & 0x01ffffff if len(data) == 1044 else 0


def check_length(data, length):
    assert len(data) == length and u32(data, 0) == MAGIC
    assert u32(data, length-4) == checksum(data[:-4])
    assert u32(data, 4) == (11 if length == 1044 else 10)
    if length == 1044:
        assert u32(data, 300) == checksum(data[:300])


def run(root, actions, group):
    # Consecutive repeated keys take a real intervening coast/key-up frame.
    keys = [0]*3 + [k for action in actions for k in (action, 0, 0)] + [0]*3
    env = dict(os.environ, XDG_DATA_HOME=str(root), SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", LD_PRELOAD=str(replay), DC_LINUX_REPLAY="")
    if shots:
        capture = shots/group
        capture.mkdir(exist_ok=True)
        env.update(DC_LINUX_SHOW=str(capture), DC_LINUX_CAPTURE_FRAMES=",".join(
            str(n) for n in range(6, len(keys)+1, 3)))
    tape = f"{len(keys)} 0\n" + "".join(f"1 {k}\n" for k in keys)
    subprocess.run([app], env=env, input=tape.encode(), check=True, timeout=30)


with tempfile.TemporaryDirectory(prefix="deadchannel-wardrobe-") as folder:
    tmp = Path(folder)
    legacy = tmp/"legacy"/"DeadChannel"/"career.dat"
    legacy.parent.mkdir(parents=True)
    subprocess.run([fixture, "--fixture", legacy], check=True)
    before = legacy.read_bytes()
    check_length(before, 304)
    assert style(before) == 0
    run(tmp/"legacy", (ENTER, ESC, UP, ENTER, TAB, RIGHT, ESC), "01-cancel")
    canceled = legacy.read_bytes()
    check_length(canceled, 304)
    assert canceled[256:288] == before[256:288]
    run(tmp/"legacy", (ENTER, ESC, UP, ENTER, TAB, RIGHT, RIGHT, RIGHT, RIGHT,
                        DOWN, RIGHT, RIGHT, DOWN, RIGHT, MAP, ENTER, CAMERA), "02-restyle")
    migrated = legacy.read_bytes()
    check_length(migrated, 1044)
    assert style(migrated) == 4|(2<<4)|(1<<8), style(migrated)
    assert migrated[8:60] == before[8:60] and migrated[256:300] == before[256:300]
    # Re-launch the same career; 4 -> 3 proves saved editor reconstruction.
    run(tmp/"legacy", (ENTER, ESC, UP, ENTER, TAB, LEFT, ENTER), "03-reload")
    restored = legacy.read_bytes()
    check_length(restored, 1044)
    assert style(restored) == 3|(2<<4)|(1<<8)
    assert restored[8:60] == before[8:60] and restored[256:300] == before[256:300]
    fresh = tmp/"fresh"/"DeadChannel"/"career.dat"
    run(tmp/"fresh", (ENTER, TAB, RIGHT, RIGHT, DOWN, RIGHT, MAP, ENTER), "04-new")
    created = fresh.read_bytes()
    check_length(created, 1044)
    assert style(created) == 2|(1<<4)
    check_length((tmp/"legacy"/"DeadChannel"/"career.dat").read_bytes(), 1044)
print("Linux ELF: SDL keyboard reaches New Career and existing Wardrobe. Cancel leaves legacy style/304 format; apply + relaunch preserve base profile and full outfit in checksummed 1044 bytes.")

"""Boot synthetic career revisions through the actual PE entry point (Linux Win32 shim).
No real user data. These byte lengths are a compatibility contract, not sizeof(Save).
"""
from pathlib import Path
import os
import struct
import subprocess
import sys
import tempfile

linux = len(sys.argv) > 1 and sys.argv[1] == "--linux"
host, game, seed_path = (Path(x).resolve() for x in sys.argv[2 if linux else 1:])
seed = seed_path.read_bytes()
assert len(seed) == 304 and struct.unpack_from("<II", seed) == (0x44433131, 10)

def checksum(data):
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xffffffff
    return h

with tempfile.TemporaryDirectory() as tmp:
    src, dst = Path(tmp) / "test-input.dat", Path(tmp) / "test-output.dat"
    def round_trip(blob):
        struct.pack_into("<I", blob, len(blob)-4, checksum(blob[:-4]))
        src.write_bytes(blob)
        dst.unlink(missing_ok=True)
        if linux:
            xdg = Path(tmp) / "xdg-data"
            linux_file = xdg / "DeadChannel/career.dat"
            linux_file.parent.mkdir(parents=True, exist_ok=True)
            linux_file.write_bytes(blob)
            env = dict(os.environ, XDG_DATA_HOME=str(xdg), SDL_VIDEODRIVER="dummy",
                       SDL_RENDER_DRIVER="software", LD_PRELOAD=str(host),
                       DC_LINUX_REPLAY="resave")
            subprocess.run([game], env=env, check=True, stdout=subprocess.DEVNULL, timeout=20)
            data = linux_file.read_bytes()
        else:
            subprocess.run([host, game, src, dst], check=True, stdout=subprocess.DEVNULL)
            data = dst.read_bytes()
        assert len(data) == 304 and struct.unpack_from("<II", data) == (0x44433131, 10)
        assert struct.unpack_from("<I", data, 300)[0] == checksum(data[:-4])
        assert src.read_bytes() == blob
        return data

    for rev, size in [(2,176), (3,180), (4,200), (5,204), (6,256), (7,292), (8,300), (9,304), (10,304)]:
        old = bytearray(seed[:size])
        struct.pack_into("<II", old, 4, rev, 3700 + rev)
        if rev < 8:
            # An old, unconfirmed audit must not confiscate dirty money.
            struct.pack_into("<ii", old, 136, 780, 32)
            struct.pack_into("<i", old, 144, 1)
            struct.pack_into("<f", old, 168, .02)
        else:
            struct.pack_into("<ii", old, 288, 7200, 497)
        data = round_trip(old)
        assert struct.unpack_from("<I", data, 8)[0] == 3700 + rev
        bank, dirty_spent = struct.unpack_from("<ii", data, 288)
        assert (bank, dirty_spent) == ((0, 0) if rev < 8 else (7200, 497))
        if rev < 8:
            assert struct.unpack_from("<iiii", data, 136) == (780, 32, 0, 0)
            assert struct.unpack_from("<f", data, 168)[0] == 0
        assert struct.unpack_from("<i", data, 296)[0] == 0  # no confirmed case
        if rev < 7:
            assert data[266] == 1  # legacy profile retained
        else:
            assert data[256:288] == seed[256:288]  # look, flags and name
    # A grandfathered case starts at five minutes, but no longer starts
    # from scratch on subsequent loads of an already-timed Rev9 or Rev10 career.
    for rev, size, started in [(7,292,18000), (8,300,18000), (9,304,4657), (10,304,4657)]:
        old = bytearray(seed[:size])
        struct.pack_into("<I", old, 4, rev)
        struct.pack_into("<i", old, 8, 203)
        struct.pack_into("<i", old, 136, 807)
        struct.pack_into("<ii", old, 148, 1, 2)
        if rev >= 8:
            struct.pack_into("<ii", old, 288, 4200, 87)
        if rev >= 9:
            struct.pack_into("<i", old, 296, started)
        data = round_trip(old)
        assert struct.unpack_from("<ii", data, 148) == (1, 2)
        assert struct.unpack_from("<i", data, 8)[0] == 203
        assert struct.unpack_from("<i", data, 136)[0] == 807
        bank, dirty_spent, remaining = struct.unpack_from("<iii", data, 288)
        assert (bank, dirty_spent) == ((0, 0) if rev < 8 else (4200, 87))
        active_frames = struct.unpack_from("<I", data, 252)[0] - struct.unpack_from("<I", old, 252)[0]
        assert 0 < active_frames < 90 and remaining == started - active_frames
    # At 7:59, preserve elapsed day time and closing identity when importing
    # timed deposits from Revenue Deadline. At the old eight-minute boundary
    # the ledger should now continue and both deposits remain pending.
    old = bytearray(seed)
    struct.pack_into("<I", old, 4, 9)
    old_ticks = 480 * 60 - 2
    old_elapsed = old_ticks / 60
    struct.pack_into("<f", old, 248, old_elapsed)
    struct.pack_into("<ii", old, 288, 321, 499)
    struct.pack_into("<iffi", old, 200, 1, 0., 480 - old_elapsed, 32)
    struct.pack_into("<iffi", old, 216, 1, 0., 960 - old_elapsed, 43)
    struct.pack_into("<iffi", old, 232, 1, 0., 0., 59)
    data = round_trip(old)
    elapsed = struct.unpack_from("<f", data, 248)[0]
    assert 480 <= elapsed < 481.5
    assert struct.unpack_from("<ii", data, 288) == (321, 499)
    for offset, amount, closing in [(200, 32, 1440), (216, 43, 2880), (232, 59, 0)]:
        owned, _, cooldown, pending = struct.unpack_from("<iffi", data, offset)
        assert (owned, pending) == (1, amount)
        assert abs(cooldown - (closing - elapsed if closing else 0)) < .002
print(("Linux ELF" if linux else "PE") + " entrypoint: synthetic Rev2–10 careers loaded and re-saved as Rev10; obsolete audits discarded, confirmed cases migrate with a five-minute timer, and stored countdown, bank, ledger and next/following closing migration are preserved.")

"""Boot real PE with a disposable case two ticks away from its seizure."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

host, game, seed_path = map(Path, sys.argv[1:])
seed = bytearray(seed_path.read_bytes())
assert len(seed) == 304 and struct.unpack_from("<II", seed) == (0x44433131, 10)
# Keep original walking coordinates, start with an active Revenue hold.
struct.pack_into("<i", seed, 8, 300)
struct.pack_into("<i", seed, 136, 900)
struct.pack_into("<ii", seed, 148, 1, 0)
struct.pack_into("<I", seed, 172, 4)
struct.pack_into("<f", seed, 248, 0.0)
struct.pack_into("<iii", seed, 288, 250, 501, 2)

def checksum(blob):
    n = 2166136261
    for b in blob:
        n = ((n ^ b) * 16777619) & 0xffffffff
    return n

struct.pack_into("<I", seed, 300, checksum(seed[:-4]))
with tempfile.TemporaryDirectory() as scratch:
    src, out, reload_out = (Path(scratch) / name for name in ("in.dat", "out.dat", "reload.dat"))
    src.write_bytes(seed)
    subprocess.run([host, game, src, out], check=True, stdout=subprocess.DEVNULL)
    data = out.read_bytes()
    assert src.read_bytes() == seed and len(data) == 1044
    assert struct.unpack_from("<I", data, 4) == (11,)
    assert struct.unpack_from("<I", data, 1040) == (checksum(data[:1040]),)
    assert struct.unpack_from("<4I", data, 432+49*4) == (0x52561000, 300, 900, 0)
    assert struct.unpack_from("<i", data, 8)[0] == struct.unpack_from("<i", data, 136)[0] == 0
    assert struct.unpack_from("<i", data, 148) == (1,)
    bank, spend, elapsed = struct.unpack_from("<iii", data, 288)
    assert (bank, spend) == (250, 501) and 17900 < elapsed < 18000
    assert checksum(data[:300]) == struct.unpack_from("<I", data, 300)[0]
    subprocess.run([host, game, out, reload_out], check=True, stdout=subprocess.DEVNULL)
    data2 = reload_out.read_bytes()
    assert data2[432+49*4:432+53*4] == data[432+49*4:432+53*4]
    bank, spend, remaining = struct.unpack_from("<iii", data2, 288)
    assert (bank, spend) == (250, 501) and 0 < remaining < elapsed
print("PE Deadline: both wallets seized, bank intact, case open, countdown resumes after reload.")

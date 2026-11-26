"""Exact PE migration: Lantern's retired distributor becomes an operator."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

host, game, seed_path = map(Path, sys.argv[1:])
seed = bytearray(seed_path.read_bytes())
assert len(seed) == 304 and struct.unpack_from("<II", seed) == (0x44433131, 10)

def checksum(data):
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xffffffff
    return h

# Each owned site has a pending batch 2 ticks from completion and a distributor.
# Lantern is already at its normal capacity, which was legal in direct-ship mode.
struct.pack_into("<i", seed, 24, 5)
struct.pack_into("<6if", seed, 68, 1, 2, 1, 0, 120000, 0, 2 / 60)
struct.pack_into("<6if", seed, 96, 1, 2, 1, 2, 120000, 0, 2 / 60)
struct.pack_into("<iii", seed, 124, 53, 7, 0)
struct.pack_into("<i", seed, 136, 60)
struct.pack_into("<I", seed, 172, 4)
struct.pack_into("<I", seed, 300, checksum(seed[:-4]))

with tempfile.TemporaryDirectory() as tmp:
    src, out, reload_out = (Path(tmp) / x for x in ("old.dat", "new.dat", "reload.dat"))
    src.write_bytes(seed)
    subprocess.run([host, game, src, out], check=True, stdout=subprocess.DEVNULL)
    assert src.read_bytes() == seed
    subprocess.run([host, game, out, reload_out], check=True, stdout=subprocess.DEVNULL)
    for file in (out, reload_out):
        data = file.read_bytes()
        assert len(data) == 304 and struct.unpack_from("<II", data) == (0x44433131, 10)
        assert checksum(data[:-4]) == struct.unpack_from("<I", data, 300)[0]
        assert struct.unpack_from("<i", data, 8) == struct.unpack_from("<i", seed, 8)
        assert struct.unpack_from("<i", data, 24) == (3,)
        assert struct.unpack_from("<6if", data, 68) == (1, 1, 1, 0, 0, 1, 0.0)
        cinder = struct.unpack_from("<6if", data, 96)
        assert cinder[:6] == (1, 2, 1, 2, 120000, 1) and 116 < cinder[6] < 120
        assert struct.unpack_from("<iii", data, 124) == (58, 7, 0)
        assert struct.unpack_from("<i", data, 136) == (331,)
print("PE Lantern crew: retired distributor becomes operator, in-flight stock preserved at 65, next batch waits; Cinder still direct-ships and requeues.")

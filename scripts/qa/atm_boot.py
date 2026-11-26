"""Seed a disposable career and exercise the real Windows host's ATM input."""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

host, game, seed_path = map(Path, sys.argv[1:])
seed = bytearray(seed_path.read_bytes())
assert len(seed) == 304 and struct.unpack_from("<II", seed) == (0x44433131, 10)
# Wallet and location at a real Tide Market ATM approach, not UI-internal state.
struct.pack_into("<i", seed, 8, 300)
struct.pack_into("<ff", seed, 60, 388, 736)
struct.pack_into("<i", seed, 136, 900)
struct.pack_into("<I", seed, 172, 4)
struct.pack_into("<ii", seed, 288, 50, 0)
digest = 2166136261
for b in seed[:-4]:
    digest = ((digest ^ b) * 16777619) & 0xffffffff
struct.pack_into("<I", seed, 300, digest)
with tempfile.TemporaryDirectory() as temp:
    src = Path(temp) / "atm_test_in.dat"
    dst = Path(temp) / "atm_test_out.dat"
    src.write_bytes(seed)
    subprocess.run([host, game, src, dst], env=dict(os.environ, DC_ATM_TEST="1"), check=True)
    assert len(dst.read_bytes()) == 304 and src.read_bytes() == seed

"""Exercise midnight through the packaged Windows PE with disposable saves."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

host, game, seed = map(Path, sys.argv[1:])
seed = seed.read_bytes()
assert len(seed) == 304 and struct.unpack_from("<II", seed) == (0x44433131, 10)

def checksum(data):
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xffffffff
    return h

with tempfile.TemporaryDirectory() as tmp:
    src = Path(tmp) / "career-in.dat"
    out = Path(tmp) / "career-out.dat"
    for start_ticks in (480 * 60 - 2, 1440 * 60 - 2):
        blob = bytearray(seed)
        struct.pack_into("<i", blob, 8, 300)
        struct.pack_into("<i", blob, 136, 900)
        struct.pack_into("<ii", blob, 148, 1, 2)
        struct.pack_into("<I", blob, 172, 4)
        struct.pack_into("<f", blob, 248, start_ticks / 60)
        struct.pack_into("<iii", blob, 288, 250, 501, 12345)
        struct.pack_into("<I", blob, 300, checksum(blob[:-4]))
        src.write_bytes(blob)
        out.unlink(missing_ok=True)
        subprocess.run([host, game, src, out], check=True, stdout=subprocess.DEVNULL)
        data = out.read_bytes()
        assert src.read_bytes() == blob and len(data) == 304
        assert checksum(data[:-4]) == struct.unpack_from("<I", data, 300)[0]
        frame = struct.unpack_from("<I", data, 252)[0] - struct.unpack_from("<I", blob, 252)[0]
        assert 2 <= frame < 90
        assert struct.unpack_from("<ii", data, 148) == (1, 2)
        assert struct.unpack_from("<i", data, 8) == (300,)
        assert struct.unpack_from("<i", data, 136) == (900,)
        assert struct.unpack_from("<iii", data, 288) == (
            250, 501 if start_ticks < 480 * 60 else 0, 12345 - frame
        )
        elapsed_ticks = (start_ticks + frame) % (1440 * 60)
        assert abs(struct.unpack_from("<f", data, 248)[0] * 60 - elapsed_ticks) < .02
print("PE Midnight: eight-minute mark preserves spending, true midnight resets it, and both leave wallets, bank and case cadence intact.")

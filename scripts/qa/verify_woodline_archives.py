"""Verify both release files and an exact PE entry smoke from the extraction."""
from pathlib import Path, PurePosixPath
import hashlib
import os
import struct
import sys
import subprocess
import tempfile
import tarfile
import zipfile

root = Path(__file__).resolve().parents[2]
downloads = Path(sys.argv[1]).resolve()
prefix = "Dead-Channel-Port-Vanta-Woodline-Harbor-"
win = downloads / (prefix + "Windows-11-x64.zip")
linux = downloads / (prefix + "Linux-x64.tar.gz")
manifest = dict(reversed(line.split()) for line in (downloads / "SHA256SUMS.txt").read_text().splitlines() if line)
getsha = lambda b: hashlib.sha256(b).hexdigest()
assert all(getsha(p.read_bytes()) == manifest[p.name] for p in [win, linux])


def valid_names(names):
    assert len(names) == len(set(names))
    for name in names:
        p = PurePosixPath(name)
        assert p.parts[0] == "Dead-Channel" and not p.is_absolute()
        assert len(p.parts) >= 2
        assert not {"..", ".git", "__pycache__", "build"}.intersection(p.parts)
        assert p.suffix not in (".dat", ".o", ".obj", ".pyc", ".ppm", ".bak", ".so")


with zipfile.ZipFile(win) as z, tarfile.open(linux, "r:gz") as t:
    zn, tn = z.namelist(), t.getnames()
    valid_names(zn)
    valid_names(tn)
    assert z.testzip() is None and all(m.isfile() for m in t)
    assert set(zn) - set(tn) == {"Dead-Channel/DeadChannel.exe"}
    assert set(tn) - set(zn) == {"Dead-Channel/DeadChannel"}
    assert all(z.read(p) == t.extractfile(p).read() for p in set(zn) & set(tn))
    pe = z.read("Dead-Channel/DeadChannel.exe")
    elf = t.extractfile("Dead-Channel/DeadChannel").read()
    assert pe == (root / "build/DeadChannel.exe").read_bytes()
    assert elf == (root / "build/DeadChannel").read_bytes()
    assert pe[:2] == b"MZ" and elf[:6] == b"\x7fELF\x02\x01"
    assert struct.unpack_from("<H", elf, 18)[0] == 62
    p = struct.unpack_from("<I", pe, 0x3C)[0]
    assert pe[p:p+4] == b"PE\0\0" and struct.unpack_from("<H", pe, p + 4)[0] == 0x8664
    assert struct.unpack_from("<H", pe, p + 24)[0] == 0x020B
    assert t.getmember("Dead-Channel/DeadChannel").mode & 0o100

with tempfile.TemporaryDirectory(prefix="Dead Channel Woodline extraction ") as t, \
     tempfile.TemporaryDirectory(prefix="dc-woodline-host-") as h:
    temp, host = Path(t), Path(h)
    pe_path, elf_path = temp / "DeadChannel.exe", temp / "DeadChannel"
    pe_path.write_bytes(pe)
    elf_path.write_bytes(elf)
    elf_path.chmod(0o755)
    save = temp / "win.dat"
    subprocess.run(["gcc", "-O2", root / "scripts/check_pe.c", "-o", host / "check-pe"], check=True, timeout=30)
    subprocess.run([host / "check-pe", pe_path, "-", save], check=True, timeout=30)
    subprocess.run(["gcc", "-std=c11", "-O2", "-fPIC", "-shared", root / "scripts/qa/linux_replay.c",
                    "-ldl", "-o", host / "linux-replay.so"], check=True, timeout=30)
    env = dict(os.environ, XDG_DATA_HOME=str(temp / "Linux saved games"), SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
               LD_PRELOAD=str(host / "linux-replay.so"), DC_LINUX_REPLAY="guide")
    subprocess.run([elf_path], check=True, timeout=30, env=env)
    assert save.stat().st_size == 304
    assert (temp / "Linux saved games/DeadChannel/career.dat").stat().st_size == 304
print(f"Both exact archives verified: {len(zn)} members each, identical shared source/docs.")
print("PE32+ x64 smoke used simulated Win32; exact ELF x64 launched under native SDL2.")

"""Verify the exact paired Open City: Street Contact release and smoke-launch both extracted hosts."""
from pathlib import Path, PurePosixPath
from stat import S_IFREG, S_IFMT
import gzip
import hashlib
import os
import re
import struct
import subprocess
import sys
import tarfile
import tempfile
import zipfile

root = Path(__file__).resolve().parents[2]
release = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root / "downloads"
stem = "Dead-Channel-Open-City-Street-Contact-"
archives = {
    "elf": release / (stem + "Linux-x64.tar.gz"),
    "pe": release / (stem + "Windows-11-x64.zip"),
}
digest = lambda blob: hashlib.sha256(blob).hexdigest()
manifest = {}
for line in (release / "SHA256SUMS.txt").read_text().splitlines():
    if line:
        hexd, name = line.split(maxsplit=1)
        name = name.lstrip("*")
        assert name not in manifest and re.fullmatch(r"[0-9a-f]{64}", hexd)
        manifest[name] = hexd
for p in archives.values():
    assert digest(p.read_bytes()) == manifest[p.name]
    print(f"{p.name}: {manifest[p.name]}", flush=True)


def paths(names):
    assert len(names) == len(set(names)) and names == sorted(names)
    for name in names:
        p = PurePosixPath(name)
        assert p.as_posix() == name and p.parts[0] == "Dead-Channel" and len(p.parts) >= 2
        assert not any(x.startswith(".") or x in ("build", "__pycache__") for x in p.parts)
        assert p.suffix not in (".dat", ".bak", ".so", ".o", ".obj", ".pyc", ".ppm", ".orig", ".rej")
        assert "\\" not in name
    return set(names)


with tempfile.TemporaryDirectory(prefix="Dead Channel Open City Street Contact extracted ") as d, \
     tempfile.TemporaryDirectory(prefix="dc-open-city-host-") as h:
    scratch = Path(d)
    # The executable and save paths intentionally contain spaces. The
    # dynamic loader would split LD_PRELOAD, so the shim path cannot.
    host_scratch = Path(h)
    win_root, linux_root = scratch / "Windows ZIP", scratch / "Linux Gzip"
    with tarfile.open(archives["elf"], "r:gz") as t, zipfile.ZipFile(archives["pe"]) as z:
        ts, zs = paths(t.getnames()), paths(z.namelist())
        et, ep = "Dead-Channel/DeadChannel", "Dead-Channel/DeadChannel.exe"
        assert ts - zs == {et} and zs - ts == {ep}
        assert all(item.isfile() for item in t.getmembers()) and z.testzip() is None
        assert all(S_IFMT(n.external_attr >> 16) == S_IFREG for n in z.infolist())
        assert t.getmember(et).mode == 0o755
        assert z.getinfo(ep).external_attr >> 16 & 0o777 == 0o755
        for relative in sorted(ts & zs):
            assert t.extractfile(relative).read() == z.read(relative), relative
        elf, pe = t.extractfile(et).read(), z.read(ep)
        if (root / "build" / "DeadChannel").exists():
            assert elf == (root / "build" / "DeadChannel").read_bytes()
        if (root / "build" / "DeadChannel.exe").exists():
            assert pe == (root / "build" / "DeadChannel.exe").read_bytes()
        assert elf.startswith(b"\x7fELF\x02\x01") and struct.unpack_from("<H", elf, 18)[0] == 62
        header = struct.unpack_from("<I", pe, 0x3c)[0]
        assert pe[:2] == b"MZ" and pe[header:header+4] == b"PE\0\0"
        assert struct.unpack_from("<H", pe, header+4)[0] == 0x8664
        assert struct.unpack_from("<H", pe, header+24)[0] == 0x020b
        # Materialize only files whose regular type and relative name were checked.
        for n in ts:
            p = linux_root / n
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(t.extractfile(n).read())
            p.chmod(t.getmember(n).mode & 0o777)
        for n in zs:
            p = win_root / n
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(z.read(n))
        print(f"{len(ts)} files each; {len(ts & zs)} shared exact bytes; x86-64 ELF {digest(elf)}; PE32+ {digest(pe)}.", flush=True)

    cc = os.environ.get("CC", "cc")
    interposer, check_pe = host_scratch/"sdl-replay.so", host_scratch/"check-pe"
    subprocess.run([cc, "-std=c11", "-O2", "-shared", "-fPIC",
                    root/"scripts/qa/linux_replay.c", "-ldl", "-o", interposer],
                   check=True, timeout=90)
    subprocess.run([cc, "-O2", root/"scripts/check_pe.c", "-o", check_pe],
                   check=True, timeout=90)
    tape = gzip.decompress((root/"scripts/qa/curb-lamp-tour.txt.gz").read_bytes())
    count, want = tape.splitlines()[0].split()
    assert int(count) == 3682 and want == b"7dfa7bce"
    dest = scratch / "native slots"
    snap = scratch / "explicit linux Save.dat"
    env = dict(os.environ, XDG_DATA_HOME=str(dest), SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy", LD_PRELOAD=str(interposer),
               DC_LINUX_REPLAY="", DC_LINUX_SNAPSHOT=str(snap))
    linux = linux_root / et
    windows = win_root / ep
    subprocess.run([linux], cwd=linux.parent, input=tape, env=env, check=True, timeout=180)
    assert struct.unpack_from("<I", snap.read_bytes(), 300)[0] == int(want, 16)
    subprocess.run([check_pe, windows, "--tour"], cwd=windows.parent, input=tape,
                   env=dict(os.environ, DC_TAP_REPLAY="1"), check=True, timeout=180)
    print("Current 3,682-frame courier/taxi digest 7dfa7bce: exact packaged native SDL ELF and simulated Win32 PE.")

#!/usr/bin/env python3
"""Build-independent, reproducible Open City: Street Contact release assembly.

Run both native builds, then point this at a fresh release directory.
With no directory argument it appends to (and preserves) downloads/SHA256SUMS.txt.
"""
from pathlib import Path
import gzip
import hashlib
import io
import sys
import tarfile
import zipfile

root = Path(__file__).resolve().parents[1]
out = Path(sys.argv[1]).expanduser().resolve() if len(sys.argv) == 2 else root / "downloads"
out.mkdir(parents=True, exist_ok=True)
prefix = "Dead-Channel-Open-City-Street-Contact-"
tops = {"START-HERE.txt", "WHAT-CHANGED.txt", "FONT-LICENSE.txt",
        "CITY-LISTINGS.txt", "BASE-VILLAGE.txt", "TRAP-HOUSE.txt",
        "THE-AFTERIMAGE.txt", "SIGNAL-PHONE.txt", "MOTOR-WORKS.txt",
        "OUTER-LOOP.txt", "GLASS-RELAY-AND-LOFT.txt", "CROSSROADS.txt",
        "WOODLINE.txt", "WOODLINE-GAMEPLAY.txt", "TWIN-HEADLAMPS.txt",
        "WOODLINE-TERRAIN.txt", "WOODLINE-WILDLIFE.txt", "STREET-MOTION.txt",
        "LANTERN-QUAY.txt", "HARBOR-BOATS.txt", "HARBOR-COMMISSIONS.txt",
        "WOODLINE-STABILITY.md", "STABILITY-UPDATE.md", "STREET-LIFE.md",
        "FRAME-TIMES.txt", "ALL-AREA-3D.md", "OPEN-CITY.md", "CHARACTER-WARDROBE.txt",
        "STREET-CONTACT.md", "3D-STREET-MOTION.md", "3D-DETAIL-UPDATE.md", "OPEN-CITY-MAP.md",
        "OPEN-CITY-STREET-CONTACT.md"}
source_suffixes = {".c", ".h", ".inc", ".md", ".py", ".sh", ".txt"}
files = {}
for p in [root / x for x in ("AGENTS.md", "README.md", "HANDOFF.md")] + [
        p for section in ("src", "scripts", "docs")
        for p in sorted((root / section).rglob("*"))]:
    r = p.relative_to(root)
    if (not p.is_file() or p.is_symlink() or any(x.startswith(".") for x in r.parts)
        or "__pycache__" in r.parts):
        continue
    if p.suffix not in source_suffixes and not (
            r.parts[:2] == ("scripts", "qa") and p.name.endswith("-tour.txt.gz")):
        continue
    files["Dead-Channel/Source/" + r.as_posix()] = p
for p in sorted((root / "docs").glob("*")):
    if p.name in tops or (p.suffix in (".md", ".txt") and p.name.startswith(
            ("SHADE-", "WARDROBE", "NORTHPOINT", "OPEN-CITY", "AFTERIMAGE-ART"))):
        files["Dead-Channel/" + p.name] = p
assert all("Dead-Channel/" + n in files for n in tops), tops
assert "Dead-Channel/Source/src/game.c" in files and "Dead-Channel/Source/src/windows.c" in files

manifest = out / "SHA256SUMS.txt"
history = [] if not manifest.exists() else [
    row for row in manifest.read_text().splitlines() if row.strip() and
    not row.split()[-1].lstrip("*").startswith(prefix)]
hashes = list(history)

for abi, exe, ext, magic in (
    ("Linux-x64", "DeadChannel", ".tar.gz", b"\x7fELF"),
    ("Windows-11-x64", "DeadChannel.exe", ".zip", b"MZ")
):
    binary = root / "build" / exe
    assert binary.read_bytes().startswith(magic), binary
    payloads = {key: value.read_bytes() for key, value in files.items()}
    payloads["Dead-Channel/" + exe] = binary.read_bytes()
    mode = {key: 0o755 if p.suffix in (".sh", ".py") and p.stat().st_mode & 0o111
            else 0o644 for key, p in files.items()}
    mode["Dead-Channel/" + exe] = 0o755
    dest = out / (prefix + abi + ext)
    stage = dest.with_name(dest.name + ".stage")
    names = sorted(payloads)

    try:
        if ext == ".zip":
            with zipfile.ZipFile(stage, "w") as arc:
                for key in names:
                    zi = zipfile.ZipInfo(key, (1980, 1, 1, 0, 0, 0))
                    zi.create_system = 3
                    zi.external_attr = (0o100000 | mode[key]) << 16
                    zi.compress_type = zipfile.ZIP_DEFLATED
                    arc.writestr(zi, payloads[key], compresslevel=9)
            with zipfile.ZipFile(stage) as arc:
                assert arc.namelist() == names and arc.testzip() is None
                assert all(arc.read(key) == payloads[key] for key in names)
        else:
            with stage.open("wb") as stream, gzip.GzipFile(filename="", mode="wb", fileobj=stream, mtime=0) as gz:
                with tarfile.open(fileobj=gz, mode="w") as arc:
                    for key in names:
                        ti = tarfile.TarInfo(key)
                        ti.size, ti.mode, ti.mtime = len(payloads[key]), mode[key], 0
                        arc.addfile(ti, io.BytesIO(payloads[key]))
            with tarfile.open(stage, "r:gz") as arc:
                assert arc.getnames() == names and arc.getmember("Dead-Channel/" + exe).mode == 0o755
                assert all(arc.extractfile(key).read() == payloads[key] for key in names)
        stage.replace(dest)
    finally:
        stage.unlink(missing_ok=True)
    digest = hashlib.sha256(dest.read_bytes()).hexdigest()
    hashes.append(f"{digest}  {dest.name}")
    print(f"{digest}  {dest.name} ({dest.stat().st_size:,} bytes; {len(names)} files)")

tmp = manifest.with_suffix(".stage")
tmp.write_text("\n".join(hashes) + "\n")
tmp.replace(manifest)

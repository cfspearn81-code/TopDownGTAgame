"""Package the verified native Woodline ELF and PE from the same source."""
from pathlib import Path
import gzip
import hashlib
import io
import sys
import tarfile
import zipfile

root = Path(__file__).resolve().parents[1]
out = Path(sys.argv[1]).expanduser().resolve()
out.mkdir(parents=True, exist_ok=True)
prefix = "Dead-Channel-Port-Vanta-Woodline-Harbor-"
common = {}
prior = ["AGENTS.md", "README.md", "HANDOFF.md"]
extra = ("START-HERE.txt", "WHAT-CHANGED.txt", "FONT-LICENSE.txt", "CITY-LISTINGS.txt",
         "BASE-VILLAGE.txt", "TRAP-HOUSE.txt", "THE-AFTERIMAGE.txt", "SIGNAL-PHONE.txt",
         "MOTOR-WORKS.txt", "OUTER-LOOP.txt", "GLASS-RELAY-AND-LOFT.txt",
         "CROSSROADS.txt", "WOODLINE.txt", "WOODLINE-GAMEPLAY.txt",
         "TWIN-HEADLAMPS.txt", "WOODLINE-TERRAIN.txt", "WOODLINE-WILDLIFE.txt",
         "STREET-MOTION.txt", "LANTERN-QUAY.txt", "HARBOR-BOATS.txt",
         "HARBOR-COMMISSIONS.txt",
         "QA-WOODLINE.txt")
for p in [root / x for x in prior] + [p for d in ("src", "scripts", "docs")
                                     for p in sorted((root / d).rglob("*"))]:
    if (not p.is_file() or p.is_symlink() or "__pycache__" in p.parts or
        p.suffix in {".orig", ".rej", ".pyc", ".so", ".o", ".obj", ".dat", ".exe", ".ppm"}):
        continue
    common["Dead-Channel/Source/" + p.relative_to(root).as_posix()] = p
for name in extra:
    common["Dead-Channel/" + name] = root / "docs" / name

hashes = []
for abi, exe, extension in [("Windows-11-x64", "DeadChannel.exe", ".zip"),
                            ("Linux-x64", "DeadChannel", ".tar.gz")]:
    binary = root / "build" / exe
    assert binary.read_bytes()[:4].startswith(b"MZ" if extension == ".zip" else b"\x7fELF")
    files = dict(common, **{"Dead-Channel/" + exe: binary})
    dest = out / (prefix + abi + extension)
    stage = dest.with_name(dest.name + ".stage")

    def mode(path):
        return 0o755 if path == binary or (path.suffix in (".sh", ".py") and path.stat().st_mode & 0o111) else 0o644

    if extension == ".zip":
        with zipfile.ZipFile(stage, "w") as arc:
            for name, p in sorted(files.items()):
                zi = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
                zi.create_system = 3
                zi.external_attr = (0o100000 | mode(p)) << 16
                zi.compress_type = zipfile.ZIP_DEFLATED
                arc.writestr(zi, p.read_bytes(), compresslevel=9)
        with zipfile.ZipFile(stage) as arc:
            assert arc.testzip() is None and arc.namelist() == sorted(files)
            assert all(arc.read(name) == p.read_bytes() for name, p in files.items())
    else:
        with stage.open("wb") as stream, gzip.GzipFile(filename="", mode="wb", fileobj=stream, mtime=0) as gzip_stream:
            with tarfile.open(fileobj=gzip_stream, mode="w") as arc:
                for name, p in sorted(files.items()):
                    payload = p.read_bytes()
                    ti = tarfile.TarInfo(name)
                    ti.size, ti.mode, ti.mtime = len(payload), mode(p), 0
                    arc.addfile(ti, io.BytesIO(payload))
        with tarfile.open(stage, "r:gz") as arc:
            assert arc.getnames() == sorted(files)
            assert arc.getmember("Dead-Channel/" + exe).mode & 0o100
            assert all(arc.extractfile(name).read() == p.read_bytes() for name, p in files.items())
    stage.replace(dest)
    digest = hashlib.sha256(dest.read_bytes()).hexdigest()
    hashes.append(f"{digest}  {dest.name}")
    print(f"{digest}  {dest.name} ({dest.stat().st_size:,} bytes)")
manifest = out / "SHA256SUMS.txt"
temp = manifest.with_suffix(".stage")
temp.write_text("\n".join(hashes) + "\n")
temp.replace(manifest)

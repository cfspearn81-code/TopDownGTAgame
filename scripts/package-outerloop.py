"""Package a tested native Linux executable and rebuildable sources in one tar.gz."""
import gzip
import hashlib
import os
from pathlib import Path
import stat
import sys
import tarfile

root = Path(__file__).resolve().parents[1]
binary = root / "build" / "DeadChannel"
assert binary.is_file() and binary.read_bytes()[:4] == b"\x7fELF"
out = Path(sys.argv[1]).expanduser().resolve()
out.mkdir(parents=True, exist_ok=True)
name = "Dead-Channel-Port-Vanta-Outer-Loop-Linux-x64.tar.gz"
files = {"Dead-Channel/DeadChannel": binary}
for path in (root/"AGENTS.md", root/"HANDOFF.md", root/"README.md"):
    files[f"Dead-Channel/Source/{path.name}"] = path
for d in ("src", "scripts", "docs"):
    for path in sorted((root/d).rglob("*")):
        if path.is_file() and "__pycache__" not in path.parts and path.suffix not in {".pyc",".rej",".orig"}:
            files[f"Dead-Channel/Source/{path.relative_to(root)}"] = path
for name_in in ("FONT-LICENSE.txt", "START-HERE.txt", "OUTER-LOOP.txt", "QA-OUTER-LOOP.txt", "COMPLETION-REVIEW.txt", "SIGNAL-PHONE.txt","THE-AFTERIMAGE.txt","MOTOR-WORKS.txt","GLASS-RELAY-AND-LOFT.txt"):
    files["Dead-Channel/"+name_in] = root/"docs"/name_in
dest = out/name
tmp = dest.with_name(dest.name+".stage")
with tmp.open("wb") as dest_file, gzip.GzipFile(filename="", mode="wb", fileobj=dest_file, mtime=0) as gz, tarfile.open(fileobj=gz,mode="w",format=tarfile.USTAR_FORMAT) as tar:
    for arc, path in sorted(files.items()):
        item = tarfile.TarInfo(arc)
        item.size = path.stat().st_size
        item.mtime = 0
        item.mode = 0o755 if path == binary or (path.suffix in (".sh",".py") and os.access(path,os.X_OK)) else 0o644
        with path.open("rb") as src:
            tar.addfile(item,src)
with tarfile.open(tmp,mode="r:gz") as tar:
    assert tar.getnames() == sorted(files)
    for arc,path in files.items():
        assert tar.extractfile(arc).read() == path.read_bytes()
    assert tar.getmember("Dead-Channel/DeadChannel").mode & stat.S_IXUSR
tmp.replace(dest)
digest=hashlib.sha256(dest.read_bytes()).hexdigest()
(out/(name+".sha256")).write_text(digest+"  "+name+"\n")
print(dest, dest.stat().st_size, digest)

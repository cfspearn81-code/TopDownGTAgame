"""Reproducible Crossroads QA update archives. Only known distributable source/docs are selected.
Run after testing both executables; never bundle saves, tools' outputs, or a .git tree."""
import gzip,hashlib,io,os,tarfile,zipfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=root/'downloads';out.mkdir(exist_ok=True)
common={}
for p in [root/x for x in ['AGENTS.md','README.md','HANDOFF.md']]+[p for d in ['src','scripts','docs'] for p in sorted((root/d).rglob('*')) if p.is_file()]:
 if p.is_symlink() or '__pycache__' in p.parts or p.name.endswith(('.orig','.rej','.pyc')):continue
 common['Dead-Channel/Source/'+p.relative_to(root).as_posix()]=p
for x in ['START-HERE.txt','WHAT-CHANGED.txt','CITY-LISTINGS.txt','QA-REPORT.txt','FONT-LICENSE.txt','CROSSROADS.txt','CROSSROADS-QA.md','BASE-VILLAGE.txt','TRAP-HOUSE.txt','THE-AFTERIMAGE.txt','SIGNAL-PHONE.txt','MOTOR-WORKS.txt','OUTER-LOOP.txt','GLASS-RELAY-AND-LOFT.txt']:
 common['Dead-Channel/'+x]=root/'docs'/x
manifest=[]
manifest_file=out/'SHA256SUMS.txt'
if manifest_file.exists():
 manifest=[row for row in manifest_file.read_text().splitlines() if 'Dead-Channel-Crossroads-QA-' not in row]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for target in ['Windows-11-x64','Linux-x64']:
 win=target.startswith('Windows');ex='DeadChannel.exe' if win else 'DeadChannel'
 files=dict(common);files['Dead-Channel/'+ex]=root/'build'/ex
 name='Dead-Channel-Crossroads-QA-'+target+('.zip' if win else '.tar.gz');path=out/name
 def mode(p):return 0o755 if p==root/'build'/ex or (p.stat().st_mode&0o111 and p.suffix in ('.sh','.py')) else 0o644
 if win:
  with zipfile.ZipFile(path,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
   for k,p in sorted(files.items()):
    z=zipfile.ZipInfo(k,(2026,11,26,0,0,0));z.create_system=3;z.external_attr=(0o100000|mode(p))<<16;z.compress_type=zipfile.ZIP_DEFLATED;archive.writestr(z,p.read_bytes(),compresslevel=9)
  with zipfile.ZipFile(path) as a:
   assert a.testzip() is None and set(a.namelist())==set(files)
   for k,p in files.items():assert a.read(k)==p.read_bytes(),k
 else:
  with path.open('wb') as f,gzip.GzipFile(filename='',mode='wb',fileobj=f,mtime=0) as gz,tarfile.open(fileobj=gz,mode='w') as archive:
   for k,p in sorted(files.items()):
    data=p.read_bytes();t=tarfile.TarInfo(k);t.size=len(data);t.mode=mode(p);t.mtime=1795651200;archive.addfile(t,io.BytesIO(data))
  with tarfile.open(path,'r:gz') as a:
   assert set(a.getnames())==set(files) and a.getmember('Dead-Channel/'+ex).mode&0o111
   for k,p in files.items():assert a.extractfile(k).read()==p.read_bytes(),k
 manifest.append(sha(path)+'  '+name)
manifest_file.write_text('\n'.join(manifest)+'\n')
print('\n'.join(manifest))

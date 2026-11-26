"""Independent Vault 39 replay against actual ELF + PE under simulated Win32.
Usage: python3 scripts/qa/vault_keyboard_replay.py [integrated source root]
Run build.sh and build-linux.sh first. Synthetic saves and temporary shims only.
"""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

here = Path(__file__).resolve().parent
root = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else here.parents[1]
probe = r"""
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static void v39_probe(int frame_no,const unsigned *rgb,int stride){
 struct Row{int at,x,y,w,h,done;unsigned hash;char name[64];};
 static struct Row rows[16];static int nr,loaded;
 if(!loaded){
  FILE*f=fopen(getenv("DC_VAULT_PROBES"),"r");assert(f);
  while(nr<16){struct Row*r=rows+nr;
   if(fscanf(f,"%d%d%d%d%d%x%63s",&r->at,&r->x,&r->y,&r->w,&r->h,&r->hash,r->name)!=7)break;
   nr++;
  }fclose(f);assert(nr==12);loaded=1;
 }
 for(int i=0;i<nr;i++){struct Row*r=rows+i;
  if(r->done||frame_no<r->at-2||frame_no>r->at+5)continue;
  unsigned h=2166136261u;
  for(int y=r->y;y<r->y+r->h;y++)for(int x=r->x;x<r->x+r->w;x++)h=(h^(rgb[y*stride+x]&0xffffffu))*16777619u;
  if(h==r->hash){r->done=1;fprintf(stderr,"ART %s at %d\n",r->name,frame_no);}
  else if(frame_no==r->at+5){fprintf(stderr,"MISSING %s at frame %d\n",r->name,frame_no);}
 }
}
"""

def call(args, **kw):
    p = subprocess.run(args, **kw)
    if p.returncode:
        if p.stderr: sys.stderr.buffer.write(p.stderr)
        p.check_returncode()
    return p

def save(path):
    b = path.read_bytes()
    f = lambda o: struct.unpack_from("<I",b,o)[0]
    hash = 2166136261
    for v in b[:-4]: hash = ((hash ^ v) * 16777619) & 0xffffffff
    assert len(b) == 304 and (f(0), f(4), f(300)) == (0x44433131, 10, hash)
    assert (f(8),f(12),f(40),f(136)) == (70,83,1,260)
    assert struct.unpack_from("<ff", b, 60) == (598, 730)
    print(f"Rev10 validated: clean $70, dirty $260, health 83, one job, Vault 39 street exit, digest {hash:08x}.",flush=True)
    return b

with tempfile.TemporaryDirectory(prefix="dc-vault-clients-") as t:
    tmp = Path(t)
    env = dict(os.environ, DC_TRIP_SEED="17", DC_VAULT_PROBES=str(tmp/"art"))
    call(["cc","-std=c11","-O2","-I",root/"src",here/"vault_keyboard_plan.c","-o",tmp/"route"])
    tape = call([tmp/"route",tmp/"start.dat"],env=env,stdout=subprocess.PIPE).stdout
    pe = (root/"scripts/check_pe.c").read_text()
    anchor = ' frame++;\n if(atmTest'
    assert pe.count(anchor) == 1
    pe = pe.replace(anchor,' v39_probe(frame+1,b,960);\n'+anchor)
    (tmp/"pe.c").write_text(pe.replace("static int frame,", probe+"\nstatic int frame,"))
    call(["cc","-O2",tmp/"pe.c","-o",tmp/"pe"])
    linux = (root/"scripts/qa/linux_replay.c").read_text().replace("../../src/",str(root/"src")+"/")
    (tmp/"linux.c").write_text(linux+probe+r"""
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){
 int(*real)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 assert(real&&pitch==960*4);v39_probe(frame,p,960);return real(t,r,p,pitch);
}
""")
    call(["cc","-std=c11","-O2","-shared","-fPIC",tmp/"linux.c","-l:libSDL2-2.0.so.0","-ldl","-o",tmp/"replay.so"])
    def play(args, extra_env):
        res = call(args,env=dict(env,**extra_env),input=tape,capture_output=True,timeout=150)
        print(res.stdout.decode(),res.stderr.decode(),sep="",end="",flush=True)
        assert res.stderr.count(b"ART ") == 12 and b"MISSING" not in res.stderr
    play([tmp/"pe",root/"build"/"DeadChannel.exe","--tour"],
         {"DC_TRIP_LOAD":str(tmp/"start.dat"),"DC_TRIP_SAVE":str(tmp/"win.dat")})
    windows = save(tmp/"win.dat")
    data=tmp/"xdg/DeadChannel";data.mkdir(parents=True)
    (data/"career.dat").write_bytes((tmp/"start.dat").read_bytes())
    play([root/"build"/"DeadChannel"],
         {"SDL_VIDEODRIVER":"dummy","SDL_RENDER_DRIVER":"software","SDL_AUDIODRIVER":"dummy",
          "LD_PRELOAD":str(tmp/"replay.so"),"XDG_DATA_HOME":str(data.parent),
          "DC_LINUX_REPLAY":"","DC_LINUX_SNAPSHOT":str(tmp/"lin.dat")})
    assert save(tmp/"lin.dat") == windows, "save bytes differed across entry points"
    print(f"PASS: Both shipped-style launchers matched all 12 graphics checkpoints and byte-identical saves after {tape.split()[0].decode()} frames.")

"""Pulse pose, rail and muzzle pixel replay against actual ELF + PE (simulated Win32).
Usage: python3 scripts/qa/pulse_keyboard_replay.py [integrated source root]
Run build.sh and build-linux.sh first. Synthetic saves and temporary shims only.
Set DC_PULSE_BITMAPS to a local directory to retain color crops.
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
static void pulse_probe(int frame_no,const unsigned *rgb,int stride){
 struct Tap{int x,y;unsigned color;};
 struct Row{int at,fx,fy,done;struct Tap tap[4];char name[64];};
 static struct Row rows[16];static int nr,loaded;
 if(!loaded){
  FILE*f=fopen(getenv("DC_PULSE_PROBES"),"r");assert(f);
  for(nr=0;nr<16;nr++){
   struct Row*r=rows+nr;
   int read=fscanf(f,"%d%d%d",&r->at,&r->fx,&r->fy);
   if(read==EOF)break;
   assert(read==3);
   for(int j=0;j<4;j++){
    struct Tap*t=r->tap+j;assert(fscanf(f,"%d%d%x",&t->x,&t->y,&t->color)==3);
   }
   assert(fscanf(f,"%63s",r->name)==1);
  }fclose(f);assert(nr>0);loaded=1;
 }
 for(int i=0;i<nr;i++){struct Row*r=rows+i;
  if(r->done||frame_no<r->at-2||frame_no>r->at+5)continue;
  int match=1;
  for(int j=0;j<4;j++){struct Tap*t=r->tap+j;match&=(rgb[t->y*stride+t->x]&0xffffffu)==t->color;}
  if(match){
   r->done=1;fprintf(stderr,"ART %s at %d\n",r->name,frame_no);
   const char *out=getenv("DC_PULSE_BITMAPS"),*host=getenv("DC_PULSE_HOST");
   if(out&&*out&&host){
    char name[2048];assert(snprintf(name,sizeof name,"%s/%s-%s.ppm",out,host,r->name)<(int)sizeof name);
    FILE*f=fopen(name,"wb");assert(f);
    fprintf(f,"P6\n68 56\n255\n");
    for(int y=r->fy-34;y<r->fy+22;y++)for(int x=r->fx-34;x<r->fx+34;x++){
     unsigned v=(x>=0&&x<stride&&y>=0&&y<540)?rgb[y*stride+x]:0;
     fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);
    }fclose(f);
   }
  }
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
    assert f(8)==60 and f(20)==0
    print(f"Rev10 validated: clean $60, equipped pistol, digest {hash:08x}.",flush=True)
    return b

bitmaps = os.getenv("DC_PULSE_BITMAPS")
if bitmaps:
    Path(bitmaps).mkdir(parents=True, exist_ok=True)

with tempfile.TemporaryDirectory(prefix="dc-pulse-clients-") as t:
    tmp = Path(t)
    env = dict(os.environ, DC_TRIP_SEED="17", DC_PULSE_PROBES=str(tmp/"art"))
    call(["cc","-std=c11","-O2","-I",root/"src",here/"pulse_keyboard_plan.c","-o",tmp/"route"])
    pe = (root/"scripts/check_pe.c").read_text()
    anchor = ' frame++;\n if(atmTest'
    assert pe.count(anchor) == 1
    pe = pe.replace(anchor,' pulse_probe(frame+1,b,960);\n'+anchor)
    (tmp/"pe.c").write_text(pe.replace("static int frame,", probe+"\nstatic int frame,"))
    call(["cc","-O2",tmp/"pe.c","-o",tmp/"pe"])
    linux = (root/"scripts/qa/linux_replay.c").read_text().replace("../../src/",str(root/"src")+"/")
    (tmp/"linux.c").write_text(linux+probe+r"""
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){
 int(*real)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 assert(real&&pitch==960*4);pulse_probe(frame,p,960);return real(t,r,p,pitch);
}
""")
    call(["cc","-std=c11","-O2","-shared","-fPIC",tmp/"linux.c","-l:libSDL2-2.0.so.0","-ldl","-o",tmp/"replay.so"])
    selection = os.getenv("DC_PULSE_CASE", "all")
    assert selection in ("compass", "cover", "north", "all")
    for case, count in (("compass", 16), ("cover", 4), ("north", 7)):
        if selection not in ("all", case):
            continue
        route_args = [tmp/"route", tmp/"start.dat", tmp/"art"]
        if case != "compass":
            route_args.append(case)
        tape = call(route_args,env=env,stdout=subprocess.PIPE).stdout

        def play(args, extra_env):
            res = call(args,env=dict(env,**extra_env),input=tape,capture_output=True,timeout=150)
            print(res.stdout.decode(),res.stderr.decode(),sep="",end="",flush=True)
            assert res.stderr.count(b"ART ") == count and b"MISSING" not in res.stderr

        play([tmp/"pe",root/"build"/"DeadChannel.exe","--tour"],
             {"DC_PULSE_HOST":"pe","DC_TRIP_LOAD":str(tmp/"start.dat"),"DC_TRIP_SAVE":str(tmp/"win.dat")})
        windows = save(tmp/"win.dat")
        data=tmp/("xdg-"+case)/"DeadChannel";data.mkdir(parents=True)
        (data/"career.dat").write_bytes((tmp/"start.dat").read_bytes())
        play([root/"build"/"DeadChannel"],
             {"DC_PULSE_HOST":"elf","SDL_VIDEODRIVER":"dummy","SDL_RENDER_DRIVER":"software","SDL_AUDIODRIVER":"dummy",
              "LD_PRELOAD":str(tmp/"replay.so"),"XDG_DATA_HOME":str(data.parent),
              "DC_LINUX_REPLAY":"","DC_LINUX_SNAPSHOT":str(tmp/"lin.dat")})
        assert save(tmp/"lin.dat") == windows, "save bytes differed across entry points"
        print(f"PASS: {case}: both shipped-style launchers matched all {count} checkpoints and byte-identical saves after {tape.split()[0].decode()} frames.")

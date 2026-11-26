"""Replay real NPC interaction input on x86-64 SDL and the shipped PE entry.

Usage: python3 scripts/qa/npc_keyboard_replay.py [source root]
Build scripts/build.sh and scripts/build-linux.sh first. This uses fresh
synthetic careers only; SDL runs with dummy drivers, PE uses check_pe.c
Win32 stubs. Set DC_NPC_BITMAPS to retain reference crops from each host,
or DC_NPC_CASE to run just conversation, surrender, defeat, flee, guide, or hold.
For historical reproduction, use hold with DC_NPC_HOLD_BASELINE=1 and the previous release root.
"""
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile

here = Path(__file__).resolve().parent
root = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else here.parents[1]
assert len(sys.argv) <= 2
probe = r"""
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void npc_probe(int frame_no,const unsigned *rgb,int stride){
 struct Row{int at,x,y,w,h,done;unsigned ink,value;char name[64];};
 static struct Row rows[32];static int nr,loaded;
 if(!loaded){
  FILE*f=fopen(getenv("DC_NPC_PROBES"),"r");assert(f);
  while(nr<32){
   struct Row*r=rows+nr;
   int n=fscanf(f,"%d%d%d%d%d%x%x%63s",&r->at,&r->x,&r->y,&r->w,&r->h,&r->ink,&r->value,r->name);
   if(n==EOF)break;
   assert(n==8&&r->x>=0&&r->y>=0&&r->w>0&&r->h>0&&r->x+r->w<=stride&&r->y+r->h<=540);
   nr++;
  }
  assert(!fclose(f)&&nr>0&&nr<32);loaded=1;
 }
 /* Win32 and SDL count presentations from different indices. Both allow
    one neutral presentation before processing the same native input. */
 for(int i=0;i<nr;i++){
  struct Row*r=rows+i;
  if(r->done||frame_no<r->at-2||frame_no>r->at+4)continue;
  unsigned h=2166136261u,hits=0;
  for(int y=r->y;y<r->y+r->h;y++)for(int x=r->x;x<r->x+r->w;x++){
   unsigned pix=rgb[y*stride+x]&0xffffffu;
   h=(h^pix)*16777619u;hits+=pix==r->ink;
  }
  if(r->ink?hits>=r->value:h==r->value){
   r->done=1;fprintf(stderr,"ART %s at %d\n",r->name,frame_no);
   const char *dir=getenv("DC_NPC_BITMAPS"),*host=getenv("DC_NPC_HOST"),*kind=getenv("DC_NPC_CASE");
   if(dir&&*dir&&host&&kind){
    char path[2048];assert(snprintf(path,sizeof path,"%s/%s-%s-%s.ppm",dir,host,kind,r->name)<(int)sizeof path);
    FILE*out=fopen(path,"wb");assert(out);
    int x0=r->x,y0=r->y,x1=x0+r->w,y1=y0+r->h;
    if(!strcmp(kind,"hold")){x0=746;x1=835;y0=207;y1=276;}
    else if(r->w<30&&r->h<30){x0-=26;x1+=26;y0-=12;y1+=44;}
    fprintf(out,"P6\n%d %d\n255\n",x1-x0,y1-y0);
    for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){
     unsigned c=(x>=0&&x<stride&&y>=0&&y<540)?rgb[y*stride+x]:0;fputc(c>>16,out);fputc(c>>8,out);fputc(c,out);
    }
    assert(!fclose(out));
   }
  }else if(frame_no==r->at+4)fprintf(stderr,"MISSING %s at frame %d\n",r->name,frame_no);
 }
}
"""

def run(args, **kw):
    p = subprocess.run(args, **kw)
    if p.returncode:
        if p.stdout: sys.stderr.buffer.write(p.stdout)
        if p.stderr: sys.stderr.buffer.write(p.stderr)
        p.check_returncode()
    return p

def save(path, dirty, hp, expected_digest):
    b = path.read_bytes()
    assert len(b) == 304
    field = lambda o: struct.unpack_from("<I", b, o)[0]
    digest = 2166136261
    for v in b[:-4]: digest = ((digest ^ v) * 16777619) & 0xffffffff
    assert (field(0), field(4), field(300)) == (0x44433131, 10, digest)
    assert (field(8), field(12), field(20), field(136)) == (70, hp, 0, dirty)
    assert digest == expected_digest
    return b

cases = {
    "conversation": (0, 100, 1),
    "surrender": (36, 100, 2),
    "defeat": (17, 84, 2),
    "flee": (0, 100, 1),
    "guide": (0, 100, 1),
    "hold": (0, 100, 1),
}
requested = os.getenv("DC_NPC_CASE", "all")
assert requested == "all" or requested in cases
bitmap_dir = os.getenv("DC_NPC_BITMAPS")
if bitmap_dir:
    Path(bitmap_dir).mkdir(parents=True, exist_ok=True)

with tempfile.TemporaryDirectory(prefix="dc-npc-native-") as temp:
    tmp = Path(temp)
    baseline = os.getenv("DC_NPC_HOLD_BASELINE") == "1"
    if baseline: assert requested == "hold"
    run(["cc", "-std=c11", "-O2", *(["-DDC_NPC_HOLD_BASELINE"] if baseline else []),
         "-I", root/"src", here/"npc_keyboard_plan.c", "-o", tmp/"plan"])
    pe = (root/"scripts/check_pe.c").read_text()
    anchor = " frame++;\n if(atmTest"
    assert pe.count(anchor) == 1 and pe.count("static int frame,") == 1
    pe = pe.replace(anchor, " npc_probe(frame+1,b,960);\n"+anchor)
    (tmp/"pe.c").write_text(pe.replace("static int frame,", probe+"\nstatic int frame,"))
    run(["cc", "-O2", tmp/"pe.c", "-o", tmp/"pe"])
    linux = (root/"scripts/qa/linux_replay.c").read_text().replace("../../src/", str(root/"src")+"/")
    (tmp/"linux.c").write_text(linux+probe+r"""
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){
 int(*real)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 assert(real&&pitch==960*4);npc_probe(frame,p,960);return real(t,r,p,pitch);
}
""")
    run(["cc", "-std=c11", "-O2", "-shared", "-fPIC", tmp/"linux.c",
         "-l:libSDL2-2.0.so.0", "-ldl", "-o", tmp/"replay.so"])

    for case, (dirty, hp, min_atomic_writes) in cases.items():
        if requested not in ("all", case): continue
        probes = tmp/"art.txt"
        start = tmp/"start.dat"
        tape = run([tmp/"plan", start, probes, case], stdout=subprocess.PIPE).stdout
        frame_count, expected_hash = tape.splitlines()[0].split()
        expected_digest = int(expected_hash,16)
        names = {line.split()[-1] for line in probes.read_bytes().splitlines()}

        def play(args, seed, **extra):
            # Environment from unrelated QA scenarios must not alter the game.
            env = dict((k, v) for k, v in os.environ.items() if not k.startswith("DC_"))
            if bitmap_dir: env["DC_NPC_BITMAPS"] = bitmap_dir
            env.update(DC_TRIP_SEED=str(seed), DC_NPC_CASE=case,DC_NPC_PROBES=str(probes),**extra)
            res = run(args, env=env, input=tape, capture_output=True, timeout=150)
            print(res.stdout.decode()+res.stderr.decode(), end="", flush=True)
            actual = re.findall(rb"^ART ([a-zA-Z0-9-]+) at ",res.stderr,re.MULTILINE)
            assert len(actual) == len(names) and set(actual) == names and b"MISSING" not in res.stderr
            return res.stdout

        seed = 2 if case in ("defeat", "flee") else 0
        win_path = tmp/"win.dat"
        windows_log = play([tmp/"pe", root/"build/DeadChannel.exe", "--tour"], seed, DC_NPC_HOST="pe",
                           DC_TRIP_LOAD=str(start), DC_TRIP_SAVE=str(win_path))
        atomic = re.search(rb"(\d+) atomic writes", windows_log)
        assert atomic and int(atomic[1])>=min_atomic_writes
        windows = save(win_path, dirty, hp, expected_digest)
        data = tmp/("xdg-"+case)/"DeadChannel"
        data.mkdir(parents=True)
        (data/"career.dat").write_bytes(start.read_bytes())
        lin_path = tmp/"lin.dat"
        play([root/"build/DeadChannel"], seed, DC_NPC_HOST="elf",
             SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
             LD_PRELOAD=str(tmp/"replay.so"), XDG_DATA_HOME=str(data.parent),
             DC_LINUX_SNAPSHOT=str(lin_path))
        assert save(lin_path, dirty, hp, expected_digest) == windows
        print(f"PASS {case}: {frame_count.decode()} key frames, {len(names)} RGB checkpoints per launcher, "
              f"byte-identical Rev10 {expected_digest:08x} ($70 clean, ${dirty} dirty, {hp} HP).", flush=True)

"""Exercise both unmodified distributables; compare original 960x540 framebuffer regions.
Usage: python3 scripts/qa/explore_client_replay.py <integrated-source-root> [test.c]
Build both executables first. Synthetic save, input plan, captured subimage hashes
and host shims live in a temp dir. Win32 is simulated, not physical Windows.
"""
from itertools import groupby
import os, subprocess as sp, sys, tempfile
from pathlib import Path
root=Path(sys.argv[1]).resolve()
test=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else Path(__file__).with_name("verdict_explore_test.c")
probe=r"""
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
static void ex_probe_frame(int frame_no,const unsigned *rgb,int stride){
 struct Row{int at,x,y,w,h,done;unsigned hash;char name[64];};
 static struct Row rows[16];static int nr,loaded;
 if(!loaded){
  FILE*f=fopen(getenv("DC_EX_ART"),"r");assert(f);
  while(nr<16){struct Row*r=rows+nr;
   if(fscanf(f,"%d%d%d%d%d%x%63s",&r->at,&r->x,&r->y,&r->w,&r->h,&r->hash,r->name)!=7)break;
   nr++;
  }fclose(f);assert(nr==9);loaded=1;
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
def call(args,**kwargs):return sp.run(args,check=True,**kwargs)
with tempfile.TemporaryDirectory(prefix="dc-explore-clients-") as t:
 tmp=Path(t)
 call(["cc","-std=c11","-O2","-I",root/"src",test,"-o",tmp/"planner"])
 call([tmp/"planner"],env=dict(os.environ,DC_EX_PLAN_DIR=t))
 keys=tuple(map(int,(tmp/"keys").read_text().split()))
 tape=(f'{len(keys)} 0\n'+''.join(f'{len(list(it))} {k}\n' for k,it in groupby(keys))).encode()
 src=(root/"scripts/check_pe.c").read_text()
 anchor=' frame++;\n if(atmTest'
 assert src.count(anchor)==1
 src=src.replace(anchor,' ex_probe_frame(frame+1,b,960);\n'+anchor)
 src=src.replace('static int frame,',probe+'\nstatic int frame,')
 (tmp/"pe.c").write_text(src)
 call(["cc","-std=c11","-O2",tmp/"pe.c","-o",tmp/"pe"])
 src=(root/"scripts/qa/linux_replay.c").read_text().replace('../../src/',str(root/"src")+'/')
 src+=probe+r"""
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){
 int(*real)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 assert(real&&pitch==960*4);ex_probe_frame(frame,p,960);return real(t,r,p,pitch);
}
"""
 (tmp/"linux.c").write_text(src)
 call(["cc","-std=c11","-O2","-shared","-fPIC",tmp/"linux.c","-l:libSDL2-2.0.so.0","-ldl","-o",tmp/"replay.so"])
 build=root/"build"
 common=dict(os.environ,DC_TRIP_SEED="8043",DC_EX_ART=str(tmp/"art"))
 def play(args,env):
  res=call(args,env=env,input=tape,capture_output=True,timeout=170)
  diag=res.stderr.decode()
  print(res.stdout.decode().strip(),flush=True);print(diag,flush=True)
  assert diag.count('ART ')==9 and 'MISSING' not in diag
 play([tmp/"pe",build/"DeadChannel.exe","--tour"],dict(common,DC_TRIP_LOAD=str(tmp/"start.dat"),DC_TRIP_SAVE=str(tmp/"win.dat")))
 data=tmp/"xdg/DeadChannel";data.mkdir(parents=True);(data/"career.dat").write_bytes((tmp/"start.dat").read_bytes())
 play([build/"DeadChannel"],dict(common,DC_LINUX_REPLAY="",SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",
                           LD_PRELOAD=str(tmp/"replay.so"),XDG_DATA_HOME=str(data.parent),DC_LINUX_SNAPSHOT=str(tmp/"lin.dat")))
 print(f"PASS ELF + simulated Win32 PE: {len(keys):,} planned frames, each matched all nine activity artwork checkpoints.")

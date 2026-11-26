"""Drive both distributable entry points with synthetic careers and two patches.
Build both binaries before running. Uses the original 960x540 output pixels."""
import os,struct,subprocess as sp,tempfile
from itertools import groupby
from pathlib import Path
r=Path(__file__).resolve().parents[2]
wheel=16777216
patch=33554432
def run(args,**kw):
 try:return sp.run(args,check=True,timeout=100,**kw)
 except sp.CalledProcessError as e:
  for msg in (e.stdout,e.stderr):
   if msg:print(msg.decode(errors="replace"),flush=True)
  raise
def fnv(b):
 h=2166136261
 for v in b:h=((h^v)*16777619)&0xffffffff
 return h
def get(buf,off):return struct.unpack_from('<i',buf,off)[0]
def tape(keys):return (f'{len(keys)} 0\n'+''.join(f'{len(list(g))} {k}\n' for k,g in groupby(keys))).encode()
probe=r"""
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static void wheel_probe(int n,const unsigned *b,int stride){
 const char *plan=getenv("DC_WHEEL_PROBE");
 if(!plan)return;static unsigned char done[160];FILE*f=fopen(plan,"r");assert(f);
 int first,last,kind,i=0;
 while(fscanf(f,"%d%d%d",&first,&last,&kind)==3){
  assert(i<160);
  if(n>=first&&n<=last&&!done[i]){
   int top=(b[67*stride+480]&0xffffff)==0x63d7cf;
   int bottom=(b[467*stride+480]&0xffffff)==0x63d7cf;
   int badge=(b[434*stride+673]&0xffffff)==0x85a795;
   int empty=(b[408*stride+339]&0xffffff)==0x62757e&&(b[408*stride+621]&0xffffff)==0x62757e;
   if((kind==0&&!top&&!bottom&&!badge)||(badge&&empty&&((kind==1&&top&&!bottom)||(kind==2&&bottom&&!top)))){
    done[i]=1;fprintf(stderr,"WHEEL probe %d at %d\n",i,n);
   }
  }
  ++i;
 }
 fclose(f);
}
"""
win=r"""
static int wheel_windows_ping(Msg*m){
 if(!getenv("DC_WHEEL_PROBE"))return 0;
 static char sent[160];if(frame>=160||sent[frame])return 0;
 int y=frame==37?434:frame==8||frame==40||frame==75?430:frame==25||frame==111?118:0;
 int lose=frame==50,border=frame==32||frame==65||frame==102||frame==136;
 if(!y&&!lose&&!border)return 0;
 sent[frame]=1;memset(m,0,sizeof *m);m->w=(void*)1;
 if(lose){m->msg=8;return 1;}
 m->msg=0x0200;
 /* 1300x790 client with a 731px logical viewport and 29px top bar. */
 unsigned x=frame==37?(647*1300+959)/960:650;
 m->lp=x+((unsigned)(border?6:29+(y*731+539)/540)<<16);return 1;
}
"""
linux=r"""
static void wheel_motion(SDL_Renderer*r,int x,int y,int edge){
 int w,h;assert(SDL_GetRendererOutputSize(r,&w,&h)==0);
 int vw=w,vh=h;if(w*H>h*W)vw=h*W/H;else vh=w*H/W;
 SDL_Event e={.type=SDL_MOUSEMOTION};
 e.motion.x=(w-vw)/2+(x*vw+W-1)/W;
 e.motion.y=edge?0:(h-vh)/2+(y*vh+H-1)/H;emit(e);
}
static void wheel_linux_ping(SDL_Renderer*r){
 if(!getenv("DC_WHEEL_PROBE"))return;
 int y=frame==37?434:frame==8||frame==40||frame==75?430:frame==25||frame==111?118:0;
 if(y)wheel_motion(r,frame==37?647:480,y,0);
 if(frame==32||frame==65||frame==102||frame==136)wheel_motion(r,480,0,1);
 if(frame==151){key(SDL_SCANCODE_TAB,1);key(SDL_SCANCODE_TAB,0);}
 if(frame==50){
  focus(0);focus(1);
  SDL_Event e={.type=SDL_KEYDOWN};e.key.keysym.scancode=SDL_SCANCODE_TAB;e.key.repeat=1;emit(e);
 }
}
"""
with tempfile.TemporaryDirectory(prefix="wheel-host-qa-") as d:
 d=Path(d);src=r/'src';scripts=r/'scripts';build=r/'build'
 run(['cc','-O2','-I',src,scripts/'qa/wheel_buy_plan.c','-o',d/'planner'])
 source=(scripts/'check_pe.c').read_text()
 source=source.replace('static int frame,',probe+'\nstatic int frame,').replace('static int A peek(',win+'\nstatic int A peek(')
 source=source.replace('if(frame<tourFrames)return 0;','if(wheel_windows_ping(m))return 1;\n  if(frame<tourFrames)return 0;',1)
 source=source.replace('pointerScenario?(Rect){0,0,1300,790}', '(pointerScenario||getenv("DC_WHEEL_PROBE"))?(Rect){0,0,1300,790}')
 source=source.replace('else if(pointerScenario)assert(x==0', 'else if(pointerScenario||getenv("DC_WHEEL_PROBE"))assert(x==0')
 source=source.replace('frame++;\n if(atmTest', 'wheel_probe(frame,b,960);\n frame++;\n if(atmTest')
 (d/'pe.c').write_text(source);run(['cc','-O2',d/'pe.c','-o',d/'check_pe'])
 source=(scripts/'qa/linux_replay.c').read_text().replace('../../src/',str(src)+'/')
 source=source.replace('void SDL_RenderPresent(',linux+'\nvoid SDL_RenderPresent(')
 source=source.replace('remaining--;set_keys(mask);','remaining--;set_keys(mask);wheel_linux_ping(renderer);')
 source+=probe+r"""
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*rect,const void*p,int pitch){
 int(*f)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 assert(f&&pitch==960*4);wheel_probe(frame,p,960);return f(t,rect,p,pitch);
}
"""
 (d/'linux.c').write_text(source);run(['cc','-O2','-shared','-fPIC',d/'linux.c','-ldl','-l:libSDL2-2.0.so.0','-o',d/'replay.so'])
 fixture=d/'start.dat'
 buy=run([d/'planner',fixture],capture_output=True).stdout
 def both(name,starting,keybytes,rows=()):
  dirs=d/name;dirs.mkdir();native=dirs/'windows.dat';sdl=dirs/'linux.dat'
  career=dirs/'profile'/'DeadChannel'/'career.dat';career.parent.mkdir(parents=True);career.write_bytes(starting.read_bytes())
  env=dict(os.environ,DC_TRIP_SEED="17",DC_TRIP_LOAD=str(starting),DC_TRIP_SAVE=str(native))
  if rows:
   plan=dirs/'probes';plan.write_text(''.join(f'{a} {b} {k}\n' for a,b,k in rows));env['DC_WHEEL_PROBE']=str(plan)
  p=run([d/'check_pe',build/'DeadChannel.exe','--tour'],input=keybytes,env=env,capture_output=True)
  print(p.stdout.decode().strip(),flush=True)
  p_n=p.stderr.count(b"WHEEL probe ");win_log=p.stderr.decode()
  p=run([build/'DeadChannel'],input=keybytes,capture_output=True,env=dict(env,DC_LINUX_SNAPSHOT=str(sdl),
   XDG_DATA_HOME=str(career.parents[1]),SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software',LD_PRELOAD=str(d/'replay.so')))
  if (p_n,p.stderr.count(b"WHEEL probe "))!=(len(rows),len(rows)):print("Win checkpoints",p_n,win_log,"Linux:",p.stderr.decode(),flush=True)
  assert p_n==p.stderr.count(b"WHEEL probe ")==len(rows),(p_n,p.stderr)
  left,right=native.read_bytes(),sdl.read_bytes();assert len(left)==304 and left==right,(left.hex(),right.hex())
  assert struct.unpack_from('<I',left,300)[0]==fnv(left[:300])
  print(name, len(rows), "pixel checks per host, byte-identical Rev",get(left,4),"HP",get(left,12),"patches",left[267],"clean",get(left,8),flush=True)
  return right
 data=both('two full-HP purchases',fixture,buy)
 assert (get(data,4),get(data,12),get(data,8),data[267])==(11,100,100,2)
 for label,gear in [('unowned',-1),('owned',0)]:
  modified=bytearray(data);struct.pack_into('<i',modified,12,22);struct.pack_into('<i',modified,20,gear)
  struct.pack_into('<I',modified,300,fnv(modified[:300]))
  later=d/f'{label}.dat';later.write_bytes(modified)
  k=[0]*166;k[1]=1024
  k[8:60]=[wheel]*52;k[14:22]=[wheel|patch]*8
  k[65:96]=[wheel]*31;k[102:128]=[wheel]*26;k[136:146]=[wheel]*10;k[158]=512
  o=1 if gear==0 else 2
  # Leave settling frames for host redraws on different viewport sizes.
  rows=[(11,13,2),(28,30,o),(35,37,o),(38,40,o),(45,47,2),(54,56,0),(57,59,0),
        (69,71,o),(81,83,2),(106,109,2),(116,118,o),
        (130,133,0),(140,142,o),(151,153,0)]
  data2=both(label,later,tape(k),rows)
  assert (get(data2,4),get(data2,12),get(data2,8),data2[267])==(11,72,100,1)
  assert get(data2,136)==30 and get(data2,288)==87

"""Drive live Cart relocation and district arrival through both executables.

Linux SDL under dummy/software; actual PE entrypoint under Linux Win32 stubs.
Temporary test careers only; no installed game or user data is touched.
"""
import os
import subprocess as sp
from itertools import groupby
from pathlib import Path
from tempfile import TemporaryDirectory

ROOT = Path(__file__).resolve().parents[2]
def cmd(args, **kw):
    r=sp.run(args, timeout=90, **kw)
    if r.returncode and r.stderr:
        print(r.stderr.decode(errors="replace"))
    r.check_returncode()
    return r

seed = r'''
#define DC_LINUX_NATIVE
#include "game.c"
#include "linux_save.h"
#include <assert.h>
int main(int argc,char**argv){
 assert(argc==2&&linux_disk_init()&&linux_disk_lock());
 init_city();starter_save();scene=1;sv.intro=4;disk_slot(0);
 int cart=argv[1][0]=='c';p.x=cart?2250:836;p.y=1024;p.car=-1;p.hp=100;
 sv.dayClock=cart?1289.38f:750.f;request_save();assert(saveFlash>0);linux_disk_shutdown();return 0;
}'''
probe = r'''
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
static void city_probe(int f,const unsigned *rgb){
 const char *m=getenv("DC_CITY_MODE");if(!m)return;
 if(m[0]=='c'){
  static int seen_old,seen_new;
  if(f>=37&&f<=49&&!seen_old && (rgb[236*960+572]&0xffffff)==0xff9ac6){
   seen_old=1;fprintf(stderr,"CART: first pink marker on the regional atlas.\n");
  }
  if(f>=74&&f<=92&&!seen_new && (rgb[170*960+643]&0xffffff)==0xff9ac6 &&
     (rgb[236*960+572]&0xffffff)!=0xff9ac6){
   seen_new=1;fprintf(stderr,"CART: second pink marker after the scheduled move, old marker gone.\n");
  }
 }else{
  static int seen;
  if(f>=57&&f<=95&&!seen&&(rgb[16*960+450]&0xffffff)==0x4d6974){
   seen=1;fprintf(stderr,"DISTRICT: compact arrival title after crossing Tide Market into Nightglass.\n");
  }
 }
}
'''
def tape(steps):
    return (f"{len(steps)} 0\n" + "".join(f"{len(list(g))} {k}\n" for k,g in groupby(steps))).encode()

with TemporaryDirectory(prefix="dc-live-city-") as td:
    t=Path(td)
    (t/"seed.c").write_text(seed)
    cmd(["cc","-std=c11","-O0","-I",ROOT/"src",t/"seed.c",ROOT/"src/linux_save.c","-o",t/"seed"])
    pe=(ROOT/"scripts/check_pe.c").read_text()
    anchor=" frame++;\n if(atmTest"
    assert pe.count(anchor)==1 and pe.count("static int frame,")==1
    pe=pe.replace(anchor," city_probe(frame+1,b);\n"+anchor).replace("static int frame,",probe+"\nstatic int frame,")
    (t/"pe.c").write_text(pe)
    cmd(["cc","-O2",t/"pe.c","-o",t/"pe"])
    native=(ROOT/"scripts/qa/linux_replay.c").read_text().replace("../../src/",str(ROOT/"src")+"/")
    (t/"replay.c").write_text(native+probe+r'''
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){
 int(*real)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 assert(real&&pitch==960*4);city_probe(frame,p);return real(t,r,p,pitch);
}''')
    cmd(["cc","-O2","-shared","-fPIC",t/"replay.c","-l:libSDL2-2.0.so.0","-ldl","-o",t/"replay.so"])
    # Resume, P to Signal, twice Right to Night Wire, seven Down taps to Cart,
    # Enter to follow, Escape to street, M to atlas. Remain stationary as it moves.
    cart=[0,0,1024,0,524288,0,8,0,8,0]+[2,0]*7+[1024,0,128,0,256]+[0]*85+[512,0]
    district=[0,0,1024,0]+[8]*70+[0]*35+[512,0]
    env=dict(os.environ, SDL_VIDEODRIVER="dummy",SDL_AUDIODRIVER="dummy",SDL_RENDER_DRIVER="software")
    for name,actions,required in [("cart",cart,2),("district",district,1)]:
        xdg=t/name
        host=dict(env,XDG_DATA_HOME=str(xdg),DC_CITY_MODE=name)
        cmd([t/"seed",name],env=host)
        fixture=xdg/"DeadChannel/career.dat"
        win=cmd([t/"pe",ROOT/"build/DeadChannel.exe","--tour"],input=tape(actions),capture_output=True,
                env=dict(host,DC_TRIP_LOAD=str(fixture),DC_TRIP_SAVE=str(t/f"{name}-windows.dat")))
        linux=cmd([ROOT/"build/DeadChannel"],input=tape(actions),capture_output=True,
                  env=dict(host,LD_PRELOAD=str(t/"replay.so"),DC_LINUX_REPLAY=""))
        token=b"CART:" if name=="cart" else b"DISTRICT:"
        for platform,result in [("Windows PE / simulated Win32",win),("native Linux SDL",linux)]:
            print(platform,result.stderr.decode().strip())
            assert result.stderr.count(token)==required, (name,platform,result.stderr,result.stdout)

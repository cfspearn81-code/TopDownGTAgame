"""Play an in-game Safehouse purchase and its Signal receipts on both native clients.
Disposable saves only. Simulated Win32, actual ELF via SDL's dummy driver."""
import os
from pathlib import Path
import struct
import subprocess
import tempfile

root=Path(__file__).resolve().parents[2]
def fnv(data):
    h=2166136261
    for val in data:h=((h^val)*16777619)&0xffffffff
    return h
def inspect(p):
    b=p.read_bytes()
    word=lambda at:struct.unpack_from("<I",b,at)[0]
    assert len(b)==1044 and word(4)==11 and word(300)==fnv(b[:300]) and word(1040)==fnv(b[:1040])
    minute=word(432+31*4)&2047
    assert 540<=minute<545
    assert [word(432+4*i) for i in (31,35,46,49,50,51,52)] == [
        0x52560000|(3<<11)|minute,49,501,0x52560000|(1<<11)|minute,0,400,79]
    assert (word(8),word(136),word(148),word(288),word(292)) == (0,0,1,207,501)
    return b
def call(*args,**kw):
    return subprocess.run(args,check=True,timeout=120,**kw)
probe=r"""
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static void rr_screen(int at,const unsigned*rgb){
 static struct {int at;unsigned hash;char name[48];int done;} expected[4];static int loaded;
 if(!loaded){FILE*f=fopen(getenv("DC_RECEIPT_PROBES"),"r");assert(f);
  for(int i=0;i<4;i++)assert(fscanf(f,"%d%x%47s",&expected[i].at,&expected[i].hash,expected[i].name)==3);
  fclose(f);loaded=1;
 }
 for(int i=0;i<4;i++)if(!expected[i].done&&at>=expected[i].at-2&&at<=expected[i].at+3){
  unsigned h=2166136261u;
  for(int y=90;y<425;y++)for(int x=324;x<891;x++)h=(h^(rgb[y*960+x]&0xffffffu))*16777619u;
  if(h==expected[i].hash){fprintf(stderr,"MATCH %s\n",expected[i].name);expected[i].done=1;}
 }
}
"""
with tempfile.TemporaryDirectory(prefix="dc-revenue-receipts-") as td:
    tmp=Path(td);env=dict(os.environ,DC_TRIP_SEED="17",DC_RECEIPT_PROBES=str(tmp/"probes"))
    call("cc","-std=c11","-O1","-I",root/"src",root/"scripts/qa/revenue_receipts_plan.c","-o",tmp/"route")
    tape=call(tmp/"route",tmp/"fixture",env=env,stdout=subprocess.PIPE).stdout
    pe=(root/"scripts/check_pe.c").read_text().replace("static int frame,",probe+"\nstatic int frame,")
    pe=pe.replace(" frame++;\n if(atmTest"," rr_screen(frame+1,b);\n frame++;\n if(atmTest")
    (tmp/"pe.c").write_text(pe)
    call("cc","-O2",tmp/"pe.c","-o",tmp/"pe")
    sdl=(root/"scripts/qa/linux_replay.c").read_text().replace("../../src/",str(root/"src")+"/")
    (tmp/"sdl.c").write_text(sdl+probe+r"""
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){
 int(*real)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 assert(real&&pitch==960*4);rr_screen(frame,p);return real(t,r,p,pitch);
}
""")
    call("cc","-std=c11","-O2","-shared","-fPIC",tmp/"sdl.c","-l:libSDL2-2.0.so.0","-ldl","-o",tmp/"sdl.so")
    first=call(tmp/"pe",root/"build/DeadChannel.exe","--tour",input=tape,env=dict(env,
        DC_TRIP_LOAD=str(tmp/"fixture"),DC_TRIP_SAVE=str(tmp/"win"),DC_TRIP_BACKUP=str(tmp/"winback")),
        capture_output=True)
    assert first.stderr.count(b"MATCH ")==4,first.stderr
    data=tmp/"xdg/DeadChannel";data.mkdir(parents=True)
    (data/"career.dat").write_bytes((tmp/"fixture").read_bytes())
    second=call(root/"build/DeadChannel",input=tape,env=dict(env,
        XDG_DATA_HOME=str(data.parent),SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",SDL_AUDIODRIVER="dummy",
        LD_PRELOAD=str(tmp/"sdl.so"),DC_LINUX_REPLAY="",DC_LINUX_SNAPSHOT=str(tmp/"lin")),
        capture_output=True)
    assert second.stderr.count(b"MATCH ")==4,second.stderr
    assert inspect(tmp/"win")==inspect(tmp/"lin")
    assert (tmp/"winback").read_bytes()==next(data.glob(".pre-afterhours-*.bak")).read_bytes()==(tmp/"fixture").read_bytes()
    print(f"PASS: {tape.split()[0].decode()} frames, both clients matched four Signal panels and byte-identical expanded receipts; both kept the original compact career.")

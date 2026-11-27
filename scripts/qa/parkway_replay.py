"""Observe a live 06:00 closure on the exact SDL launcher's atlas.
Uses an isolated synthetic career by the west barricade, never player saves.
Usage: python3 scripts/qa/parkway_replay.py build/DeadChannel"""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix="crossroads-parkway-") as tmp:
    tmp = Path(tmp)
    def run(args, **kw):
        return subprocess.run(args, check=True, timeout=90, **kw)
    seed = tmp/"seed.c"
    seed.write_text(r'''
#define DC_LINUX_NATIVE
#include "game.c"
#include "linux_save.h"
#include <assert.h>
int main(void) {
 assert(linux_disk_init()&&linux_disk_lock());init_city();starter_save();
 sv.intro=4;sv.dayClock=359.55f;scene=1;p.x=2250;p.y=1024;p.car=-1;p.hp=100;
 disk_slot(0);request_save();assert(saveFlash>0);linux_disk_shutdown();
}
''')
    env = dict(os.environ, XDG_DATA_HOME=str(tmp/"saves"), SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy", SDL_RENDER_DRIVER="software")
    run(["cc", "-std=c11", "-O0", "-I", root/"src", seed, root/"src/linux_save.c", "-o", tmp/"seed"])
    run([tmp/"seed"], env=env)
    code = (root/"scripts/qa/linux_replay.c").read_text().replace("../../src/",str(root/"src")+"/")
    anchor = 'if(!remaining)assert(fscanf(plan,"%u %u",&remaining,&mask)==2&&remaining);'
    assert code.count(anchor)==1
    code = code.replace(anchor, r'''
  if(frame==6)click(renderer,668,199,0); /* Parkway Overlook pin */
  if(frame==12||frame==48){
   int purple=0;
   /* Inside the direct Parkway corridor, not on the player or destination. */
   for(int x=617;x<=631;x++)for(int y=197;y<=202;y++)purple+=margin(renderer,x,y)==0xc68aff;
   fprintf(stderr,"Native atlas direct corridor: frame %u, purple count %d\n",frame,purple);
   if(frame==12)assert(purple>=35);else assert(purple==0);
  }
''' + anchor)
    (tmp/"replay.c").write_text(code)
    run(["cc","-std=c11","-O2","-fPIC","-shared",tmp/"replay.c","-ldl","-o",tmp/"replay.so"])
    # Start, load the career, open M. Do not move, close/reopen, or re-pin it.
    tape=b"55 0\n2 0\n1 1024\n1 0\n1 256\n50 0\n"
    run([exe],input=tape,env=dict(env,LD_PRELOAD=str(tmp/"replay.so"),DC_LINUX_REPLAY=""))
    print("Exact Linux executable: the stationary player's purple GPS rerouted on the live atlas at 06:00.")

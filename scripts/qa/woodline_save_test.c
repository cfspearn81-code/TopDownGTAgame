/* Launch twice with the same empty XDG_DATA_HOME: write, then read. */
#define DC_LINUX_NATIVE
#include "../../src/game.c"
#include "../../src/linux_save.h"
#include <assert.h>
#include <stdio.h>
int main(int argc,char **argv){
 assert(argc==2&&linux_disk_init()&&linux_disk_lock());
 init_city();profiles_activate();
 OuterPoint at=woodline_sites[WOOD_MEADOW];
 assert(outer_walkable(at.x,at.y,10)&&!outer_area(at.x,at.y,10));
 if(argv[1][0]=='w'){
  sv.intro=4;sv.cash=161;scene=1;p.x=at.x+.375f;p.y=at.y+.625f;
  request_save();assert(saveFlash>0);
  Save11 b;assert(disk_read(&b,sizeof b));
  assert(b.base.x<2013&&b.base.y<2013);
  assert((b.night.world[NW_SAVE_X]&0x7fffffff)==(U)(p.x*64+.5f));
  assert(b.night.world[NW_SAVE_Y]==(U)(p.y*64+.5f));
 }else{
  assert(wason&&sv.cash==161&&p.x==at.x+.375f&&p.y==at.y+.625f);
  assert(p.car<0&&street_foot_clear(p.x,p.y,-1));
  puts("PASS: fresh Linux process resumes exact fractional off-road forest location; compatible city fallback preserved.");
 }
 linux_disk_shutdown();return 0;
}

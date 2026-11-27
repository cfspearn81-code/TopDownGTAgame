/* Actual combined pause/wardrobe, Shade checkout and POSIX save layer.
 * Build -DDC_LINUX_NATIVE and run with a disposable XDG_DATA_HOME. */
#include "../../src/game.c"
#include "../../src/linux_save.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static void key(U k){tick(k);tick(0);}
static void reinit(void){
 linux_disk_shutdown();assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);
 init_city();assert(wason);key(ENTER);assert(scene==1);
}
int main(void){
 assert(getenv("XDG_DATA_HOME"));assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);
 init_city();assert(!wason);sv.intro=4;sv.medPatches=9;sv.cash=400;sv.bank=123;
 sv.look.part[HAIR]=3;
 scene=1;menu=-1;mapOn=0;
 p=(Player){.x=OC_GEAR_X,.y=OC_GEAR_Y,.a=1.57f,.hp=85,.car=-1};
 assert(!solid(p.x,p.y,10));
 nv.world[NVH_DEED]=3;nv.world[NVK_BUYS]=0x2au;nv.garage[WF_FLAGS]=5u;
 request_save();assert(saveFlash>0);reinit();
 assert(nvs_penthouse()&&nvs_renovated()&&nvk_owned(1)&&nvk_owned(3)&&nvk_owned(5));
 assert(!ward_live()&&!night_gear_mask()&&sv.medPatches==9);
 key(ENTER);assert(scene==OPEN_CITY_GEAR_SCENE);
 key(ENTER);assert(sg_arm&&sv.cash==400);
 key(ENTER);assert(night_gear_mask()&&sv.cash==310);key(ESC);assert(scene==1);
 key(ESC);key(UP);assert(opt[sel].action==CW_PAUSE_OPTION);
 key(ENTER);key(WHEEL);key(RIGHT);key(RIGHT);key(DOWN);key(RIGHT);key(ENTER);
 assert(scene==1&&ward_hat(ward_live())==2&&night_gear_mask()&&sv.cash==310);
 Save11 onDisk;assert(disk_read(&onDisk,sizeof onDisk));
 assert(onDisk.hash==night_hash(&onDisk,sizeof onDisk-4));
 assert(onDisk.base.hash==night_hash(&onDisk.base,sizeof onDisk.base-4));
 assert(onDisk.base.medPatches==9&&onDisk.base.hp==85&&onDisk.base.look.part[HAIR]==3);
 assert(onDisk.night.garage[WF_FLAGS]==5);
 reinit();assert(ward_live()==(2u|(1u<<4))&&night_gear_mask());
 assert((nv.world[NVH_DEED]&3u)==3u&&(nv.world[NVK_BUYS]&255u)==0x2au);
 assert(nv.garage[WF_FLAGS]==5&&sv.cash==310&&sv.bank==123&&sv.medPatches==9&&p.hp==85);
 key(ENTER);assert(scene==OPEN_CITY_GEAR_SCENE);key(ENTER);key(ESC);
 assert(!night_gear_mask()&&sg_owns(SG_MASK)&&ward_live()==(2u|(1u<<4))&&sv.cash==310);
 reinit();assert(!night_gear_mask()&&sg_owns(SG_MASK)&&ward_live()==(2u|(1u<<4)));
 assert((nv.world[NVH_DEED]&3u)==3u&&(nv.world[NVK_BUYS]&255u)==0x2au&&nv.garage[WF_FLAGS]==5u);
 linux_disk_shutdown();
 puts("Native disk: existing penthouse/Pulse/forest/meds + authentic wardrobe and Shade controls kept separate before and after both relaunches.");
 return 0;
}

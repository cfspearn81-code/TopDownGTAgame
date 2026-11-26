/* Run writer and reader separately with the same empty XDG_DATA_HOME. */
#define DC_LINUX_NATIVE
#include "game.c"
#include "linux_save.h"
#include <assert.h>
#include <stdio.h>
static void press(U k){tick(k);tick(0);}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 assert(linux_disk_init()&&linux_disk_lock());init_city();profiles_activate();
 if(argv[1][0]=='w'){
  scene=1;sv.cash=1800;sv.intro=4;sv.home=1;sv.gear=1;sv.stocks[0]=3;
  int x,y;sv.dayClock=1150.f;assert(night_venue_vendor_pos(&x,&y));p.x=x;p.y=y;press(USE);assert(nvs_back==1);press(USE);
  press(RIGHT);press(DOWN);press(USE);press(RIGHT);press(DOWN);press(USE);press(ESC);
  walkin_open(NVH_GLASS_CROWN);press(ENTER);press(USE);press(LEFT);press(USE);
  press(RIGHT);press(DOWN);press(USE);press(RIGHT);press(USE);press(ESC);
  walkin_open(nvs_aquaria[0]);press(ENTER);press(USE);assert(nvs_stage()==1);
  request_save();assert(saveFlash>0);
 }else{
  assert(wason&&nvs_stage()==1&&nvs_penthouse()&&nvs_renovated());
  assert(nvk_frame()==1&&nvk_glass()==1&&nvk_kind()==1&&nvk_load()==6&&nvk_reserve(0)==6);
  assert(nvk_cfg(NVK_CASE_BIT,3)==1&&nv.world[NVS_MOUNT]);
  assert(nv.world[NVS_SHELF+1]==1&&sv.stocks[0]==2);
  assert(sv.cash==1800-175-62-28-NVH_PRICE-NVH_RENOVATE-3);
  puts("PASS separate-process Linux restart: aquarium progress, renovated penthouse, graded inventory, kit upgrades, charge type/load/reserve, mounted snapshot and wallet.");
 }
 linux_disk_shutdown();return 0;
}

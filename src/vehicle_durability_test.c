/* Real game physics, damage thresholds, recovery/repair and full prior save
 * compatibility. Uses the shipped POSIX writer in a caller-created isolated
 * XDG directory. Build with -DDC_LINUX_NATIVE and link src/linux_save.c. */
#include "game.c"
#include "linux_save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void ready(void){
 starter_save();scene=1;menu=-1;mapOn=0;fastGfx=0;end_job();vd_reset();
 ng_reset();night_world_reset();clear_input();bank_ui_reset();outer_life_reset();
 sv.intro=4;sv.hp=100;sv.cash=420;p=(Player){.x=512,.y=1024,.car=-1,.hp=100};
 nc=3;z(cars,sizeof cars);z(streetDrivers,sizeof streetDrivers);
 for(int i=0;i<LEN(cars);i++)cars[i].x=cars[i].y=-100;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
}
static void same_barrier(int type,int role){
 ready();cars[1]=(Car){512,1024,0,0,type,role,100};
 cars[2]=(Car){1390,1024,1.56f,0,1,0,100};p.car=1;
 int i;float top=specs[type].vmax;int raw=cl((int)(top*top/1450),1,37);
 for(i=0;i<520&&vehicle_hp(cars+1)==300;i++){tick(UP);assert(!street_overlap(cars+1,cars+2));}
 assert(i<520&&vehicle_hp(cars+1)==300-2*raw&&cars[1].s==-top*.27f&&p.car==1&&p.hp==100);
 if(type==3){
  char want[75],display[35];vehicle_condition_text(display,cars+1);
  snprintf(want,sizeof want,"CRASH  /  CONDITION %s",display);assert(!strcmp(want,notice));
  assert(!strcmp(display,"80.7"));
 }
 printf("%-9s role=%d impact frame=%d raw=%d legacy=%d current=%.3f impulse=%.3f\n",specs[type].name,role,i,raw,100-raw,vehicle_hp(cars+1)/3.0,cars[1].s);
}
static void threshold(void){
 ready();Car sample={.cond=100};char out[20];vehicle_impact(&sample,1);vehicle_condition_text(out,&sample);
 assert(!strcmp(out,"99.3")&&sample.cond==100);
 for(int i=2;i<=149;i++){vehicle_impact(&sample,1);assert(vehicle_hp(&sample)==300-i*2&&sample.cond>0);}
 vehicle_impact(&sample,1);assert(!vehicle_hp(&sample)&&!sample.cond&&!sample.wear);
 cars[1]=(Car){512,1024,0,28,0,2,1};vehicle_set_health(cars+1,1);p.car=1;
 cars[2]=(Car){544,1024,0,0,1,0,100};for(int i=0;i<20&&p.car>=0;i++)tick(UP);
 assert(!vehicle_hp(cars+1)&&p.car<0&&p.hp==73&&strstr(notice,"Wrecked"));
}
static void save_and_repair(void){
 ready();disk_slot(0);sv.owned=17;sv.selection=0;sv.carCond[0]=69;sv.carCond[4]=79;sv.vehicleCond=69;
 sv.x=p.x;sv.y=p.y;sv.hash=savehash();Save orig=sv;
 assert(disk_write(&orig,sizeof orig));
 load_save();assert(wason&&sv.rev==10&&vehicle_saved_hp(0)==207&&vehicle_saved_hp(4)==237&&!nv.garage[61]);
 p.x=orig.x;p.y=orig.y;request_save();
 Save unchanged;assert(disk_read(&unchanged,sizeof unchanged)&&!memcmp(&orig,&unchanged,sizeof unchanged));
 scene=1;spawnPlayer();assert(vehicle_hp(cars)==207);
 vehicle_impact(cars,1);sv.vehicleCond=cars[0].cond;vehicle_store_health(0,cars);
 Car stored={.cond=79};vehicle_impact(&stored,2);vehicle_store_health(4,&stored);
 request_save();Save11 revised;
 assert(disk_read(&revised,sizeof revised)&&revised.base.rev==11&&revised.night.garage[61]==(2u|(1u<<8)));
 load_save();assert(wason&&vehicle_saved_hp(0)==205&&vehicle_saved_hp(4)==233);
 spawnPlayer();assert(vehicle_hp(cars)==205&&sv.carCond[0]==69&&cars[0].wear==2);
 p.x=1038;p.y=748;scene=1;lastCar=-1;doOpt(34);assert(sv.selection==4&&vehicle_hp(cars)==233);
 doOpt(30);assert(sv.selection==0&&vehicle_hp(cars)==205);
 /* Subpercent wear must not display or repair as already serviced. */
 vehicle_set_health(cars,298);vehicle_store_health(0,cars);
 active(3);makeMenu();int found=0;for(int i=0;i<nopt;i++)if(opt[i].action==35)found=1;
 int cash=sv.cash;assert(found);doOpt(35);
 assert(vehicle_hp(cars)==300&&vehicle_saved_hp(0)==300&&sv.vehicleCond==100&&sv.cash==cash-32);
 assert(vehicle_saved_hp(4)==233);
 /* In an invalid remainder field, keep the pre-existing whole points. */
 nv.garage[61]|=3u;assert(vehicle_saved_hp(0)==300);
}
int main(void){
 assert(linux_disk_init()&&linux_disk_lock());init_city();
 for(int i=0;i<LEN(specs);i++)same_barrier(i,i%3);
 threshold();save_and_repair();linux_disk_shutdown();
 puts("Vehicle integrity: exact 150% lifetime, original collision impulse, exact repair, pre-update save and independent saved wear passed.");return 0;
}

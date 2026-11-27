/* Geometry, graph, closure reroute, atlas selection, and safe career resume. */
#define _POSIX_C_SOURCE 200809L
#define DC_LINUX_NATIVE
#include "../../src/game.c"
#include "../../src/linux_save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void hit(U b){tick(b);tick(0);}
static void survey(void){
 int samples=0;Car car={.cond=100};
 for(int i=0;i<LEN(outerRoads);i++){
  const OuterRoad*r=outerRoads+i;
  OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];
  float dx=(float)b.x-a.x,dy=(float)b.y-a.y;
  car.a=look(dx,dy);
  for(int s=2;s<=80;s+=2){
   float t=s/80.f;car.x=a.x+dx*t;car.y=a.y+dy*t;
   if(car.x<2023&&car.y<2023)continue;
   assert(!street_hits_wall(&car));assert(!solid(car.x,car.y,10));samples++;
  }
 }
 assert(samples>1600);assert(solid(2998,2200,10)&&solid(2600,2780,10));
 printf("Outer Loop: %d actual drivable hull and foot samples across surveyed road segments.\n",samples);
}
static void map_route(void){
 scene=1;p.x=2250;p.y=1024;p.car=-1;menu=-1;sv.intro=4;mapOn=0;
 clear_input();hit(MAP);assert(mapOn&&nav_span()==ROOM);
 NavPoint q=nav_outer_point(outerStops[1].node);
 int sx0=NAV_MAP_X+(q.x*NAV_MAP_SIZE+ROOM-1)/ROOM,sy0=NAV_MAP_Y+(q.y*NAV_MAP_SIZE+ROOM-1)/ROOM;
 pointer_input(sx0,sy0,1);assert(navHasManual&&navManualLoc==NAV_OUT_BASE+1&&navManual.x==q.x);
 sv.dayClock=245;paint();assert(navCount>=2&&navDist<820);
 sv.dayClock=405;navGoalKind=0;paint();
 assert(navCount>=5&&navDist>800);
 for(int i=1;i<navCount;i++){
  NavPoint a=navNodes[i-1],b=navNodes[i];
  assert(nav_traversable(a,b,7)&&!night_phone_lane_stop(a.x,a.y,b.x,b.y,4));
 }
 hit(ESC);p.x=1450;p.y=1024;tick(0);hit(MAP);assert(nav_span()==BASE_ROOM);
 hit(ATLAS_WORLD);assert(nav_span()==ROOM);hit(ESC);
 puts("Outer Loop: real pointer pins, F8 atlas layers, detour preview, access graph and collision all agree.");
}
static void saved_position(void){
 starter_save();sv.intro=4;sv.owned=1;sv.cash=261;sv.dayClock=250;
 int x=2704,y=1824;
 scene=1;p.x=x;p.y=y;p.car=-1;p.hp=100;
 disk_slot(0);
 request_save();Save11 b;assert(disk_read(&b,sizeof b)&&b.base.rev==11);
 assert(b.base.x<2013&&b.base.y<2013&&valid_banking_save());
 assert(b.night.world[NW_SAVE_X]==(0x80000000u|(U)x*64u)&&b.night.world[NW_SAVE_Y]==(U)y*64u);
 starter_save();load_save();spawnCars();spawnPlayer();
 assert(wason&&sv.cash==261&&p.x==x&&p.y==y&&street_foot_clear(x,y,-1));
 assert(d2(cars[0].x-x,cars[0].y-y)<110*110);
 b.night.world[NW_SAVE_X]&=0x7fffffffu; /* Synthetic prior format, not player data. */
 b.hash=night_hash(&b,sizeof b-4);
 assert(disk_write(&b,sizeof b));
 request_save();assert(saveFlash>0);
 char name[512];snprintf(name,sizeof name,"%s/DeadChannel/.pre-outerloop-career.dat-r11-%08x.bak",getenv("XDG_DATA_HOME"),b.hash);
 FILE*f=fopen(name,"rb");Save11 prior;
 assert(f&&fread(&prior,1,sizeof prior,f)==sizeof prior&&fclose(f)==0);
 assert(!memcmp(&prior,&b,sizeof b));
 puts("Outer Loop: exact new-ground location resumes with nearby car, legacy fallback and one-time byte-identical Rev11 backup.");
}
int main(int argc,char**argv){
 assert(argc==2&&setenv("XDG_DATA_HOME",argv[1],1)==0&&linux_disk_init()&&linux_disk_lock());
 init_city();survey();map_route();saved_position();linux_disk_shutdown();
 return 0;
}

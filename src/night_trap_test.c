/* Native fixture with real Linux disk and renderer-independent simulation assertions.
 * Without --seed, it only uses an anonymous test-provided XDG_DATA_HOME. */
#include <stdio.h>
#include "game.c"
#include "linux_save.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
static void nt_test_enter(void){tick(0);tick(ENTER);tick(0);}
static void nt_test_key(U k){tick(0);tick(k);tick(0);}
static void nt_walk(U k,int n){for(int i=0;i<n;i++)tick(k);tick(0);}
static void nt_dump(const char*name){FILE*f=fopen(name,"wb");assert(f);paint();fprintf(f,"P6\n%d %d\n255\n",W,H);for(int i=0;i<W*H;i++){U a=pixels[i];fputc(a>>16,f);fputc(a>>8,f);fputc(a,f);}fclose(f);}
int main(int argc,char **argv){
 assert(getenv("XDG_DATA_HOME"));assert(linux_disk_init());assert(linux_disk_lock());disk_slot(0);init_city();scene=1;menu=-1;mapOn=0;p.dead=0;p.car=-1;p.x=NT_DOOR_X;p.y=NT_DOOR_Y;p.hp=100;
 assert(!solid(p.x,p.y,8));assert(!solid(p.x,p.y+14,8));assert(nav_traversable((NavPoint){174,944},(NavPoint){174,1024},8));
 nav_solve((NavPoint){loc[12].x,loc[12].y},(NavPoint){174,944});assert(navCount>=2&&navDist<1600);
 if(argc==2&&!strcmp(argv[1],"--seed")){
  /* This seeds only QA prerequisites: actual movement, buy, construction, storage,
   * interim restart and dispatch collection happen via the shipping Linux UI. */
  sv.cash=2500;sv.intro=4;nv.phone[NP_BAG+8]=6;request_save();assert(saveFlash>0);linux_disk_shutdown();return 0;
 }
 int saved_cash=sv.cash;sv.cash=2500;nv.phone[NP_BAG+8]=15;
 if(argc>1)nt_dump(argv[1]);
 night_phone_open();np_app=NP_WIRE;np_row=12;np_apply();assert(navHasManual&&navManualLoc==NAV_TRAP&&navManual.x==174&&navManual.y==944);
 np_row=0;np_apply();int club=night_club_listing();
 assert(club>=0&&navManualLoc==LEN(loc)+club&&navManual.x==ordinaryDoorX[club]&&navManual.y==ordinaryDoorY[club]);
 np_row=9;np_apply();assert(navManualLoc==LEN(loc)+club&&navManual.x==ordinaryDoorX[club]&&navManual.y==ordinaryDoorY[club]);
 night_phone_tick(0,ESC);
 mapOn=1;navReady=1;navigation_pointer(NAV_MAP_X+NT_DOOR_X*NAV_MAP_SIZE/BASE_ROOM,NAV_MAP_Y+NT_DOOR_Y*NAV_MAP_SIZE/BASE_ROOM,0);
 assert(navSelected==NAV_TRAP);mapOn=0;
 job.kind=JOB_COURIER;assert(!nt_door_ready()&&!night_trap_entry(ENTER));job.kind=-1;
 nt_test_enter();assert(scene==NIGHT_TRAP_SCENE&&nt_pane==NT_VISIT&&!night_trap_owned());
 nt_test_enter();assert(!night_trap_owned()&&nt_confirm==1);nt_test_enter();
 assert(night_trap_owned()&&sv.cash==2020&&nt_pane==NT_BUILD&&!sv.b[0].own);
 nt_test_enter();nt_test_enter();assert(nt_level(NT_RACK)==1&&nt_capacity()==18);
 night_phone_heat_add(NT_DOOR_X,NT_DOOR_Y,80);assert(nt_overhead()==15);
 nt_test_key(DOWN);nt_test_enter();nt_test_enter();assert(nt_level(NT_SHUTTER)==1&&nt_overhead()==7);
 nt_test_key(DOWN);nt_test_enter();nt_test_enter();
 assert(nt_level(NT_RELAY)==1&&nt_delay(1)==20&&nt_income(1)==63&&nt_delay(2)==38&&nt_income(2)==96);
 nt_test_key(ESC);assert(nt_pane<0);
 if(argc>2)nt_dump(argv[2]);
 nt_walk(BRAKE|UP|LEFT,61);nt_walk(BRAKE|LEFT,42);nt_test_key(USE);assert(nt_pane==NT_STOCK);
 nt_test_key(UP);assert(nt_sel==8);
 for(int i=0;i<12;i++)nt_test_key(USE);
 assert(nt_count(8)==12&&!nt_count(7)&&nv.phone[NP_BAG+8]==3);
 nt_test_key(FIRE);assert(nt_count(8)==11&&nv.phone[NP_BAG+8]==4);nt_test_key(ESC);
 nt_walk(BRAKE|RIGHT,167);nt_test_key(USE);assert(nt_pane==NT_DISPATCH);
 nt_test_key(DOWN);nt_test_enter();nt_test_enter();assert(nt_ticket()==2&&nt_clock()>2200);request_save();
 int remaining=nt_clock();linux_disk_shutdown();assert(linux_disk_init());assert(linux_disk_lock());disk_slot(0);init_city();
 assert(wason&&night_trap_owned()&&nt_capacity()==18&&nt_count(8)==11&&nt_level(NT_SHUTTER)==1&&nt_level(NT_RELAY)==1);
 assert(nt_ticket()==2&&nt_clock()==remaining&&nv.phone[NP_BAG+8]==4);
 scene=1;nt_test_enter();assert(scene==NIGHT_TRAP_SCENE);
 int cash=sv.cash;for(int i=0;i<remaining;i++)night_trap_world_tick(1.f/60.f);
 assert(nt_clock()==0&&nt_ticket()==2);
 nt_open(NT_DISPATCH);nt_sel=2;nt_test_enter();assert(nt_jobs()==1&&nt_ticket()==0&&sv.cash==cash+96);
 assert(sv.dirty==0);if(argc>3)nt_dump(argv[3]);
 (void)saved_cash;linux_disk_shutdown();
 puts("PASS Trap House: distinct entrance, phone/atlas/club pins, deed, 3 upgrade effects, exact grades, physical room controls, real Linux disk restart, retained timer and correct payout.");
 return 0;
}

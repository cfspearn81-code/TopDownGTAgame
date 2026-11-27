/* Native input, fixture-backed Linux save/read/reload and scoped stats. */
#include <stdio.h>
#include "game.c"
#include "linux_save.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static const U shade_predecessor=(1u<<30)|0x4du;
static void sg_tap(U k){tick(0);tick(k);tick(0);}
static void sg_stand(int x,int y){scene=1;menu=-1;mapOn=0;p.x=x;p.y=y;p.car=-1;p.hp=100;p.dead=0;p.heat=0;}
static void sg_restart(void){
 request_save();assert(saveFlash>0);
 linux_disk_shutdown();assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);
 init_city();assert(wason);scene=1;menu=-1;
}
static void sg_dump(const char *name){
 if(!name)return;paint();FILE*f=fopen(name,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U a=pixels[i];fputc(a>>16,f);fputc(a>>8,f);fputc(a,f);}assert(!fclose(f));
}
static void sg_shop_map(const char*snap){
 sg_stand(OC_GEAR_X,OC_GEAR_Y);
 assert(!solid(p.x,p.y,10)&&nav_traversable((NavPoint){512,512},(NavPoint){OC_GEAR_X,OC_GEAR_Y},10));
 assert(ordinaryDoorX[13]==608&&ordinaryDoorY[13]==464);
 sg_tap(PHONE);sg_tap(LEFT);assert(scene==NIGHT_PHONE_SCENE&&np_app==NP_PREF&&np_rows()==5);
 sg_tap(UP);assert(np_row==4);sg_tap(ENTER);assert(navHasManual&&navManualLoc==NAV_GEAR&&navManual.x==OC_GEAR_X&&navManual.y==OC_GEAR_Y);sg_tap(ESC);
 mapOn=1;int span=nav_span();navigation_pointer(NAV_MAP_X+OC_GEAR_X*NAV_MAP_SIZE/span,NAV_MAP_Y+OC_GEAR_Y*NAV_MAP_SIZE/span,0);
 assert(navSelected==NAV_GEAR);mapOn=0;
 job.kind=JOB_COURIER;assert(!night_gear_enter(ENTER));job.kind=-1;
 p.heat=2;assert(!night_gear_enter(ENTER));p.heat=0;
 sg_dump(snap);
 sg_tap(ENTER);assert(scene==OPEN_CITY_GEAR_SCENE);
}
static void sg_browsing(const char*snap){
 int cash=sv.cash;
 sv.cash=89;sg_tap(ENTER);sg_tap(ENTER);assert(!sg_owns(SG_MASK)&&sv.cash==89);
 sv.cash=cash;
 sg_tap(ENTER);assert(sg_arm&&sv.cash==cash&&!sg_owns(SG_MASK));
 sg_tap(DOWN);assert(!sg_arm);sg_tap(UP);sg_tap(ENTER);assert(sg_arm&&sv.cash==cash);sg_tap(ENTER);
 assert(night_gear_mask()&&sv.cash==cash-90);
 sg_tap(DOWN);sg_tap(USE);assert(sg_arm&&!sg_owns(SG_HUSH));sg_tap(USE);
 assert(night_gear_hush()&&sv.cash==cash-210);
 sg_tap(DOWN);sg_tap(ENTER);sg_tap(ENTER);
 assert(night_gear_lamp()&&sv.cash==cash-248);
 sg_dump(snap);
 sg_tap(PHONE);assert(scene==NIGHT_PHONE_SCENE&&np_return==OPEN_CITY_GEAR_SCENE);sg_tap(ESC);
 assert(scene==OPEN_CITY_GEAR_SCENE&&sg_row==SG_LAMP);
 sg_tap(ENTER);assert(!night_gear_lamp()&&sg_owns(SG_LAMP)&&sv.cash==cash-248);
 sg_tap(ENTER);assert(night_gear_lamp());sg_tap(ESC);
}
static void sg_clean_ping(int expect){
 int addr=12;
 scene=1;sg_stand(loc[addr].x,loc[addr].y);
 nv.phone[NP_JOB]=2;nv.phone[NP_STREAK]=0;nv.phone[NP_LASTSALE]=0;nv.phone[NP_BAG]=2;
 int district_x=loc[addr].x,district_y=loc[addr].y;
 nv.phone[np_heat_index(np_area(district_x,district_y))]=0;
 assert(night_phone_action(USE)&&np_job()<0);
 assert(night_phone_heat_at(district_x,district_y)==expect&&p.heat==0.f&&nv.phone[NP_BAG]==0);
}
static void sg_isolation(void){
 assert(night_gear_mask()&&sg_left()==0);
 sg_clean_ping(0);assert(sg_left()==SG_COOL_TICKS&&sg_notice_time>0);
 /* Paired toggle does not refill a previously used charge. */
 int t=sg_left();sg_stand(OC_GEAR_X,OC_GEAR_Y);
 sg_tap(ENTER);sg_tap(ENTER);assert(!night_gear_mask()&&sg_left()<t&&sg_left()>t-8);
 sg_tap(ENTER);assert(night_gear_mask()&&sg_left()>t-8);sg_tap(ESC);
 t=sg_left();sg_restart();assert(night_gear_mask()&&night_gear_hush()&&night_gear_lamp()&&sg_left()==t);
 assert((nv.world[OC_GEAR_WORD]&~((U)SG_USED_MASK<<OC_GEAR_SHIFT))==shade_predecessor&&sv.medPatches==3);
 sg_clean_ping(7);
 for(int i=0;i<SG_COOL_TICKS;i++)night_gear_world(1.f/60.f);assert(!sg_left());
 nv.phone[np_heat_index(np_area((int)p.x,(int)p.y))]=0;
 assert(night_gear_witness_delta((int)p.x,(int)p.y,14)==14&&!sg_left());
 heat(18);float before=p.heat;assert(night_gear_witness_delta((int)p.x,(int)p.y,7)==7&&p.heat==before&&!sg_left());
 for(int i=0;i<120;i++)dispatch_tick(1.f/60.f);
 assert(dispatch_engaged()>0);
 p.heat=0;assert(night_gear_witness_delta((int)p.x,(int)p.y,7)==7&&!sg_left());
 /* Ordinary Pulse stats are unchanged even for an equipped Hush module. */
 assert(absf(night_gear_trail(.11f)-.0506f)<.0001f);
 sg_stand(512,512);p.a=0.f;p.shot=0;sv.gear=0;wheelFists=0;pulse_guard_reset();
 for(int i=0;i<LEN(peds);i++)peds[i].hp=0; /* restrained test lane */
 shoot();assert(p.heat==24.f&&p.shot==.33f&&absf(p.trail-.0506f)<.0001f);
 sg_write((U)sg_bits()&~(2u<<(2*SG_HUSH)));
 assert(night_gear_trail(.11f)==.11f);
}
int main(int argc,char**argv){
 assert(getenv("XDG_DATA_HOME"));assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);init_city();
 sv.intro=4;sv.cash=500;sg_stand(OC_GEAR_X,OC_GEAR_Y);
 if(argc==2&&!strcmp(argv[1],"--seed")){
  nv.world[OC_GEAR_WORD]=shade_predecessor;request_save();assert(saveFlash>0);
  linux_disk_shutdown();return 0;
 }
 request_save();Save compact;assert(disk_read(&compact,sizeof compact)&&compact.rev==10);
 for(int i=0;i<20;i++)night_gear_world(1.f/60.f);assert(!sg_bits());
 sv.medPatches=3;sg_restart();assert(sv.medPatches==3&&disk_read(&compact,sizeof compact)&&compact.rev==11&&!sg_bits());
 nv.world[OC_GEAR_WORD]=shade_predecessor;sg_restart();
 assert(!sg_bits()&&nv.world[OC_GEAR_WORD]==shade_predecessor);
 sg_shop_map(argc>1?argv[1]:NULL);
 sg_browsing(argc>2?argv[2]:NULL);
 sg_isolation();
 linux_disk_shutdown();
 puts("PASS Shade Supply: actual store/atlas/Signal, clean prices and one-time ownership, three fit toggles across disk reload, old 304/1044 compatibility and unchanged Pulse ownership, one-shot abstract Witness charge, active pursuit and Pulse combat unchanged.");
 return 0;
}

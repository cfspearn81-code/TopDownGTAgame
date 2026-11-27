/* Exercises the playable scene through the ordinary engine tick/paint paths.
 * Setup warps to doors and supplies a stock/credit fixture only. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void vpress(U key){tick(key);tick(0);}
static void capture(const char *name){
 paint();const char *path=getenv("DC_VENUE_CAPTURES");if(!path)return;
 char fn[1024];snprintf(fn,sizeof fn,"%s/%s.ppm",path,name);FILE*f=fopen(fn,"wb");assert(f);
 fprintf(f,"P6\n%d %d\n255\n",W,H);for(int i=0;i<W*H;i++){U p=pixels[i];fputc(p>>16,f);fputc(p>>8,f);fputc(p,f);}fclose(f);
}
static void test_crown(void){
 sv.cash=2000;sv.home=0;
 walkin_open(NVH_GLASS_CROWN);vpress(ENTER);assert(nvs_room==NVR_LEASE);capture("lease");
 vpress(USE);assert(!nvs_penthouse()&&sv.cash==2000&&nvs_room==NVR_LEASE);
 sv.home=1;vpress(USE);assert(nvs_penthouse()&&nvs_room==NVR_HOME&&sv.cash==2000-NVH_PRICE);
 vpress(LEFT);assert(nvs_tab==4);vpress(USE);assert(nvs_renovated()&&sv.cash==2000-NVH_PRICE-NVH_RENOVATE);
 vpress(USE);assert(sv.cash==2000-NVH_PRICE-NVH_RENOVATE);
 capture("penthouse");vpress(ESC);assert(scene==4);
}
static void visit_van(void){
 int x,y;scene=1;p.car=-1;sv.dayClock=1150.f;
 assert(night_venue_vendor_pos(&x,&y));p.x=x;p.y=y;vpress(USE);assert(scene==10&&nvs_room==NVR_CART&&nvs_tab==1);
}
static void test_schedule(void){
 const float slots[]={1140,1290,0,150};int x[4],y[4],a,b;
 for(int i=0;i<4;i++){
  sv.dayClock=slots[i];assert(night_venue_vendor_pos(x+i,y+i));
  for(int j=0;j<i;j++)assert(x[j]!=x[i]||y[j]!=y[i]);
 }
 sv.dayClock=300;assert(!night_venue_vendor_pos(&a,&b));
 sv.dayClock=1139;assert(!night_venue_vendor_pos(&a,&b));
 sv.dayClock=1140;scene=1;p.x=x[0];p.y=y[0];p.car=0;assert(!night_venue_vendor_entry(USE));
 p.car=-1;p.x+=45;assert(!night_venue_vendor_entry(USE));p.x-=45;
 assert(!night_venue_vendor_entry(ENTER));
}
static void test_cart(void){
 sv.cash=2000;sv.gear=-1;int a,b;sv.dayClock=899;assert(!night_venue_vendor_pos(&a,&b));visit_van();
 vpress(USE);assert(!nvk_frame()&&sv.cash==2000);sv.gear=1;
 vpress(DOWN);vpress(USE);assert(nvk_frame()==2&&nvk_owned(2)&&sv.cash==1790&&nvk_load()==6);
 vpress(USE);assert(sv.cash==1790);
 vpress(RIGHT);vpress(DOWN);vpress(DOWN);vpress(USE);assert(nvk_glass()==2&&nvk_cap()==14&&sv.cash==1695);
 vpress(RIGHT);vpress(DOWN);vpress(USE);assert(nvk_kind()==1&&nvk_load()==6&&nvk_reserve(0)==6&&sv.cash==1667);
 vpress(USE);assert(nvk_load()==12&&sv.cash==1639);capture("cells");
 vpress(ESC);assert(scene==1);
 scene=1;p.car=-1;p.shot=0;shoot();
 assert(nvk_load()==10&&trailInk==nvk_cell_ink[1]&&p.shot>.10f&&p.shot<.12f);
 int r=0,d=0;float c=0;U i=0;nvk_stats(&r,&d,&c,&i);assert(r==112&&d==2&&c>.10f&&c<.12f&&i==nvk_cell_ink[1]);
 nvk_put(1,5);vpress(LOADOUT);assert(nvk_load()==14&&nvk_reserve(1)==1&&p.shot>.55f);
 nvk_set(NVK_LOAD_BIT,15,1);float heat=p.heat; p.shot=0; shoot();assert(nvk_load()==1&&p.heat==heat);
 visit_van();vpress(FIRE);assert(!nvk_frame());
 p.shot=0;shoot();assert(trailInk==GOLD);
 vpress(USE);assert(nvk_frame()==1&&nvk_owned(1));vpress(ESC);
 open_interior(12);vpress(ENTER);vpress(RIGHT);vpress(USE);assert(nvk_cfg(NVK_CASE_BIT,3)==1&&nvk_cfg(NVK_CASE_LENS_BIT,3)==2);
 capture("display");vpress(ESC);
 p.heat=0;
}
static void test_afterglow(void){
 open_interior(12);vpress(ENTER);vpress(RIGHT);vpress(RIGHT);vpress(RIGHT);
 vpress(USE);assert(take_radio_record()>0);
 vpress(DOWN);int cash=sv.cash;sv.cash=0;vpress(USE);assert(!nv.world[NVS_ECHO]&&!take_radio_record());
 sv.cash=cash;vpress(USE);assert(take_radio_record()==18&&sv.cash==cash-NVF_NIGHTCAP_PRICE);
 vpress(DOWN);
#if __has_include("night_phone.inc")
 int held=np_get(NP_BAG,99);
 nv.phone[NP_BAG]=0;vpress(USE);assert(!nv.world[NVF_EXTRA]);
 nv.phone[NP_BAG]=1;vpress(USE);assert(!np_get(NP_BAG,99));
 nv.phone[NP_BAG]=held;
#else
 vpress(USE);
#endif
 assert(nv.world[NVF_EXTRA]>0&&take_radio_record()==18);capture("leaf");
 vpress(DOWN);vpress(USE);assert(!nv.world[NVF_EXTRA]&&!nv.world[NVS_ECHO]&&!nv.world[NVS_SURF]&&!nv.world[NVS_TOL]);vpress(ESC);
}
int main(void){
 init_city();profiles_activate();assert(scene==3&&interior_active_loc==12);
 vpress(ENTER);assert(scene==10&&nvs_room==NVR_HOME);capture("loft");vpress(MAP);assert(navHasManual&&navManualLoc>=LEN(loc));
 #if __has_include("night_phone.inc")
 nv.phone[NP_BAG+8]=2;vpress(TAXI_NEXT);assert(nvs_folio);vpress(UP);assert(nvs_sel==8);
 vpress(USE);assert(nvs_ed_count(8)==1&&np_get(NP_BAG+8,99)==1&&!nvs_ed_count(7));
 vpress(FIRE);assert(!nvs_ed_count(8)&&np_get(NP_BAG+8,99)==2);vpress(TAXI_NEXT);assert(!nvs_folio);
#endif
 vpress(USE);assert(!nv.world[NVS_SHELF]&&!sv.stocks[0]);sv.stocks[2]=2;sv.cash=100;
 for(int i=0;i<5;i++)vpress(DOWN);vpress(USE);
 assert(sv.stocks[2]==1&&nv.world[NVS_SHELF+5]==1&&sv.cash==97);
 vpress(FIRE);assert(sv.stocks[2]==2&&!nv.world[NVS_SHELF+5]);
 vpress(RIGHT);assert(nvs_tab==1);vpress(MAP);int px,py;if(night_venue_vendor_pos(&px,&py)){assert(nav_cart_tracking()&&navManual.x==px&&navManual.y==py);}else assert(navManualLoc==LEN(loc)+nvs_cart_here());vpress(USE);assert(!nv.world[NVS_MOUNT]);
 sv.gear=1;vpress(USE);assert(nv.world[NVS_MOUNT]&&!nv.world[NVS_FINISH]);
 vpress(DOWN);vpress(USE);assert(!nv.world[NVS_FINISH]);
 vpress(RIGHT);float before=sv.dayClock;vpress(DOWN);vpress(USE);assert(sv.dayClock>before+479);
 vpress(RIGHT);vpress(USE);assert(nv.world[NVS_SURF]>0&&night_venues_walk_factor()>1.f);
 int time1=nv.world[NVS_SURF];vpress(USE);assert(nv.world[NVS_SURF]<time1);vpress(DOWN);vpress(USE);assert(nv.world[NVS_ECHO]>0);
 vpress(ESC);assert(scene==3&&interior_active_loc==12);
 open_interior(9);sv.home=0;vpress(ENTER);assert(scene==3);sv.home=1;vpress(ENTER);assert(scene==10);vpress(ESC);
 sv.intro=4;walkin_open(nvs_cart_here());vpress(ENTER);assert(scene==10&&nvs_room==NVR_CART);capture("signal");
 before=sv.cash;vpress(USE);assert(nvs_has(1)&&sv.cash==(int)before-24);vpress(USE);assert(sv.cash==(int)before-24);vpress(ESC);
 open_interior(12);vpress(ENTER);vpress(RIGHT);vpress(DOWN);vpress(USE);assert(nvs_finish_id()==1);
 vpress(FIRE);assert(!nv.world[NVS_MOUNT]);vpress(ESC);
 walkin_open(74);vpress(ENTER);assert(scene==10&&nvs_tab==3);capture("shutter");
 vpress(USE);assert(nvs_stage()==1);vpress(RIGHT);assert(!nvs_tab);vpress(USE);assert(nvs_mode==1);
 vpress(USE);assert(nvs_stage()==1);while(absf(nvs_wrasse(nvs_frames+1)-475)>42)tick(0);vpress(USE);assert(nvs_stage()==2);
 vpress(RIGHT);vpress(USE);assert(nvs_mode==2);capture("jelly");
 vpress(USE);vpress(RIGHT);vpress(USE);vpress(USE);vpress(USE);vpress(RIGHT);vpress(USE);vpress(USE);assert(nvs_stage()==3);
 vpress(RIGHT);vpress(USE);assert(nvs_mode==3);capture("rays");vpress(UP);assert(nvs_step==0);
 vpress(LEFT);vpress(RIGHT);vpress(UP);assert(nvs_stage()==4);vpress(RIGHT);before=sv.cash;vpress(USE);assert(nvs_stage()==5&&sv.cash==(int)before+75);
 capture("eastwing");vpress(USE);assert(sv.cash==(int)before+75);
 vpress(FIRE);assert(nvs_mode==4);vpress(USE);assert(nvs_bonus==1);
 for(int q=1;q<4;q++){for(int i=0;i<74;i++)tick(RIGHT);tick(0);vpress(USE);}
 assert(nvs_bonus==15&&(nv.world[NVS_STAGE]&256u)&&sv.cash==(int)before+105);capture("promenade");
 vpress(USE);assert(sv.cash==(int)before+105);vpress(ESC);vpress(ESC);assert(scene==4&&walkin_active==74);
 for(int i=0;i<4;i++){walkin_open(nvs_aquaria[i]);vpress(ENTER);assert(scene==10&&nvs_tab==i&&nvs_stage()==5);vpress(ESC);}
 assert(!!nv.world[NVS_SURF]&&!!nv.world[NVS_ECHO]);scene=1;
 for(int i=0;i<3605;i++)night_venues_world_tick();
 assert(!nv.world[NVS_SURF]&&!nv.world[NVS_ECHO]&&!nv.world[NVS_TOL]&&night_venues_walk_factor()==1.f);
 test_crown();test_schedule();test_cart();test_afterglow();
 puts("PASS: four aquarium entrances, wrong input/retry, restoration stages, one grant, loft locks, deposits/withdrawals, cart catalogue, mounting, time skip, tolerance, blend and fade, timed cues, open promenade, distinct penthouse repairs, cell spend, load, empty frame, original Pulse compatibility.");
}

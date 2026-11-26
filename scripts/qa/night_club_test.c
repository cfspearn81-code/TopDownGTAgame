/* Linux native runtime and real-disk integration fixture. Use fresh temp XDG:
   cc -O2 -std=c11 scripts/qa/night_club_test.c src/linux_save.c -o /tmp/night_club_test
   /tmp/night_club_test "$(mktemp -d)" [optional screenshot directory]
   Controls go through tick/pointer_input; only position, initial cash, and
   simulated customer inventory are seeded. A separate seeded matrix isolates
   each turnover factor, cross-checking a real in-engine closing. */
#define _POSIX_C_SOURCE 200809L
#define DC_LINUX_NATIVE
#include "../../src/game.c"
#include "../../src/linux_save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void snap(const char*dir,const char*name){
 if(!dir)return;char path[900];snprintf(path,sizeof path,"%s/%s.ppm",dir,name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc((c>>16)&255,f);fputc((c>>8)&255,f);fputc(c&255,f);}fclose(f);
}
static U image_hash(void){paint();return night_hash(pixels,sizeof pixels);}
static void hit(U key){tick(key);tick(0);}
static void click(int row){pointer_input(203,154+30*row,1);}
static void move(float x,float y){
 for(int i=0;i<500&&absf(ncX-x)>2.6f;i++)tick(ncX>x?LEFT:RIGHT);
 for(int i=0;i<500&&absf(ncY-y)>2.6f;i++)tick(ncY>y?UP:DOWN);
 tick(0);
 if(absf(ncX-x)>4.f||absf(ncY-y)>4.f)fprintf(stderr,"Cannot reach %g,%g from %g,%g in room %d\n",x,y,ncX,ncY,ncRoom);
 assert(absf(ncX-x)<4.f&&absf(ncY-y)<4.f);
}
static void reach_club(void){
 p.car=-1;p.x=ordinaryDoorX[NC_ID];p.y=ordinaryDoorY[NC_ID];scene=1;menu=-1;mapOn=0;clear_input();
 hit(USE);assert(scene==4&&walkin_active==NC_ID);hit(ENTER);
 assert(scene==NIGHT_CLUB_SCENE&&ncRoom==NC_FLOOR);
}
static void reach_office(void){move(168,361);hit(USE);assert(ncRoom==NC_OFFICE);move(480,264);hit(USE);assert(ncPanel==NC_DESK);}
static void leave_subroom(void){move(480,416);hit(USE);assert(ncRoom==NC_FLOOR);}
static void reach_suite(void){move(808,361);hit(USE);assert(ncRoom==NC_SUITE);}
static void reach_booth(int booth){static const int x[3]={210,476,742};move(x[booth],268);hit(USE);assert(ncPanel==NC_TABLE&&ncBooth==booth);}
static void until_next_set(void){
 int n=nc_get(NC_SHOWS),i=0;
 while(nc_get(NC_SHOWS)==n&&i++<=NC_SHOW)tick(0);
 assert(nc_get(NC_SHOWS)==n+1);
}
typedef struct{int attendance,gross,rep;} Outcome;
static Outcome eval(U *data,int track,int clean,int service,int incident,int cover,int staff){
 cp(nv.club,data,sizeof nv.club);nv.club[NC_TRACK]=track;nv.club[NC_CLEAN]=clean;nv.club[NC_SERVICE]=service;
 nv.club[NC_COVER]=cover;nv.club[NC_STAFF]=staff;nv.club[NC_DISPUTE]=incident;nv.club[NC_CLOCK]=NC_SHOW-1;night_club_business_tick();
 return (Outcome){nc_get(NC_DOOR),nc_get(NC_LAST),nc_get(NC_REP)};
}
static void better(Outcome a,Outcome b){assert(a.attendance>b.attendance&&a.gross>b.gross&&a.rep>b.rep);}
int main(int argc,char**argv){
 assert(argc>=2&&setenv("XDG_DATA_HOME",argv[1],1)==0&&linux_disk_init()&&linux_disk_lock());
 const char*out=argc>2?argv[2]:0;init_city();starter_save();sv.intro=4;sv.cash=2400;sv.dirty=40;sv.dayClock=22*60;
 spawnCars();spawnPlayer();
 nv.phone[NP_BAG+0]=1; /* grade C must never leave the satchel into club stock */
 nv.phone[NP_BAG+2]=1;nv.phone[NP_BAG+5]=1;nv.phone[NP_BAG+8]=1;
 assert(night_club_queue()==4);reach_club();assert(night_club_track()==1);
 U floor0=image_hash();tick(0);for(int i=0;i<10;i++)tick(0);assert(image_hash()!=floor0);snap(out,"public-floor");
 reach_office();paint();snap(out,"public-office");
 click(0);assert(nc_owned()&&sv.cash==1920);
 click(1);assert(nc_get(NC_COVER)==2&&ncCoverPrice[nc_get(NC_COVER)]==6);
 click(2);click(2);click(3);click(4);click(5);click(5);click(6);click(7);
 assert(nc_get(NC_ACT)==2&&nc_get(NC_STAFF)==1&&nc_get(NC_DRINK)==8&&nc_get(NC_DECOR)==2);
 assert(nc_get(NC_EXCHANGE)==30&&sv.dirty==0);
 assert(nc_get(NC_SERVICE)==100&&nc_get(NC_CLEAN)==100);
 hit(ESC);leave_subroom();reach_suite();
 move(799,379);hit(USE);assert(ncPanel==NC_STOCK);
 click(1);click(2);click(3);click(1);assert(nc_get(NC_VIP)==3&&np_get(NP_BAG,99)==1);
 for(int i=0;i<3;i++){assert(nc_typed(i)==1&&night_phone_signature(i,0)==0);}
 paint();snap(out,"cabinet");hit(ESC);move(680,268);
 for(int b=0;b<3;b++){
  reach_booth(b);assert(night_club_signature_demand(b)==b);
  click(3);assert(!(nc_get(NC_SHOWCASE)&(1<<b))); /* drink first */
  int order=nc_guest_order(b);click((order+1)%3);assert(!(nc_get(NC_TABLES)&(1<<b)));
  int till=nc_get(NC_TILL);click(order);assert(nc_get(NC_TABLES)&(1<<b));click(order);assert(nc_get(NC_TILL)==till+8+b*4);
  if(!b){click(3);click(3);assert(nc_get(NC_SHOWCASE)==1&&nc_get(NC_TILL)==till+8+18);}
  paint();snap(out,b==0?"velvet":b==1?"mirage":"aurora");hit(ESC);
 }
 assert(nc_get(NC_TABLES)==7&&nc_get(NC_TILL)==54&&nc_get(NC_VIP)==2&&nc_get(NC_DRINK)==5);
 paint();snap(out,"suite");
 leave_subroom();move(580,404);hit(USE);assert(ncPanel==NC_GUARD);nv.club[NC_DISPUTE]=3;
 click(1);assert(nc_get(NC_DISPUTE)==3);click(2);assert(nc_get(NC_DISPUTE)==0&&nc_get(NC_PREVENT)&&ncPanel==0);
 move(480,416);hit(USE);assert(scene==1&&night_club_track()==0);
 tick(SAVE);assert(saveFlash>0); U have[32];cp(have,nv.club,sizeof have);cp(nv.club,(U[32]){0},sizeof have);
 load_save();assert(memcmp(have,nv.club,sizeof have)==0&&nc_get(NC_TABLES)==7&&nc_get(NC_SHOWCASE)==1&&nc_typed(1)==1&&nc_typed(2)==1);
 until_next_set();assert(nc_get(NC_TILL)==54+30+nc_get(NC_LAST)&&nc_get(NC_LAST_VIP)==57&&nc_get(NC_VIP)==0);
 assert(nc_get(NC_LAST)==nc_get(NC_LAST_DOOR)+nc_get(NC_LAST_DRINKS)+nc_get(NC_LAST_VIP)+nc_get(NC_LAST_AUX)-nc_get(NC_LAST_COST));
 assert(!nc_get(NC_TABLES)&&!nc_get(NC_SHOWCASE)&&!nc_get(NC_EXCHANGE)&&night_club_signature_demand(0)==1);
 reach_club();move(256,270);hit(USE);assert(ncPanel==NC_DECK);click(1);assert(night_club_track()==2);click(2);assert(night_club_track()==3);hit(ESC);
 paint();snap(out,"floor");
 /* Legacy aggregate inventory is never erased by the typed extension. */
 nv.club[NC_VIP]=2;assert(nc_legacy_packs()==2&&nc_typed(0)==0);
 until_next_set();assert(nc_legacy_packs()==1&&nc_get(NC_LAST_VIP)==18);
 /* Register one exact withdrawal, using primary gameplay controls. */
 move(360,385);reach_office();paint();snap(out,"receipt");
 int till=nc_get(NC_TILL),cash=sv.cash;click(0);click(0);
 assert(till>150&&nc_get(NC_TILL)==0&&sv.cash==cash+till);
 hit(SAVE);assert(saveFlash>0);load_save();assert(nc_owned()&&nc_get(NC_DECOR)==2&&nc_get(NC_ACT)==2&&nc_get(NC_STAFF)==1&&nc_get(NC_TILL)==0);
 assert(nc_get(NC_COVER)==2&&nc_get(NC_TRACK)==2&&nc_get(NC_SHOWS)==2);
 /* Opt-in clothed private cabaret: booking never moves the till or adds stock.
    Three exits are wired: click, Esc, and automatic ending. */
 reach_club();reach_suite();reach_booth(1);
 int booked_cash=sv.cash,booked_till=nc_get(NC_TILL),stock=nc_get(NC_VIP),rep=nc_get(NC_REP);
 click(4);assert(ncPanel==NC_DANCE_CONFIRM&&ncChoice==0&&sv.cash==booked_cash);
 hit(ENTER);assert(ncPanel==NC_TABLE&&sv.cash==booked_cash);
 click(4);hit(ESC);assert(ncPanel==NC_TABLE&&sv.cash==booked_cash);
 click(4);sv.cash=NC_DANCE_FEE-1;click(1);assert(ncPanel==NC_DANCE_CONFIRM&&sv.cash==NC_DANCE_FEE-1);
 sv.cash=booked_cash;click(1);assert(ncPanel==NC_DANCE_PLAY&&saveFlash>0&&sv.cash==booked_cash-NC_DANCE_FEE);
 U dance_frame=image_hash();for(int f=0;f<25;f++)tick(0);
 assert(image_hash()!=dance_frame&&nc_get(NC_TILL)==booked_till&&nc_get(NC_VIP)==stock&&nc_get(NC_REP)==rep);
 snap(out,"private-cabaret");pointer_input(717,427,1);assert(ncPanel==NC_TABLE);
 click(4);click(1);hit(ESC);assert(ncPanel==NC_TABLE&&sv.cash==booked_cash-2*NC_DANCE_FEE);
 click(4);click(1);for(int f=0;f<NC_DANCE_DURATION;f++)tick(0);
 assert(ncPanel==NC_TABLE&&sv.cash==booked_cash-3*NC_DANCE_FEE);
 load_save();assert(wason&&sv.cash==booked_cash-3*NC_DANCE_FEE&&nc_get(NC_TILL)==booked_till&&nc_get(NC_VIP)==stock);
 /* Independently vary all four requirement factors at fixed cover/show. */
 U data[32]={0};data[NC_OWN]=1;data[NC_DRINK]=10;data[NC_REP]=55;data[NC_SHOWS]=3;
 int ok=0;
 Outcome good=eval(data,0,80,80,0,1,0);better(good,eval(data,1,80,80,0,1,0));ok++;
 better(good,eval(data,0,30,80,0,1,0));ok++;
 better(good,eval(data,0,80,30,0,1,0));ok++;
 better(good,eval(data,0,80,80,3,1,0));ok++;
 data[NC_SHOWS]=2;(void)eval(data,2,80,80,0,1,0);assert(nc_get(NC_DISPUTE)>0);
 (void)eval(data,0,80,80,0,1,2);assert(nc_get(NC_DISPUTE)==0&&nc_get(NC_LAST_COST)==8);
 (void)eval(data,0,80,80,0,0,0);assert(nc_get(NC_LAST_DOOR)==0);
 data[NC_CLOCK]=NC_SHOW-1;data[NC_EXCHANGE]=30;cp(nv.club,data,sizeof nv.club);sv.charges=1;night_club_business_tick();
 assert(nc_get(NC_EXCHANGE)==30);sv.charges=0;nv.club[NC_CLOCK]=NC_SHOW-1;night_club_business_tick();assert(nc_get(NC_EXCHANGE)==0);
 for(int i=0;i<32;i++)nv.club[i]=~0u;assert(!nc_owned()&&night_club_signature_demand(0)==-1);paint();
 printf("PASS: normal club controls / three VIP booths / real satchel grade / exact native save reload / VIP receipts / %d independent economy factors / queue remedy & hire / legacy stock preserved / private cabaret confirmed, skipped, timed, saved\n",ok);
}

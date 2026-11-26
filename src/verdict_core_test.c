/* The disk fixture and keys are test-only. No gameplay-side test shortcuts. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void pressv(U k){tick(k);tick(0);}
static void near_rack(void){wi_x=wi_f[0].x+wi_f[0].w/2;wi_y=a1_rack_y();assert(!wi_solid(wi_x,wi_y)&&!wi_at_station());}
static void near_board(void){wi_x=wi_f[0].x+wi_f[0].w/2;wi_y=wi_f[0].y+wi_f[0].h+11;assert(!wi_solid(wi_x,wi_y)&&wi_at_station());}
static void sleeve_case(int cash,int track,int late){
 vd_reset();end_job();sv.intro=4;sv.cash=cash;sv.dirty=75;sv.bank=200;p.heat=0;
 walkin_open(A1_HALL);near_rack();pressv(USE);assert(vdActive==VA_A1&&vdPanel==2);
 int mX=loc[3].x,mY=loc[3].y;nav_slot();navHasManual=1;navManualLoc=3;navManual=(NavPoint){mX,mY};
 if(track==2)pressv(RIGHT);pressv(ENTER);assert(a1Track==track&&navHasManual&&sv.cash==cash&&vdGoalX==ordinaryDoorX[A1_STUDIO]);
 near_board();assert(!!tram_block()==(cash<5));tick(ENTER);for(int i=0;i<5;i++)tick(ENTER);tick(0);
 assert(walkin_active==(cash>=5?A1_ARRIVAL:A1_HALL)&&sv.cash==cash-(cash>=5?5:0));
 walkin_open(A1_STUDIO);near_board();int before=a1Ticks;
 pressv(HELP);for(int j=0;j<60;j++)tick(0);assert(a1Ticks==before);pressv(ESC);
 if(late){for(int i=0;i<A1_SLOT+1;i++)tick(0);}
 pressv(USE);assert(vdActive<0&&!a1Track&&a1Headline==track&&a1Late==late);
 int paid=sv.cash;pressv(USE);assert(sv.cash==paid&&paid==cash-(cash>=5?5:0)+95&&sv.bank==200&&sv.dirty==75);
 wi_x=W/2;wi_y=416;pressv(USE);assert(scene==1&&a1StreetFrames>0);paint();
}
int main(void){
 init_city();profiles_activate();sv.intro=2;walkin_open(A1_HALL);near_rack();pressv(USE);assert(vdActive<0);
 sleeve_case(0,1,1);sleeve_case(4,2,0);sleeve_case(5,1,0);sleeve_case(5,2,1);
 vd_reset();assert(!a1Headline&&!a1AwaitStreet&&vdActive<0&&vdGoalX<0);
 walkin_open(A1_HALL);near_board();sv.cash=50;job.kind=JOB_COURIER;pressv(ENTER);assert(walkin_active==A1_HALL&&sv.cash==50);job.kind=JOB_TAXI;job.step=1;job.time=120;taxi.phase=TAXI_RIDING;assert(strstr(tram_block(),"passenger"));pressv(ENTER);assert(walkin_active==A1_HALL&&sv.cash==50);end_job();p.heat=1;assert(strstr(tram_block(),"outside"));pressv(ENTER);assert(walkin_active==A1_HALL&&sv.cash==50);
 /* Canonical addresses come from the single map/door coordinate. */
 int ids[]={28,31,34,35,36,37,38,39,42};const char*ds[]={"NIGHTGLASS","EAST FREIGHT","LANTERN WARD","LANTERN WARD","LANTERN WARD","LANTERN WARD","BREAKWATER","BREAKWATER","BREAKWATER"};
 for(int i=0;i<9;i++){NavPoint d=nav_door(LEN(loc)+ids[i]);assert(!strcmp(venue_address(ids[i]),ds[i])&&!strcmp(district(d.x,d.y),ds[i]));}
 for(int i=0;i<VA_COUNT;i++)assert(vdHandlers[i]);
 p.heat=0;sv.owned=1;sv.intro=4;profiles_activate();assert(cars[0].x==284&&cars[0].y==528);
 navigation_input(MAP);navigation_input(TAXI_NEXT);assert(navSelected==LEN(loc)+31);
 navigation_input(TAXI_NEXT);assert(navSelected==LEN(loc)+69);
 navigation_input(TAXI_NEXT);assert(navSelected==NAV_OWN_CAR&&nav_door(navSelected).x==(int)cars[0].x);
 navigation_input(ENTER);assert(navHasManual&&navManualLoc==NAV_OWN_CAR);
 nav_update();assert(navGoalKind==3);nav_clear();
 sv.charges=1;sv.sweepTicks=120;int before=sv.sweepTicks;pressv(MAP);assert(mapOn&&sv.sweepTicks<before);paint();int prevTicks=sv.sweepTicks;
 pressv(MAP);pressv(HELP);for(int i=0;i<12;i++)tick(0);assert(sv.sweepTicks<prevTicks);int frozen=sv.sweepTicks;tick(0);assert(sv.sweepTicks==frozen);pressv(ESC);
 puts("Navigation: all nine border addresses agree; T inspects both rail stops and the owned car without erasing progress; atlas advances the sweep.");
 puts("A1: $0/$4 no debit, $5 single debit; both sleeves, early/late, $95 paid once; personal pin retained, guide freezes job clock, base fare/contract/pursuit blocks, session reset.");
}

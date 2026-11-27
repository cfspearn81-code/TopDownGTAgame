/* Reproduce real Crossroads routing failures plus the complete outer-node
 * pair sweep. Test only, isolated from user save files. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
static void reset(void){
 starter_save();scene=1;menu=-1;end_job();vd_reset();ng_reset();sv.intro=4;nav_slot();p.car=-1;navHasManual=0;navGoalKind=0;sv.dayClock=359;
}
static void stationary_parkway(void){
 reset();p.x=2250;p.y=1024;navManual=nav_outer_point(outerStops[1].node);navManualLoc=NAV_OUT_BASE+1;navHasManual=1;
 nav_update();int original=navDist;assert(navCount==2&&original==574);
 for(int day=0;day<2;day++)for(int event=0;event<4;event++){
  int times[]={360,480,960,1080};sv.dayClock=times[event];anim+=.1f;nav_update();
  if(event%2==0){assert(navCount>=5&&navDist>1000);}
  else assert(navCount==2&&navDist==original);
  for(int i=1;i<navCount;i++)assert(!night_phone_lane_stop(navNodes[i-1].x,navNodes[i-1].y,navNodes[i].x,navNodes[i].y,4));
 }
}
static void shortcut(void){
 NavPoint a={1024,2025},b={2015,512};
 for(int rev=0;rev<2;rev++){
  nav_solve(rev?b:a,rev?a:b);
  assert(navDist==2504&&nav_same(navNodes[0],rev?b:a)&&nav_same(navNodes[navCount-1],rev?a:b));
  for(int i=1;i<navCount;i++)assert(nav_traversable(navNodes[i-1],navNodes[i],10));
 }
}
static void orders(void){
 reset();nv.phone[NP_JOB]=2;ng.event=NG_RACE;ng.route=0;ng.leg=1;NavPoint dest;int venue;
 assert(nav_objective(&dest,&venue)==7&&nav_same(dest,ngRun[0].gate[1]));
 ng_clear_event();assert(nav_objective(&dest,&venue)==6&&nav_same(dest,(NavPoint){loc[np_job_dest()].x,loc[np_job_dest()].y}));
}
static void booked_course(void){
 reset();sv.owned=1;sv.selection=0;sv.carCond[0]=100;sv.cash=500;
 ng_enter_panel(NG_RUNS,1);ng.route=3;ng.meet=1;ng.stake=1;
 assert(ng_begin_race()&&sv.cash==425);
 /* Only open and browse: never confirm a replacement booking. */
 ng_enter_panel(NG_MAIN,3);ng.idx=2;ng_ui_accept();assert(ng.tab==NG_RUNS);
 night_garage_tick(RIGHT,RIGHT);int x,y;assert(ng.route==1&&ng_route_goal(&x,&y)&&x==2690&&y==384);
 assert(ng.bookedCourse==3);ng.event=NG_RACE;ng.leg=4;
 assert(ng_route_goal(&x,&y)&&x==1792&&y==1024);
 ng.frames=1200;ng_finish_run();assert(ng.event==NG_IDLE&&ng_read(ng_best(3),36000)==1200&&!ng_read(ng_best(1),36000)&&sv.cash==610);
}
static void diagonal(void){
 navCount=3;navNodes[0]=(NavPoint){100,100};navNodes[1]=(NavPoint){200,200};navNodes[2]=(NavPoint){100,300};
 NavReadout r=nav_readout(navNodes[0]);assert(r.turn==1&&r.until==141&&r.total==282);
 r=nav_readout((NavPoint){150,150});assert(r.turn==1&&r.until==71&&r.total==212);
 navNodes[2]=(NavPoint){300,100};assert(nav_readout(navNodes[0]).turn==-1);
 navNodes[2]=(NavPoint){300,275};assert(!nav_readout(navNodes[0]).turn);
}
static void graph_sweep(void){
 int plans=0,segments=0;
 for(int phase=0;phase<2;phase++){sv.dayClock=phase?370:600;
  for(int i=0;i<LEN(outerNodes);i++)for(int j=0;j<LEN(outerNodes);j++){
   NavPoint a=nav_outer_point(i),b=nav_outer_point(j);
   if(nav_same(a,b))continue;
   /* The closed barricade itself isn't a valid destination. */
   if(phase&&(i==17||j==17))continue;
   nav_solve(a,b);
   assert(navCount>=2&&navCount<=LEN(navNodes)&&nav_same(navNodes[0],a)&&nav_same(navNodes[navCount-1],b));
   for(int k=1;k<navCount;k++){
    NavPoint s=navNodes[k-1],t=navNodes[k];
    assert(nav_traversable(s,t,5)&&!night_phone_lane_stop(s.x,s.y,t.x,t.y,4));segments++;
   }
   plans++;
  }
 }
 printf("Driving GPS: %d ordered open/closed node-pair routes, %d collision-checked segments; stationary closure transitions, core-city shortcut, live race/delivery priority and diagonal instructions passed.\n",plans,segments);
}
int main(void){init_city();stationary_parkway();shortcut();orders();booked_course();diagonal();graph_sweep();return 0;}
